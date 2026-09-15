<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// FUN_8c02ec50 is the AtHeight counterpart of FUN_8c02e69c: same
// convex/concave split by shapeFlag_0x10 (njSqrt/acosf for the concave
// path), but the full cell search additionally rejects a candidate whose
// first vertex's y is more than 20 units from the query point's y --
// same idea as FUN_8c02eab4 is to FUN_8c02e51c. The track (re-test) path
// does not height-check, and (like FUN_8c02e69c) a track hit does not
// rewrite *out.

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
        $this->setSize('_njSqrt', 4);
        $this->setSize('_acosf', 4);
    }

    private function allocIntArray(array $values): int {
        $addr = $this->alloc(4 * count($values));
        foreach ($values as $i => $v) {
            $this->initUint32($addr + $i * 4, $v);
        }
        return $addr;
    }

    private function allocVertices(array $verts): int {
        $addr = $this->alloc(12 * count($verts));
        foreach ($verts as $i => [$x, $y, $z]) {
            $this->initFloat($addr + $i * 12 + 0, $this->f32($x));
            $this->initFloat($addr + $i * 12 + 4, $this->f32($y));
            $this->initFloat($addr + $i * 12 + 8, $this->f32($z));
        }
        return $addr;
    }

    // JunctionPoly layout: 0x00 vertexCount, 0x04 vertexIds, 0x08 attr,
    // 0x0c unused, 0x10 shapeFlag, 0x14 unused (0x18 bytes total).
    private function allocPoly(int $vertexCount, int $vertexIds, int $attr, int $shapeFlag): int {
        $poly = $this->alloc(0x18);
        $this->initUint32($poly + 0x00, $vertexCount);
        $this->initUint32($poly + 0x04, $vertexIds);
        $this->initUint32($poly + 0x08, $attr);
        $this->initUint32($poly + 0x0c, 0);
        $this->initUint32($poly + 0x10, $shapeFlag);
        $this->initUint32($poly + 0x14, 0);
        return $poly;
    }

    // Single-cell, single-poly grid. Returns [attrAddr, slotAddr].
    private function makeGridWithOnePoly(int $poly, int $verts): array {
        $grid = $this->alloc(0x1c);
        $cell = $this->alloc(8);
        $polyIds = $this->allocIntArray([0]);

        $this->initUint32($grid + 0x00, 1); // cellsX
        $this->initUint32($grid + 0x04, 1); // cellsZ
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $poly);
        $this->initUint32($grid + 0x18, $verts);

        $this->initUint32($cell + 0x00, 1); // count
        $this->initUint32($cell + 0x04, $polyIds);

        $this->initUint32($this->addressOf('_var_activeAttrGrid_8c228b3c'), $grid);

        return [$poly + 0x08, $polyIds];
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

    // Two convex quads (shapeFlag = 0) in (x, z), CCW winding, offset along
    // x by xOff1 so they never overlap, each at its own height (first
    // vertex's y). Returns
    // [attrAddr0, ids0, attrAddr1, ids1, slotAddr0, slotAddr1].
    private function makeConvexGrid(int $attr0, int $attr1, float $y0, float $y1, float $xOff1): array {
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
        $this->initUint32($polyIds + 0, 0);
        $this->initUint32($polyIds + 4, 1);

        $attrs = [];
        $idsAddrs = [];
        $xOffs = [0.0, $xOff1];
        $ys = [$y0, $y1];
        foreach ([0, 1] as $p) {
            $poly = $polys + $p * 0x18;
            $ids = $this->alloc(4 * 4);

            $this->initUint32($poly + 0x00, 4);
            $this->initUint32($poly + 0x04, $ids);
            $attr = ($p === 0) ? $attr0 : $attr1;
            $this->initUint32($poly + 0x08, $attr);
            $this->initUint32($poly + 0x0c, 0);
            $this->initUint32($poly + 0x10, 0); // convex
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

    // Mocks the 3-edge concave loop for the reference triangle
    // (5,0)-(4,3)-(3,4) around the origin, same values as
    // 020914_ground_query's test_concaveWindingAccepts.
    private function mockConcaveAccept(): void {
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_acosf')->with(0.6)->andReturn(0.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_acosf')->with(0.8)->andReturn(3.14159265);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_acosf')->with(0.96)->andReturn(3.14159265);
    }

    private function makeConcaveTrianglePoly(): array {
        $verts = $this->allocVertices([
            [5.0, 0.0, 0.0],
            [4.0, 0.0, 3.0],
            [3.0, 0.0, 4.0],
        ]);
        $ids = $this->allocIntArray([0, 1, 2]);
        $poly = $this->allocPoly(3, $ids, -1, 1); // shapeFlag != 0 -> concave
        return [$poly, $ids, $verts];
    }

    // No previous match; full-search hits quad 0 (y=0, convex) for an
    // interior point at y=0, since quad 1 (y=100) would fail the height
    // filter anyway.
    public function test_noTrack_fullSearchHit_convex(): void {
        $this->resolveSymbols();
        [$attr0, $ids0, , , $slotAddr0, ] =
            $this->makeConvexGrid(0xabcd, 0xdead, 0.0, 100.0, 20.0);
        [$out, ] = $this->makeOut(0, 0, 0);

        $this->call('_FUN_8c02ec50')->with(
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
        $this->makeConvexGrid(0xabcd, 0xdead, 200.0, 300.0, 20.0);
        [$out, ] = $this->makeOut(0, 0, 0);

        $this->call('_FUN_8c02ec50')->with(
            $this->f32(5.0), $this->f32(0.0), $this->f32(5.0), $out
        );

        $this->shouldWriteLong($out + 0x04, 0);
        $this->shouldWriteLong($out + 0x08, 0);
        $this->shouldReturn(0);
    }

    // Previous match (quad 0, convex) still contains the query point:
    // returns immediately from the track re-test (no height check, no
    // rewrite of *out).
    public function test_track_stillInside_hit_convex(): void {
        $this->resolveSymbols();
        [$attr0, $ids0, , , , ] =
            $this->makeConvexGrid(0x1234, 0x9999, 0.0, 100.0, 20.0);
        [$out, ] = $this->makeOut(0, $ids0, 4);

        $this->call('_FUN_8c02ec50')->with(
            $this->f32(5.0), $this->f32(0.0), $this->f32(5.0), $out
        );

        $this->shouldReturn($attr0);
    }

    // Previous match (quad 0) no longer contains the query point (moved
    // into quad 1's footprint, at a matching height): the track re-test
    // fails geometrically, falls through to the full search, which skips
    // quad 0 (vertex-id pointer), passes the height filter for quad 1, and
    // matches it.
    public function test_track_movedToOtherPolygon_fallsThroughToSearch(): void {
        $this->resolveSymbols();
        [, $ids0, $attr1, $ids1, , $slotAddr1] =
            $this->makeConvexGrid(0x1234, 0x9999, 0.0, 0.0, 20.0);
        [$out, ] = $this->makeOut(0, $ids0, 4);

        $this->call('_FUN_8c02ec50')->with(
            $this->f32(25.0), $this->f32(0.0), $this->f32(5.0), $out
        );

        $this->shouldWriteLong($out + 0x00, $slotAddr1);
        $this->shouldWriteLong($out + 0x04, $ids1);
        $this->shouldWriteLong($out + 0x08, 4);
        $this->shouldReturn($attr1);
    }

    // No previous match; full-search hits the reference triangle via the
    // concave path (shapeFlag != 0), height filter trivially passes (y=0
    // matches the triangle's own y=0 vertices).
    public function test_noTrack_fullSearchHit_concave(): void {
        $this->resolveSymbols();
        [$poly, $ids, $verts] = $this->makeConcaveTrianglePoly();
        [$attrAddr, $slotAddr] = $this->makeGridWithOnePoly($poly, $verts);
        [$out, ] = $this->makeOut(0, 0, 0);

        $this->call('_FUN_8c02ec50')->with(
            $this->f32(0.0), $this->f32(0.0), $this->f32(0.0), $out
        );

        $this->mockConcaveAccept();

        $this->shouldWriteLong($out + 0x00, $slotAddr);
        $this->shouldWriteLong($out + 0x04, $ids);
        $this->shouldWriteLong($out + 0x08, 3);
        $this->shouldReturn($attrAddr);
    }

    // Previous match (the reference triangle, concave) still contains the
    // query point: the track re-test's angle-sum path also accepts, no
    // height check, no rewrite of *out.
    public function test_track_stillInside_hit_concave(): void {
        $this->resolveSymbols();
        [$poly, $ids, $verts] = $this->makeConcaveTrianglePoly();
        [$attrAddr, ] = $this->makeGridWithOnePoly($poly, $verts);
        [$out, ] = $this->makeOut(0, $ids, 3);

        $this->call('_FUN_8c02ec50')->with(
            $this->f32(0.0), $this->f32(0.0), $this->f32(0.0), $out
        );

        $this->mockConcaveAccept();

        $this->shouldReturn($attrAddr);
    }
};
