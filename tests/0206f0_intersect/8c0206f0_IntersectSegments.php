<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// IntersectSegments_8c0206f0: 2D segment-segment intersection of (a0,a1) vs
// (b0,b1), each an {x,y} float pair. Fits a line through each segment
// (y = m*x + b, special-cased for a vertical segment where m is undefined),
// solves for the shared x, then accepts the hit only if that x falls within
// both segments' x-extents (inclusive). Returns 1 and writes {x,y} to out on
// a hit, 0 otherwise.

return new class extends TestCase {
    private function initFloat(int $addr, float $value): void {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    private function point(float $x, float $y): int {
        $addr = $this->alloc(2 * 4);
        $this->initFloat($addr, $x);
        $this->initFloat($addr + 4, $y);
        return $addr;
    }

    // Both segments vertical: degenerate, no slope for either -> reject
    // before any bounds check or write.
    public function test_bothVertical(): void {
        $a0 = $this->point(5.0, 0.0);
        $a1 = $this->point(5.0, 9.0);
        $b0 = $this->point(2.0, -3.0);
        $b1 = $this->point(2.0, 3.0);
        $out = $this->point(0.0, 0.0);

        $this->call('_IntersectSegments_8c0206f0')->with($a0, $a1, $b0, $b1, $out);

        $this->shouldReturn(0);
    }

    // a vertical, b horizontal: x is a0.x, y is b's line evaluated there.
    public function test_aVerticalHit(): void {
        $a0 = $this->point(5.0, 1.0);
        $a1 = $this->point(5.0, 9.0);
        $b0 = $this->point(0.0, 3.0);
        $b1 = $this->point(10.0, 3.0);
        $out = $this->point(0.0, 0.0);

        $this->call('_IntersectSegments_8c0206f0')->with($a0, $a1, $b0, $b1, $out);

        $this->shouldWriteFloat($out, 5.0);
        $this->shouldWriteFloat($out + 4, 3.0);
        $this->shouldReturn(1);
    }

    // a horizontal (non-vertical), b vertical: x is b0.x, y is a's line
    // evaluated there.
    public function test_bVerticalHit(): void {
        $a0 = $this->point(0.0, 0.0);
        $a1 = $this->point(10.0, 0.0);
        $b0 = $this->point(4.0, -5.0);
        $b1 = $this->point(4.0, 5.0);
        $out = $this->point(0.0, 0.0);

        $this->call('_IntersectSegments_8c0206f0')->with($a0, $a1, $b0, $b1, $out);

        $this->shouldWriteFloat($out, 4.0);
        $this->shouldWriteFloat($out + 4, 0.0);
        $this->shouldReturn(1);
    }

    // Neither vertical, same slope: parallel -> reject via the zero-denominator
    // special case, before any write.
    public function test_generalParallel(): void {
        $a0 = $this->point(0.0, 0.0);
        $a1 = $this->point(2.0, 2.0);
        $b0 = $this->point(0.0, 1.0);
        $b1 = $this->point(2.0, 3.0);
        $out = $this->point(0.0, 0.0);

        $this->call('_IntersectSegments_8c0206f0')->with($a0, $a1, $b0, $b1, $out);

        $this->shouldReturn(0);
    }

    // Neither vertical, genuine crossing within both extents: this is the
    // general-case hit. The original game's compiled y computation reads
    // whatever float already sits at out[0] (normally stale data from a
    // previous call, since every caller reuses one scratch point) instead of
    // the x it just solved for -- reusing the output slot as if it still
    // held the earlier value. Preserve that: seed out[0] with a sentinel
    // and confirm y tracks the sentinel, not the true line-intersection y.
    public function test_generalHitUsesStaleOutForY(): void {
        $a0 = $this->point(0.0, 0.0);
        $a1 = $this->point(2.0, 4.0); // ma = 2, ba = 0
        $b0 = $this->point(0.0, 4.0);
        $b1 = $this->point(2.0, 0.0); // mb = -2, bb = 4; true x = 1, true y = 2
        $sentinel = 7.5;
        $out = $this->point($sentinel, 0.0);

        $this->call('_IntersectSegments_8c0206f0')->with($a0, $a1, $b0, $b1, $out);

        $this->shouldWriteFloat($out, 1.0);
        $this->shouldWriteFloat($out + 4, $sentinel * 2.0); // ma * sentinel + ba, not the real y
        $this->shouldReturn(1);
    }

    // General case, otherwise a hit, but the shared x falls outside segment
    // a's own x-extent -> reject.
    public function test_generalRejectOutsideA(): void {
        $a0 = $this->point(0.0, 0.0);
        $a1 = $this->point(0.5, 1.0); // ma = 2, ba = 0; extent [0, 0.5]
        $b0 = $this->point(0.0, 4.0);
        $b1 = $this->point(2.0, 0.0); // mb = -2, bb = 4; shared x = 1, outside a's extent
        $out = $this->point(0.0, 0.0);

        $this->call('_IntersectSegments_8c0206f0')->with($a0, $a1, $b0, $b1, $out);

        $this->shouldReturn(0);
    }

    // Segment a's endpoints given in descending x order: exercises the
    // a1.x < a0.x bounds-check branch, still a hit.
    public function test_generalHitDescendingA(): void {
        $a0 = $this->point(2.0, 4.0);
        $a1 = $this->point(0.0, 0.0); // ma = 2, ba = 0, extent still [0, 2]
        $b0 = $this->point(0.0, 4.0);
        $b1 = $this->point(2.0, 0.0); // mb = -2, bb = 4; x = 1
        $out = $this->point(1.0, 0.0); // sentinel = 1.0

        $this->call('_IntersectSegments_8c0206f0')->with($a0, $a1, $b0, $b1, $out);

        $this->shouldWriteFloat($out, 1.0);
        $this->shouldWriteFloat($out + 4, 2.0); // ma * sentinel + ba
        $this->shouldReturn(1);
    }

    // Segment b's endpoints given in descending x order: exercises the
    // b1.x < b0.x bounds-check branch, still a hit.
    public function test_generalHitDescendingB(): void {
        $a0 = $this->point(0.0, 0.0);
        $a1 = $this->point(2.0, 4.0); // ma = 2, ba = 0
        $b0 = $this->point(2.0, 0.0);
        $b1 = $this->point(0.0, 4.0); // mb = -2, bb = 4; x = 1
        $out = $this->point(1.0, 0.0); // sentinel = 1.0

        $this->call('_IntersectSegments_8c0206f0')->with($a0, $a1, $b0, $b1, $out);

        $this->shouldWriteFloat($out, 1.0);
        $this->shouldWriteFloat($out + 4, 2.0); // ma * sentinel + ba
        $this->shouldReturn(1);
    }

    // General case, x within segment a's extent but outside segment b's own
    // extent -> reject.
    public function test_generalRejectOutsideB(): void {
        $a0 = $this->point(0.0, 0.0);
        $a1 = $this->point(10.0, 10.0); // ma = 1, ba = 0; extent [0, 10]
        $b0 = $this->point(3.0, 5.0);
        $b1 = $this->point(3.5, 4.0); // mb = -2, bb = 11; shared x = 3.667, outside b's extent [3, 3.5]
        $out = $this->point(0.0, 0.0);

        $this->call('_IntersectSegments_8c0206f0')->with($a0, $a1, $b0, $b1, $out);

        $this->shouldReturn(0);
    }
};
