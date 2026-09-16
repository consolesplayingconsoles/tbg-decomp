<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// TRUE if either of two convex quads (4 NJS_POINT3s each, x/z only) holds
// a vertex of the other, after a one-sided height cull on vertex 1.
return new class extends TestCase {
    private function initFloat(int $address, float $value): void
    {
        $raw = unpack('L', pack('f', $value))[1];
        $this->initUint32($address, $raw);
    }

    // Writes 4 (x, y, z) points (12 bytes each) starting at $base.
    private function initQuad(int $base, array $points, float $y): void
    {
        foreach ($points as $i => [$x, $z]) {
            $this->initFloat($base + $i * 12 + 0, $x);
            $this->initFloat($base + $i * 12 + 4, $y);
            $this->initFloat($base + $i * 12 + 8, $z);
        }
    }

    public function test_yGateRejectsFarApartQuads(): void
    {
        $a = $this->alloc(4 * 12);
        $b = $this->alloc(4 * 12);

        // a[1].y - b[1].y = 110 - 0 = 110 > 5.0 -> gate rejects before any
        // real overlap test (same x/z footprint, so a positive result here
        // would otherwise be certain).
        $this->initQuad($a, [[-1, -1], [1, -1], [1, 1], [-1, 1]], 110.0);
        $this->initQuad($b, [[-1, -1], [1, -1], [1, 1], [-1, 1]], 0.0);

        $this->call('_GeomQuadOverlap_8c020842')->with($a, $b);

        $this->shouldReturn(0);
    }

    public function test_identicalQuadsOverlap(): void
    {
        $a = $this->alloc(4 * 12);
        $b = $this->alloc(4 * 12);

        $this->initQuad($a, [[-1, -1], [1, -1], [1, 1], [-1, 1]], 0.0);
        $this->initQuad($b, [[-1, -1], [1, -1], [1, 1], [-1, 1]], 0.0);

        $this->call('_GeomQuadOverlap_8c020842')->with($a, $b);

        $this->shouldReturn(1);
    }

    public function test_farApartQuadsWithinYGateDoNotOverlap(): void
    {
        $a = $this->alloc(4 * 12);
        $b = $this->alloc(4 * 12);

        $this->initQuad($a, [[-1, -1], [1, -1], [1, 1], [-1, 1]], 0.0);
        $this->initQuad($b, [[100, 100], [102, 100], [102, 102], [100, 102]], 0.0);

        $this->call('_GeomQuadOverlap_8c020842')->with($a, $b);

        $this->shouldReturn(0);
    }
};
