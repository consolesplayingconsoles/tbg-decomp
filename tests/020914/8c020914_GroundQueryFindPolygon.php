<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

// GroundQueryFindPolygon_8c020914 looks up the ground polygon under world
// point (x, z) -- y is a dead argument -- in the grid pointed to by
// var_activeGroundGrid_8c2264d4 (GroundGrid: cellsX/cellsZ, cells, polys,
// verts). It picks a 150-unit grid cell (clamped only on the upper bound --
// no lower clamp, matching the original), then walks that cell's poly id
// list. Each candidate poly is tested by one of two containment checks
// chosen by the sign of its attr word: a convex cross-product walk (attr
// >= 0), or a concave winding-angle sum in BAMS via njSqrt/acosf (attr < 0).
// A hit writes the poly's {attr (sign bit stripped), polyIdSlot, vertexIds,
// count}; a miss (including an empty cell) writes only a zeroed count and
// vertexIds.
return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_njSqrt', 4);
        $this->setSize('_acosf', 4);
    }

    // GroundGrid layout: 0x00 cellsX, 0x04 cellsZ, 0x10 cells, 0x14 polys, 0x18 verts.
    private function allocGrid(int $cellsX, int $cellsZ): int {
        $grid = $this->alloc(0x1c);
        for ($i = 0; $i < 0x1c; $i++) {
            $this->initUint8($grid + $i, 0);
        }
        $this->initUint32($grid + 0x00, $cellsX);
        $this->initUint32($grid + 0x04, $cellsZ);
        return $grid;
    }

    // GroundCell layout: 0x00 count, 0x04 polyIds. Returns its address.
    private function allocCell(int $count, ?int $polyIds): int {
        $cell = $this->alloc(8);
        $this->initUint32($cell + 0x00, $count);
        $this->initUint32($cell + 0x04, $polyIds ?? 0);
        return $cell;
    }

    // GroundPoly layout: 0x00 vertexCount, 0x04 vertexIds, 0x14 attr (0x18 bytes total).
    private function allocPoly(int $vertexCount, int $vertexIds, int $attr): int {
        $poly = $this->alloc(0x18);
        for ($i = 0; $i < 0x18; $i++) {
            $this->initUint8($poly + $i, 0);
        }
        $this->initUint32($poly + 0x00, $vertexCount);
        $this->initUint32($poly + 0x04, $vertexIds);
        $this->initUint32($poly + 0x14, $attr);
        return $poly;
    }

    // Allocates a contiguous array of int32 values; returns its base address.
    private function allocIntArray(array $values): int {
        $addr = $this->alloc(4 * count($values));
        foreach ($values as $i => $v) {
            $this->initUint32($addr + $i * 4, $v);
        }
        return $addr;
    }

    // Allocates a contiguous array of 3-float vertices {x, y, z}; returns its base address.
    private function allocVertices(array $verts): int {
        $addr = $this->alloc(12 * count($verts));
        foreach ($verts as $i => list($x, $y, $z)) {
            $this->initUint32($addr + $i * 12 + 0, fdec($x));
            $this->initUint32($addr + $i * 12 + 4, fdec($y));
            $this->initUint32($addr + $i * 12 + 8, fdec($z));
        }
        return $addr;
    }

    private function allocOut(): int {
        $out = $this->alloc(0x10);
        for ($i = 0; $i < 0x10; $i++) {
            $this->initUint8($out + $i, 0);
        }
        return $out;
    }

    private function setActiveGrid(int $grid): void {
        $this->initUint32($this->addressOf('_var_activeGroundGrid_8c2264d4'), $grid);
    }

    private function shouldMiss(int $out): void {
        $this->shouldWriteLong($out + 0x08, 0);
        $this->shouldWriteLong($out + 0x0c, 0);
    }

    // ------------------------------------------------------------------
    // Empty cell -> miss.
    // ------------------------------------------------------------------
    public function test_emptyCellMisses(): void {
        $this->resolveSymbols();

        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(0, null);
        $this->initUint32($grid + 0x10, $cell);
        $this->setActiveGrid($grid);

        $out = $this->allocOut();

        $this->call('_GroundQueryFindPolygon_8c020914')->with(0.0, 0.0, 0.0, $out);

        $this->shouldMiss($out);
    }

    // ------------------------------------------------------------------
    // Convex (attr >= 0) hit: query point inside a CCW triangle.
    // ------------------------------------------------------------------
    public function test_convexHit(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [-10.0, 0.0, -10.0],
            [10.0, 0.0, -10.0],
            [0.0, 0.0, 10.0],
        ]);
        $ids = $this->allocIntArray([0, 1, 2]);
        $poly = $this->allocPoly(3, $ids, 5);
        $polyIds = $this->allocIntArray([0]); // poly index 0

        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(1, $polyIds);
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $poly);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $out = $this->allocOut();

        $this->call('_GroundQueryFindPolygon_8c020914')->with(0.0, 0.0, 0.0, $out);

        $this->shouldWriteLong($out + 0x00, 5);          // attr, sign bit already clear
        $this->shouldWriteLong($out + 0x04, $polyIds);   // &cell->polyIds[0]
        $this->shouldWriteLong($out + 0x08, $ids);
        $this->shouldWriteLong($out + 0x0c, 3);
    }

    // ------------------------------------------------------------------
    // Convex (attr >= 0) reject: query point outside the same triangle,
    // so the cross-product walk breaks early -- falls through to the
    // cell's overall miss (only poly in the cell).
    // ------------------------------------------------------------------
    public function test_convexRejectFallsThroughToMiss(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [-10.0, 0.0, -10.0],
            [10.0, 0.0, -10.0],
            [0.0, 0.0, 10.0],
        ]);
        $ids = $this->allocIntArray([0, 1, 2]);
        $poly = $this->allocPoly(3, $ids, 5);
        $polyIds = $this->allocIntArray([0]);

        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(1, $polyIds);
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $poly);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $out = $this->allocOut();

        // Far outside the triangle.
        $this->call('_GroundQueryFindPolygon_8c020914')->with(100.0, 0.0, 100.0, $out);

        $this->shouldMiss($out);
    }

    // ------------------------------------------------------------------
    // Concave (attr < 0) accept: 3 length-5 vertices around the query
    // point, none perpendicular or collinear with its predecessor, so
    // every dot product is a clean nonzero value (avoids a +-0.0 cosT that
    // can't be pinned to one sign) and every njSqrt argument is 25.0.
    // njSqrt/acosf are mocked, so the returned angle per iteration is
    // chosen (not the real acos) purely to control the winding sum.
    // ------------------------------------------------------------------
    public function test_concaveWindingAccepts(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [5.0, 0.0, 0.0],
            [4.0, 0.0, 3.0],
            [3.0, 0.0, 4.0],
        ]);
        $ids = $this->allocIntArray([0, 1, 2]);
        $poly = $this->allocPoly(3, $ids, -1); // attr < 0 selects the concave path
        $polyIds = $this->allocIntArray([0]);

        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(1, $polyIds);
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $poly);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $out = $this->allocOut();

        $this->call('_GroundQueryFindPolygon_8c020914')->with(0.0, 0.0, 0.0, $out);

        // iter0: prev=(3,4) cur=(5,0): dot=15 cosT=0.6, cross=-15 (negative -> acc -= t)
        // iter1: prev=(5,0) cur=(4,3): dot=20 cosT=0.8, cross=15  (positive -> acc += t)
        // iter2: prev=(4,3) cur=(3,4): dot=24 cosT=0.96, cross=7  (positive -> acc += t)
        // Mock acosf(0.6)->0 (t=0), acosf(0.8)->pi and acosf(0.96)->pi so
        // the two added angles alone sum past 0x8000: 0 + 32768 + 32768 == 0x10000.
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_acosf')->with(0.6)->andReturn(0.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_acosf')->with(0.8)->andReturn(3.14159265);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_acosf')->with(0.96)->andReturn(3.14159265);

        $this->shouldWriteLong($out + 0x00, 0x7fffffff); // -1 & 0x7fffffff
        $this->shouldWriteLong($out + 0x04, $polyIds);
        $this->shouldWriteLong($out + 0x08, $ids);
        $this->shouldWriteLong($out + 0x0c, 3);
    }

    // ------------------------------------------------------------------
    // Concave (attr < 0) reject: same triangle, but acosf is mocked to
    // return small angles so the summed BAMS angle stays under 0x8000.
    // ------------------------------------------------------------------
    public function test_concaveWindingRejects(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [5.0, 0.0, 0.0],
            [4.0, 0.0, 3.0],
            [3.0, 0.0, 4.0],
        ]);
        $ids = $this->allocIntArray([0, 1, 2]);
        $poly = $this->allocPoly(3, $ids, -1);
        $polyIds = $this->allocIntArray([0]);

        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(1, $polyIds);
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $poly);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $out = $this->allocOut();

        $this->call('_GroundQueryFindPolygon_8c020914')->with(0.0, 0.0, 0.0, $out);

        // 0 + (int)(0.1*65536/6.283184) + (int)(0.1*65536/6.283184)
        // == 0 + 1043 + 1043 == 2086, well under 0x8000.
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_acosf')->with(0.6)->andReturn(0.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_acosf')->with(0.8)->andReturn(0.1);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_acosf')->with(0.96)->andReturn(0.1);

        $this->shouldMiss($out);
    }

    // ------------------------------------------------------------------
    // Concave cosT clamp, upper bound: njSqrt is mocked to return values
    // far smaller than the real vector lengths, so dot/(s1*s2) comes out
    // above 1.0 and must be clamped before acosf is called with it.
    // ------------------------------------------------------------------
    public function test_concaveCosineClampsAboveOne(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [5.0, 0.0, 0.0],
        ]);
        $ids = $this->allocIntArray([0]); // n=1: prev == cur, dot == 25 (positive)
        $poly = $this->allocPoly(1, $ids, -1);
        $polyIds = $this->allocIntArray([0]);

        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(1, $polyIds);
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $poly);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $out = $this->allocOut();

        $this->call('_GroundQueryFindPolygon_8c020914')->with(0.0, 0.0, 0.0, $out);

        // dot = 25, but njSqrt is mocked to 0.1 for both calls, so
        // cosT = 25 / (0.1*0.1) = 2500 -- clamped to 1.0 before acosf.
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_acosf')->with(1.0)->andReturn(0.0);

        $this->shouldMiss($out);
    }

    // ------------------------------------------------------------------
    // Concave cosT clamp, lower bound: two opposite-facing vertices give a
    // negative dot; njSqrt mocked small again pushes cosT below -1.0.
    // ------------------------------------------------------------------
    public function test_concaveCosineClampsBelowNegativeOne(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [5.0, 0.0, 0.0],
            [-5.0, 0.0, 0.0],
        ]);
        $ids = $this->allocIntArray([0, 1]);
        $poly = $this->allocPoly(2, $ids, -1);
        $polyIds = $this->allocIntArray([0]);

        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(1, $polyIds);
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $poly);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $out = $this->allocOut();

        $this->call('_GroundQueryFindPolygon_8c020914')->with(0.0, 0.0, 0.0, $out);

        // Both edges: dot = -25, njSqrt mocked to 0.1 each call, so
        // cosT = -25 / (0.1*0.1) = -2500 -- clamped to -1.0 before acosf.
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_acosf')->with(-1.0)->andReturn(0.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_acosf')->with(-1.0)->andReturn(0.0);

        $this->shouldMiss($out);
    }

    // ------------------------------------------------------------------
    // Upper-bound clamp: cellsX/cellsZ are 2, but the query point's cell
    // coordinates (x/150, z/150) both compute past the grid, so both get
    // clamped to cellsX-1/cellsZ-1 (index 3 of a 2x2 grid). The other 3
    // cells are left as misses so a wrong (unclamped, out-of-range) index
    // would read one of those instead and miss where this expects a hit.
    // ------------------------------------------------------------------
    public function test_upperClampSelectsLastCell(): void {
        $this->resolveSymbols();

        // Same triangle shape as test_convexHit, but re-centered on
        // (1000, 1000) -- the query point below must still fall geometrically
        // inside it; only the *cell* lookup is meant to exercise the clamp.
        $verts = $this->allocVertices([
            [990.0, 0.0, 990.0],
            [1010.0, 0.0, 990.0],
            [1000.0, 0.0, 1010.0],
        ]);
        $ids = $this->allocIntArray([0, 1, 2]);
        $poly = $this->allocPoly(3, $ids, 5);
        $polyIds = $this->allocIntArray([0]);

        $grid = $this->allocGrid(2, 2);
        $cells = $this->alloc(4 * 8);
        // cells[0..2] miss, cells[3] (cz=1, cx=1) is the clamp target.
        for ($i = 0; $i < 3; $i++) {
            $this->initUint32($cells + $i * 8 + 0, 0); // count
            $this->initUint32($cells + $i * 8 + 4, 0); // polyIds
        }
        $this->initUint32($cells + 3 * 8 + 0, 1);
        $this->initUint32($cells + 3 * 8 + 4, $polyIds);
        $this->initUint32($grid + 0x10, $cells);
        $this->initUint32($grid + 0x14, $poly);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $out = $this->allocOut();

        // x/150 = 1000/150 = 6 -> clamp to 1; z/150 = 1000/150 = 6 -> clamp to 1.
        $this->call('_GroundQueryFindPolygon_8c020914')->with(1000.0, 0.0, 1000.0, $out);

        $this->shouldWriteLong($out + 0x00, 5);
        $this->shouldWriteLong($out + 0x04, $polyIds);
        $this->shouldWriteLong($out + 0x08, $ids);
        $this->shouldWriteLong($out + 0x0c, 3);
    }
};
