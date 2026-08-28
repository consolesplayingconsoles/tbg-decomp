<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function floatBits(float $value): int {
        return unpack('L', pack('f', $value))[1];
    }

    public function test_draws_the_tile_at_the_bus_position(): void {
        $this->setSize('_njControl3D', 4);
        $this->setSize('_njFogDisable', 4);
        $this->setSize('_njTranslate', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njCnkSimpleDrawObject', 4);
        $this->setSize('_njFogEnable', 4);

        // BusState.posX_0x2fc/posY_0x300/posZ_0x304 at offset 0x2fc.
        $this->setSize('_var_busState_8c1bb9d0', 0x2fc + 12);
        $busBase = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($busBase + 0x2fc, $this->floatBits(100.0));
        $this->initUint32($busBase + 0x300, $this->floatBits(2.0));
        $this->initUint32($busBase + 0x304, $this->floatBits(-50.0));

        $slot = $this->alloc(8);
        $texlist = $this->alloc(4);
        $model = $this->alloc(4);
        $this->initUint32($slot + 0, $texlist);
        $this->initUint32($slot + 4, $model);

        $this->call('_TileStreamDrawTile_8c021b34')->with($slot);

        $this->shouldCall('_njControl3D')->with(0);
        $this->shouldCall('_njFogDisable');
        $this->shouldCall('_njTranslate')->with(0, 100.0, 2.0, -50.0);
        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($model);
        $this->shouldCall('_njControl3D')->with(0x100);
        $this->shouldCall('_njFogEnable');
    }
};
