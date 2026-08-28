<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// FUN_8c026da4 is a relocation fixup applied to a freshly-loaded blob: an
// array of self-relative dword offsets (relative to the blob's own base,
// "handle"), terminated by 0. Each offset addresses a run of 0xc-byte
// records; the outer array slot is overwritten with the record's absolute
// address, and each record's dword at +4 is likewise turned from a
// self-relative offset into an absolute pointer, walking 0xc-byte strides
// until a record's +4 field is 0.
return new class extends TestCase {
    public function test_emptyOuterArray(): void {
        // Outer array's very first slot is already the 0 terminator: no
        // relocation happens at all.
        $handle = $this->alloc(4);
        $this->initUint32($handle + 0x00, 0);

        $this->call('_FUN_8c026da4')->with($handle);
    }

    // One outer entry whose record chain is a single record (its +4 field
    // is 0, so the inner loop stops immediately after the outer slot is
    // relocated).
    public function test_singleEntrySingleRecord(): void {
        $handle = $this->alloc(0x20);

        // Outer array: one real entry pointing at the record area (offset
        // 0x10), then the 0 terminator.
        $this->initUint32($handle + 0x00, 0x10);
        $this->initUint32($handle + 0x04, 0);

        // Record at handle+0x10: field0 untouched, +4 is the terminator (0),
        // +8 padding.
        $this->initUint32($handle + 0x10, 0x1234);
        $this->initUint32($handle + 0x14, 0);
        $this->initUint32($handle + 0x18, 0x5678);

        $this->call('_FUN_8c026da4')->with($handle);

        // Outer slot becomes the record's absolute address.
        $this->shouldWriteLong($handle + 0x00, $handle + 0x10);
    }

    // Two outer entries; the first entry's record chain has two records (the
    // first record's +4 field is relocated, the second's is 0 and stops the
    // walk). This is the discriminating case: an implementation that stops
    // the outer loop after one entry, or that relocates using the record's
    // own address instead of the blob base, or that never advances past the
    // first record, would all diverge here.
    public function test_multipleEntriesAndRecordChain(): void {
        $handle = $this->alloc(0x40);

        // Outer array: two real entries, then the terminator.
        $this->initUint32($handle + 0x00, 0x10); // -> record chain A
        $this->initUint32($handle + 0x04, 0x30); // -> record chain B (single record)
        $this->initUint32($handle + 0x08, 0);

        // Record chain A: two records at handle+0x10 and handle+0x1c.
        // Record 0: +4 field holds a self-relative offset to relocate.
        $this->initUint32($handle + 0x10, 0x1111);
        $this->initUint32($handle + 0x14, 0x30); // relocated to $handle+0x30
        $this->initUint32($handle + 0x18, 0x2222);
        // Record 1 (handle+0x10 + 0xc): +4 field is 0, stops the walk.
        $this->initUint32($handle + 0x1c, 0x3333);
        $this->initUint32($handle + 0x20, 0);
        $this->initUint32($handle + 0x24, 0x4444);

        // Record chain B: single record, already-terminated.
        $this->initUint32($handle + 0x30, 0x5555);
        $this->initUint32($handle + 0x34, 0);
        $this->initUint32($handle + 0x38, 0x6666);

        $this->call('_FUN_8c026da4')->with($handle);

        // Outer entry A relocated to its absolute record address.
        $this->shouldWriteLong($handle + 0x00, $handle + 0x10);
        // Record 0's +4 field relocated (blob-base + original offset).
        $this->shouldWriteLong($handle + 0x14, $handle + 0x30);
        // Outer entry B relocated to its absolute record address.
        $this->shouldWriteLong($handle + 0x04, $handle + 0x30);
    }
};
