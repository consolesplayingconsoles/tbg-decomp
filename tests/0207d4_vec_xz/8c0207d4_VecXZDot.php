<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Dot of (a - origin) and (b - origin) in the XZ plane. origin and a are
// full x/y/z points, b a packed x/z pair.
return new class extends TestCase {
    private function initFloat(int $address, float $value): void
    {
        $raw = unpack('L', pack('f', $value))[1];
        $this->initUint32($address, $raw);
    }

    // A full x/y/z point (12 bytes).
    private function initPoint3(int $base, float $x, float $y, float $z): void
    {
        $this->initFloat($base + 0, $x);
        $this->initFloat($base + 4, $y);
        $this->initFloat($base + 8, $z);
    }

    // A packed x/z pair (8 bytes).
    private function initPointXZ(int $base, float $x, float $z): void
    {
        $this->initFloat($base + 0, $x);
        $this->initFloat($base + 4, $z);
    }

    public function test_positiveWhileBIsOnAsSideOfOrigin(): void
    {
        $origin = $this->alloc(12);
        $a = $this->alloc(12);
        $b = $this->alloc(8);

        $this->initPoint3($origin, 2.0, 3.0, 4.0);
        $this->initPoint3($a, 5.0, 6.0, 7.0);
        $this->initPointXZ($b, 8.0, 9.0);

        $this->call('_VecXZDot_8c0207d4')->with($origin, $a, $b);

        // (7-4)*(9-4) + (8-2)*(5-2)
        $this->shouldReturn(33.0);
    }

    public function test_negativeWhenBIsBehindOrigin(): void
    {
        $origin = $this->alloc(12);
        $a = $this->alloc(12);
        $b = $this->alloc(8);

        // a is 10 ahead of origin on +z, b is 5 behind it.
        $this->initPoint3($origin, 0.0, 1.0, 0.0);
        $this->initPoint3($a, 0.0, 1.0, 10.0);
        $this->initPointXZ($b, 0.0, -5.0);

        $this->call('_VecXZDot_8c0207d4')->with($origin, $a, $b);

        $this->shouldReturn(-50.0);
    }
};
