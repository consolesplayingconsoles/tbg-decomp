<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

// GroundProbeFindPolygonAtHeight_8c020fe4 is the same full cell search as
// GroundQueryFindPolygon_8c020914, with two differences: the grid cell size
// comes from the grid itself (cellSizeX_0x08/cellSizeZ_0x0c) instead of the
// fixed 150-unit CELL_SIZE, and each candidate is additionally rejected
// -- before the containment test -- when its first vertex's height is more
// than 20.0 away from y. This is how an elevated road is told apart from
// the surface street below it.
return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_activeGroundGrid_8c2264d4', 4);
        $this->setSize('_njSqrt', 4);
        $this->setSize('_acosf', 4);
    }

    // GroundGrid layout: 0x00 cellsX, 0x04 cellsZ, 0x08 cellSizeX, 0x0c cellSizeZ,
    // 0x10 cells, 0x14 polys, 0x18 verts.
    private function allocGrid(int $cellsX, int $cellsZ, float $cellSizeX = 150.0, float $cellSizeZ = 150.0): int {
        $grid = $this->alloc(0x1c);
        for ($i = 0; $i < 0x1c; $i++) {
            $this->initUint8($grid + $i, 0);
        }
        $this->initUint32($grid + 0x00, $cellsX);
        $this->initUint32($grid + 0x04, $cellsZ);
        $this->initUint32($grid + 0x08, fdec($cellSizeX));
        $this->initUint32($grid + 0x0c, fdec($cellSizeZ));
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

        $this->call('_GroundProbeFindPolygonAtHeight_8c020fe4')->with(0.0, 0.0, 0.0, $out);

        $this->shouldMiss($out);
    }

    // ------------------------------------------------------------------
    // Query point is inside the triangle in XZ, and its first vertex's
    // height (100.0) is within tolerance of y (105.0, diff 5.0) -> hit.
    // ------------------------------------------------------------------
    public function test_withinHeightToleranceHits(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [-10.0, 100.0, -10.0],
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

        $this->call('_GroundProbeFindPolygonAtHeight_8c020fe4')->with(0.0, 105.0, 0.0, $out);

        $this->shouldWriteLong($out + 0x00, 5);
        $this->shouldWriteLong($out + 0x04, $polyIds);
        $this->shouldWriteLong($out + 0x08, $ids);
        $this->shouldWriteLong($out + 0x0c, 3);
    }

    // ------------------------------------------------------------------
    // Same triangle, same XZ containment, but y is 100.0 away from the
    // first vertex's height (100.0), well past the 20.0 tolerance -> the
    // height check rejects it before the containment test even runs, and
    // it's the only poly in the cell, so the overall result is a miss.
    // ------------------------------------------------------------------
    public function test_outsideHeightToleranceMisses(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [-10.0, 100.0, -10.0],
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

        $this->call('_GroundProbeFindPolygonAtHeight_8c020fe4')->with(0.0, 200.0, 0.0, $out);

        $this->shouldMiss($out);
    }

    // ------------------------------------------------------------------
    // Cell index uses the grid's own cell size (50.0), not the fixed
    // 150-unit CELL_SIZE: query x=140 -> cx = int(140/50) = 2. Cells 0 and
    // 1 are left as misses so a wrong (150-based, cx=0) index would land
    // on a miss instead of the hit in cell 2.
    // ------------------------------------------------------------------
    public function test_usesGridOwnCellSize(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [130.0, 0.0, -10.0],
            [150.0, 0.0, -10.0],
            [140.0, 0.0, 10.0],
        ]);
        $ids = $this->allocIntArray([0, 1, 2]);
        $poly = $this->allocPoly(3, $ids, 5);
        $polyIds = $this->allocIntArray([0]);

        $grid = $this->allocGrid(3, 1, 50.0, 50.0);
        $cells = $this->alloc(3 * 8);
        for ($i = 0; $i < 2; $i++) {
            $this->initUint32($cells + $i * 8 + 0, 0);
            $this->initUint32($cells + $i * 8 + 4, 0);
        }
        $this->initUint32($cells + 2 * 8 + 0, 1);
        $this->initUint32($cells + 2 * 8 + 4, $polyIds);
        $this->initUint32($grid + 0x10, $cells);
        $this->initUint32($grid + 0x14, $poly);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $out = $this->allocOut();

        $this->call('_GroundProbeFindPolygonAtHeight_8c020fe4')->with(140.0, 0.0, 0.0, $out);

        $this->shouldWriteLong($out + 0x00, 5);
        $this->shouldWriteLong($out + 0x04, $polyIds);
        $this->shouldWriteLong($out + 0x08, $ids);
        $this->shouldWriteLong($out + 0x0c, 3);
    }

    // ------------------------------------------------------------------
    // Concave (attr < 0) path still runs behind the height check, exactly
    // as GroundQueryFindPolygon_8c020914's concave-accept case.
    // ------------------------------------------------------------------
    public function test_concaveWindingAcceptsWithinHeight(): void {
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

        $this->call('_GroundProbeFindPolygonAtHeight_8c020fe4')->with(0.0, 0.0, 0.0, $out);

        // Same mock sequence as GroundQueryFindPolygon_8c020914's concaveWindingAccepts.
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_acosf')->with(0.6)->andReturn(0.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_acosf')->with(0.8)->andReturn(3.14159265);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_acosf')->with(0.96)->andReturn(3.14159265);

        $this->shouldWriteLong($out + 0x00, 0x7fffffff);
        $this->shouldWriteLong($out + 0x04, $polyIds);
        $this->shouldWriteLong($out + 0x08, $ids);
        $this->shouldWriteLong($out + 0x0c, 3);
    }
};
