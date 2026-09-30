<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_skips_recompute_when_counter_nonzero(): void {
        $obj = $this->alloc(0x20);

        $this->initUint32($obj + 0x04, 10); // bound for counter (field_0x10)
        $this->initUint32($obj + 0x10, 5);  // counter (field_0x10), non-zero -> skip recompute

        $this->call('_advanceDatBlob_8c029f54')->with($obj);

        $this->shouldWriteLong($obj + 0x10, 6);

        $this->shouldReturn(1);
    }

    public function test_wraps_counter_to_zero_at_bound(): void {
        $obj = $this->alloc(0x20);

        $this->initUint32($obj + 0x04, 10); // bound for counter (field_0x10)
        $this->initUint32($obj + 0x10, 11); // counter (field_0x10), above bound

        $this->call('_advanceDatBlob_8c029f54')->with($obj);

        $this->shouldWriteLong($obj + 0x10, 12); // unconditional counter + 1
        $this->shouldWriteLong($obj + 0x10, 0);  // then reset since bound <= counter

        $this->shouldReturn(1);
    }

    public function test_recomputes_advances_index_when_count_is_one(): void {
        $obj = $this->alloc(0x20);

        // parent object with a child pointer at +0x2c (NJS_OBJECT layout);
        // never dereferenced since the walk loop starts at index 1 and the
        // count below is 1, so it never executes.
        $parent = $this->alloc(0x30);
        $this->initUint32($parent + 0x2c, 0x12345678);

        $this->initUint32($obj + 0x00, 1);      // count (n)
        $this->initUint32($obj + 0x08, 100);    // bound for index (field_0x0c)
        $this->initUint32($obj + 0x0c, 0);      // index (field_0x0c)
        $this->initUint32($obj + 0x10, 0);      // counter (field_0x10), zero -> recompute
        $this->initUint32($obj + 0x14, 0);      // mask table base (field_0x14), unused
        $this->initUint32($obj + 0x04, 100);    // bound for counter (field_0x10)
        $this->initUint32($obj + 0x1c, $parent); // target (field_0x1c)

        $this->call('_advanceDatBlob_8c029f54')->with($obj);

        $this->shouldWriteLong($obj + 0x0c, 1); // index advances
        $this->shouldWriteLong($obj + 0x10, 1); // counter advances

        $this->shouldReturn(1);
    }

    public function test_recomputes_single_node_bit_set(): void {
        $obj = $this->alloc(0x20);

        // single child node (NJS_OBJECT layout); sibling (+0x30) is null,
        // and it is the last node walked (n - 1 == 1), so the walk ends
        // cleanly instead of bailing out early.
        $node = $this->alloc(0x34);
        $this->initUint32($node + 0x30, 0); // sibling

        $parent = $this->alloc(0x30);
        $this->initUint32($parent + 0x2c, $node); // child

        $mask = $this->alloc(4);
        $this->initUint32($mask + 0, 2); // bit 1 set

        $this->initUint32($obj + 0x00, 2);      // count (n)
        $this->initUint32($obj + 0x08, 100);    // bound for index (field_0x0c)
        $this->initUint32($obj + 0x0c, 0);      // index (field_0x0c)
        $this->initUint32($obj + 0x10, 0);      // counter (field_0x10), zero -> recompute
        $this->initUint32($obj + 0x14, $mask);  // mask table base (field_0x14)
        $this->initUint32($obj + 0x04, 100);    // bound for counter (field_0x10)
        $this->initUint32($obj + 0x1c, $parent); // target (field_0x1c)

        $this->call('_advanceDatBlob_8c029f54')->with($obj);

        $this->shouldWriteLong($node + 0x00, 0x36); // bit set -> 0x36
        $this->shouldWriteLong($obj + 0x0c, 1);      // index advances
        $this->shouldWriteLong($obj + 0x10, 1);      // counter advances

        $this->shouldReturn(1);
    }

    public function test_recomputes_single_node_bit_clear(): void {
        $obj = $this->alloc(0x20);

        $node = $this->alloc(0x34);
        $this->initUint32($node + 0x30, 0); // sibling

        $parent = $this->alloc(0x30);
        $this->initUint32($parent + 0x2c, $node); // child

        $mask = $this->alloc(4);
        $this->initUint32($mask + 0, 0); // bit 1 clear

        $this->initUint32($obj + 0x00, 2);      // count (n)
        $this->initUint32($obj + 0x08, 100);    // bound for index (field_0x0c)
        $this->initUint32($obj + 0x0c, 0);      // index (field_0x0c)
        $this->initUint32($obj + 0x10, 0);      // counter (field_0x10), zero -> recompute
        $this->initUint32($obj + 0x14, $mask);  // mask table base (field_0x14)
        $this->initUint32($obj + 0x04, 100);    // bound for counter (field_0x10)
        $this->initUint32($obj + 0x1c, $parent); // target (field_0x1c)

        $this->call('_advanceDatBlob_8c029f54')->with($obj);

        $this->shouldWriteLong($node + 0x00, 0x3e); // bit clear -> 0x3e
        $this->shouldWriteLong($obj + 0x0c, 1);      // index advances
        $this->shouldWriteLong($obj + 0x10, 1);      // counter advances

        $this->shouldReturn(1);
    }

    public function test_recomputes_bails_out_when_chain_too_short(): void {
        $obj = $this->alloc(0x20);

        // single node whose sibling is null, but count says there should be
        // 3 nodes -- walk bails out early (returns 0) without touching the
        // tail bookkeeping.
        $node = $this->alloc(0x34);
        $this->initUint32($node + 0x30, 0); // sibling

        $parent = $this->alloc(0x30);
        $this->initUint32($parent + 0x2c, $node); // child

        $mask = $this->alloc(4);
        $this->initUint32($mask + 0, 2); // bit 1 set

        $this->initUint32($obj + 0x00, 3);      // count (n)
        $this->initUint32($obj + 0x0c, 0);      // index (field_0x0c)
        $this->initUint32($obj + 0x10, 0);      // counter (field_0x10), zero -> recompute
        $this->initUint32($obj + 0x14, $mask);  // mask table base (field_0x14)
        $this->initUint32($obj + 0x1c, $parent); // target (field_0x1c)

        $this->call('_advanceDatBlob_8c029f54')->with($obj);

        $this->shouldWriteLong($node + 0x00, 0x36); // bit set -> 0x36

        $this->shouldReturn(0);
    }

    public function test_recomputes_walks_sibling_chain(): void {
        $obj = $this->alloc(0x20);

        // two chained nodes; the walk visits index 1 (node1) then index 2
        // (node2) via node1's sibling pointer, then stops since 2 == n - 1.
        $node2 = $this->alloc(0x34);
        $this->initUint32($node2 + 0x30, 0); // sibling

        $node1 = $this->alloc(0x34);
        $this->initUint32($node1 + 0x30, $node2); // sibling

        $parent = $this->alloc(0x30);
        $this->initUint32($parent + 0x2c, $node1); // child

        $mask = $this->alloc(4);
        $this->initUint32($mask + 0, 0x6); // bits 1 and 2 set

        $this->initUint32($obj + 0x00, 3);      // count (n)
        $this->initUint32($obj + 0x08, 100);    // bound for index (field_0x0c)
        $this->initUint32($obj + 0x0c, 0);      // index (field_0x0c)
        $this->initUint32($obj + 0x10, 0);      // counter (field_0x10), zero -> recompute
        $this->initUint32($obj + 0x14, $mask);  // mask table base (field_0x14)
        $this->initUint32($obj + 0x04, 100);    // bound for counter (field_0x10)
        $this->initUint32($obj + 0x1c, $parent); // target (field_0x1c)

        $this->call('_advanceDatBlob_8c029f54')->with($obj);

        $this->shouldWriteLong($node1 + 0x00, 0x36); // bit 1 set -> 0x36
        $this->shouldWriteLong($node2 + 0x00, 0x36); // bit 2 set -> 0x36
        $this->shouldWriteLong($obj + 0x0c, 1);       // index advances
        $this->shouldWriteLong($obj + 0x10, 1);       // counter advances

        $this->shouldReturn(1);
    }

    public function test_recomputes_wraps_index_to_zero_at_bound(): void {
        $obj = $this->alloc(0x20);

        $node = $this->alloc(0x34);
        $this->initUint32($node + 0x30, 0); // sibling

        $parent = $this->alloc(0x30);
        $this->initUint32($parent + 0x2c, $node); // child

        $mask = $this->alloc(0x14);
        $this->initUint32($mask + 4 * 4, 2); // mask[index=4]: bit 1 set

        $this->initUint32($obj + 0x00, 2);      // count (n)
        $this->initUint32($obj + 0x08, 5);      // bound for index (field_0x0c)
        $this->initUint32($obj + 0x0c, 4);      // index (field_0x0c), one below bound
        $this->initUint32($obj + 0x10, 0);      // counter (field_0x10), zero -> recompute
        $this->initUint32($obj + 0x14, $mask);  // mask table base (field_0x14)
        $this->initUint32($obj + 0x04, 100);    // bound for counter (field_0x10)
        $this->initUint32($obj + 0x1c, $parent); // target (field_0x1c)

        $this->call('_advanceDatBlob_8c029f54')->with($obj);

        $this->shouldWriteLong($node + 0x00, 0x36); // bit set -> 0x36
        $this->shouldWriteLong($obj + 0x0c, 5);      // unconditional index + 1
        $this->shouldWriteLong($obj + 0x0c, 0);      // then reset since bound <= index
        $this->shouldWriteLong($obj + 0x10, 1);      // counter advances

        $this->shouldReturn(1);
    }
};
