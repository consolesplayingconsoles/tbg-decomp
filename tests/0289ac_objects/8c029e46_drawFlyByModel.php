<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function f(float $value): int
    {
        return unpack('L', pack('f', $value))[1];
    }

    private function initFloat(int $address, float $value): void
    {
        $this->initUint32($address, $this->f($value));
    }

    private function resolveSymbols(): void {
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njCnkSimpleDrawMotion', 4);
    }

    public function test_draws_texture_and_motion_at_current_frame(): void {
        $this->resolveSymbols();

        $state = $this->alloc(0x60);
        $texlist = $this->alloc(4);
        $model = $this->alloc(4);
        $motion = $this->alloc(4);
        $this->initUint32($state + 0x40, $texlist);
        $this->initUint32($state + 0x44, $model);
        $this->initUint32($state + 0x48, $motion);
        $this->initFloat($state + 0x58, 3.5);

        $this->call('_drawFlyByModel_8c029e46')->with($state);

        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njCnkSimpleDrawMotion')->with($model, $motion, 3.5);
    }
};
