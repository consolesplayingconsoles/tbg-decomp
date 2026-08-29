<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

// FUN_8c02d968: course-start setup for the bus-stop passenger subsystem.
// See the function's header comment in src/02d968.c for the full
// branch/field breakdown. This suite covers the demo-mode early-return
// path only; the full spawn logic (route dispatch, waiting-passenger and
// scripted-schedule task spawning, the Fisher-Yates shuffle) is not yet
// covered -- see docs/next_units.md.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_TaskPush_8c014ae8', 4);
        $this->setSize('_FUN_8c02d8f0', 4);
        $this->setSize('_var_tasks_8c1ba5e8', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_8c226410', 4);
        $this->setSize('_var_8c2285c4', 4 * 32);
        // Loaded into R12 unconditionally at function entry by the asm
        // object even though this test's path never uses it.
        $this->setSize('_var_8c228934', 12);
    }

    // playMode == 1 (demo) and bit 3 of var_8c226410 clear: pushes the
    // "return to demo" task and sets var_8c2285c4[0] = 2, without doing
    // any of the normal course-start spawning.
    public function test_demoModeEarlyReturn(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1);
        $this->initUint32($this->addressOf('_var_8c226410'), 0);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x20);

        $this->call('_FUN_8c02d968');

        $this->shouldCall('_TaskPush_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba5e8'),
                $this->addressOf('_FUN_8c02d8f0'),
            )
            ->do(function () use ($task, $state) {
                $this->memory->writeUInt32($this->getRegister(6)->value, U32::of($task));
                $this->memory->writeUInt32($this->getRegister(7)->value, U32::of($state));
            })
            ->andReturn(1);

        $this->shouldWriteLongTo('_var_8c2285c4', 2);

        $this->forceStop();
    }

    private function f32(float $value): float {
        return unpack('f', pack('f', $value))[1];
    }

    private function initFloat(int $addr, float $value): void {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    // Not demo mode, no waiting passengers and no scripted schedule slots in
    // use: exercises the main task push, the interior texture/camera setup
    // calls, the route-dependent anchor-point branch (all six interior
    // points plus both njCalcPoint transforms), and the (empty) spawn
    // loops. $route selects ROUTE_SHINJUKU (0)/ROUTE_WANGAN (1) vs
    // ROUTE_OME (2); $pts gives the six points' expected (x, y, z) in
    // var_8c228928/910/91c/934/940/94c order. A null $pts (for a route
    // value with no dispatch arm) skips the whole anchor-point block.
    private function runEmptyScheduleRoute(int $route, ?array $pts): void {
        $this->setSize('_TaskPush_8c014ae8', 4);
        $this->setSize('_FUN_8c02d644', 4);
        $this->setSize('_var_tasks_8c1ba5e8', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_8c226410', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njLoadCacheTexture', 4);
        $this->setSize('_FUN_8c025870', 4);
        $this->setSize('_var_interiorTexlist_8c1bc438', 4);
        $this->setSize('_var_route_8c18ad1c', 4);
        $this->setSize('_var_busWorldMatrix_8c1bba54', 0x40);
        $this->setSize('_njCalcPoint', 4);
        $this->setSize('_var_8c228928', 12);
        $this->setSize('_var_8c228910', 12);
        $this->setSize('_var_8c22891c', 12);
        $this->setSize('_var_8c228934', 12);
        $this->setSize('_var_8c228940', 12);
        $this->setSize('_var_8c22894c', 12);
        $this->setSize('_var_8c228960', 20);
        $this->setSize('_var_8c22895c', 4);
        $this->setSize('_var_8c228718', 31 * 4);
        $this->setSize('_var_8c228794', 4);
        $this->setSize('_syMalloc', 4);
        $this->setSize('_TaskClear_8c014a9c', 4);
        $this->setSize('_var_stopTaskGroup_8c2288f8', 4);
        $this->setSize('_syFree', 4);
        $this->setSize('_rand', 4);
        $this->setSize('_AsqGetRandomInRangeA_8c012178', 4);
        $this->setSize('_var_8c228798', 20 * 16);
        $this->setSize('_var_passengerCount_8c1bb8e4', 4);
        $this->setSize('_init_8c04c3e4', 8 * 31);
        $this->setSize('_var_currentSegment_8c228708', 4);
        $this->setSize('_task_8c02d5ca', 4);
        $this->setSize('_FUN_8c02d21c', 4);
        $this->setSize('_FUN_8c02d46c', 4);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0); // not demo mode
        $this->initUint32($this->addressOf('_var_8c226410'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), $route);
        $this->initUint32($this->addressOf('_var_8c228794'), 0); // no waiting passengers

        $interiorTexlist = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_interiorTexlist_8c1bc438'), $interiorTexlist);

        $scheduleBase = $this->addressOf('_var_8c228718');
        for ($i = 0; $i < 31; $i++) {
            $this->initUint32($scheduleBase + $i * 4, 0xffffffff); // unused
        }

        $mainState = $this->alloc(8);
        $matchedBuf = $this->alloc(0xf8);
        $group = $this->alloc(0x20);

        $this->call('_FUN_8c02d968');

        $mainTask = $this->alloc(0x20);
        $this->shouldCall('_TaskPush_8c014ae8')
            ->with($this->addressOf('_var_tasks_8c1ba5e8'), $this->addressOf('_FUN_8c02d644'))
            ->do(function () use ($mainTask, $mainState) {
                $this->memory->writeUInt32($this->getRegister(6)->value, U32::of($mainTask));
                $this->memory->writeUInt32($this->getRegister(7)->value, U32::of($mainState));
            })
            ->andReturn(1);
        $this->shouldWriteLong($mainState + 0, 0);
        $this->shouldWriteLong($mainState + 4, 0);

        $this->shouldCall('_njSetTexture')->with($interiorTexlist);
        $this->shouldCall('_njLoadCacheTexture')->with($interiorTexlist);
        $this->shouldCall('_FUN_8c025870');

        // Six bus-interior anchor points, each written x, then y, then z
        // (verified against the real instruction order for ROUTE_SHINJUKU,
        // which does not match Ghidra's statement order for this block).
        // A route with no dispatch arm (e.g. an out-of-range value) skips
        // this whole block, including both njCalcPoint transforms.
        if ($pts !== null) {
            $matrix = $this->addressOf('_var_busWorldMatrix_8c1bba54');
            $addrs = [
                $this->addressOf('_var_8c228928'),
                $this->addressOf('_var_8c228910'),
                $this->addressOf('_var_8c22891c'),
                $this->addressOf('_var_8c228934'),
                $this->addressOf('_var_8c228940'),
                $this->addressOf('_var_8c22894c'),
            ];

            foreach ($addrs as $i => $addr) {
                [$x, $y, $z] = $pts[$i];
                $this->shouldWriteFloat($addr + 0, $this->f32($x));
                $this->shouldWriteFloat($addr + 4, $this->f32($y));
                $this->shouldWriteFloat($addr + 8, $this->f32($z));
                if ($i === 0 || $i === 5) {
                    $this->shouldCall('_njCalcPoint')->with($matrix, $addr, $addr);
                }
            }
        }

        $p960 = $this->addressOf('_var_8c228960');
        $this->shouldWriteLongTo('_var_8c22895c', 0);
        $this->shouldWriteFloat($p960 + 0, $this->f32(0.0));
        $this->shouldWriteFloat($p960 + 4, $this->f32(1.0));
        $this->shouldWriteFloat($p960 + 8, $this->f32(1.0));
        $this->shouldWriteFloat($p960 + 0xc, $this->f32(1.0));

        $this->shouldCall('_syMalloc')->with(0x20)->andReturn($group);
        $this->shouldWriteLongTo('_var_stopTaskGroup_8c2288f8', $group);
        $this->shouldCall('_TaskClear_8c014a9c')->with($group, 0);

        $this->shouldCall('_syMalloc')->with(0xf8)->andReturn($matchedBuf);
        $this->shouldCall('_syFree')->with($matchedBuf);
    }

    public function test_emptyScheduleShinjukuRoute(): void {
        $this->runEmptyScheduleRoute(0, [ // ROUTE_SHINJUKU
            [-1.6, 0.0, 0.9],    // var_8c228928
            [0.15, 0.68, 0.27],  // var_8c228910
            [0.128, 0.68, 0.76], // var_8c22891c
            [0.35, 0.68, 3.0],   // var_8c228934
            [1.0, 0.35, 2.8],    // var_8c228940
            [-1.39, 0.0, 2.63],  // var_8c22894c
        ]);
    }

    public function test_emptyScheduleWanganRoute(): void {
        // ROUTE_WANGAN takes the same dispatch arm as ROUTE_SHINJUKU.
        $this->runEmptyScheduleRoute(1, [
            [-1.6, 0.0, 0.9],
            [0.15, 0.68, 0.27],
            [0.128, 0.68, 0.76],
            [0.35, 0.68, 3.0],
            [1.0, 0.35, 2.8],
            [-1.39, 0.0, 2.63],
        ]);
    }

    public function test_emptyScheduleOmeRoute(): void {
        $this->runEmptyScheduleRoute(2, [ // ROUTE_OME
            [-1.39, 0.0, 2.63],  // var_8c228928
            [1.0, 0.35, 2.8],    // var_8c228910
            [0.35, 0.68, 3.0],   // var_8c22891c
            [0.128, 0.68, 0.76], // var_8c228934
            [0.15, 0.68, 0.27],  // var_8c228940
            [-1.6, 0.0, 0.9],    // var_8c22894c
        ]);
    }

    // A route value with no dispatch arm (only SHINJUKU/WANGAN/OME exist in
    // practice, but the C preserves the original branch structure) skips
    // the whole anchor-point block entirely.
    public function test_emptyScheduleUnknownRoute(): void {
        $this->runEmptyScheduleRoute(3, null);
    }

    // One already-picked waiting passenger (var_8c228794 == 1), no scripted
    // schedule slots: exercises the FUN_8c02d21c spawn loop's per-iteration
    // field writes and rand()-derived random offsets.
    public function test_oneWaitingPassenger(): void {
        $this->setSize('_TaskPush_8c014ae8', 4);
        $this->setSize('_FUN_8c02d644', 4);
        $this->setSize('_FUN_8c02d21c', 4);
        $this->setSize('_var_tasks_8c1ba5e8', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_8c226410', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njLoadCacheTexture', 4);
        $this->setSize('_FUN_8c025870', 4);
        $this->setSize('_var_interiorTexlist_8c1bc438', 4);
        $this->setSize('_var_route_8c18ad1c', 4);
        $this->setSize('_var_busWorldMatrix_8c1bba54', 0x40);
        $this->setSize('_njCalcPoint', 4);
        $this->setSize('_var_8c228928', 12);
        $this->setSize('_var_8c228910', 12);
        $this->setSize('_var_8c22891c', 12);
        $this->setSize('_var_8c228934', 12);
        $this->setSize('_var_8c228940', 12);
        $this->setSize('_var_8c22894c', 12);
        $this->setSize('_var_8c228960', 20);
        $this->setSize('_var_8c22895c', 4);
        $this->setSize('_var_8c228718', 31 * 4);
        $this->setSize('_var_8c228794', 4);
        $this->setSize('_var_8c228798', 20);
        $this->setSize('_var_passengerCount_8c1bb8e4', 4);
        $this->setSize('_syMalloc', 4);
        $this->setSize('_TaskClear_8c014a9c', 4);
        $this->setSize('_var_stopTaskGroup_8c2288f8', 4);
        $this->setSize('_syFree', 4);
        $this->setSize('_rand', 4);
        $this->setSize('__quick_odd_mvn', 4);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_8c226410'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 3); // skip anchor points
        $this->initUint32($this->addressOf('_var_8c228794'), 1);
        $this->initUint32($this->addressOf('_var_passengerCount_8c1bb8e4'), 5);

        $scheduleBase = $this->addressOf('_var_8c228718');
        for ($i = 0; $i < 31; $i++) {
            $this->initUint32($scheduleBase + $i * 4, 0xffffffff);
        }

        // WaitingPassengerSlot: spot_0x00, NJS_POINT3 pos_0x04, index_0x10.
        $slotBase = $this->addressOf('_var_8c228798');
        $spot = $this->alloc(4);
        $this->initUint32($slotBase + 0x00, $spot);
        $this->initFloat($slotBase + 0x04, 1.5);
        $this->initFloat($slotBase + 0x08, 0.0);
        $this->initFloat($slotBase + 0x0c, -2.5);
        $this->initUint8($spot, 2); // *(char*)spot -> init_8c04c4dc index

        $interiorTexlist = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_interiorTexlist_8c1bc438'), $interiorTexlist);

        $mainState = $this->alloc(8);
        $mainTask = $this->alloc(0x20);
        $group = $this->alloc(0x20);
        $matchedBuf = $this->alloc(0xf8);
        $state = $this->alloc(0x38);
        $task = $this->alloc(0x20);

        $this->call('_FUN_8c02d968');

        $this->shouldCall('_TaskPush_8c014ae8')
            ->with($this->addressOf('_var_tasks_8c1ba5e8'), $this->addressOf('_FUN_8c02d644'))
            ->do(function () use ($mainTask, $mainState) {
                $this->memory->writeUInt32($this->getRegister(6)->value, U32::of($mainTask));
                $this->memory->writeUInt32($this->getRegister(7)->value, U32::of($mainState));
            })
            ->andReturn(1);
        $this->shouldWriteLong($mainState + 0, 0);
        $this->shouldWriteLong($mainState + 4, 0);

        $this->shouldCall('_njSetTexture')->with($interiorTexlist);
        $this->shouldCall('_njLoadCacheTexture')->with($interiorTexlist);
        $this->shouldCall('_FUN_8c025870');

        $p960 = $this->addressOf('_var_8c228960');
        $this->shouldWriteLongTo('_var_8c22895c', 0);
        $this->shouldWriteFloat($p960 + 0, $this->f32(0.0));
        $this->shouldWriteFloat($p960 + 4, $this->f32(1.0));
        $this->shouldWriteFloat($p960 + 8, $this->f32(1.0));
        $this->shouldWriteFloat($p960 + 0xc, $this->f32(1.0));

        $this->shouldCall('_syMalloc')->with((1 + 0 + 1) * 0x20)->andReturn($group);
        $this->shouldWriteLongTo('_var_stopTaskGroup_8c2288f8', $group);
        $this->shouldCall('_TaskClear_8c014a9c')->with($group, 1);

        // init_8c04c4dc[2] == 0x02 -> 0x02 + 0x32 = 0x34.
        $rand1 = 1000;
        $rand2 = 2000;
        $expectRandX = $this->f32(((float)$rand1 / 32768.0) * 0.2);
        $expectRandZ = $this->f32(((float)$rand2 / 32768.0) * 0.2);

        $this->shouldCall('_TaskPush_8c014ae8')
            ->with($group, $this->addressOf('_FUN_8c02d21c'))
            ->do(function () use ($task, $state) {
                $this->memory->writeUInt32($this->getRegister(6)->value, U32::of($task));
                $this->memory->writeUInt32($this->getRegister(7)->value, U32::of($state));
            })
            ->andReturn(1);
        $this->shouldWriteLong($state + 0x00, $spot);
        $this->shouldWriteLong($state + 0x04, 1);
        // pos_0x08..0x14 = var_8c228798[0].pos_0x04 (NJS_POINT3 copy), via
        // the SHC runtime struct-copy helper: dest in R1, src in R2, byte
        // count in R0 (see docs/lessons_learned.md).
        $this->shouldCall('__quick_odd_mvn')->do(function () use ($state, $slotBase) {
            $got = sprintf('dest=%08x src=%08x n=%d',
                $this->getRegister(1)->value, $this->getRegister(2)->value, $this->getRegister(0)->value);
            $want = sprintf('dest=%08x src=%08x n=12', $state + 0x08, $slotBase + 0x04);
            if ($got !== $want) {
                throw new \RuntimeException("_quick_odd_mvn: expected $want, got $got");
            }
        });
        $this->shouldCall('_rand')->andReturn($rand1);
        $this->shouldWriteFloat($state + 0x18, $expectRandX);
        $this->shouldCall('_rand')->andReturn($rand2);
        $this->shouldWriteFloat($state + 0x1c, $expectRandZ);
        $this->shouldWriteLong($state + 0x20, 0); // index
        $this->shouldWriteLong($state + 0x28, 0);
        $this->shouldWriteLong($state + 0x2c, 0); // 0 % 4
        $this->shouldWriteLong($state + 0x30, 0x34);
        $this->shouldWriteLong($state + 0x34, 0x38);
        $this->shouldWriteLongTo('_var_passengerCount_8c1bb8e4', 6);

        $this->shouldCall('_syMalloc')->with(0xf8)->andReturn($matchedBuf);
        $this->shouldCall('_syFree')->with($matchedBuf);
    }
};
