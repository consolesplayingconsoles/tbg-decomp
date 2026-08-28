<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_njControl3D', 4);
        $this->setSize('_njFogDisable', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njCnkSimpleDrawObject', 4);
        $this->setSize('_njFogEnable', 4);
    }

    public function test_param2_zero_disables_fog(): void {
        $this->resolveSymbols();

        $obj = $this->alloc(0x48);
        $texlist = $this->alloc(4);
        $model = $this->alloc(4);
        $this->initUint32($obj + 0x40, $texlist);
        $this->initUint32($obj + 0x44, $model);

        $this->call('_drawRowSimpleModel_8c02a1b2')->with($obj, 0);

        $this->shouldCall('_njControl3D')->with(0);
        $this->shouldCall('_njFogDisable');
        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($model);
        $this->shouldCall('_njFogEnable');
        $this->shouldCall('_njControl3D')->with(0x100);
    }

    public function test_param2_nonzero_skips_fog_disable(): void {
        $this->resolveSymbols();

        $obj = $this->alloc(0x48);
        $texlist = $this->alloc(4);
        $model = $this->alloc(4);
        $this->initUint32($obj + 0x40, $texlist);
        $this->initUint32($obj + 0x44, $model);

        $this->call('_drawRowSimpleModel_8c02a1b2')->with($obj, 1);

        $this->shouldCall('_njControl3D')->with(0);
        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($model);
        $this->shouldCall('_njFogEnable');
        $this->shouldCall('_njControl3D')->with(0x100);
    }
};
