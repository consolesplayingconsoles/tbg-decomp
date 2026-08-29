<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Distance between world point a (x at [0], z at [2], y unused) and 2D
// point b (x at [0], z at [1]) via njSqrt(dx*dx+dz*dz). A tail call: this
// function's return value IS njSqrt's return value.
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_njSqrt', 4);
    }

    private function initFloat(int $address, float $value): void
    {
        $raw = unpack('L', pack('f', $value))[1];
        $this->initUint32($address, $raw);
    }

    public function test_computesDistance(): void
    {
        $this->resolveSymbols();

        $a = $this->alloc(12);
        $b = $this->alloc(8);

        $this->initFloat($a + 0, 5.0);  // a.x
        $this->initFloat($a + 4, 99.0); // a.y (unused)
        $this->initFloat($a + 8, 9.0);  // a.z

        $this->initFloat($b + 0, 2.0); // b.x
        $this->initFloat($b + 4, 4.0); // b.z

        $this->call('_FUN_8c02081c')->with($a, $b);

        // dx = 5-2=3; dz = 9-4=5; njSqrt(3^2+5^2) = njSqrt(34)
        $this->shouldCall('_njSqrt')->with(34.0)->andReturn(5.830951895);
        $this->shouldReturn(5.830951895);
    }
};
