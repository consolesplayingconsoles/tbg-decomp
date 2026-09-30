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
        $this->setSize('_njControl3D', 4);
        $this->setSize('_njFogDisable', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njCnkSimpleDrawMotion', 4);
        $this->setSize('_njFogEnable', 4);
    }

    private function allocState(int $flag78, int $flag79): array {
        $state = $this->alloc(0x7a);
        $texlist = $this->alloc(4);
        $model = $this->alloc(4);
        $motion = $this->alloc(4);
        $this->initUint32($state + 0x40, $texlist);
        $this->initUint32($state + 0x44, $model);
        $this->initUint32($state + 0x48, $motion);
        $this->initFloat($state + 0x58, 3.5);
        $this->initUint8($state + 0x78, $flag78);
        $this->initUint8($state + 0x79, $flag79);
        return [$state, $texlist, $model, $motion];
    }

    public function test_both_flags_clear_disables_control3d_and_fog(): void {
        $this->resolveSymbols();

        [$state, $texlist, $model, $motion] = $this->allocState(0, 0);

        $this->call('_drawRowMotionModel_8c02a0d6')->with($state);

        $this->shouldCall('_njControl3D')->with(0);
        $this->shouldCall('_njFogDisable');
        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njCnkSimpleDrawMotion')->with($model, $motion, 3.5);
        $this->shouldCall('_njFogEnable');
        $this->shouldCall('_njControl3D')->with(0x100);
    }

    public function test_flag79_set_skips_control3d_zero(): void {
        $this->resolveSymbols();

        [$state, $texlist, $model, $motion] = $this->allocState(0, 1);

        $this->call('_drawRowMotionModel_8c02a0d6')->with($state);

        $this->shouldCall('_njFogDisable');
        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njCnkSimpleDrawMotion')->with($model, $motion, 3.5);
        $this->shouldCall('_njFogEnable');
        $this->shouldCall('_njControl3D')->with(0x100);
    }

    public function test_flag78_set_skips_fog_disable(): void {
        $this->resolveSymbols();

        [$state, $texlist, $model, $motion] = $this->allocState(1, 0);

        $this->call('_drawRowMotionModel_8c02a0d6')->with($state);

        $this->shouldCall('_njControl3D')->with(0);
        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njCnkSimpleDrawMotion')->with($model, $motion, 3.5);
        $this->shouldCall('_njFogEnable');
        $this->shouldCall('_njControl3D')->with(0x100);
    }

    public function test_both_flags_set_skips_both_toggles(): void {
        $this->resolveSymbols();

        [$state, $texlist, $model, $motion] = $this->allocState(1, 1);

        $this->call('_drawRowMotionModel_8c02a0d6')->with($state);

        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njCnkSimpleDrawMotion')->with($model, $motion, 3.5);
        $this->shouldCall('_njFogEnable');
        $this->shouldCall('_njControl3D')->with(0x100);
    }
};
