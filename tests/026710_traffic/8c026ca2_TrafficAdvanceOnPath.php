<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

// TrafficAdvanceOnPath_8c026ca2 walks the entry's current path segment run
// (entry+0x2b8 segment pointer, entry+0x2bc distance-into-segment) forward by
// whatever distance the caller has already accumulated into entry+0x2bc,
// looking for the {len,x,y,dx,dy} record that contains it. Once found (or
// immediately, if the distance is still within the current record), it
// re-derives the vehicle's world position (entry+0xf4/0xfc) by projecting a
// fixed distance (entry+0x2c4) from the record point (entry+0xec/0xf0)
// towards the *previous* world position, and updates the secondary heading
// angle at entry+0x254. Returns 1 when a record was found; 0 when the
// segment run is exhausted (len==0 sentinel), in which case only the
// segment pointer/distance are updated (the caller must supply a new block).
return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_njSqrt', 4);
        $this->setSize('_acosf', 4);
    }

    // Allocates a single {len,x,y,dx,dy} path record.
    private function allocSeg(float $len, float $x, float $y, float $dx, float $dy): int {
        return $this->allocPath([[$len, $x, $y, $dx, $dy]]);
    }

    // Allocates a run of {len,x,y,dx,dy} records back-to-back, as the real
    // path block is laid out (the walk advances by "pfVar1 + 5" floats, so
    // consecutive records must be contiguous, not separately allocated).
    // Returns the base address of the first record.
    private function allocPath(array $records): int {
        $base = $this->alloc(0x14 * count($records));
        foreach ($records as $i => [$len, $x, $y, $dx, $dy]) {
            $rec = $base + $i * 0x14;
            $this->initUint32($rec + 0x00, fdec($len));
            $this->initUint32($rec + 0x04, fdec($x));
            $this->initUint32($rec + 0x08, fdec($y));
            $this->initUint32($rec + 0x0c, fdec($dx));
            $this->initUint32($rec + 0x10, fdec($dy));
        }
        return $base;
    }

    private function allocEntry(int $seg, float $distIntoSeg, float $curX, float $curY): int {
        $entry = $this->alloc(0x300);
        $this->initUint32($entry + 0x2b8, $seg);
        $this->initUint32($entry + 0x2bc, fdec($distIntoSeg));
        $this->initUint32($entry + 0x2c0, fdec(0.0)); // untouched on the fast path
        $this->initUint32($entry + 0x2c4, fdec(2.0)); // fixed look-ahead distance
        $this->initUint32($entry + 0x2ec, fdec(0.0)); // path origin bias X
        $this->initUint32($entry + 0x2f0, fdec(0.0)); // path origin bias Y
        $this->initUint32($entry + 0xf4, fdec($curX));
        $this->initUint32($entry + 0xfc, fdec($curY));
        return $entry;
    }

    // Distance (3.0) is already within the first record's length (10.0):
    // resolved immediately, no segment walk needed.
    public function test_withinCurrentSegment(): void {
        $this->resolveSymbols();
        $seg = $this->allocSeg(10.0, 50.0, 60.0, 0.6, 0.8);
        $entry = $this->allocEntry($seg, 3.0, 54.8, 66.4);

        $this->call('_TrafficAdvanceOnPath_8c026ca2')->with(25.0, $entry);

        // Record point: 3*0.6+50=51.8, 3*0.8+60=62.4.
        $this->shouldWriteLong($entry + 0x2b8, $seg);
        $this->shouldWriteFloat($entry + 0x2bc, 3.0);
        $this->shouldWriteFloat($entry + 0xec, 51.8);
        $this->shouldWriteFloat($entry + 0xf0, 62.4);

        // dx=54.8-51.8=3, dy=66.4-62.4=4 (3-4-5 triangle). The incoming
        // float argument (25.0) is never read; njSqrt gets dx*dx+dy*dy.
        $this->shouldCall('_njSqrt')->with(25.000006914142)->andReturn(5.0);

        // Normalized (0.6, 0.8), projected 2.0 ahead of the record point.
        $this->shouldWriteFloat($entry + 0xf4, 53.0);  // 0.6*2 + 51.8
        $this->shouldWriteFloat($entry + 0xfc, 64.0);  // 0.8*2 + 62.4

        $this->shouldCall('_acosf')->with(0.80000029802322)->andReturn(0.6435011);
        $this->shouldWriteLong($entry + 0x254, (int)((0.6435011 * 65536.0) / 6.283184));

        $this->shouldReturn(1);
    }

    // Current position is west of the record point: normalized dx <= 0, so
    // the angle must be negated -- the branch the first case can't exercise.
    public function test_negativeDxFlipsAngleSign(): void {
        $this->resolveSymbols();
        $seg = $this->allocSeg(10.0, 50.0, 60.0, 0.6, 0.8);
        $entry = $this->allocEntry($seg, 3.0, 48.8, 66.4);

        $this->call('_TrafficAdvanceOnPath_8c026ca2')->with(25.0, $entry);

        $this->shouldWriteLong($entry + 0x2b8, $seg);
        $this->shouldWriteFloat($entry + 0x2bc, 3.0);
        $this->shouldWriteFloat($entry + 0xec, 51.8);
        $this->shouldWriteFloat($entry + 0xf0, 62.4);

        // dx=48.8-51.8=-3, dy=66.4-62.4=4.
        $this->shouldCall('_njSqrt')->with(25.000016927722)->andReturn(5.0);

        $this->shouldWriteFloat($entry + 0xf4, 50.6);  // -0.6*2 + 51.8
        $this->shouldWriteFloat($entry + 0xfc, 64.0);  // 0.8*2 + 62.4

        $this->shouldCall('_acosf')->with(0.80000029802322)->andReturn(0.6435011);

        // Same acosf result as the positive-dx case, but negated.
        $this->shouldWriteLong($entry + 0x254, -(int)((0.6435011 * 65536.0) / 6.283184));

        $this->shouldReturn(1);
    }

    // The accumulated distance (7.0) exceeds the first record's length
    // (5.0): the walk advances to the second record before resolving.
    public function test_advancesToNextRecord(): void {
        $this->resolveSymbols();
        $seg0 = $this->allocPath([
            [5.0, 0.0, 0.0, 1.0, 0.0],
            [10.0, 50.0, 60.0, 0.6, 0.8],
        ]);
        $seg1 = $seg0 + 0x14;
        $entry = $this->allocEntry($seg0, 7.0, 51.2, 64.0);

        $this->call('_TrafficAdvanceOnPath_8c026ca2')->with(25.0, $entry);

        // After consuming record 0 (length 5): remaining distance = 2.0,
        // now measured into record 1. Record point: 2*0.6+50=51.2, 2*0.8+60=61.6.
        $this->shouldWriteLong($entry + 0x2b8, $seg1);
        $this->shouldWriteFloat($entry + 0x2bc, 2.0);
        $this->shouldWriteFloat($entry + 0xec, 51.2);
        $this->shouldWriteFloat($entry + 0xf0, 61.6);

        // dx=51.2-51.2=0, dy=64.0-61.6=2.4.
        $this->shouldCall('_njSqrt')->with(5.7599998855596)->andReturn(2.4);

        $this->shouldWriteFloat($entry + 0xf4, 51.2);  // 0*2 + 51.2
        $this->shouldWriteFloat($entry + 0xfc, 63.6);  // 1*2 + 61.6

        $this->shouldCall('_acosf')->with(1.0)->andReturn(0.0);
        $this->shouldWriteLong($entry + 0x254, 0);

        $this->shouldReturn(1);
    }

    // The accumulated distance (7.0) exceeds every record in the run
    // (only one 5.0-length record before the len==0 sentinel): the cursor
    // and remaining distance are updated but no position/heading is
    // recomputed, and the function reports the run as exhausted.
    public function test_exhaustedSegmentRun(): void {
        $this->resolveSymbols();
        $seg0 = $this->allocPath([
            [5.0, 0.0, 0.0, 1.0, 0.0],
            [0.0, 0.0, 0.0, 0.0, 0.0], // len==0 sentinel
        ]);
        $sentinel = $seg0 + 0x14;
        $entry = $this->allocEntry($seg0, 7.0, 0.0, 0.0);

        $this->call('_TrafficAdvanceOnPath_8c026ca2')->with(25.0, $entry);

        $this->shouldWriteLong($entry + 0x2b8, $sentinel);
        $this->shouldWriteFloat($entry + 0x2bc, 2.0);
        $this->shouldWriteFloat($entry + 0x2c0, 2.0);

        $this->shouldReturn(0);
    }
};
