<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Cross (z component) of (a - origin) and (b - origin) in the XZ plane.
// origin is a full x/y/z point, a and b packed x/z pairs.
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

    public function test_signedAreaOfTheTwoDeltas(): void
    {
        $origin = $this->alloc(12);
        $a = $this->alloc(8);
        $b = $this->alloc(8);

        $this->initPoint3($origin, 2.0, 3.0, 4.0);
        $this->initPointXZ($a, 5.0, 6.0);
        $this->initPointXZ($b, 8.0, 9.0);

        $this->call('_VecXZCross_8c0207fa')->with($origin, $a, $b);

        // (5-2)*(9-4) - (6-4)*(8-2)
        $this->shouldReturn(3.0);
    }

    public function test_signFlipsWithBsSideOfTheOriginToALine(): void
    {
        $origin = $this->alloc(12);
        $a = $this->alloc(8);
        $b = $this->alloc(8);

        $this->initPoint3($origin, 10.0, 9.0, 8.0);
        $this->initPointXZ($a, 7.0, 6.0);
        $this->initPointXZ($b, 1.0, 5.0);

        $this->call('_VecXZCross_8c0207fa')->with($origin, $a, $b);

        // (7-10)*(5-8) - (6-8)*(1-10)
        $this->shouldReturn(-9.0);
    }
};
