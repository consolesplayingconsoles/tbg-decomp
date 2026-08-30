<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value): int
    {
        return unpack('L', pack('f', $value))[1];
    }
}

/*
 * busDrawSimpleCb_8c027a88(TrafficEntry *entity, int lod): FadeCmdPushCall2
 * near-draw callback registered by BusDrawPlaceEntity_8c027c3c. lod == 0 draws the
 * detailed near model (texlistLarge_0x04/modelLarge_0x0c plus bodyModel_0x14
 * bracketed by njControl3D) and also refreshes blinker lights via
 * BusDrawUpdateModels_8c027958; lod != 0 draws texlistSmall_0x08/modelSmall_0x10 only, with
 * simple light at night (var_timeOfDay_8c18ad20 == 2) or easy light by day.
 */
return new class extends TestCase {
    /** Allocates an entity with its texlist/model pointer fields and both
     * light rows wired to known scratch targets/values. */
    private function makeEntity(): array
    {
        $entity = $this->alloc(0x514);
        $this->doNotRandomizeMemory();

        $texlistLarge = $this->alloc(4);
        $texlistSmall = $this->alloc(4);
        $modelLarge = $this->alloc(4);
        $modelSmall = $this->alloc(4);
        $bodyModel = $this->alloc(4);

        $this->initUint32($entity + 0x04, $texlistLarge);
        $this->initUint32($entity + 0x08, $texlistSmall);
        $this->initUint32($entity + 0x0c, $modelLarge);
        $this->initUint32($entity + 0x10, $modelSmall);
        $this->initUint32($entity + 0x14, $bodyModel);

        $this->initUint32($entity + 0xc4, fdec(1.0));
        $this->initUint32($entity + 0xc8, fdec(2.0));
        $this->initUint32($entity + 0xcc, fdec(3.0));
        $this->initUint32($entity + 0xd0, fdec(4.0));
        $this->initUint32($entity + 0xd4, fdec(5.0));

        $this->initUint32($entity + 0xd8, fdec(6.0));
        $this->initUint32($entity + 0xdc, fdec(7.0));
        $this->initUint32($entity + 0xe0, fdec(8.0));
        $this->initUint32($entity + 0xe4, fdec(9.0));
        $this->initUint32($entity + 0xe8, fdec(10.0));

        return [$entity, $texlistLarge, $texlistSmall, $modelLarge, $modelSmall, $bodyModel];
    }

    public function test_lod0_draws_detail_and_updates_blinkers(): void
    {
        [$entity, $texlistLarge, , $modelLarge, , $bodyModel] = $this->makeEntity();

        $this->call('_busDrawSimpleCb_8c027a88')->with($entity, 0);

        $this->shouldCall('_njMultiMatrix')->with(0, $entity + 0x84);
        $this->shouldCall('_njCnkSetSimpleLightIntensity')->with(1.0, 2.0);
        $this->shouldCall('_njCnkSetSimpleLightColor')->with(3.0, 4.0, 5.0);
        $this->shouldCall('_BusDrawUpdateModels_8c027958')->with($entity);
        $this->shouldCall('_njSetTexture')->with($texlistLarge);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($modelLarge);
        $this->shouldCall('_njControl3D')->with(0x2500);
        $this->shouldCall('_njCnkModDrawObject')->with($bodyModel);
        $this->shouldCall('_njControl3D')->with(0x100);
    }

    public function test_lod1_night_draws_simple_far_model(): void
    {
        [$entity, , $texlistSmall, , $modelSmall] = $this->makeEntity();
        $this->setSize('_var_timeOfDay_8c18ad20', 4);
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 2);

        $this->call('_busDrawSimpleCb_8c027a88')->with($entity, 1);

        $this->shouldCall('_njMultiMatrix')->with(0, $entity + 0x84);
        $this->shouldCall('_njCnkSetSimpleLightIntensity')->with(1.0, 2.0);
        $this->shouldCall('_njCnkSetSimpleLightColor')->with(3.0, 4.0, 5.0);
        $this->shouldCall('_njSetTexture')->with($texlistSmall);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($modelSmall);
    }

    public function test_lod1_day_draws_easy_far_model(): void
    {
        [$entity, , $texlistSmall, , $modelSmall] = $this->makeEntity();
        $this->setSize('_var_timeOfDay_8c18ad20', 4);
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0);

        $this->call('_busDrawSimpleCb_8c027a88')->with($entity, 1);

        $this->shouldCall('_njMultiMatrix')->with(0, $entity + 0x84);
        $this->shouldCall('_njCnkSetEasyLightIntensity')->with(6.0, 7.0);
        $this->shouldCall('_njCnkSetEasyLightColor')->with(8.0, 9.0, 10.0);
        $this->shouldCall('_njSetTexture')->with($texlistSmall);
        $this->shouldCall('_njCnkEasyDrawObject')->with($modelSmall);
    }
};
