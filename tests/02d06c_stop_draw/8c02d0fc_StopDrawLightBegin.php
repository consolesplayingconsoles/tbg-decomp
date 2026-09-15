<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_njCnkSetSimpleLight', 4);
        $this->setSize('_njCnkSetSimpleLightIntensity', 4);
        $this->setSize('_njCnkSetSimpleLightColor', 4);
        $this->setSize('_njControl3D', 4);
        $this->setSize('_njSetConstantAttr', 4);
        $this->setSize('_njSetConstantMaterial', 4);
        $this->setSize('_var_fadeLightDir0_8c2264d8', 0xc);
        $this->setSize('_var_fadeLightIntensity_8c2264f0', 8);
        $this->setSize('_var_fadeLightColor_8c2264f8', 0xc);
        $this->setSize('_var_passengerFadeColor_8c228960', 0x14);
    }

    public function test_sets_up_light_and_material(): void
    {
        $this->resolveSymbols();

        $dir = $this->addressOf('_var_fadeLightDir0_8c2264d8');
        $this->initUint32($dir + 0x0, $this->f(1.0));
        $this->initUint32($dir + 0x4, $this->f(2.0));
        $this->initUint32($dir + 0x8, $this->f(3.0));

        $inten = $this->addressOf('_var_fadeLightIntensity_8c2264f0');
        $this->rellocate('_var_8c2264f4', $inten + 4); // coincides with fadeLightIntensity[1]
        $this->initUint32($inten + 0x0, $this->f(0.5));
        $this->initUint32($inten + 0x4, $this->f(0.25));

        $col = $this->addressOf('_var_fadeLightColor_8c2264f8');
        $this->initUint32($col + 0x0, $this->f(0.1));
        $this->initUint32($col + 0x4, $this->f(0.2));
        $this->initUint32($col + 0x8, $this->f(0.3));

        $mat = $this->addressOf('_var_passengerFadeColor_8c228960');
        $this->initUint32($mat + 0x0, $this->f(9.0)); // not read by this fn
        $this->initUint32($mat + 0x4, $this->f(9.0));
        $this->initUint32($mat + 0x8, $this->f(9.0));
        $this->initUint32($mat + 0xc, $this->f(9.0));
        $this->initUint32($mat + 0x10, $this->f(9.0));

        $this->call('_StopDrawLightBegin_8c02d0fc')->with(0);

        $this->shouldCall('_njCnkSetSimpleLight')->with(1.0, 2.0, 3.0);
        $this->shouldCall('_njCnkSetSimpleLightIntensity')->with(0.5, 0.25);
        $this->shouldCall('_njCnkSetSimpleLightColor')->with(0.1, 0.2, 0.3);
        $this->shouldCall('_njControl3D')->with(0x120);
        $this->shouldCall('_njSetConstantAttr')->with(0xffffffff, 0x100000);
        $this->shouldCall('_njSetConstantMaterial')->with($mat);
    }

    public function test_ignores_its_arg(): void
    {
        $this->resolveSymbols();

        $dir = $this->addressOf('_var_fadeLightDir0_8c2264d8');
        $this->initUint32($dir + 0x0, $this->f(1.0));
        $this->initUint32($dir + 0x4, $this->f(1.0));
        $this->initUint32($dir + 0x8, $this->f(1.0));

        $inten = $this->addressOf('_var_fadeLightIntensity_8c2264f0');
        $this->rellocate('_var_8c2264f4', $inten + 4); // coincides with fadeLightIntensity[1]
        $this->initUint32($inten + 0x0, $this->f(1.0));
        $this->initUint32($inten + 0x4, $this->f(1.0));

        $col = $this->addressOf('_var_fadeLightColor_8c2264f8');
        $this->initUint32($col + 0x0, $this->f(1.0));
        $this->initUint32($col + 0x4, $this->f(1.0));
        $this->initUint32($col + 0x8, $this->f(1.0));

        $mat = $this->addressOf('_var_passengerFadeColor_8c228960');
        $this->initUint32($mat + 0x0, 0);
        $this->initUint32($mat + 0x4, 0);
        $this->initUint32($mat + 0x8, 0);
        $this->initUint32($mat + 0xc, 0);
        $this->initUint32($mat + 0x10, 0);

        $this->call('_StopDrawLightBegin_8c02d0fc')->with(42);

        $this->shouldCall('_njCnkSetSimpleLight')->with(1.0, 1.0, 1.0);
        $this->shouldCall('_njCnkSetSimpleLightIntensity')->with(1.0, 1.0);
        $this->shouldCall('_njCnkSetSimpleLightColor')->with(1.0, 1.0, 1.0);
        $this->shouldCall('_njControl3D')->with(0x120);
        $this->shouldCall('_njSetConstantAttr')->with(0xffffffff, 0x100000);
        $this->shouldCall('_njSetConstantMaterial')->with($mat);
    }

    /** Raw uint32 bits of a float, for initUint32. */
    private function f(float $v): int
    {
        return unpack('L', pack('f', $v))[1];
    }
};
