<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// FUN_8c02e51c looks up the road junction under (x, z) in the attribute
// grid pointed to by var_activeAttrGrid_8c228b3c. *out already holding a previous match
// (out->count != 0, at offset 0x08) re-tests that polygon first (a plain
// convex cross-product walk, no concave path in this unit's simple pair);
// only on a miss does it fall back to a 1x1-cell full search, which skips
// the just-tested polygon by vertex-id pointer. On a hit it returns
// &polys[*slot].attr_0x08; on a miss, NULL.

return new class extends TestCase {
    private function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }

    private function initFloat(int $address, float $value): void
    {
        $this->initUint32($address, unpack('L', pack('f', $value))[1]);
    }

    private function resolveSymbols(): void {
    }

    // Two convex quads in (x, z), CCW winding, offset along x so they never
    // overlap: quad 0 = (0,0)-(10,0)-(10,10)-(0,10); quad 1 the same shape
    // shifted +20 in x. All 1x1-cell candidates. Returns
    // [attrAddr0, ids0, attrAddr1, ids1, slotAddr0, slotAddr1].
    private function makeGrid(int $attr0, int $attr1): array {
        $grid = $this->alloc(0x1c);
        $cell = $this->alloc(8);
        $polyIds = $this->alloc(2 * 4);
        $polys = $this->alloc(2 * 0x18);
        $verts = $this->alloc(8 * 0xc);

        $this->initUint32($grid + 0x00, 1); // cellsX
        $this->initUint32($grid + 0x04, 1); // cellsZ
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $polys);
        $this->initUint32($grid + 0x18, $verts);

        $this->initUint32($cell + 0x00, 2); // count
        $this->initUint32($cell + 0x04, $polyIds);
        $this->initUint32($polyIds + 0, 0); // poly index 0
        $this->initUint32($polyIds + 4, 1); // poly index 1

        $attrs = [];
        $idsAddrs = [];
        foreach ([0, 1] as $p) {
            $poly = $polys + $p * 0x18;
            $ids = $verts; // placeholder, real ids array allocated separately below
            $ids = $this->alloc(4 * 4);
            $xOff = $p * 20.0;

            $this->initUint32($poly + 0x00, 4); // vertexCount
            $this->initUint32($poly + 0x04, $ids);
            $attr = ($p === 0) ? $attr0 : $attr1;
            $this->initUint32($poly + 0x08, $attr);
            $this->initUint32($poly + 0x0c, 0);
            $this->initUint32($poly + 0x10, 0); // shapeFlag: convex
            $this->initUint32($poly + 0x14, 0);

            $verticesXZ = [[0.0, 0.0], [10.0, 0.0], [10.0, 10.0], [0.0, 10.0]];
            foreach ($verticesXZ as $i => [$vx, $vz]) {
                $vBase = $verts + ($p * 4 + $i) * 0xc;
                $this->initUint32($ids + $i * 4, $p * 4 + $i);
                $this->initFloat($vBase + 0x00, $this->f32($vx + $xOff));
                $this->initFloat($vBase + 0x04, 0.0);
                $this->initFloat($vBase + 0x08, $this->f32($vz));
            }

            $attrs[$p] = $poly + 0x08;
            $idsAddrs[$p] = $ids;
        }

        $this->initUint32($this->addressOf('_var_activeAttrGrid_8c228b3c'), $grid);

        return [$attrs[0], $idsAddrs[0], $attrs[1], $idsAddrs[1], $polyIds + 0, $polyIds + 4];
    }

    private function makeOut(int $slotVal, int $vertexIds, int $count): array {
        $slot = $this->alloc(4);
        $this->initUint32($slot, $slotVal);
        $out = $this->alloc(0x0c);
        $this->initUint32($out + 0x00, $slot);
        $this->initUint32($out + 0x04, $vertexIds);
        $this->initUint32($out + 0x08, $count);
        return [$out, $slot];
    }

    // No previous match (count == 0); full-search finds quad 0 for an
    // interior point. Returns &poly.attr_0x08 and fills *out.
    public function test_noTrack_fullSearchHit(): void {
        $this->resolveSymbols();
        [$attr0, $ids0, , , $slotAddr0, ] = $this->makeGrid(0xabcd, 0xdead);
        [$out, ] = $this->makeOut(0, 0, 0);

        $this->call('_FUN_8c02e51c')->with(
            $this->f32(5.0), $this->f32(0.0), $this->f32(5.0), $out
        );

        $this->shouldWriteLong($out + 0x00, $slotAddr0);
        $this->shouldWriteLong($out + 0x04, $ids0);
        $this->shouldWriteLong($out + 0x08, 4);
        $this->shouldReturn($attr0);
    }

    // No previous match; full-search finds nothing for a point outside
    // both quads.
    public function test_noTrack_fullSearchMiss(): void {
        $this->resolveSymbols();
        $this->makeGrid(0xabcd, 0xdead);
        [$out, ] = $this->makeOut(0, 0, 0);

        $this->call('_FUN_8c02e51c')->with(
            $this->f32(50.0), $this->f32(0.0), $this->f32(50.0), $out
        );

        $this->shouldWriteLong($out + 0x04, 0);
        $this->shouldWriteLong($out + 0x08, 0);
        $this->shouldReturn(0);
    }

    // Previous match (quad 0) still contains the query point: returns
    // immediately from the track re-test, never touching the cell array.
    public function test_track_stillInside_hit(): void {
        $this->resolveSymbols();
        [$attr0, $ids0, , , , ] = $this->makeGrid(0x1234, 0x9999);
        [$out, $slot] = $this->makeOut(0, $ids0, 4);

        $this->call('_FUN_8c02e51c')->with(
            $this->f32(5.0), $this->f32(0.0), $this->f32(5.0), $out
        );

        $this->shouldWriteLong($out + 0x00, $slot);
        $this->shouldWriteLong($out + 0x04, $ids0);
        $this->shouldWriteLong($out + 0x08, 4);
        $this->shouldReturn($attr0);
    }

    // Previous match (quad 0) no longer contains the query point (it moved
    // into quad 1): the track re-test fails, so it falls through to the
    // full cell search, which skips quad 0 (by vertex-id pointer) and
    // matches quad 1 instead.
    public function test_track_movedToOtherPolygon_fallsThroughToSearch(): void {
        $this->resolveSymbols();
        [, $ids0, $attr1, $ids1, , $slotAddr1] = $this->makeGrid(0x1234, 0x9999);
        [$out, ] = $this->makeOut(0, $ids0, 4);

        $this->call('_FUN_8c02e51c')->with(
            $this->f32(25.0), $this->f32(0.0), $this->f32(5.0), $out
        );

        $this->shouldWriteLong($out + 0x00, $slotAddr1);
        $this->shouldWriteLong($out + 0x04, $ids1);
        $this->shouldWriteLong($out + 0x08, 4);
        $this->shouldReturn($attr1);
    }
};
