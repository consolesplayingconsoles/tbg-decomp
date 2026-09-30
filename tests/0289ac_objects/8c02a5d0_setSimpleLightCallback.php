<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    public function test_layer_0_uses_first_light_direction()
    {
        $this->setSize('_njCnkSetSimpleLight', 4);
        $this->setSize('_njCnkSetSimpleLightIntensity', 4);
        $this->setSize('_njCnkSetSimpleLightColor', 4);

        $this->setSize('_var_simpleLightDir_8c2264d8', 0xc);
        $this->initUint32($this->addressOf('_var_simpleLightDir_8c2264d8') + 0, fdec(-1.4));
        $this->initUint32($this->addressOf('_var_simpleLightDir_8c2264d8') + 4, fdec(1.0));
        $this->initUint32($this->addressOf('_var_simpleLightDir_8c2264d8') + 8, fdec(-0.2));

        $this->setSize('_var_mirrorSimpleLightDir_8c2264e4', 0xc);
        $this->initUint32($this->addressOf('_var_mirrorSimpleLightDir_8c2264e4') + 0, fdec(0.1));
        $this->initUint32($this->addressOf('_var_mirrorSimpleLightDir_8c2264e4') + 4, fdec(1.3));
        $this->initUint32($this->addressOf('_var_mirrorSimpleLightDir_8c2264e4') + 8, fdec(-1.2));

        $this->setSize('_var_simpleLightIntensity_8c2264f0', 8);
        $this->initUint32($this->addressOf('_var_simpleLightIntensity_8c2264f0') + 0, fdec(1.0));
        $this->initUint32($this->addressOf('_var_simpleLightIntensity_8c2264f0') + 4, fdec(0.65));

        $this->setSize('_var_simpleLightColor_8c2264f8', 0xc);
        $this->initUint32($this->addressOf('_var_simpleLightColor_8c2264f8') + 0, fdec(1.0));
        $this->initUint32($this->addressOf('_var_simpleLightColor_8c2264f8') + 4, fdec(1.0));
        $this->initUint32($this->addressOf('_var_simpleLightColor_8c2264f8') + 8, fdec(1.0));

        $this->call('_setSimpleLightCallback_8c02a5d0')->with(0);

        $this->shouldCall('_njCnkSetSimpleLight')->with(-1.4, 1.0, -0.2);
        $this->shouldCall('_njCnkSetSimpleLightIntensity')->with(1.0, 0.65);
        $this->shouldCall('_njCnkSetSimpleLightColor')->with(1.0, 1.0, 1.0);
    }

    public function test_nonzero_layer_uses_second_light_direction()
    {
        $this->setSize('_njCnkSetSimpleLight', 4);
        $this->setSize('_njCnkSetSimpleLightIntensity', 4);
        $this->setSize('_njCnkSetSimpleLightColor', 4);

        $this->setSize('_var_simpleLightDir_8c2264d8', 0xc);
        $this->initUint32($this->addressOf('_var_simpleLightDir_8c2264d8') + 0, fdec(-1.4));
        $this->initUint32($this->addressOf('_var_simpleLightDir_8c2264d8') + 4, fdec(1.0));
        $this->initUint32($this->addressOf('_var_simpleLightDir_8c2264d8') + 8, fdec(-0.2));

        $this->setSize('_var_mirrorSimpleLightDir_8c2264e4', 0xc);
        $this->initUint32($this->addressOf('_var_mirrorSimpleLightDir_8c2264e4') + 0, fdec(0.1));
        $this->initUint32($this->addressOf('_var_mirrorSimpleLightDir_8c2264e4') + 4, fdec(1.3));
        $this->initUint32($this->addressOf('_var_mirrorSimpleLightDir_8c2264e4') + 8, fdec(-1.2));

        $this->setSize('_var_simpleLightIntensity_8c2264f0', 8);
        $this->initUint32($this->addressOf('_var_simpleLightIntensity_8c2264f0') + 0, fdec(1.0));
        $this->initUint32($this->addressOf('_var_simpleLightIntensity_8c2264f0') + 4, fdec(0.65));

        $this->setSize('_var_simpleLightColor_8c2264f8', 0xc);
        $this->initUint32($this->addressOf('_var_simpleLightColor_8c2264f8') + 0, fdec(1.0));
        $this->initUint32($this->addressOf('_var_simpleLightColor_8c2264f8') + 4, fdec(1.0));
        $this->initUint32($this->addressOf('_var_simpleLightColor_8c2264f8') + 8, fdec(1.0));

        $this->call('_setSimpleLightCallback_8c02a5d0')->with(1);

        $this->shouldCall('_njCnkSetSimpleLight')->with(0.1, 1.3, -1.2);
        $this->shouldCall('_njCnkSetSimpleLightIntensity')->with(1.0, 0.65);
        $this->shouldCall('_njCnkSetSimpleLightColor')->with(1.0, 1.0, 1.0);
    }
};
