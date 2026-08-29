<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

/*
 * FUN_8c021ec4(int width, int height): the mirror-render twin of
 * FUN_8c021b9c -- reachable only via the function-pointer literal
 * FUN_8c0221d0 pushes to FadeCmdPushCall2_8c022420(1, ...), never called
 * directly, so it stays private (exported under UNIT_TESTING only). Same
 * visible-window/Easy-Simple-Easy-Easy structure; its Simple-light block
 * reads var_fadeLightDir1_8c2264e4[0] plus the separately-exported aliases
 * var_8c2264e8/var_8c2264ec for elements 1/2.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_var_fogParam_8c226508', 4);
        $this->setSize('_var_fogParam_8c226504', 4);
        $this->setSize('_njControl3D', 4);
        $this->setSize('_var_tileLayerSlots_8c226520', 4 * 5);
        $this->setSize('_var_8c226544', 4 * 2);
        $this->setSize('_var_8c22654c', 4 * 3);
        $this->setSize('_njCnkSetEasyLight', 4);
        $this->setSize('_var_fadeLightDir1_8c2264e4', 4 * 3);
        $this->setSize('_var_8c2264e8', 4);
        $this->setSize('_var_8c2264ec', 4);
        $this->setSize('_var_fadeLightIntensity_8c2264f0', 4 * 2);
        $this->setSize('_var_fadeLightColor_8c2264f8', 4 * 3);
        $this->setSize('_var_fadeCamera_8c226558', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njSetCamera', 4);
        $this->setSize('_var_8c226538', 4 * 3);
        $this->setSize('_njCnkSetEasyLightIntensity', 4);
        $this->setSize('_njCnkSetEasyLightColor', 4);
        $this->setSize('_njCnkEasyDrawObject', 4);
        $this->setSize('_njCnkSetSimpleLight', 4);
        $this->setSize('_njCnkSetSimpleLightIntensity', 4);
        $this->setSize('_njCnkSetSimpleLightColor', 4);
        $this->setSize('_njCnkSimpleDrawObject', 4);
        $this->setSize('__divls', 4);
        // Signed division (dividend R1, divisor R0) -- UInt::div() is unsigned.
        $this->onCall('__divls', function () {
            $dividend = $this->getRegister(1)->signedValue();
            $divisor = $this->getRegister(0)->signedValue();
            $this->setRegister(0, new U32(intdiv($dividend, $divisor) & 0xffffffff));
        });
    }

    /** Raw uint32 bits of a float, for initUint32. */
    private function f(float $v): int
    {
        return unpack('L', pack('f', $v))[1];
    }

    private function seedVector(string $symbol, float $x, float $y, float $z): void
    {
        $base = $this->addressOf($symbol);
        $this->initUint32($base + 0, $this->f($x));
        $this->initUint32($base + 4, $this->f($y));
        $this->initUint32($base + 8, $this->f($z));
    }

    private function seedLightGlobals(): void
    {
        $this->seedVector('_var_8c226538', 1.0, 2.0, 3.0);
        $this->initUint32($this->addressOf('_var_8c226544') + 0, $this->f(4.0));
        $this->initUint32($this->addressOf('_var_8c226544') + 4, $this->f(5.0));
        $this->seedVector('_var_8c22654c', 6.0, 7.0, 8.0);
        $this->seedVector('_var_fadeLightDir1_8c2264e4', 9.0, 10.0, 11.0);
        $this->initUint32($this->addressOf('_var_fadeLightIntensity_8c2264f0') + 0, $this->f(12.0));
        $this->initUint32($this->addressOf('_var_fadeLightIntensity_8c2264f0') + 4, $this->f(13.0));
        $this->seedVector('_var_fadeLightColor_8c2264f8', 14.0, 15.0, 16.0);
        $this->initUint32($this->addressOf('_var_fadeCamera_8c226558'), 0x87654321);
    }

    private function assertEasyDraw(int $texlist, int $njDest): void
    {
        $camera = 0x87654321;
        $this->shouldCall('_njSetCamera')->with($camera);
        $this->shouldCall('_njCnkSetEasyLight')->with(1.0, 2.0, 3.0);
        $this->shouldCall('_njCnkSetEasyLightIntensity')->with(4.0, 5.0);
        $this->shouldCall('_njCnkSetEasyLightColor')->with(6.0, 7.0, 8.0);
        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njCnkEasyDrawObject')->with($njDest);
    }

    private function assertSimpleDraw(int $texlist, int $njDest): void
    {
        $camera = 0x87654321;
        $this->shouldCall('_njSetCamera')->with($camera);
        // dir1[0]=9.0 (array), dir1[1]/[2] via the separate var_8c2264e8/ec aliases.
        $this->shouldCall('_njCnkSetSimpleLight')->with(9.0, 10.0, 11.0);
        $this->shouldCall('_njCnkSetSimpleLightIntensity')->with(12.0, 13.0);
        $this->shouldCall('_njCnkSetSimpleLightColor')->with(14.0, 15.0, 16.0);
        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($njDest);
    }

    /** Allocates a LoadedModel {texlist, njDest} and returns its address. */
    private function allocSlot(int $texlist, int $njDest): int
    {
        $addr = $this->alloc(8);
        $this->initUint32($addr + 0, $texlist);
        $this->initUint32($addr + 4, $njDest);
        return $addr;
    }

    public function test_single_tile_draws_all_four_layers(): void
    {
        $this->resolveSymbols();
        $this->seedLightGlobals();

        // dir1 is var_fadeLightDir1_8c2264e4[0]; the aliases hold [1]/[2].
        $this->initUint32($this->addressOf('_var_8c2264e8'), $this->f(10.0));
        $this->initUint32($this->addressOf('_var_8c2264ec'), $this->f(11.0));

        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x2fc, $this->f(0.0));
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x304, $this->f(0.0));
        $this->initUint32($this->addressOf('_var_fogParam_8c226508'), 0);
        $this->initUint32($this->addressOf('_var_fogParam_8c226504'), 0);

        $slots = $this->addressOf('_var_tileLayerSlots_8c226520');
        $slot0 = $this->allocSlot(0x1000, 0x2000);
        $slot1 = $this->allocSlot(0x1001, 0x2001);
        $slot2 = $this->allocSlot(0x1002, 0x2002);
        $slot3 = $this->allocSlot(0x1003, 0x2003);
        $this->initUint32($slots + 0 * 4, $slot0);
        $this->initUint32($slots + 1 * 4, $slot1);
        $this->initUint32($slots + 2 * 4, $slot2);
        $this->initUint32($slots + 3 * 4, $slot3);

        $this->call('_FUN_8c021ec4')->with(1, 1);

        $this->shouldCall('__divls');
        $this->shouldCall('__divls');
        $this->shouldCall('_njControl3D')->with(0x2500);

        $this->assertEasyDraw(0x1000, 0x2000);
        $this->assertSimpleDraw(0x1001, 0x2001);
        $this->assertEasyDraw(0x1002, 0x2002);
        $this->assertEasyDraw(0x1003, 0x2003);

        $this->shouldCall('_njControl3D')->with(0x100);
    }

    public function test_null_slots_are_skipped(): void
    {
        $this->resolveSymbols();
        $this->seedLightGlobals();
        $this->initUint32($this->addressOf('_var_8c2264e8'), $this->f(10.0));
        $this->initUint32($this->addressOf('_var_8c2264ec'), $this->f(11.0));

        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x2fc, $this->f(0.0));
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x304, $this->f(0.0));
        $this->initUint32($this->addressOf('_var_fogParam_8c226508'), 0);
        $this->initUint32($this->addressOf('_var_fogParam_8c226504'), 0);

        $slots = $this->addressOf('_var_tileLayerSlots_8c226520');
        $slot0 = $this->allocSlot(0, 0);
        $slot1 = $this->allocSlot(0, 0);
        $slot2 = $this->allocSlot(0, 0);
        $slot3 = $this->allocSlot(0, 0);
        $this->initUint32($slots + 0 * 4, $slot0);
        $this->initUint32($slots + 1 * 4, $slot1);
        $this->initUint32($slots + 2 * 4, $slot2);
        $this->initUint32($slots + 3 * 4, $slot3);

        $this->call('_FUN_8c021ec4')->with(1, 1);

        $this->shouldCall('__divls');
        $this->shouldCall('__divls');
        $this->shouldCall('_njControl3D')->with(0x2500);
        $this->shouldCall('_njControl3D')->with(0x100);
    }
};
