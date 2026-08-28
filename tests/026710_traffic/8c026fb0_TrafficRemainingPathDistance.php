<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

// TrafficRemainingPathDistance_8c026fb0 sums the {len,x,y,dx,dy} record
// lengths from the entry's current path record (entry+0x2b8) up to (but not
// including) the len==0 terminator record, minus the distance already
// consumed into the current record (entry+0x2bc). Unlike
// TrafficAdvanceOnPath_8c026ca2, it never writes to the entry -- pure
// read-only query of the remaining distance in the current path block.
return new class extends TestCase {
    // Allocates a run of {len,x,y,dx,dy} records back-to-back, as the real
    // path block is laid out (the walk advances by "seg + 5" floats).
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

    private function allocEntry(int $seg, float $distIntoSeg): int {
        $entry = $this->alloc(0x2c0);
        $this->initUint32($entry + 0x2b8, $seg);
        $this->initUint32($entry + 0x2bc, fdec($distIntoSeg));
        return $entry;
    }

    // Only one record before the len==0 terminator: remaining distance is
    // just that record's length minus what's already consumed.
    public function test_singleRecord(): void {
        $seg = $this->allocPath([
            [10.0, 0.0, 0.0, 1.0, 0.0],
            [0.0, 0.0, 0.0, 0.0, 0.0], // len==0 sentinel
        ]);
        $entry = $this->allocEntry($seg, 3.0);

        $this->call('_TrafficRemainingPathDistance_8c026fb0')->with($entry);

        // 10.0 - 3.0 = 7.0
        $this->shouldReturn(7.0);
    }

    // Two records before the terminator: both lengths are summed, then the
    // already-consumed distance into the *first* record is subtracted --
    // this function does not advance past record boundaries, it just sums
    // forward from wherever entry+0x2b8 currently points.
    public function test_multipleRecords(): void {
        $seg = $this->allocPath([
            [5.0, 0.0, 0.0, 1.0, 0.0],
            [10.0, 50.0, 60.0, 0.6, 0.8],
            [0.0, 0.0, 0.0, 0.0, 0.0], // len==0 sentinel
        ]);
        $entry = $this->allocEntry($seg, 2.0);

        $this->call('_TrafficRemainingPathDistance_8c026fb0')->with($entry);

        // (5.0 + 10.0) - 2.0 = 13.0
        $this->shouldReturn(13.0);
    }

    // Rejects a naive "just read the current record's length" implementation:
    // with a distance-into-segment larger than that first record's own
    // length, a correct sum-then-subtract still yields a small positive
    // remainder once the second record is included, whereas reading only
    // the first record's length minus distIntoSeg would go negative.
    public function test_distanceExceedsFirstRecord(): void {
        $seg = $this->allocPath([
            [5.0, 0.0, 0.0, 1.0, 0.0],
            [10.0, 50.0, 60.0, 0.6, 0.8],
            [0.0, 0.0, 0.0, 0.0, 0.0], // len==0 sentinel
        ]);
        $entry = $this->allocEntry($seg, 8.0);

        $this->call('_TrafficRemainingPathDistance_8c026fb0')->with($entry);

        // (5.0 + 10.0) - 8.0 = 7.0
        $this->shouldReturn(7.0);
    }
};
