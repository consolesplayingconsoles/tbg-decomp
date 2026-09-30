<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    public function test_no_groups_frees_scratch_buffer_and_draws_nothing()
    {
        $this->setSize('_syMalloc', 4);
        $this->setSize('_syFree', 4);
        $this->setSize('_atan2f', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);

        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x308), fdec(7.0));
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x310), fdec(5.0));
        $this->initUint32($this->addressOf('_var_pedGroupCount_8c228234'), 0);
        $this->initUint32($this->addressOf('_var_pedGroups_8c228230'), 0xbebacafe);

        $mem = $this->alloc(0x400);

        $this->call('_drawPedestrians_8c028b74')->with(0);

        $this->shouldCall('_syMalloc')->with(0x400)->andReturn($mem);
        $this->shouldCall('_atan2f')->with(-7.0, -5.0)->andReturn(0.0);
        $this->shouldCall('_syFree')->with($mem);
    }

    public function test_type2_object_draws_sprite_from_own_facing_state()
    {
        $this->setSize('_syMalloc', 4);
        $this->setSize('_syFree', 4);
        $this->setSize('_atan2f', 4);
        $this->setSize('_njDrawSprite3D', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);

        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x308), fdec(7.0));
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x310), fdec(5.0));
        $this->initUint32($this->addressOf('_var_pedGroupCount_8c228234'), 1);

        $obj = $this->alloc(0x60);
        $this->initUint32($obj + 0x3c, 2); // type
        $this->initUint32($obj + 0x40, 1); // own facing state

        // one group, active, with a hole then the object then the terminator
        $list = $this->alloc(3 * 0x20);
        $this->initUint32($list + 0x00, -1); // hole marker
        $this->initUint32($list + 0x20, 1); // valid marker
        $this->initUint32($list + 0x24, $obj);
        $this->initUint32($list + 0x40, 0); // terminator

        $groups = $this->alloc(0xc);
        $this->initUint32($groups + 0x00, 1); // active
        $this->initUint32($groups + 0x04, 0);
        $this->initUint32($groups + 0x08, $list);
        $this->initUint32($this->addressOf('_var_pedGroups_8c228230'), $groups);

        $mem = $this->alloc(0x400);

        $this->call('_drawPedestrians_8c028b74')->with(0);

        $this->shouldCall('_syMalloc')->with(0x400)->andReturn($mem);
        $this->shouldCall('_atan2f')->with(-7.0, -5.0)->andReturn(0.0);
        $this->shouldCall('_njDrawSprite3D')->with($obj, 0x21, 0x30);
        $this->shouldCall('_syFree')->with($mem);
    }

    public function test_inactive_group_is_skipped()
    {
        $this->setSize('_syMalloc', 4);
        $this->setSize('_syFree', 4);
        $this->setSize('_atan2f', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);

        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x308), fdec(7.0));
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x310), fdec(5.0));
        $this->initUint32($this->addressOf('_var_pedGroupCount_8c228234'), 1);

        $groups = $this->alloc(0xc);
        $this->initUint32($groups + 0x00, 0); // inactive
        $this->initUint32($groups + 0x04, 0);
        $this->initUint32($groups + 0x08, 0xbebacafe); // never dereferenced
        $this->initUint32($this->addressOf('_var_pedGroups_8c228230'), $groups);

        $mem = $this->alloc(0x400);

        $this->call('_drawPedestrians_8c028b74')->with(0);

        $this->shouldCall('_syMalloc')->with(0x400)->andReturn($mem);
        $this->shouldCall('_atan2f')->with(-7.0, -5.0)->andReturn(0.0);
        $this->shouldCall('_syFree')->with($mem);
    }

    public function test_non_type2_object_computes_bucket_from_relative_angle_and_caches_it()
    {
        $this->setSize('_syMalloc', 4);
        $this->setSize('_syFree', 4);
        $this->setSize('_atan2f', 4);
        $this->setSize('_njDrawSprite3D', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);

        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x308), fdec(0.0));
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x310), fdec(0.0));
        $this->initUint32($this->addressOf('_var_pedGroupCount_8c228234'), 1);

        $target = $this->alloc(0x14);
        $this->initUint32($target + 0x0c, fdec(0.0));
        $this->initUint32($target + 0x10, fdec(0.0));

        // two objects sharing the same target (0x4c), so the second reuses
        // the cached bucket instead of calling atan2f again
        $obj1 = $this->alloc(0x60);
        $this->initUint32($obj1 + 0x3c, 1); // type (not 2)
        $this->initUint32($obj1 + 0x40, 0); // own facing state
        $this->initUint32($obj1 + 0x4c, $target);
        $this->initUint32($obj1 + 0x5c, 0); // animation frame bits

        $obj2 = $this->alloc(0x60);
        $this->initUint32($obj2 + 0x3c, 1);
        $this->initUint32($obj2 + 0x40, 1); // own facing state, mirrored
        $this->initUint32($obj2 + 0x4c, $target);
        $this->initUint32($obj2 + 0x5c, 0x20); // (0x20 >> 2) & 7 == 0

        $list = $this->alloc(3 * 0x20);
        $this->initUint32($list + 0x00, 1);
        $this->initUint32($list + 0x04, $obj1);
        $this->initUint32($list + 0x20, 1);
        $this->initUint32($list + 0x24, $obj2);
        $this->initUint32($list + 0x40, 0); // terminator

        $groups = $this->alloc(0xc);
        $this->initUint32($groups + 0x00, 1);
        $this->initUint32($groups + 0x04, 0);
        $this->initUint32($groups + 0x08, $list);
        $this->initUint32($this->addressOf('_var_pedGroups_8c228230'), $groups);

        $mem = $this->alloc(0x400);

        $this->call('_drawPedestrians_8c028b74')->with(0);

        $this->shouldCall('_syMalloc')->with(0x400)->andReturn($mem);
        $this->shouldCall('_atan2f')->with(-0.0, -0.0)->andReturn(0.0);
        $this->shouldWriteLong($mem + 0x00, $target); // cache entry key
        // relative angle (self - target) is 0, own facing 0 -> bucket 0 maps to sprite 0
        $this->shouldCall('_atan2f')->with(-0.0, -0.0)->andReturn(0.0);
        $this->shouldWriteLong($mem + 0x04, 0); // cache entry bucket
        $this->shouldCall('_njDrawSprite3D')->with($obj1, 0, 0x30);
        // cache hit for obj2: no second atan2f target call; mirrored facing -> sprite 8
        $this->shouldCall('_njDrawSprite3D')->with($obj2, 8, 0x30);
        $this->shouldCall('_syFree')->with($mem);
    }

    /** Converts a BAMS angle (0..65536 == full circle) to the radians atan2f would return. */
    private function bams(int $value): float
    {
        return $value / 65536 * 2 * M_PI;
    }

    /**
     * Single non-type-2 object whose self/target atan2f calls are mocked
     * directly, so `diff = selfAngle(0) - targetAngle` lands exactly on
     * $targetBams's negation. Pins one bucket threshold band from the asm's
     * LP_GEN_30626 compare chain.
     */
    private function runAngleBucketCase(int $targetBams, int $facing, int $expectedBucket, int $expectedSprite): void
    {
        $this->setSize('_syMalloc', 4);
        $this->setSize('_syFree', 4);
        $this->setSize('_atan2f', 4);
        $this->setSize('_njDrawSprite3D', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);

        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x308), fdec(0.0));
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x310), fdec(0.0));
        $this->initUint32($this->addressOf('_var_pedGroupCount_8c228234'), 1);

        $target = $this->alloc(0x14);
        $this->initUint32($target + 0x0c, fdec(0.0));
        $this->initUint32($target + 0x10, fdec(0.0));

        $obj = $this->alloc(0x60);
        $this->initUint32($obj + 0x3c, 1); // type (not 2)
        $this->initUint32($obj + 0x40, $facing);
        $this->initUint32($obj + 0x4c, $target);
        $this->initUint32($obj + 0x5c, 0);

        $list = $this->alloc(2 * 0x20);
        $this->initUint32($list + 0x00, 1);
        $this->initUint32($list + 0x04, $obj);
        $this->initUint32($list + 0x20, 0);

        $groups = $this->alloc(0xc);
        $this->initUint32($groups + 0x00, 1);
        $this->initUint32($groups + 0x04, 0);
        $this->initUint32($groups + 0x08, $list);
        $this->initUint32($this->addressOf('_var_pedGroups_8c228230'), $groups);

        $mem = $this->alloc(0x400);

        $this->call('_drawPedestrians_8c028b74')->with(0);

        $this->shouldCall('_syMalloc')->with(0x400)->andReturn($mem);
        $this->shouldCall('_atan2f')->with(-0.0, -0.0)->andReturn(0.0); // self angle == 0
        $this->shouldWriteLong($mem + 0x00, $target);
        $this->shouldCall('_atan2f')->with(-0.0, -0.0)->andReturn($this->bams($targetBams));
        $this->shouldWriteLong($mem + 0x04, $expectedBucket);
        $this->shouldCall('_njDrawSprite3D')->with($obj, $expectedSprite, 0x30);
        $this->shouldCall('_syFree')->with($mem);
    }

    public function test_positive_small_band_bucket_0x20000000()
    {
        // diff = 0 - (-0x3800) = 0x3800, in [0x2000,0x5fff].
        $this->runAngleBucketCase(-0x3800, 0, 0x20000000, 0x10);
    }

    public function test_positive_large_band_bucket_0x30000000()
    {
        // diff = 0 - (-0xc000) = 0xc000, in [0xa000,0xdfff].
        $this->runAngleBucketCase(-0xc000, 0, 0x30000000, 0x18);
    }

    public function test_negative_far_band_bucket_0x20000000()
    {
        // diff = 0 - 0xc000 = -0xc000, in [-0xe000,-0xa001].
        $this->runAngleBucketCase(0xc000, 0, 0x20000000, 0x10);
    }

    public function test_negative_mid_band_bucket_0x30000000()
    {
        // diff = 0 - 0x3000 = -0x3000, in [-0x6000,-0x2001]: the negative
        // mirror of the positive [0xa000,0xdfff] -> 0x30000000 band.
        $this->runAngleBucketCase(0x3000, 0, 0x30000000, 0x18);
    }

    public function test_negative_far_mid_band_bucket_0x10000000()
    {
        // diff = 0 - 0x8000 = -0x8000, in [-0xa000,-0x6001]: the negative
        // mirror of the positive [0x6000,0x9fff] -> 0x10000000 band.
        $this->runAngleBucketCase(0x8000, 0, 0x10000000, 8);
    }

    public function test_cache_full_skips_drawing_further_distinct_targets()
    {
        $this->setSize('_syMalloc', 4);
        $this->setSize('_syFree', 4);
        $this->setSize('_atan2f', 4);
        $this->setSize('_njDrawSprite3D', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);

        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x308), fdec(0.0));
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x310), fdec(0.0));
        $this->initUint32($this->addressOf('_var_pedGroupCount_8c228234'), 1);

        // 128 distinct targets fill the 0x400-byte / 8-byte-entry cache
        // exactly; a 129th distinct target then finds it full.
        $entryCount = 129;
        $objs = [];
        $targets = [];
        for ($i = 0; $i < $entryCount; $i++) {
            $target = $this->alloc(0x14);
            $this->initUint32($target + 0x0c, fdec(0.0));
            $this->initUint32($target + 0x10, fdec(0.0));
            $targets[] = $target;

            $obj = $this->alloc(0x60);
            $this->initUint32($obj + 0x3c, 1); // type (not 2)
            $this->initUint32($obj + 0x40, 0); // own facing state
            $this->initUint32($obj + 0x4c, $target);
            $this->initUint32($obj + 0x5c, 0); // animation frame bits
            $objs[] = $obj;
        }

        $list = $this->alloc(($entryCount + 1) * 0x20);
        foreach ($objs as $i => $obj) {
            $this->initUint32($list + $i * 0x20, 1);
            $this->initUint32($list + $i * 0x20 + 0x04, $obj);
        }
        $this->initUint32($list + $entryCount * 0x20, 0); // terminator

        $groups = $this->alloc(0xc);
        $this->initUint32($groups + 0x00, 1);
        $this->initUint32($groups + 0x04, 0);
        $this->initUint32($groups + 0x08, $list);
        $this->initUint32($this->addressOf('_var_pedGroups_8c228230'), $groups);

        $mem = $this->alloc(0x400);

        $this->call('_drawPedestrians_8c028b74')->with(0);

        $this->shouldCall('_syMalloc')->with(0x400)->andReturn($mem);
        $this->shouldCall('_atan2f')->with(-0.0, -0.0)->andReturn(0.0); // self angle

        // the first 128 objects each miss the cache, get a fresh entry, and draw
        for ($i = 0; $i < 128; $i++) {
            $this->shouldWriteLong($mem + $i * 8, $targets[$i]); // cache entry key
            $this->shouldCall('_atan2f')->with(-0.0, -0.0)->andReturn(0.0); // target angle
            $this->shouldWriteLong($mem + $i * 8 + 4, 0); // cache entry bucket
            $this->shouldCall('_njDrawSprite3D')->with($objs[$i], 0, 0x30);
        }

        // the 129th object's target isn't cached and the cache is full: skipped
        $this->shouldCall('_syFree')->with($mem);
    }
};
