<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_clears_all_entries()
    {
        // The asm computes this loop's base as var_pedCrossingFlags_8c227e2c + 0x200 rather
        // than relocating var_crossingOccupiedFlags_8c22802c directly; pin the two consistently.
        $e2cBase = $this->addressOf('_var_pedCrossingFlags_8c227e2c');
        $this->rellocate('_var_crossingOccupiedFlags_8c22802c', $e2cBase + 0x200);
        $base = $this->addressOf('_var_crossingOccupiedFlags_8c22802c');
        for ($i = 0; $i < 64; $i++) {
            $this->initUint32($base + $i * 8, 0x11111111);
            $this->initUint32($base + $i * 8 + 4, 0x22222222);
        }

        $this->call('_SignalClearCrossingOccupied_8c028958');

        for ($i = 0; $i < 64; $i++) {
            $this->shouldWriteLong($base + $i * 8, 0);
            $this->shouldWriteLong($base + $i * 8 + 4, 0);
        }
    }
};
