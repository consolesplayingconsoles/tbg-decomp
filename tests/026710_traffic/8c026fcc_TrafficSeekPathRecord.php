<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

// FUN_8c026fcc seeks the entry's path-record cursor forward by the distance
// already accumulated at entry+0x2c0, walking {len,x,y,dx,dy} records from
// the passed-in segment pointer. On a len==0 block-end sentinel it advances
// the entry's block index (entry+0x300) and continues from the next block's
// base pointer (entry+0x304, an array of per-block path pointers indexed by
// that block index). Unlike TrafficAdvanceOnPath_8c026ca2, it does not
// recompute position/heading -- it only updates the cursor: the resolved
// record pointer is written back both into the per-block array slot for the
// entry's current block index (caching progress within that block) and into
// entry+0x2b8/0x2bc (current record / remaining distance into it).
return new class extends TestCase {
    // Allocates a run of {len,x,y,dx,dy} records back-to-back (the walk
    // advances by "seg + 5" floats, so records must be contiguous).
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

    // entry+0x300 = current block index; entry+0x304 = array of per-block
    // path base pointers (indexed by that block index).
    private function allocEntry(int $dist, int $blockIndex, array $blockBases): int {
        $entry = $this->alloc(0x304 + 4 * count($blockBases));
        $this->initUint32($entry + 0x2c0, $dist);
        $this->initUint32($entry + 0x300, $blockIndex);
        foreach ($blockBases as $i => $base) {
            $this->initUint32($entry + 0x304 + $i * 4, $base);
        }
        return $entry;
    }

    // Distance (3.0) is within the passed-in record's length (10.0): the
    // walk resolves immediately, no block boundary crossed.
    public function test_withinCurrentRecord(): void {
        $seg = $this->allocPath([
            [10.0, 50.0, 60.0, 0.6, 0.8],
        ]);
        $entry = $this->allocEntry(fdec(3.0), 0, [$seg]);

        $this->call('_FUN_8c026fcc')->with($entry, $seg);

        // Block index never advances: the array write-back lands back on
        // slot 0, and the seg pointer is unchanged.
        $this->shouldWriteLong($entry + 0x304, $seg);
        $this->shouldWriteLong($entry + 0x2b8, $seg);
        $this->shouldWriteFloat($entry + 0x2bc, 3.0);
    }

    // Distance (7.0) exceeds the current record (5.0) and reaches the
    // len==0 block-end sentinel, so the walk crosses into the next block
    // (fetched from entry+0x304's block-index array) before resolving
    // (2.0 remains, within the next block's 10.0-length record).
    public function test_advancesToNextBlock(): void {
        $seg0 = $this->allocPath([
            [5.0, 0.0, 0.0, 1.0, 0.0],
            [0.0, 0.0, 0.0, 0.0, 0.0], // len==0 block-end sentinel
        ]);
        $seg1 = $this->allocPath([
            [10.0, 50.0, 60.0, 0.6, 0.8],
        ]);
        $entry = $this->allocEntry(fdec(7.0), 0, [$seg0, $seg1]);

        $this->call('_FUN_8c026fcc')->with($entry, $seg0);

        $this->shouldWriteLong($entry + 0x300, 1);
        // Array write-back lands on the new block index's slot (1).
        $this->shouldWriteLong($entry + 0x304 + 4, $seg1);
        $this->shouldWriteLong($entry + 0x2b8, $seg1);
        $this->shouldWriteFloat($entry + 0x2bc, 2.0);
    }
};
