<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// AttrQueryFindConvexPolygonAtHeight_8c02eab4 is the height-filtered counterpart of AttrQueryFindConvexPolygon_8c02e51c: same
// track-then-search shape and convex-only test, but the full cell search
// additionally rejects a candidate whose first vertex's y is more than 20
// units from the query point's y -- how an elevated road is told apart
// from the surface street beneath it. The track (re-test) path does not
// height-check.

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

    // Two overlapping (same x, z footprint) convex quads at different
    // heights: quad 0 at y=0, quad 1 at y=100 (first vertex's y is what the
    // height filter reads). Returns
    // [attrAddr0, ids0, attrAddr1, ids1, slotAddr0, slotAddr1].
    private function makeGrid(int $attr0, int $attr1, float $y0, float $y1, float $xOff1 = 0.0): array {
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
        $ys = [$y0, $y1];
        $xOffs = [0.0, $xOff1];
        foreach ([0, 1] as $p) {
            $poly = $polys + $p * 0x18;
            $ids = $this->alloc(4 * 4);

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
                $this->initFloat($vBase + 0x00, $this->f32($vx + $xOffs[$p]));
                $this->initFloat($vBase + 0x04, $this->f32($ys[$p]));
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

    // No previous match; full-search finds quad 0 (y=0) for a query at
    // y=0, since quad 1 (y=100) fails the height filter.
    public function test_noTrack_fullSearchHit_heightFilterExcludesOther(): void {
        $this->resolveSymbols();
        [$attr0, $ids0, , , $slotAddr0, ] = $this->makeGrid(0xabcd, 0xdead, 0.0, 100.0);
        [$out, ] = $this->makeOut(0, 0, 0);

        $this->call('_AttrQueryFindConvexPolygonAtHeight_8c02eab4')->with(
            $this->f32(5.0), $this->f32(0.0), $this->f32(5.0), $out
        );

        $this->shouldWriteLong($out + 0x00, $slotAddr0);
        $this->shouldWriteLong($out + 0x04, $ids0);
        $this->shouldWriteLong($out + 0x08, 4);
        $this->shouldReturn($attr0);
    }

    // No previous match; full-search rejects the only geometrically
    // matching candidate because its height is too far from the query's y.
    public function test_noTrack_fullSearchMiss_heightFilterRejects(): void {
        $this->resolveSymbols();
        $this->makeGrid(0xabcd, 0xdead, 200.0, 300.0);
        [$out, ] = $this->makeOut(0, 0, 0);

        $this->call('_AttrQueryFindConvexPolygonAtHeight_8c02eab4')->with(
            $this->f32(5.0), $this->f32(0.0), $this->f32(5.0), $out
        );

        $this->shouldWriteLong($out + 0x04, 0);
        $this->shouldWriteLong($out + 0x08, 0);
        $this->shouldReturn(0);
    }

    // Previous match (quad 0) still contains the query point: returns
    // immediately from the track re-test (no height check on this path).
    public function test_track_stillInside_hit(): void {
        $this->resolveSymbols();
        [$attr0, $ids0, , , , ] = $this->makeGrid(0x1234, 0x9999, 0.0, 100.0);
        [$out, $slot] = $this->makeOut(0, $ids0, 4);

        $this->call('_AttrQueryFindConvexPolygonAtHeight_8c02eab4')->with(
            $this->f32(5.0), $this->f32(0.0), $this->f32(5.0), $out
        );

        $this->shouldWriteLong($out + 0x00, $slot);
        $this->shouldWriteLong($out + 0x04, $ids0);
        $this->shouldWriteLong($out + 0x08, 4);
        $this->shouldReturn($attr0);
    }

    // Previous match (quad 0) no longer contains the query point (moved
    // into quad 1's footprint, which is at a matching height): the track
    // re-test fails geometrically, falls through to the full search, which
    // skips quad 0 (vertex-id pointer), passes the height filter for
    // quad 1, and matches it.
    public function test_track_movedToOtherPolygon_fallsThroughToSearch(): void {
        $this->resolveSymbols();
        [, $ids0, $attr1, $ids1, , $slotAddr1] =
            $this->makeGrid(0x1234, 0x9999, 0.0, 0.0, 20.0);
        [$out, ] = $this->makeOut(0, $ids0, 4);

        $this->call('_AttrQueryFindConvexPolygonAtHeight_8c02eab4')->with(
            $this->f32(25.0), $this->f32(0.0), $this->f32(5.0), $out
        );

        $this->shouldWriteLong($out + 0x00, $slotAddr1);
        $this->shouldWriteLong($out + 0x04, $ids1);
        $this->shouldWriteLong($out + 0x08, 4);
        $this->shouldReturn($attr1);
    }
};
