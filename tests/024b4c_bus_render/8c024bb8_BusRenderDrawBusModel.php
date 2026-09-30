<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _BusRenderDrawBusModel_8c024bb8(void *altLight): lights, textures and draws the
 * third-person bus model with its door/etc shape motion. altLight only
 * selects the light direction (non-NULL -> var_mirrorLightDir_8c227dc4, NULL ->
 * var_busSimpleLightDir_8c227db8). The drawn object is always
 * busState.modelLarge_0x00c; only the animation frame depends on the special
 * bus substate (doorState_0x3c0 != 0 -> var_busDoorFrame_8c227db0, else frame 0).
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_var_busDoorMotion_8c1bc410', 4);
        $this->setSize('_var_busDoorShape_8c1bc414', 4);
        $this->setSize('_BusDrawUpdateModels_8c027958', 4);
        $this->setSize('_njCnkSetSimpleLight', 4);
        $this->setSize('_njCnkSetSimpleLightIntensity', 4);
        $this->setSize('_njCnkSetSimpleLightColor', 4);
        $this->setSize('_njMultiMatrix', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njCnkSimpleDrawShapeMotion', 4);
        $this->setSize('_njControl3D', 4);
        $this->setSize('_njCnkModDrawObject', 4);
    }

    private function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }

    private function initFloat(int $address, float $value): void
    {
        $this->initUint32($address, unpack('L', pack('f', $value))[1]);
    }

    private function seedCommonBusState(int $base): void
    {
        $this->initFloat($base + 0xc4, 1.5);
        $this->initFloat($base + 0xc8, 2.5);
        $this->initFloat($base + 0xcc, 3.5);
        $this->initFloat($base + 0xd0, 4.5);
        $this->initFloat($base + 0xd4, 5.5);
        $this->initUint32($base + 0x4, 0x11223344);
        $this->initUint32($base + 0xc, 0x77665544);
        $this->initUint32($base + 0x14, 0xaabbccdd);
        $this->initUint32($this->addressOf('_var_busDoorMotion_8c1bc410'), 0x22334455);
        $this->initUint32($this->addressOf('_var_busDoorShape_8c1bc414'), 0x33445566);
    }

    public function test_null_light_normal_substate_draws_frame_zero(): void
    {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($base + 0x3c0, 0);
        $this->seedCommonBusState($base);

        $lightDir = $this->addressOf('_var_busSimpleLightDir_8c227db8');
        $this->initFloat($lightDir + 0x0, 0.1);
        $this->initFloat($lightDir + 0x4, 0.2);
        $this->initFloat($lightDir + 0x8, 0.3);

        $this->call('_BusRenderDrawBusModel_8c024bb8')->with(0);

        $this->shouldCall('_njCnkSetSimpleLight')->with($this->f32(0.1), $this->f32(0.2), $this->f32(0.3));
        $this->shouldCall('_njCnkSetSimpleLightIntensity')->with(1.5, 2.5);
        $this->shouldCall('_njCnkSetSimpleLightColor')->with(3.5, 4.5, 5.5);
        $this->shouldCall('_BusDrawUpdateModels_8c027958')->with($base);
        $this->shouldCall('_njMultiMatrix')->with(0, $base + 0x84);
        $this->shouldCall('_njSetTexture')->with(0x11223344);
        $this->shouldCall('_njCnkSimpleDrawShapeMotion')->with(0x77665544, 0x22334455, 0x33445566, 0.0);
        $this->shouldCall('_njControl3D')->with(0x2500);
        $this->shouldCall('_njCnkModDrawObject')->with(0xaabbccdd);
        $this->shouldCall('_njControl3D')->with(0x100);
    }

    public function test_nonnull_light_special_substate_draws_current_frame(): void
    {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($base + 0x3c0, 1);
        $this->seedCommonBusState($base);
        $this->initFloat($this->addressOf('_var_busDoorFrame_8c227db0'), 12.5);

        $altDir = $this->addressOf('_var_mirrorLightDir_8c227dc4');
        $this->initFloat($altDir + 0x0, 0.4);
        $this->initFloat($altDir + 0x4, 0.5);
        $this->initFloat($altDir + 0x8, 0.6);

        $this->call('_BusRenderDrawBusModel_8c024bb8')->with(0x99887766);

        $this->shouldCall('_njCnkSetSimpleLight')->with($this->f32(0.4), $this->f32(0.5), $this->f32(0.6));
        $this->shouldCall('_njCnkSetSimpleLightIntensity')->with(1.5, 2.5);
        $this->shouldCall('_njCnkSetSimpleLightColor')->with(3.5, 4.5, 5.5);
        $this->shouldCall('_BusDrawUpdateModels_8c027958')->with($base);
        $this->shouldCall('_njMultiMatrix')->with(0, $base + 0x84);
        $this->shouldCall('_njSetTexture')->with(0x11223344);
        $this->shouldCall('_njCnkSimpleDrawShapeMotion')->with(0x77665544, 0x22334455, 0x33445566, 12.5);
        $this->shouldCall('_njControl3D')->with(0x2500);
        $this->shouldCall('_njCnkModDrawObject')->with(0xaabbccdd);
        $this->shouldCall('_njControl3D')->with(0x100);
    }
};
