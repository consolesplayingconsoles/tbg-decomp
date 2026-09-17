<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _pickWaitingPassengers_8c02c8ae(void): picks the waiting passengers for the
 * upcoming stop (var_nextStopSegment_8c228710). Snaps var_nextStopPoint_8c228900 to ground,
 * collects the segment's candidate stop spots (segment record's list at +8)
 * whose var_segmentHasStop_8c2286a4 active-stop flag is set into a scratch list, then
 * randomly picks 1-16 of them without replacement into var_waitingPassengers_8c228798,
 * positioned along the picked stop's spawn-area strip (the course's lineHum_0x2c
 * table entry selected by the segment record's field_0x06) with per-passenger jitter --
 * except on ROUTE_OME, which skips the jitter.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_activeGroundGrid_8c2264d4', 4);
        $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $this->setSize('_var_route_8c18ad1c', 4);

        $this->setSize('_GroundQueryFindPolygon_8c020914', 4);
        $this->setSize('_GroundProbeInterpolateHeight_8c020f7e', 4);
        $this->setSize('_AsqGetRandomInRangeA_8c012178', 4);
        $this->setSize('_syMalloc', 4);
        $this->setSize('_syFree', 4);
        // _BusStopGetSegment_8c02cd6a is same-object (dummy stub in the C, real asm in the
        // .src) -- mock with shouldCall() directly, no setSize().
    }

    private function initFloat(int $address, float $value): void
    {
        $this->initUint32($address, unpack('L', pack('f', $value))[1]);
    }

    /* var_segmentHasStop_8c2286a4 is a word-per-segment flag array; the test memory it lives
     * in does not start zeroed, so every case must fill it explicitly. */
    private function initActiveStopFlags(array $activeSegments): void
    {
        $flags = $this->addressOf('_var_segmentHasStop_8c2286a4');
        for ($i = 0; $i < 24; $i++) {
            $this->initUint32($flags + $i * 4, 0);
        }
        foreach ($activeSegments as $segment) {
            $this->initUint32($flags + $segment * 4, 1);
        }
    }

    private function shouldWriteFloatSymbolOffset(string $name, int $offset, float $value): void
    {
        $this->shouldWriteFloat($this->addressOf($name) + $offset, $value);
    }

    public function test_no_active_candidates_picks_nothing(): void
    {
        $this->resolveSymbols();

        $point = $this->addressOf('_var_nextStopPoint_8c228900');
        $this->initFloat($point + 0x0, 5.0);
        $this->initFloat($point + 0x4, 6.0);
        $this->initFloat($point + 0x8, 7.0);

        $this->initUint32($this->addressOf('_var_currentCourse_8c1bb868') + 0x04, 0x22222222); // atariBus_0x04

        $nextSeg = $this->addressOf('_var_nextStopSegment_8c228710');
        $this->initUint32($nextSeg, 3);

        $this->initActiveStopFlags([]); // no segment has an active stop

        // Candidate list with a real entry -- rejected by the flag filter.
        $list = $this->alloc(4);
        $this->initUint8($list + 0, 0xaa);
        $this->initUint8($list + 1, 5);
        $this->initUint8($list + 2, 0xaa);
        $this->initUint8($list + 3, 0);

        $seg = $this->alloc(12);
        $this->initUint32($seg + 8, $list);

        $candidates = $this->alloc(0x40);

        $this->call('_pickWaitingPassengers_8c02c8ae')->with();

        $this->shouldWriteLongTo('_var_waitingPassengerCount_8c228794', 0);
        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x22222222);

        $groundPtr = null;
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')
            ->with(5.0, 6.0, 7.0)
            ->do(function () use (&$groundPtr) {
                $groundPtr = $this->registers[4]->value;
            });
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')
            ->do(function () use (&$groundPtr, $point) {
                if ($this->registers[4]->value !== $groundPtr || $this->registers[5]->value !== $point) {
                    throw new RuntimeException(sprintf(
                        '_GroundProbeInterpolateHeight_8c020f7e: expected result=%08x point=%08x, got result=%08x point=%08x',
                        $groundPtr, $point, $this->registers[4]->value, $this->registers[5]->value,
                    ));
                }
            });

        $this->shouldCall('_BusStopGetSegment_8c02cd6a')->with(3)->andReturn($seg);
        $this->shouldCall('_syMalloc')->with(0x40)->andReturn($candidates);

        $this->shouldCall('_syFree')->with($candidates);
    }

    public function test_route_ome_picks_one_candidate_without_jitter(): void
    {
        $this->resolveSymbols();

        $point = $this->addressOf('_var_nextStopPoint_8c228900');
        $this->initFloat($point + 0x0, 5.0);
        $this->initFloat($point + 0x4, 6.0);
        $this->initFloat($point + 0x8, 7.0);

        $this->initUint32($this->addressOf('_var_currentCourse_8c1bb868') + 0x04, 0x22222222); // atariBus_0x04

        $nextSeg = $this->addressOf('_var_nextStopSegment_8c228710');
        $this->initUint32($nextSeg, 3);

        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 2); // ROUTE_OME

        $this->initActiveStopFlags([5]); // segment 5 has an active stop

        // Candidate list: an inactive entry (segment 7), an active one
        // (segment 5), then a terminator.
        $list = $this->alloc(6);
        $this->initUint8($list + 0, 0xaa);
        $this->initUint8($list + 1, 7);
        $this->initUint8($list + 2, 0xaa);
        $this->initUint8($list + 3, 5);
        $this->initUint8($list + 4, 0xaa);
        $this->initUint8($list + 5, 0);

        // Segment record: field_0x06 (short) = spawn-area index 0,
        // field_0x08 (long) = candidate list pointer.
        $seg = $this->alloc(12);
        $this->initUint16($seg + 6, 0);
        $this->initUint32($seg + 8, $list);

        // Spawn-area strip: x0=10, z0=20, dx=1, dz=2.
        $area = $this->alloc(0x14);
        $this->initFloat($area + 0x4, 10.0);
        $this->initFloat($area + 0x8, 20.0);
        $this->initFloat($area + 0xc, 1.0);
        $this->initFloat($area + 0x10, 2.0);

        // lineHum_0x2c holds a pointer to a table of 12-byte records;
        // record[0]'s field_0x00 is the spawn-area pointer.
        $recordsTable = $this->alloc(12);
        $this->initUint32($recordsTable, $area);
        $this->initUint32($this->addressOf('_var_currentCourse_8c1bb868') + 0x2c, $recordsTable); // lineHum_0x2c

        $candidates = $this->alloc(0x40);

        $this->call('_pickWaitingPassengers_8c02c8ae')->with();

        $this->shouldWriteLongTo('_var_waitingPassengerCount_8c228794', 0);
        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x22222222);

        $groundPtr1 = null;
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')
            ->with(5.0, 6.0, 7.0)
            ->do(function () use (&$groundPtr1) {
                $groundPtr1 = $this->registers[4]->value;
            });
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')
            ->do(function () use (&$groundPtr1, $point) {
                if ($this->registers[4]->value !== $groundPtr1 || $this->registers[5]->value !== $point) {
                    throw new RuntimeException('unexpected _GroundProbeInterpolateHeight_8c020f7e args (initial snap)');
                }
            });

        $this->shouldCall('_BusStopGetSegment_8c02cd6a')->with(3)->andReturn($seg);
        $this->shouldCall('_syMalloc')->with(0x40)->andReturn($candidates);

        // Candidate scan skips the inactive entry and keeps the active one.
        $this->shouldWriteLong($candidates + 0, $list + 2);

        $this->shouldWriteLongTo('_var_nextStopArea_8c22890c', $area);
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(1)->andReturn(0);
        $this->shouldWriteLongTo('_var_waitingPassengerCount_8c228794', 1);

        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(1)->andReturn(0);

        $slot = $this->addressOf('_var_waitingPassengers_8c228798');
        $this->shouldWriteSymbolOffset('_var_waitingPassengers_8c228798', 0x00, $list + 2);
        $this->shouldWriteFloatSymbolOffset('_var_waitingPassengers_8c228798', 0x04, 10.0); // x = 0*dx + x0
        $this->shouldWriteFloatSymbolOffset('_var_waitingPassengers_8c228798', 0x0c, 20.0); // z = 0*dz + z0

        $groundPtr2 = null;
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')
            ->with(10.0, 0.0, 20.0)
            ->do(function () use (&$groundPtr2) {
                $groundPtr2 = $this->registers[4]->value;
            });
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')
            ->do(function () use (&$groundPtr2, $slot) {
                if ($this->registers[4]->value !== $groundPtr2 || $this->registers[5]->value !== $slot + 0x04) {
                    throw new RuntimeException('unexpected _GroundProbeInterpolateHeight_8c020f7e args (per-passenger snap)');
                }
            });

        $this->shouldWriteFloatSymbolOffset('_var_waitingPassengers_8c228798', 0x10, 0.0); // index = 0
        $this->shouldWriteLong($candidates + 0, -1);

        $this->shouldCall('_syFree')->with($candidates);
    }

    public function test_non_ome_route_applies_rand_jitter(): void
    {
        $this->resolveSymbols();
        $this->setSize('_rand', 4);

        $point = $this->addressOf('_var_nextStopPoint_8c228900');
        $this->initFloat($point + 0x0, 5.0);
        $this->initFloat($point + 0x4, 6.0);
        $this->initFloat($point + 0x8, 7.0);

        $this->initUint32($this->addressOf('_var_currentCourse_8c1bb868') + 0x04, 0x22222222); // atariBus_0x04

        $nextSeg = $this->addressOf('_var_nextStopSegment_8c228710');
        $this->initUint32($nextSeg, 3);

        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 0); // ROUTE_SHINJUKU

        $this->initActiveStopFlags([5]); // segment 5 has an active stop

        $list = $this->alloc(4);
        $this->initUint8($list + 0, 0xaa);
        $this->initUint8($list + 1, 5);
        $this->initUint8($list + 2, 0xaa);
        $this->initUint8($list + 3, 0);

        $seg = $this->alloc(12);
        $this->initUint16($seg + 6, 0);
        $this->initUint32($seg + 8, $list);

        // Spawn-area strip: x0=10, z0=20, dx=1, dz=2.
        $area = $this->alloc(0x14);
        $this->initFloat($area + 0x4, 10.0);
        $this->initFloat($area + 0x8, 20.0);
        $this->initFloat($area + 0xc, 1.0);
        $this->initFloat($area + 0x10, 2.0);

        $recordsTable = $this->alloc(12);
        $this->initUint32($recordsTable, $area);
        $this->initUint32($this->addressOf('_var_currentCourse_8c1bb868') + 0x2c, $recordsTable); // lineHum_0x2c

        $candidates = $this->alloc(0x40);

        $this->call('_pickWaitingPassengers_8c02c8ae')->with();

        $this->shouldWriteLongTo('_var_waitingPassengerCount_8c228794', 0);
        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x22222222);

        $groundPtr1 = null;
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')
            ->with(5.0, 6.0, 7.0)
            ->do(function () use (&$groundPtr1) {
                $groundPtr1 = $this->registers[4]->value;
            });
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')
            ->do(function () use (&$groundPtr1, $point) {
                if ($this->registers[4]->value !== $groundPtr1 || $this->registers[5]->value !== $point) {
                    throw new RuntimeException('unexpected _GroundProbeInterpolateHeight_8c020f7e args (initial snap)');
                }
            });

        $this->shouldCall('_BusStopGetSegment_8c02cd6a')->with(3)->andReturn($seg);
        $this->shouldCall('_syMalloc')->with(0x40)->andReturn($candidates);

        $this->shouldWriteLong($candidates + 0, $list);

        $this->shouldWriteLongTo('_var_nextStopArea_8c22890c', $area);
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(1)->andReturn(0);
        $this->shouldWriteLongTo('_var_waitingPassengerCount_8c228794', 1);

        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(1)->andReturn(0);

        $slot = $this->addressOf('_var_waitingPassengers_8c228798');
        $this->shouldWriteSymbolOffset('_var_waitingPassengers_8c228798', 0x00, $list);

        // rand()/32768.0 - 0.5 == 0.0 for rand() == 16384, so x/z land back
        // on the strip's origin, same as the ROUTE_OME test above.
        $this->shouldCall('_rand')->andReturn(16384);
        $this->shouldWriteFloatSymbolOffset('_var_waitingPassengers_8c228798', 0x04, 10.0); // x = 0*dx + x0 + 0
        $this->shouldCall('_rand')->andReturn(16384);
        $this->shouldWriteFloatSymbolOffset('_var_waitingPassengers_8c228798', 0x0c, 20.0); // z = 0*dz + z0 + 0

        $groundPtr2 = null;
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')
            ->with(10.0, 0.0, 20.0)
            ->do(function () use (&$groundPtr2) {
                $groundPtr2 = $this->registers[4]->value;
            });
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')
            ->do(function () use (&$groundPtr2, $slot) {
                if ($this->registers[4]->value !== $groundPtr2 || $this->registers[5]->value !== $slot + 0x04) {
                    throw new RuntimeException('unexpected _GroundProbeInterpolateHeight_8c020f7e args (per-passenger snap)');
                }
            });

        $this->shouldWriteFloatSymbolOffset('_var_waitingPassengers_8c228798', 0x10, 0.0); // index = 0
        $this->shouldWriteLong($candidates + 0, -1);

        $this->shouldCall('_syFree')->with($candidates);
    }

    /*
     * AsqGetRandomInRangeA_8c012178(candidateCount) always returns
     * 0..candidateCount-1, so var_waitingPassengerCount_8c228794 (its result + 1, clamped to 16)
     * can never exceed candidateCount -- the do-while reroll always
     * terminates, and the > 0x10 clamp only matters for a segment with more
     * than 16 real candidates (not exercised here).
     */
    public function test_duplicate_pick_rerolls_to_the_other_candidate(): void
    {
        $this->resolveSymbols();

        $point = $this->addressOf('_var_nextStopPoint_8c228900');
        $this->initFloat($point + 0x0, 5.0);
        $this->initFloat($point + 0x4, 6.0);
        $this->initFloat($point + 0x8, 7.0);

        $this->initUint32($this->addressOf('_var_currentCourse_8c1bb868') + 0x04, 0x22222222); // atariBus_0x04

        $nextSeg = $this->addressOf('_var_nextStopSegment_8c228710');
        $this->initUint32($nextSeg, 3);

        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 2); // ROUTE_OME

        $this->initActiveStopFlags([5, 6]);

        // Candidate list: two active entries (stop index 5, 6), terminator.
        $list = $this->alloc(6);
        $this->initUint8($list + 0, 0xaa);
        $this->initUint8($list + 1, 5);
        $this->initUint8($list + 2, 0xaa);
        $this->initUint8($list + 3, 6);
        $this->initUint8($list + 4, 0xaa);
        $this->initUint8($list + 5, 0);

        $seg = $this->alloc(12);
        $this->initUint16($seg + 6, 0);
        $this->initUint32($seg + 8, $list);

        $area = $this->alloc(0x14);
        $this->initFloat($area + 0x4, 10.0);
        $this->initFloat($area + 0x8, 20.0);
        $this->initFloat($area + 0xc, 1.0);
        $this->initFloat($area + 0x10, 2.0);

        $recordsTable = $this->alloc(12);
        $this->initUint32($recordsTable, $area);
        $this->initUint32($this->addressOf('_var_currentCourse_8c1bb868') + 0x2c, $recordsTable); // lineHum_0x2c

        $candidates = $this->alloc(0x40);

        $this->call('_pickWaitingPassengers_8c02c8ae')->with();

        $this->shouldWriteLongTo('_var_waitingPassengerCount_8c228794', 0);
        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x22222222);

        $groundPtr1 = null;
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')
            ->with(5.0, 6.0, 7.0)
            ->do(function () use (&$groundPtr1) {
                $groundPtr1 = $this->registers[4]->value;
            });
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')
            ->do(function () use (&$groundPtr1, $point) {
                if ($this->registers[4]->value !== $groundPtr1 || $this->registers[5]->value !== $point) {
                    throw new RuntimeException('unexpected _GroundProbeInterpolateHeight_8c020f7e args (initial snap)');
                }
            });

        $this->shouldCall('_BusStopGetSegment_8c02cd6a')->with(3)->andReturn($seg);
        $this->shouldCall('_syMalloc')->with(0x40)->andReturn($candidates);

        // Both entries pass the active-stop filter.
        $this->shouldWriteLong($candidates + 0, $list);
        $this->shouldWriteLong($candidates + 4, $list + 2);

        $this->shouldWriteLongTo('_var_nextStopArea_8c22890c', $area);
        // AsqGetRandomInRangeA_8c012178(2) returns 1 -> count = 2.
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(2)->andReturn(1);
        $this->shouldWriteLongTo('_var_waitingPassengerCount_8c228794', 2);

        $slot = $this->addressOf('_var_waitingPassengers_8c228798');

        // i = 0: first roll picks slot 0 (candidates[0] == $list, not used).
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(2)->andReturn(0);
        $this->shouldWriteSymbolOffset('_var_waitingPassengers_8c228798', 0x00, $list);
        $this->shouldWriteFloatSymbolOffset('_var_waitingPassengers_8c228798', 0x04, 10.0);
        $this->shouldWriteFloatSymbolOffset('_var_waitingPassengers_8c228798', 0x0c, 20.0);
        $ground1 = null;
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(10.0, 0.0, 20.0)->do(function () use (&$ground1) {
            $ground1 = $this->registers[4]->value;
        });
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->do(function () use (&$ground1, $slot) {
            if ($this->registers[4]->value !== $ground1 || $this->registers[5]->value !== $slot + 0x04) {
                throw new RuntimeException('unexpected _GroundProbeInterpolateHeight_8c020f7e args (i=0)');
            }
        });
        $this->shouldWriteFloatSymbolOffset('_var_waitingPassengers_8c228798', 0x10, 0.0);
        $this->shouldWriteLong($candidates + 0, -1);

        // i = 1: first roll re-picks the now-used slot 0, rerolls to slot 1.
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(2)->andReturn(0);
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(2)->andReturn(1);
        $this->shouldWriteSymbolOffset('_var_waitingPassengers_8c228798', 0x14, $list + 2);
        $this->shouldWriteFloatSymbolOffset('_var_waitingPassengers_8c228798', 0x18, 11.0); // x = 1*dx + x0
        $this->shouldWriteFloatSymbolOffset('_var_waitingPassengers_8c228798', 0x20, 22.0); // z = 1*dz + z0
        $ground2 = null;
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(11.0, 0.0, 22.0)->do(function () use (&$ground2) {
            $ground2 = $this->registers[4]->value;
        });
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->do(function () use (&$ground2, $slot) {
            if ($this->registers[4]->value !== $ground2 || $this->registers[5]->value !== $slot + 0x14 + 0x04) {
                throw new RuntimeException('unexpected _GroundProbeInterpolateHeight_8c020f7e args (i=1)');
            }
        });
        $this->shouldWriteFloatSymbolOffset('_var_waitingPassengers_8c228798', 0x24, 1.0);
        $this->shouldWriteLong($candidates + 4, -1);

        $this->shouldCall('_syFree')->with($candidates);
    }
};
