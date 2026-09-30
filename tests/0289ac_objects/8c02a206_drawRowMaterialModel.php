<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_njControl3D', 4);
        $this->setSize('_njSetConstantAttr', 4);
        $this->setSize('_njSetConstantMaterial', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njCnkSimpleDrawObject', 4);
    }

    private function initFloat(int $addr, float $value): void
    {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    public function test_draws_with_constant_material(): void {
        $this->resolveSymbols();

        $state = $this->alloc(0x7c);
        $texlist = $this->alloc(4);
        $model = $this->alloc(4);
        $this->initUint32($state + 0x40, $texlist);
        $this->initUint32($state + 0x44, $model);
        $this->initFloat($state + 0x68, -0.25);

        $this->call('_drawRowMaterialModel_8c02a206')->with($state);

        $this->shouldCall('_njControl3D')->with(0x920);
        $this->shouldCall('_njSetConstantAttr')->with(0xffffffff, 0x800);
        $this->shouldCall('_njSetConstantMaterial')->with($state + 0x68);
        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($model);
        $this->shouldCall('_njControl3D')->with(0x100);
    }
};
