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
 * drawMirror_8c027bac(TrafficEntry *entity, int lod): the rear-view-mirror
 * draw callback BusDrawPlaceEntity_8c027c3c registers. Same shape as
 * drawAhead_8c027a88 minus the shadow volume and the night/day split: lod == 0
 * draws the simple-light model and refreshes the entity's model nodes, lod != 0
 * always the easy-light one.
 */
return new class extends TestCase {
    private function makeEntity(): array
    {
        $entity = $this->alloc(0x514);
        $this->doNotRandomizeMemory();

        $texlistLarge = $this->alloc(4);
        $texlistSmall = $this->alloc(4);
        $modelLarge = $this->alloc(4);
        $modelSmall = $this->alloc(4);

        $this->initUint32($entity + 0x04, $texlistLarge);
        $this->initUint32($entity + 0x08, $texlistSmall);
        $this->initUint32($entity + 0x0c, $modelLarge);
        $this->initUint32($entity + 0x10, $modelSmall);

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

        return [$entity, $texlistLarge, $texlistSmall, $modelLarge, $modelSmall];
    }

    public function test_lod0_draws_simple_near_model(): void
    {
        [$entity, $texlistLarge, , $modelLarge] = $this->makeEntity();

        $this->call('_drawMirror_8c027bac')->with($entity, 0);

        $this->shouldCall('_njMultiMatrix')->with(0, $entity + 0x84);
        $this->shouldCall('_njCnkSetSimpleLightIntensity')->with(1.0, 2.0);
        $this->shouldCall('_njCnkSetSimpleLightColor')->with(3.0, 4.0, 5.0);
        $this->shouldCall('_BusDrawUpdateModels_8c027958')->with($entity);
        $this->shouldCall('_njSetTexture')->with($texlistLarge);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($modelLarge);
    }

    public function test_lod1_draws_easy_far_model(): void
    {
        [$entity, , $texlistSmall, , $modelSmall] = $this->makeEntity();

        $this->call('_drawMirror_8c027bac')->with($entity, 1);

        $this->shouldCall('_njMultiMatrix')->with(0, $entity + 0x84);
        $this->shouldCall('_njCnkSetEasyLightIntensity')->with(6.0, 7.0);
        $this->shouldCall('_njCnkSetEasyLightColor')->with(8.0, 9.0, 10.0);
        $this->shouldCall('_njSetTexture')->with($texlistSmall);
        $this->shouldCall('_njCnkEasyDrawObject')->with($modelSmall);
    }
};
