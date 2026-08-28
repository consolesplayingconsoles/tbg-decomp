<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

// GroundProbeInterpolateHeight_8c020f7e interpolates point[1] (y) from the
// polygon match in *result, using the plane equation
// nx*(x-x0) + ny*(y-y0) + nz*(z-z0) = 0 solved for y around the polygon's
// first vertex (x0,y0,z0). A vertical plane (ny == 0) can't be solved that
// way, so it just takes the first vertex's height outright. A miss
// (count_0x0c == 0) leaves point untouched.
return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_activeGroundGrid_8c2264d4', 4);
    }

    // GroundGrid layout: 0x14 polys, 0x18 verts (only fields this function reads).
    private function allocGrid(): int {
        $grid = $this->alloc(0x1c);
        for ($i = 0; $i < 0x1c; $i++) {
            $this->initUint8($grid + $i, 0);
        }
        return $grid;
    }

    // GroundPoly layout: 0x08/0x0c/0x10 are the plane normal (nx, ny, nz) here;
    // other fields unused by this function but zero-filled for safety.
    private function allocPoly(float $nx, float $ny, float $nz): int {
        $poly = $this->alloc(0x18);
        for ($i = 0; $i < 0x18; $i++) {
            $this->initUint8($poly + $i, 0);
        }
        $this->initUint32($poly + 0x08, fdec($nx));
        $this->initUint32($poly + 0x0c, fdec($ny));
        $this->initUint32($poly + 0x10, fdec($nz));
        return $poly;
    }

    private function allocVertex(float $x, float $y, float $z): int {
        $v = $this->alloc(12);
        $this->initUint32($v + 0, fdec($x));
        $this->initUint32($v + 4, fdec($y));
        $this->initUint32($v + 8, fdec($z));
        return $v;
    }

    private function allocIntArray(array $values): int {
        $addr = $this->alloc(4 * count($values));
        foreach ($values as $i => $v) {
            $this->initUint32($addr + $i * 4, $v);
        }
        return $addr;
    }

    // GroundQueryResult layout: 0x00 attr, 0x04 polyIdSlot, 0x08 vertexIds, 0x0c count.
    private function allocResult(int $polyIdSlot, int $vertexIds, int $count): int {
        $out = $this->alloc(0x10);
        $this->initUint32($out + 0x00, 0);
        $this->initUint32($out + 0x04, $polyIdSlot);
        $this->initUint32($out + 0x08, $vertexIds);
        $this->initUint32($out + 0x0c, $count);
        return $out;
    }

    private function allocPoint(float $x, float $y, float $z): int {
        $p = $this->alloc(12);
        $this->initUint32($p + 0, fdec($x));
        $this->initUint32($p + 4, fdec($y));
        $this->initUint32($p + 8, fdec($z));
        return $p;
    }

    private function setActiveGrid(int $grid): void {
        $this->initUint32($this->addressOf('_var_activeGroundGrid_8c2264d4'), $grid);
    }

    // ------------------------------------------------------------------
    // count_0x0c == 0: miss, point is left completely untouched. Grid's
    // polys/verts arrays are left null -- a wrong implementation that
    // dereferenced them anyway would fault or, worse, write to point.
    // ------------------------------------------------------------------
    public function test_missLeavesPointUntouched(): void {
        $this->resolveSymbols();

        $grid = $this->allocGrid();
        $this->setActiveGrid($grid);

        $polyIdSlot = $this->allocIntArray([0]);
        $vertexIds = $this->allocIntArray([0]);
        $result = $this->allocResult($polyIdSlot, $vertexIds, 0);

        $point = $this->allocPoint(3.0, 999.0, 6.0);

        $this->call('_GroundProbeInterpolateHeight_8c020f7e')->with($result, $point);

        // No shouldWriteFloat expectation for point+4 -- any write there fails.
    }

    // ------------------------------------------------------------------
    // Degenerate plane (ny == 0.0, e.g. a vertical wall face): can't solve
    // for y, so point[1] just becomes the first vertex's y outright.
    // ------------------------------------------------------------------
    public function test_verticalPlaneTakesFirstVertexHeight(): void {
        $this->resolveSymbols();

        $poly = $this->allocPoly(1.0, 0.0, 1.0);
        $v0 = $this->allocVertex(1.0, 5.0, 2.0);

        $polyIdSlot = $this->allocIntArray([0]); // *polyIdSlot -> poly index 0
        $vertexIds = $this->allocIntArray([0]);  // *vertexIds -> vertex index 0

        $grid = $this->allocGrid();
        $this->initUint32($grid + 0x14, $poly);
        $this->initUint32($grid + 0x18, $v0);
        $this->setActiveGrid($grid);

        $result = $this->allocResult($polyIdSlot, $vertexIds, 3);

        $point = $this->allocPoint(4.0, 999.0, 6.0);

        $this->call('_GroundProbeInterpolateHeight_8c020f7e')->with($result, $point);

        $this->shouldWriteFloat($point + 4, 5.0);
    }

    // ------------------------------------------------------------------
    // Real interpolation: solves the plane equation for y around the first
    // vertex. nx=1, ny=2, nz=3, v0=(1,5,2), point=(4,_,6):
    // y = (-(1*(4-1)) - 3*(6-2)) / 2 + 5 = (-3 - 12)/2 + 5 = -2.5
    // ------------------------------------------------------------------
    public function test_interpolatesFromPlaneEquation(): void {
        $this->resolveSymbols();

        $poly = $this->allocPoly(1.0, 2.0, 3.0);
        $v0 = $this->allocVertex(1.0, 5.0, 2.0);

        $polyIdSlot = $this->allocIntArray([0]);
        $vertexIds = $this->allocIntArray([0]);

        $grid = $this->allocGrid();
        $this->initUint32($grid + 0x14, $poly);
        $this->initUint32($grid + 0x18, $v0);
        $this->setActiveGrid($grid);

        $result = $this->allocResult($polyIdSlot, $vertexIds, 3);

        $point = $this->allocPoint(4.0, 999.0, 6.0);

        $this->call('_GroundProbeInterpolateHeight_8c020f7e')->with($result, $point);

        $this->shouldWriteFloat($point + 4, -2.5);
    }
};
