<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_8c2285c4', 8 * 4);
        $this->setSize('__divls', 4);
    }

    public function test_large_remaining_divides_by_5(): void
    {
        $this->resolveSymbols();
        $this->onCall('__divls', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });

        $base = $this->addressOf('_var_8c2285c4');
        $this->initUint32($base + 5 * 4, 100); // idx5
        $this->initUint32($base + 6 * 4, 0);   // idx6, remaining = 100

        $this->call('_FUN_8c02d5d8');

        $this->shouldCall('__divls');
        $this->shouldWriteLong($base + 7 * 4, 20);
    }

    public function test_small_remaining_clamps_to_10(): void
    {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_8c2285c4');
        $this->initUint32($base + 5 * 4, 30); // idx5
        $this->initUint32($base + 6 * 4, 5);  // idx6, remaining = 25

        $this->call('_FUN_8c02d5d8');

        $this->shouldWriteLong($base + 7 * 4, 0xa);
    }

    public function test_boundary_at_50_divides_by_5(): void
    {
        $this->resolveSymbols();
        $this->onCall('__divls', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });

        $base = $this->addressOf('_var_8c2285c4');
        $this->initUint32($base + 5 * 4, 50); // idx5
        $this->initUint32($base + 6 * 4, 0);  // idx6, remaining = 50

        $this->call('_FUN_8c02d5d8');

        $this->shouldCall('__divls');
        $this->shouldWriteLong($base + 7 * 4, 10);
    }
};
