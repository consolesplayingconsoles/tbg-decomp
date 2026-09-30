<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/* VehiclePartsBind_8c02786c(void *entry, Uint32 typeCode)
 *
 * Caches vehicle body-part nodes into the traffic entry blob so later code
 * (script/animation) can address them by fixed offset, and un-marks
 * NJD_EVAL_UNIT_ANG (bit 0x02 of evalflags -- "ignore rotation") on the
 * first four/five nodes so they resume rotating once bound as parts.
 *
 * entry+0x0c holds the vehicle's NJS_OBJECT* root ("nj").
 *
 * First walk: nj->child, then its sibling chain, cached at
 * entry+0x18/0x1c/0x20/0x24 (always) and +0x28 (only when typeCode is
 * 0x14, 0x16, 0x0e or 0x10); each cached node has NJD_EVAL_UNIT_ANG
 * cleared.
 *
 * Second walk: nj->child->child (re-read from entry+0x0c, not from the
 * entry+0x18 cache), then its sibling chain, cached at entry+0x2c through
 * +0x3c (always, no flag clear). From there, deeper nodes are cached only
 * for specific typeCodes, and the sibling chain can terminate in NULL,
 * which bails out of the rest of the function:
 *   - typeCode 0x1a: entry+0x58/0x5c/0x60 = the +0x3c node's child and its
 *     next two siblings.
 *   - next sibling (guard: NULL bails): typeCode 0x1c or 0x1e -> entry+0x40.
 *   - next sibling (guard: NULL bails): typeCode 0x1a -> entry+0x48;
 *     typeCode 0x14 or 0x16 -> entry+0x44; typeCode 0x1a additionally
 *     caches entry+0x4c/0x50/0x54 from that node's sibling's child and its
 *     next two siblings.
 */
return new class extends TestCase {
    private function newNode(): int {
        return $this->alloc(0x34);
    }

    private function setEvalflags(int $node, int $value): void {
        $this->initUint32($node + 0x00, $value);
    }

    private function setChild(int $node, int $child): void {
        $this->initUint32($node + 0x2c, $child);
    }

    private function setSibling(int $node, int $sibling): void {
        $this->initUint32($node + 0x30, $sibling);
    }

    /* typeCode outside every special set: only the four unconditional
     * first-walk caches and the five unconditional second-walk caches are
     * written; the sibling chain then hits NULL right away, bailing out
     * before entry+0x40 is even considered. */
    public function test_defaultTypeCode_bailsOnNullSibling(): void {
        $entry = $this->alloc(0x64);
        $typeCode = 0x12;

        $nj = $this->newNode();

        $n1 = $this->newNode();
        $n2 = $this->newNode();
        $n3 = $this->newNode();
        $n4 = $this->newNode();
        $this->setEvalflags($n1, 0x03);
        $this->setEvalflags($n2, 0x02);
        $this->setEvalflags($n3, 0x1f);
        $this->setEvalflags($n4, 0x0a);
        $this->setChild($nj, $n1);
        $this->setSibling($n1, $n2);
        $this->setSibling($n2, $n3);
        $this->setSibling($n3, $n4);

        $d1 = $this->newNode();
        $d2 = $this->newNode();
        $d3 = $this->newNode();
        $d4 = $this->newNode();
        $d5 = $this->newNode();
        $this->setChild($n1, $d1);
        $this->setSibling($d1, $d2);
        $this->setSibling($d2, $d3);
        $this->setSibling($d3, $d4);
        $this->setSibling($d4, $d5);
        $this->setSibling($d5, 0); // NULL -> bail

        $this->initUint32($entry + 0x0c, $nj);

        $this->call('_VehiclePartsBind_8c02786c')->with($entry, $typeCode);

        $this->shouldWriteLong($entry + 0x00, $typeCode);

        $this->shouldWriteLong($entry + 0x18, $n1);
        $this->shouldWriteLong($n1 + 0x00, 0x01); // 0x03 & ~0x02
        $this->shouldWriteLong($entry + 0x1c, $n2);
        $this->shouldWriteLong($n2 + 0x00, 0x00); // 0x02 & ~0x02
        $this->shouldWriteLong($entry + 0x20, $n3);
        $this->shouldWriteLong($n3 + 0x00, 0x1d); // 0x1f & ~0x02
        $this->shouldWriteLong($entry + 0x24, $n4);
        $this->shouldWriteLong($n4 + 0x00, 0x08); // 0x0a & ~0x02

        $this->shouldWriteLong($entry + 0x2c, $d1);
        $this->shouldWriteLong($entry + 0x30, $d2);
        $this->shouldWriteLong($entry + 0x34, $d3);
        $this->shouldWriteLong($entry + 0x38, $d4);
        $this->shouldWriteLong($entry + 0x3c, $d5);
    }

    /* typeCode 0x10 triggers the +0x28 optional first-walk cache; the
     * second-walk deep chain reaches a valid +0x40 candidate node but
     * typeCode 0x10 doesn't match {0x1c,0x1e} so it's left unwritten, and
     * the chain then bails on a NULL sibling before +0x44/+0x48. */
    public function test_typeCode10_optionalFirstCache_thenBailsAtSecondGuard(): void {
        $entry = $this->alloc(0x64);
        $typeCode = 0x10;

        $nj = $this->newNode();

        $n1 = $this->newNode();
        $n2 = $this->newNode();
        $n3 = $this->newNode();
        $n4 = $this->newNode();
        $n5 = $this->newNode();
        $this->setEvalflags($n1, 0x03);
        $this->setEvalflags($n2, 0x03);
        $this->setEvalflags($n3, 0x03);
        $this->setEvalflags($n4, 0x03);
        $this->setEvalflags($n5, 0x03);
        $this->setChild($nj, $n1);
        $this->setSibling($n1, $n2);
        $this->setSibling($n2, $n3);
        $this->setSibling($n3, $n4);
        $this->setSibling($n4, $n5);

        $d1 = $this->newNode();
        $d2 = $this->newNode();
        $d3 = $this->newNode();
        $d4 = $this->newNode();
        $d5 = $this->newNode();
        $d6 = $this->newNode();
        $this->setChild($n1, $d1);
        $this->setSibling($d1, $d2);
        $this->setSibling($d2, $d3);
        $this->setSibling($d3, $d4);
        $this->setSibling($d4, $d5);
        $this->setSibling($d5, $d6);
        $this->setSibling($d6, 0); // NULL -> bail at second guard

        $this->initUint32($entry + 0x0c, $nj);

        $this->call('_VehiclePartsBind_8c02786c')->with($entry, $typeCode);

        $this->shouldWriteLong($entry + 0x00, $typeCode);

        $this->shouldWriteLong($entry + 0x18, $n1);
        $this->shouldWriteLong($n1 + 0x00, 0x01);
        $this->shouldWriteLong($entry + 0x1c, $n2);
        $this->shouldWriteLong($n2 + 0x00, 0x01);
        $this->shouldWriteLong($entry + 0x20, $n3);
        $this->shouldWriteLong($n3 + 0x00, 0x01);
        $this->shouldWriteLong($entry + 0x24, $n4);
        $this->shouldWriteLong($n4 + 0x00, 0x01);
        $this->shouldWriteLong($entry + 0x28, $n5);
        $this->shouldWriteLong($n5 + 0x00, 0x01);

        $this->shouldWriteLong($entry + 0x2c, $d1);
        $this->shouldWriteLong($entry + 0x30, $d2);
        $this->shouldWriteLong($entry + 0x34, $d3);
        $this->shouldWriteLong($entry + 0x38, $d4);
        $this->shouldWriteLong($entry + 0x3c, $d5);
    }

    /* typeCode 0x1c: none of the first-walk-optional or second-walk 0x1a
     * conditions match, so this exercises entry+0x40 with a real (non-NULL)
     * node further down the chain that then fails every remaining
     * condition -- proving the gates discriminate rather than a NULL
     * pointer merely short-circuiting the writes. */
    public function test_typeCode1c_writesOnlyDeepFortyCache(): void {
        $entry = $this->alloc(0x64);
        $typeCode = 0x1c;

        $nj = $this->newNode();

        $n1 = $this->newNode();
        $n2 = $this->newNode();
        $n3 = $this->newNode();
        $n4 = $this->newNode();
        $this->setEvalflags($n1, 0x03);
        $this->setEvalflags($n2, 0x03);
        $this->setEvalflags($n3, 0x03);
        $this->setEvalflags($n4, 0x03);
        $this->setChild($nj, $n1);
        $this->setSibling($n1, $n2);
        $this->setSibling($n2, $n3);
        $this->setSibling($n3, $n4);

        $d1 = $this->newNode();
        $d2 = $this->newNode();
        $d3 = $this->newNode();
        $d4 = $this->newNode();
        $d5 = $this->newNode();
        $d6 = $this->newNode();
        $d7 = $this->newNode();
        $this->setChild($n1, $d1);
        $this->setSibling($d1, $d2);
        $this->setSibling($d2, $d3);
        $this->setSibling($d3, $d4);
        $this->setSibling($d4, $d5);
        $this->setSibling($d5, $d6);
        $this->setSibling($d6, $d7); // non-NULL, but no condition matches

        $this->initUint32($entry + 0x0c, $nj);

        $this->call('_VehiclePartsBind_8c02786c')->with($entry, $typeCode);

        $this->shouldWriteLong($entry + 0x00, $typeCode);

        $this->shouldWriteLong($entry + 0x18, $n1);
        $this->shouldWriteLong($n1 + 0x00, 0x01);
        $this->shouldWriteLong($entry + 0x1c, $n2);
        $this->shouldWriteLong($n2 + 0x00, 0x01);
        $this->shouldWriteLong($entry + 0x20, $n3);
        $this->shouldWriteLong($n3 + 0x00, 0x01);
        $this->shouldWriteLong($entry + 0x24, $n4);
        $this->shouldWriteLong($n4 + 0x00, 0x01);

        $this->shouldWriteLong($entry + 0x2c, $d1);
        $this->shouldWriteLong($entry + 0x30, $d2);
        $this->shouldWriteLong($entry + 0x34, $d3);
        $this->shouldWriteLong($entry + 0x38, $d4);
        $this->shouldWriteLong($entry + 0x3c, $d5);

        $this->shouldWriteLong($entry + 0x40, $d6);
    }

    /* typeCode 0x16 hits the first-walk optional +0x28 cache AND the
     * second-walk +0x44 cache (shared by 0x14/0x16), skipping +0x40. */
    public function test_typeCode16_writesTwentyEightAndFortyFour(): void {
        $entry = $this->alloc(0x64);
        $typeCode = 0x16;

        $nj = $this->newNode();

        $n1 = $this->newNode();
        $n2 = $this->newNode();
        $n3 = $this->newNode();
        $n4 = $this->newNode();
        $n5 = $this->newNode();
        $this->setEvalflags($n1, 0x03);
        $this->setEvalflags($n2, 0x03);
        $this->setEvalflags($n3, 0x03);
        $this->setEvalflags($n4, 0x03);
        $this->setEvalflags($n5, 0x03);
        $this->setChild($nj, $n1);
        $this->setSibling($n1, $n2);
        $this->setSibling($n2, $n3);
        $this->setSibling($n3, $n4);
        $this->setSibling($n4, $n5);

        $d1 = $this->newNode();
        $d2 = $this->newNode();
        $d3 = $this->newNode();
        $d4 = $this->newNode();
        $d5 = $this->newNode();
        $d6 = $this->newNode();
        $d7 = $this->newNode();
        $this->setChild($n1, $d1);
        $this->setSibling($d1, $d2);
        $this->setSibling($d2, $d3);
        $this->setSibling($d3, $d4);
        $this->setSibling($d4, $d5);
        $this->setSibling($d5, $d6);
        $this->setSibling($d6, $d7);

        $this->initUint32($entry + 0x0c, $nj);

        $this->call('_VehiclePartsBind_8c02786c')->with($entry, $typeCode);

        $this->shouldWriteLong($entry + 0x00, $typeCode);

        $this->shouldWriteLong($entry + 0x18, $n1);
        $this->shouldWriteLong($n1 + 0x00, 0x01);
        $this->shouldWriteLong($entry + 0x1c, $n2);
        $this->shouldWriteLong($n2 + 0x00, 0x01);
        $this->shouldWriteLong($entry + 0x20, $n3);
        $this->shouldWriteLong($n3 + 0x00, 0x01);
        $this->shouldWriteLong($entry + 0x24, $n4);
        $this->shouldWriteLong($n4 + 0x00, 0x01);
        $this->shouldWriteLong($entry + 0x28, $n5);
        $this->shouldWriteLong($n5 + 0x00, 0x01);

        $this->shouldWriteLong($entry + 0x2c, $d1);
        $this->shouldWriteLong($entry + 0x30, $d2);
        $this->shouldWriteLong($entry + 0x34, $d3);
        $this->shouldWriteLong($entry + 0x38, $d4);
        $this->shouldWriteLong($entry + 0x3c, $d5);

        $this->shouldWriteLong($entry + 0x44, $d7);
    }

    /* typeCode 0x1a: the full deep path -- entry+0x58/0x5c/0x60 from the
     * +0x3c node's grandchildren, entry+0x48, and entry+0x4c/0x50/0x54
     * from a further nested child/sibling chain. */
    public function test_typeCode1a_writesFullDeepChain(): void {
        $entry = $this->alloc(0x64);
        $typeCode = 0x1a;

        $nj = $this->newNode();

        $n1 = $this->newNode();
        $n2 = $this->newNode();
        $n3 = $this->newNode();
        $n4 = $this->newNode();
        $this->setEvalflags($n1, 0x03);
        $this->setEvalflags($n2, 0x03);
        $this->setEvalflags($n3, 0x03);
        $this->setEvalflags($n4, 0x03);
        $this->setChild($nj, $n1);
        $this->setSibling($n1, $n2);
        $this->setSibling($n2, $n3);
        $this->setSibling($n3, $n4);

        $d1 = $this->newNode();
        $d2 = $this->newNode();
        $d3 = $this->newNode();
        $d4 = $this->newNode();
        $d5 = $this->newNode();
        $d6 = $this->newNode();
        $d7 = $this->newNode();
        $this->setChild($n1, $d1);
        $this->setSibling($d1, $d2);
        $this->setSibling($d2, $d3);
        $this->setSibling($d3, $d4);
        $this->setSibling($d4, $d5);
        $this->setSibling($d5, $d6);
        $this->setSibling($d6, $d7);

        // d5's grandchild chain -> entry+0x58/0x5c/0x60
        $e1 = $this->newNode();
        $e2 = $this->newNode();
        $e3 = $this->newNode();
        $this->setChild($d5, $e1);
        $this->setSibling($e1, $e2);
        $this->setSibling($e2, $e3);

        // d7's sibling's child chain -> entry+0x4c/0x50/0x54
        $s = $this->newNode();
        $c1 = $this->newNode();
        $c2 = $this->newNode();
        $c3 = $this->newNode();
        $this->setSibling($d7, $s);
        $this->setChild($s, $c1);
        $this->setSibling($c1, $c2);
        $this->setSibling($c2, $c3);

        $this->initUint32($entry + 0x0c, $nj);

        $this->call('_VehiclePartsBind_8c02786c')->with($entry, $typeCode);

        $this->shouldWriteLong($entry + 0x00, $typeCode);

        $this->shouldWriteLong($entry + 0x18, $n1);
        $this->shouldWriteLong($n1 + 0x00, 0x01);
        $this->shouldWriteLong($entry + 0x1c, $n2);
        $this->shouldWriteLong($n2 + 0x00, 0x01);
        $this->shouldWriteLong($entry + 0x20, $n3);
        $this->shouldWriteLong($n3 + 0x00, 0x01);
        $this->shouldWriteLong($entry + 0x24, $n4);
        $this->shouldWriteLong($n4 + 0x00, 0x01);

        $this->shouldWriteLong($entry + 0x2c, $d1);
        $this->shouldWriteLong($entry + 0x30, $d2);
        $this->shouldWriteLong($entry + 0x34, $d3);
        $this->shouldWriteLong($entry + 0x38, $d4);
        $this->shouldWriteLong($entry + 0x3c, $d5);

        $this->shouldWriteLong($entry + 0x58, $e1);
        $this->shouldWriteLong($entry + 0x5c, $e2);
        $this->shouldWriteLong($entry + 0x60, $e3);

        $this->shouldWriteLong($entry + 0x48, $d7);

        $this->shouldWriteLong($entry + 0x4c, $c1);
        $this->shouldWriteLong($entry + 0x50, $c2);
        $this->shouldWriteLong($entry + 0x54, $c3);
    }
};
