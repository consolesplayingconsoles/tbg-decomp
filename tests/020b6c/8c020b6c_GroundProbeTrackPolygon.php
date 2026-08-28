<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

// GroundProbeTrackPolygon_8c020b6c is the per-frame form of
// GroundQueryFindPolygon_8c020914: *out already holds a previous match. It
// re-tests that polygon first (same convex/concave test as 020914, chosen by
// the sign of the poly's attr word) and only falls back to a full cell search
// when the point has left it -- skipping the just-rejected polygon in that
// search (compared by its vertexIds pointer) since retesting it would be
// redundant. y (FR5) is a dead argument -- never read, only ever clobbered as
// scratch space for the concave test's running value.
return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_activeGroundGrid_8c2264d4', 4);
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
    // Tracked polygon still contains the point (convex, attr >= 0): the
    // recheck hits immediately. The grid's cell array is left unallocated
    // (cellsX/cellsZ both 0, cells_0x10 null) -- a wrong implementation
    // that fell through to the full search would dereference that and
    // fail, instead of the miss/hit this test actually checks.
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
        $slot = $this->allocIntArray([0]); // poly index 0

        $grid = $this->allocGrid(0, 0); // no cells -- must not be searched
        $this->initUint32($grid + 0x14, $poly);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $out = $this->allocOut($slot, $ids, 3);

        // y (42.0) is dead -- never read.
        $this->call('_GroundProbeTrackPolygon_8c020b6c')->with(0.0, 42.0, 0.0, $out);

        $this->shouldWriteLong($out + 0x00, 5);
        $this->shouldWriteLong($out + 0x04, $slot);
        $this->shouldWriteLong($out + 0x08, $ids);
        $this->shouldWriteLong($out + 0x0c, 3);
    }

    // ------------------------------------------------------------------
    // count_0x0c == 0 bails straight to the full cell search (the
    // GroundQueryFindPolygon_8c020914-equivalent path), and hits.
    // ------------------------------------------------------------------
    public function test_zeroCountGoesStraightToSearch(): void {
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

        $this->call('_GroundProbeTrackPolygon_8c020b6c')->with(0.0, 0.0, 0.0, $out);

        $this->shouldWriteLong($out + 0x00, 5);
        $this->shouldWriteLong($out + 0x04, $polyIds);
        $this->shouldWriteLong($out + 0x08, $ids);
        $this->shouldWriteLong($out + 0x0c, 3);
    }

    // ------------------------------------------------------------------
    // Tracked polygon (concave, attr < 0, single vertex so the winding sum
    // is deterministic: dot == 25, njSqrt args both 25.0, angle 0 -> acc
    // stays 0, a reject) no longer contains the point. Falls back to the
    // full search, whose cell lists that SAME polygon first, then a second
    // (convex) one -- proving the fallback skips the just-rejected polygon
    // (matched by its vertexIds pointer) rather than re-testing it: if it
    // didn't skip, the mocked njSqrt/acosf calls below would be consumed
    // twice and this test would report unmet/extra call expectations.
    // ------------------------------------------------------------------
    public function test_recheckMissFallsBackAndSkipsSamePolygon(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [5.0, 0.0, 0.0],       // A's single vertex
            [-10.0, 0.0, -10.0],   // B's vertices
            [10.0, 0.0, -10.0],
            [0.0, 0.0, 10.0],
        ]);
        $idsA = $this->allocIntArray([0]);
        $polyA = $this->allocPoly(1, $idsA, -1); // concave

        $idsB = $this->allocIntArray([1, 2, 3]);
        $polyB = $this->allocPoly(3, $idsB, 5); // convex

        $polys = $this->alloc(2 * 0x18);
        // Copy A into polys[0], B into polys[1] (allocPoly already returned
        // standalone blocks; re-seed a contiguous array indexed by poly id).
        for ($i = 0; $i < 0x18; $i++) {
            $this->initUint8($polys + $i, 0);
        }
        $this->initUint32($polys + 0x00, 1);
        $this->initUint32($polys + 0x04, $idsA);
        $this->initUint32($polys + 0x14, 0xffffffff); // -1
        for ($i = 0; $i < 0x18; $i++) {
            $this->initUint8($polys + 0x18 + $i, 0);
        }
        $this->initUint32($polys + 0x18 + 0x00, 3);
        $this->initUint32($polys + 0x18 + 0x04, $idsB);
        $this->initUint32($polys + 0x18 + 0x14, 5);

        $cellPolyIds = $this->allocIntArray([0, 1]); // poly index 0 (A) then 1 (B)
        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(2, $cellPolyIds);
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $polys);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        // Previously tracking A.
        $slotA = $this->allocIntArray([0]);
        $out = $this->allocOut($slotA, $idsA, 1);

        $this->call('_GroundProbeTrackPolygon_8c020b6c')->with(0.0, 0.0, 0.0, $out);

        // Recheck of A: n=1, prev==cur vertex, dot=25, cross=0 -> acc stays
        // 0 -> reject. Only these three calls happen for A, ever (the
        // fallback search must not repeat them).
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_acosf')->with(1.0)->andReturn(0.0);

        // Fallback search: i=0 is A again (same idsA pointer) -> skipped;
        // i=1 is B, convex, point (0,0,0) inside -> hit.
        $this->shouldWriteLong($out + 0x00, 5);
        $this->shouldWriteLong($out + 0x04, $cellPolyIds + 4); // &cellPolyIds[1]
        $this->shouldWriteLong($out + 0x08, $idsB);
        $this->shouldWriteLong($out + 0x0c, 3);
    }

    // ------------------------------------------------------------------
    // Tracked polygon no longer contains the point, and the full search
    // (same single-polygon cell) also misses -> overall miss.
    // ------------------------------------------------------------------
    public function test_recheckMissFallsBackAndOverallMisses(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [-10.0, 0.0, -10.0],
            [10.0, 0.0, -10.0],
            [0.0, 0.0, 10.0],
        ]);
        $ids = $this->allocIntArray([0, 1, 2]);
        $poly = $this->allocPoly(3, $ids, 5); // convex
        $polyIds = $this->allocIntArray([0]);

        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(1, $polyIds);
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $poly);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $slot = $this->allocIntArray([0]);
        $out = $this->allocOut($slot, $ids, 3);

        // Far outside the triangle -- recheck rejects, and the cell's only
        // polygon is the tracked one, so it's skipped in the fallback too:
        // no candidate left, overall miss.
        $this->call('_GroundProbeTrackPolygon_8c020b6c')->with(100.0, 0.0, 100.0, $out);

        $this->shouldMiss($out);
    }

    // ------------------------------------------------------------------
    // Tracked polygon is concave (attr < 0) and still contains the point:
    // recheck hits via the winding-angle sum, exercising the negative-cross
    // "acc -= angle" branch (iter0 below has cross < 0).
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

        $this->call('_GroundProbeTrackPolygon_8c020b6c')->with(0.0, 0.0, 0.0, $out);

        // Same mocked sequence as GroundQueryFindPolygon_8c020914's
        // concaveWindingAccepts: iter0 has cross < 0 (acc -= 0), iter1/2 have
        // cross > 0 and sum past 0x8000.
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
    // Tracked polygon's recheck (concave) drives cosT above 1.0 -- njSqrt
    // mocked far smaller than the real vector lengths -- so the clamp to
    // 1.0 fires before acosf. Rejects (acc stays 0), and the cell is empty
    // so the fallback also misses.
    // ------------------------------------------------------------------
    public function test_recheckConcaveClampsAboveOne(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [5.0, 0.0, 0.0],
        ]);
        $ids = $this->allocIntArray([0]); // n=1: prev == cur, dot == 25 (positive)
        $poly = $this->allocPoly(1, $ids, -1);

        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(0, null); // empty -- fallback must miss
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $poly);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $slot = $this->allocIntArray([0]);
        $out = $this->allocOut($slot, $ids, 1);

        $this->call('_GroundProbeTrackPolygon_8c020b6c')->with(0.0, 0.0, 0.0, $out);

        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_acosf')->with(1.0)->andReturn(0.0);

        $this->shouldMiss($out);
    }

    // ------------------------------------------------------------------
    // Same, but two opposite-facing vertices give a negative dot, driving
    // cosT below -1.0 -- clamp to -1.0.
    // ------------------------------------------------------------------
    public function test_recheckConcaveClampsBelowNegativeOne(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [5.0, 0.0, 0.0],
            [-5.0, 0.0, 0.0],
        ]);
        $ids = $this->allocIntArray([0, 1]);
        $poly = $this->allocPoly(2, $ids, -1);

        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(0, null);
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $poly);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $slot = $this->allocIntArray([0]);
        $out = $this->allocOut($slot, $ids, 2);

        $this->call('_GroundProbeTrackPolygon_8c020b6c')->with(0.0, 0.0, 0.0, $out);

        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_acosf')->with(-1.0)->andReturn(0.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_acosf')->with(-1.0)->andReturn(0.0);

        $this->shouldMiss($out);
    }

    // ------------------------------------------------------------------
    // Recheck rejects on an unrelated polygon; the fallback then exercises
    // its own convex-reject branch (loop runs to completion without a hit)
    // on a DIFFERENT polygon (different vertexIds pointer, so not skipped),
    // and the cell is selected via the same cx/cz upper clamp as
    // GroundQueryFindPolygon_8c020914's test_upperClampSelectsLastCell.
    // ------------------------------------------------------------------
    public function test_fallbackConvexRejectsWithClamp(): void {
        $this->resolveSymbols();

        // Shared vertex array: [0..2] the tracked polygon's triangle, [3..5]
        // the fallback candidate's -- same shape (near the origin), so
        // neither contains the query point far away at (1000, 1000), but a
        // distinct vertexIds pointer so the fallback candidate isn't skipped.
        $verts = $this->allocVertices([
            [-1.0, 0.0, -1.0],
            [1.0, 0.0, -1.0],
            [0.0, 0.0, 1.0],
            [-1.0, 0.0, -1.0],
            [1.0, 0.0, -1.0],
            [0.0, 0.0, 1.0],
        ]);
        $prevIds = $this->allocIntArray([0, 1, 2]);
        $ids = $this->allocIntArray([3, 4, 5]); // different pointer than $prevIds -- not skipped

        // polys[0] = tracked polygon, polys[1] = fallback candidate.
        $polys = $this->alloc(2 * 0x18);
        for ($i = 0; $i < 2 * 0x18; $i++) {
            $this->initUint8($polys + $i, 0);
        }
        $this->initUint32($polys + 0x00, 3);
        $this->initUint32($polys + 0x04, $prevIds);
        $this->initUint32($polys + 0x14, 5);
        $this->initUint32($polys + 0x18 + 0x00, 3);
        $this->initUint32($polys + 0x18 + 0x04, $ids);
        $this->initUint32($polys + 0x18 + 0x14, 5);

        $prevSlot = $this->allocIntArray([0]); // tracked polygon is poly index 0
        $fallbackPolyIds = $this->allocIntArray([1]); // fallback cell lists poly index 1

        $grid = $this->allocGrid(2, 2);
        $cells = $this->alloc(4 * 8);
        for ($i = 0; $i < 3; $i++) {
            $this->initUint32($cells + $i * 8 + 0, 0);
            $this->initUint32($cells + $i * 8 + 4, 0);
        }
        $this->initUint32($cells + 3 * 8 + 0, 1);
        $this->initUint32($cells + 3 * 8 + 4, $fallbackPolyIds);
        $this->initUint32($grid + 0x10, $cells);
        $this->initUint32($grid + 0x14, $polys);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $out = $this->allocOut($prevSlot, $prevIds, 3);

        // x/150 = 1000/150 = 6 -> clamp to 1; z/150 likewise -> cell index 3
        // (cz=1, cx=1 of the 2x2 grid), same clamp as
        // GroundQueryFindPolygon_8c020914's test_upperClampSelectsLastCell.
        $this->call('_GroundProbeTrackPolygon_8c020b6c')->with(1000.0, 0.0, 1000.0, $out);

        $this->shouldMiss($out);
    }

    // ------------------------------------------------------------------
    // Recheck rejects (convex, tracked polygon); the fallback candidate is
    // concave (attr < 0, different vertexIds pointer, so not skipped) and
    // hits -- exercises the fallback loop's own copy of the concave test.
    // ------------------------------------------------------------------
    public function test_fallbackConcaveHits(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [100.0, 0.0, 100.0], // tracked (convex) polygon's triangle -- far from the origin query point
            [102.0, 0.0, 100.0],
            [101.0, 0.0, 102.0],
            [5.0, 0.0, 0.0],      // fallback (concave) polygon's single vertex
        ]);
        $prevIds = $this->allocIntArray([0, 1, 2]);
        $ids = $this->allocIntArray([3]);

        $polys = $this->alloc(2 * 0x18);
        for ($i = 0; $i < 2 * 0x18; $i++) {
            $this->initUint8($polys + $i, 0);
        }
        $this->initUint32($polys + 0x00, 3);
        $this->initUint32($polys + 0x04, $prevIds);
        $this->initUint32($polys + 0x14, 5);
        $this->initUint32($polys + 0x18 + 0x00, 1);
        $this->initUint32($polys + 0x18 + 0x04, $ids);
        $this->initUint32($polys + 0x18 + 0x14, 0xffffffff); // -1, concave

        $prevSlot = $this->allocIntArray([0]);
        $cellPolyIds = $this->allocIntArray([1]);

        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(1, $cellPolyIds);
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $polys);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $out = $this->allocOut($prevSlot, $prevIds, 3);

        // Far outside the tracked triangle -- recheck rejects.
        $this->call('_GroundProbeTrackPolygon_8c020b6c')->with(0.0, 0.0, 0.0, $out);

        // Fallback candidate: n=1, prev==cur vertex, dot=25 -- same
        // deterministic shape as the recheck clamp tests, but acosf mocked
        // well past pi so the single-iteration angle alone clears 0x8000
        // (acosf's real range is unenforced since it's fully mocked here).
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldCall('_acosf')->with(1.0)->andReturn(4.0);

        $this->shouldWriteLong($out + 0x00, 0x7fffffff);
        $this->shouldWriteLong($out + 0x04, $cellPolyIds);
        $this->shouldWriteLong($out + 0x08, $ids);
        $this->shouldWriteLong($out + 0x0c, 1);
    }

    // ------------------------------------------------------------------
    // Recheck rejects (unrelated convex tracked polygon); the fallback
    // candidate is concave with a real triangle (not a degenerate single
    // vertex) and hits via the negative-cross "acc -= angle" branch --
    // exercises that branch in the fallback loop's own copy of the test.
    // ------------------------------------------------------------------
    public function test_fallbackConcaveNegativeCrossHits(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [100.0, 0.0, 100.0], // tracked (convex) polygon -- far from the origin query
            [102.0, 0.0, 100.0],
            [101.0, 0.0, 102.0],
            [5.0, 0.0, 0.0],     // fallback (concave) polygon's triangle
            [4.0, 0.0, 3.0],
            [3.0, 0.0, 4.0],
        ]);
        $prevIds = $this->allocIntArray([0, 1, 2]);
        $ids = $this->allocIntArray([3, 4, 5]);

        $polys = $this->alloc(2 * 0x18);
        for ($i = 0; $i < 2 * 0x18; $i++) {
            $this->initUint8($polys + $i, 0);
        }
        $this->initUint32($polys + 0x00, 3);
        $this->initUint32($polys + 0x04, $prevIds);
        $this->initUint32($polys + 0x14, 5);
        $this->initUint32($polys + 0x18 + 0x00, 3);
        $this->initUint32($polys + 0x18 + 0x04, $ids);
        $this->initUint32($polys + 0x18 + 0x14, 0xffffffff); // -1, concave

        $prevSlot = $this->allocIntArray([0]);
        $cellPolyIds = $this->allocIntArray([1]);

        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(1, $cellPolyIds);
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $polys);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $out = $this->allocOut($prevSlot, $prevIds, 3);

        $this->call('_GroundProbeTrackPolygon_8c020b6c')->with(0.0, 0.0, 0.0, $out);

        // Same mocked sequence as the recheck negative-cross test.
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
        $this->shouldWriteLong($out + 0x04, $cellPolyIds);
        $this->shouldWriteLong($out + 0x08, $ids);
        $this->shouldWriteLong($out + 0x0c, 3);
    }

    // ------------------------------------------------------------------
    // Recheck rejects; the fallback candidate's concave test drives cosT
    // above 1.0 (njSqrt mocked far smaller than the real length) -- clamp
    // to 1.0 fires in the fallback loop's own copy. Rejects; overall miss.
    // ------------------------------------------------------------------
    public function test_fallbackConcaveClampsAboveOne(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [100.0, 0.0, 100.0],
            [102.0, 0.0, 100.0],
            [101.0, 0.0, 102.0],
            [5.0, 0.0, 0.0],
        ]);
        $prevIds = $this->allocIntArray([0, 1, 2]);
        $ids = $this->allocIntArray([3]);

        $polys = $this->alloc(2 * 0x18);
        for ($i = 0; $i < 2 * 0x18; $i++) {
            $this->initUint8($polys + $i, 0);
        }
        $this->initUint32($polys + 0x00, 3);
        $this->initUint32($polys + 0x04, $prevIds);
        $this->initUint32($polys + 0x14, 5);
        $this->initUint32($polys + 0x18 + 0x00, 1);
        $this->initUint32($polys + 0x18 + 0x04, $ids);
        $this->initUint32($polys + 0x18 + 0x14, 0xffffffff);

        $prevSlot = $this->allocIntArray([0]);
        $cellPolyIds = $this->allocIntArray([1]);

        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(1, $cellPolyIds);
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $polys);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $out = $this->allocOut($prevSlot, $prevIds, 3);

        $this->call('_GroundProbeTrackPolygon_8c020b6c')->with(0.0, 0.0, 0.0, $out);

        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_acosf')->with(1.0)->andReturn(0.0);

        $this->shouldMiss($out);
    }

    // ------------------------------------------------------------------
    // Same, but the fallback candidate's two opposite-facing vertices give
    // a negative dot, driving cosT below -1.0 -- clamp to -1.0 fires in
    // the fallback loop's own copy. Rejects; overall miss.
    // ------------------------------------------------------------------
    public function test_fallbackConcaveClampsBelowNegativeOne(): void {
        $this->resolveSymbols();

        $verts = $this->allocVertices([
            [100.0, 0.0, 100.0],
            [102.0, 0.0, 100.0],
            [101.0, 0.0, 102.0],
            [5.0, 0.0, 0.0],
            [-5.0, 0.0, 0.0],
        ]);
        $prevIds = $this->allocIntArray([0, 1, 2]);
        $ids = $this->allocIntArray([3, 4]);

        $polys = $this->alloc(2 * 0x18);
        for ($i = 0; $i < 2 * 0x18; $i++) {
            $this->initUint8($polys + $i, 0);
        }
        $this->initUint32($polys + 0x00, 3);
        $this->initUint32($polys + 0x04, $prevIds);
        $this->initUint32($polys + 0x14, 5);
        $this->initUint32($polys + 0x18 + 0x00, 2);
        $this->initUint32($polys + 0x18 + 0x04, $ids);
        $this->initUint32($polys + 0x18 + 0x14, 0xffffffff);

        $prevSlot = $this->allocIntArray([0]);
        $cellPolyIds = $this->allocIntArray([1]);

        $grid = $this->allocGrid(1, 1);
        $cell = $this->allocCell(1, $cellPolyIds);
        $this->initUint32($grid + 0x10, $cell);
        $this->initUint32($grid + 0x14, $polys);
        $this->initUint32($grid + 0x18, $verts);
        $this->setActiveGrid($grid);

        $out = $this->allocOut($prevSlot, $prevIds, 3);

        $this->call('_GroundProbeTrackPolygon_8c020b6c')->with(0.0, 0.0, 0.0, $out);

        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_acosf')->with(-1.0)->andReturn(0.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(0.1);
        $this->shouldCall('_acosf')->with(-1.0)->andReturn(0.0);

        $this->shouldMiss($out);
    }
};
