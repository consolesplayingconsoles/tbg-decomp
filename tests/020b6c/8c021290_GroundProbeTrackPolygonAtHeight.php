<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

// GroundProbeTrackPolygonAtHeight_8c021290 combines the two prior siblings:
// *out's previous match is re-tested first, exactly as
// GroundProbeTrackPolygon_8c020b6c does, and only on a miss does it fall
// back to a full cell search shaped like
// GroundProbeFindPolygonAtHeight_8c020fe4's -- grid-defined cell size, each
// candidate rejected first by the HEIGHT_TOLERANCE check on y -- with
// GroundProbeTrackPolygon_8c020b6c's extra step of skipping the polygon just
// re-tested (compared by its vertexIds pointer).
//
// Unlike GroundProbeTrackPolygon_8c020b6c, y is NOT dead here: it feeds the
// height filter directly in the fallback search, same as in
// GroundProbeFindPolygonAtHeight_8c020fe4. The re-test itself never looks at
// height (same as GroundProbeTrackPolygon_8c020b6c's re-test).
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

    // GroundCell layout: 0x00 count, 0x04 polyIds.
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

    private function allocIntArray(array $values): int {
        $addr = $this->alloc(4 * count($values));
        foreach ($values as $i => $v) {
            $this->initUint32($addr + $i * 4, $v);
        }
        return $addr;
    }

    private function allocVertices(array $verts): int {
        $addr = $this->alloc(12 * count($verts));
        foreach ($verts as $i => list($x, $y, $z)) {
            $this->initUint32($addr + $i * 12 + 0, fdec($x));
            $this->initUint32($addr + $i * 12 + 4, fdec($y));
            $this->initUint32($addr + $i * 12 + 8, fdec($z));
        }
        return $addr;
    }

    // GroundQueryResult layout: 0x00 attr, 0x04 polyIdSlot, 0x08 vertexIds, 0x0c count.
    private function allocOut(int $polyIdSlot, int $vertexIds, int $count): int {
        $out = $this->alloc(0x10);
        $this->initUint32($out + 0x00, 0);
        $this->initUint32($out + 0x04, $polyIdSlot);
        $this->initUint32($out + 0x08, $vertexIds);
        $this->initUint32($out + 0x0c, $count);
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
    // Tracked polygon still contains the point: the re-test hits
    // immediately, with no height check involved and no cell search --
    // the grid's cell array is left unallocated (cellsX/cellsZ both 0),
    // so a wrong implementation falling through to the fallback would
    // dereference that and fail.
    // ------------------------------------------------------------------
    public function test_recheckHitsWithoutSearching(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [-10.0, 0.0, -10.0],
            [10.0, 0.0, -10.0],
            [0.0, 0.0, 10.0],
        ]);
        $ids = $this->allocIntArray([0, 1, 2]);
        $poly = $this->allocPoly(3, $ids, 5);
        $slot = $this->allocIntArray([0]);

        $grid = $this->allocGrid(0, 0); // no cells -- must not be searched
        $this->initUint32($grid + 0x14, $poly);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $out = $this->allocOut($slot, $ids, 3);

        // y (999.0) is far outside any HEIGHT_TOLERANCE window -- if the
        // re-test wrongly applied the height filter it would reject here.
        $this->call('_GroundProbeTrackPolygonAtHeight_8c021290')->with(0.0, 999.0, 0.0, $out);

        $this->shouldWriteLong($out + 0x00, 5);
        $this->shouldWriteLong($out + 0x04, $slot);
        $this->shouldWriteLong($out + 0x08, $ids);
        $this->shouldWriteLong($out + 0x0c, 3);
    }

    // ------------------------------------------------------------------
    // count_0x0c == 0 bails straight to the fallback search. Its cell
    // lists two candidates: the first fails the height check (its first
    // vertex's y is 100.0 away from the query's 0.0, past the 20.0
    // tolerance) and must be rejected WITHOUT running the containment
    // test at all (no njSqrt/acosf calls expected for it); the second is
    // within tolerance and hits.
    // ------------------------------------------------------------------
    public function test_heightRejectsFirstCandidateThenSecondHits(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [-10.0, 100.0, -10.0], // candidate A's triangle -- height 100.0, far off
            [10.0, 100.0, -10.0],
            [0.0, 100.0, 10.0],
            [-10.0, 0.0, -10.0],   // candidate B's triangle -- height 0.0, within tolerance
            [10.0, 0.0, -10.0],
            [0.0, 0.0, 10.0],
        ]);
        $idsA = $this->allocIntArray([0, 1, 2]);
        $idsB = $this->allocIntArray([3, 4, 5]);

        $polys = $this->alloc(2 * 0x18);
        for ($i = 0; $i < 2 * 0x18; $i++) {
            $this->initUint8($polys + $i, 0);
        }
        $this->initUint32($polys + 0x00, 3);
        $this->initUint32($polys + 0x04, $idsA);
        $this->initUint32($polys + 0x14, 5);
        $this->initUint32($polys + 0x18 + 0x00, 3);
        $this->initUint32($polys + 0x18 + 0x04, $idsB);
        $this->initUint32($polys + 0x18 + 0x14, 5);

        $cellPolyIds = $this->allocIntArray([0, 1]);
        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(2, $cellPolyIds);
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $polys);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $out = $this->allocOut(0, 0, 0);

        $this->call('_GroundProbeTrackPolygonAtHeight_8c021290')->with(0.0, 0.0, 0.0, $out);

        $this->shouldWriteLong($out + 0x00, 5);
        $this->shouldWriteLong($out + 0x04, $cellPolyIds + 4); // &cellPolyIds[1]
        $this->shouldWriteLong($out + 0x08, $idsB);
        $this->shouldWriteLong($out + 0x0c, 3);
    }

    // ------------------------------------------------------------------
    // Single candidate, within height tolerance, but outside the triangle
    // in XZ -> containment rejects too -> overall miss. Proves the height
    // check alone doesn't cause a hit; containment still runs and can
    // still reject.
    // ------------------------------------------------------------------
    public function test_withinHeightButOutsideContainmentMisses(): void {
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

        $out = $this->allocOut(0, 0, 0);

        // Far outside the triangle, but height (0.0) matches exactly.
        $this->call('_GroundProbeTrackPolygonAtHeight_8c021290')->with(1000.0, 0.0, 1000.0, $out);

        $this->shouldMiss($out);
    }

    // ------------------------------------------------------------------
    // Tracked polygon no longer contains the point (re-test rejects); the
    // fallback cell lists that SAME polygon first (within height
    // tolerance) -- it must be skipped by vertexIds pointer, not
    // re-tested -- then a second, different polygon which hits. If the
    // skip didn't fire, the re-test's njSqrt/acosf calls would be
    // consumed a second time by the fallback and this test would report
    // unmet/extra call expectations.
    // ------------------------------------------------------------------
    public function test_recheckMissSkipsSamePolygonInFallback(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [5.0, 0.0, 0.0],       // A's single vertex (concave, degenerate)
            [-10.0, 0.0, -10.0],   // B's triangle
            [10.0, 0.0, -10.0],
            [0.0, 0.0, 10.0],
        ]);
        $idsA = $this->allocIntArray([0]);
        $idsB = $this->allocIntArray([1, 2, 3]);

        $polys = $this->alloc(2 * 0x18);
        for ($i = 0; $i < 2 * 0x18; $i++) {
            $this->initUint8($polys + $i, 0);
        }
        $this->initUint32($polys + 0x00, 1);
        $this->initUint32($polys + 0x04, $idsA);
        $this->initUint32($polys + 0x14, 0xffffffff); // -1, concave
        $this->initUint32($polys + 0x18 + 0x00, 3);
        $this->initUint32($polys + 0x18 + 0x04, $idsB);
        $this->initUint32($polys + 0x18 + 0x14, 5);

        $cellPolyIds = $this->allocIntArray([0, 1]); // A then B
        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(2, $cellPolyIds);
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $polys);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $slotA = $this->allocIntArray([0]);
        $out = $this->allocOut($slotA, $idsA, 1);

        $this->call('_GroundProbeTrackPolygonAtHeight_8c021290')->with(0.0, 0.0, 0.0, $out);

        // Re-test of A: n=1, prev==cur vertex, dot=25, cross=0 -> acc stays
        // 0 -> reject. These three calls happen for A only once -- the
        // fallback must not repeat them.
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_acosf')->with(1.0)->andReturn(0.0);

        $this->shouldWriteLong($out + 0x00, 5);
        $this->shouldWriteLong($out + 0x04, $cellPolyIds + 4); // &cellPolyIds[1]
        $this->shouldWriteLong($out + 0x08, $idsB);
        $this->shouldWriteLong($out + 0x0c, 3);
    }

    // ------------------------------------------------------------------
    // Fallback cell index uses the grid's own cell size (50.0), not the
    // fixed 150-unit CELL_SIZE from GroundProbeTrackPolygon_8c020b6c:
    // query x=140 -> cx = int(140/50) = 2. Cells 0 and 1 are left as
    // misses so a wrong (150-based, cx=0) index would land on a miss
    // instead of the hit in cell 2.
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

        $out = $this->allocOut(0, 0, 0);

        $this->call('_GroundProbeTrackPolygonAtHeight_8c021290')->with(140.0, 0.0, 0.0, $out);

        $this->shouldWriteLong($out + 0x00, 5);
        $this->shouldWriteLong($out + 0x04, $polyIds);
        $this->shouldWriteLong($out + 0x08, $ids);
        $this->shouldWriteLong($out + 0x0c, 3);
    }

    // ------------------------------------------------------------------
    // Tracked polygon is concave and still contains the point: re-test
    // hits via the winding-angle sum, exercising the negative-cross
    // "acc -= angle" branch (iter0 has cross < 0) -- proves the re-test's
    // njSqrt/acosf sequence still runs correctly on this function.
    // ------------------------------------------------------------------
    public function test_recheckConcaveHitWithNegativeCrossBranch(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [5.0, 0.0, 0.0],
            [4.0, 0.0, 3.0],
            [3.0, 0.0, 4.0],
        ]);
        $ids = $this->allocIntArray([0, 1, 2]);
        $poly = $this->allocPoly(3, $ids, -1);

        $grid = $this->allocGrid(0, 0); // no cells -- recheck must hit first
        $this->initUint32($grid + 0x14, $poly);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $slot = $this->allocIntArray([0]);
        $out = $this->allocOut($slot, $ids, 3);

        $this->call('_GroundProbeTrackPolygonAtHeight_8c021290')->with(0.0, 0.0, 0.0, $out);

        // Same mocked sequence as the sibling functions' concaveWindingAccepts.
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
        $this->shouldWriteLong($out + 0x04, $slot);
        $this->shouldWriteLong($out + 0x08, $ids);
        $this->shouldWriteLong($out + 0x0c, 3);
    }

    // ------------------------------------------------------------------
    // Empty fallback cell -> overall miss.
    // ------------------------------------------------------------------
    public function test_emptyCellMisses(): void {
        $this->resolveSymbols();

        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(0, null);
        $this->initUint32($grid + 0x10, $cell);
        $this->setActiveGrid($grid);

        $out = $this->allocOut(0, 0, 0);

        $this->call('_GroundProbeTrackPolygonAtHeight_8c021290')->with(0.0, 0.0, 0.0, $out);

        $this->shouldMiss($out);
    }
};
