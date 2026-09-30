<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /** Wires a single object into a one-group list and invokes the entrypoint. */
    private function drawSingleObject(int $obj): void
    {
        $this->setSize('_njDrawSprite3D', 4);

        $this->initUint32($this->addressOf('_var_pedGroupCount_8c228234'), 1);

        $list = $this->alloc(2 * 0x20);
        $this->initUint32($list + 0x00, 1);
        $this->initUint32($list + 0x04, $obj);
        $this->initUint32($list + 0x20, 0); // terminator

        $groups = $this->alloc(0xc);
        $this->initUint32($groups + 0x00, 1);
        $this->initUint32($groups + 0x04, 0);
        $this->initUint32($groups + 0x08, $list);
        $this->initUint32($this->addressOf('_var_pedGroups_8c228230'), $groups);

        $this->call('_drawPedestriansMirror_8c028a38')->with(0);
    }

    public function test_type2_facing0_draws_sprite_0x20()
    {
        $obj = $this->alloc(0x64);
        $this->initUint32($obj + 0x3c, 2); // type
        $this->initUint32($obj + 0x40, 0); // own facing state

        $this->drawSingleObject($obj);

        $this->shouldCall('_njDrawSprite3D')->with($obj, 0x20, 0x30);
    }

    public function test_type2_facing2_draws_sprite_0x22()
    {
        $obj = $this->alloc(0x64);
        $this->initUint32($obj + 0x3c, 2);
        $this->initUint32($obj + 0x40, 2);

        $this->drawSingleObject($obj);

        $this->shouldCall('_njDrawSprite3D')->with($obj, 0x22, 0x30);
    }

    public function test_type2_facing3_draws_sprite_0x23()
    {
        $obj = $this->alloc(0x64);
        $this->initUint32($obj + 0x3c, 2);
        $this->initUint32($obj + 0x40, 3);

        $this->drawSingleObject($obj);

        $this->shouldCall('_njDrawSprite3D')->with($obj, 0x23, 0x30);
    }

    public function test_non_type2_facing1_bucket0_draws_sprite_0x00()
    {
        $obj = $this->alloc(0x64);
        $this->initUint32($obj + 0x3c, 1);
        $this->initUint32($obj + 0x40, 1); // facing == 1
        $this->initUint32($obj + 0x5c, 0); // low bits offset 0
        $this->initUint32($obj + 0x60, 0); // bucket 0

        $this->drawSingleObject($obj);

        $this->shouldCall('_njDrawSprite3D')->with($obj, 0x00, 0x30);
    }

    public function test_non_type2_facing1_bucket0x10000000_draws_sprite_0x08()
    {
        $obj = $this->alloc(0x64);
        $this->initUint32($obj + 0x3c, 1);
        $this->initUint32($obj + 0x40, 1); // facing == 1
        $this->initUint32($obj + 0x5c, 0);
        $this->initUint32($obj + 0x60, 0x10000000);

        $this->drawSingleObject($obj);

        $this->shouldCall('_njDrawSprite3D')->with($obj, 0x08, 0x30);
    }

    public function test_non_type2_facing1_bucket0x30000000_draws_sprite_0x18()
    {
        $obj = $this->alloc(0x64);
        $this->initUint32($obj + 0x3c, 1);
        $this->initUint32($obj + 0x40, 1); // facing == 1
        $this->initUint32($obj + 0x5c, 0);
        $this->initUint32($obj + 0x60, 0x30000000);

        $this->drawSingleObject($obj);

        $this->shouldCall('_njDrawSprite3D')->with($obj, 0x18, 0x30);
    }

    public function test_non_type2_facing0_bucket0_draws_sprite_0x08()
    {
        $obj = $this->alloc(0x64);
        $this->initUint32($obj + 0x3c, 1);
        $this->initUint32($obj + 0x40, 0); // facing == 0
        $this->initUint32($obj + 0x5c, 0);
        $this->initUint32($obj + 0x60, 0);

        $this->drawSingleObject($obj);

        $this->shouldCall('_njDrawSprite3D')->with($obj, 0x08, 0x30);
    }

    public function test_non_type2_facing0_bucket0x10000000_draws_sprite_0x00()
    {
        $obj = $this->alloc(0x64);
        $this->initUint32($obj + 0x3c, 1);
        $this->initUint32($obj + 0x40, 0); // facing == 0
        $this->initUint32($obj + 0x5c, 0);
        $this->initUint32($obj + 0x60, 0x10000000);

        $this->drawSingleObject($obj);

        $this->shouldCall('_njDrawSprite3D')->with($obj, 0x00, 0x30);
    }

    public function test_non_type2_facing0_bucket0x20000000_draws_sprite_0x18()
    {
        $obj = $this->alloc(0x64);
        $this->initUint32($obj + 0x3c, 1);
        $this->initUint32($obj + 0x40, 0); // facing == 0
        $this->initUint32($obj + 0x5c, 0);
        $this->initUint32($obj + 0x60, 0x20000000);

        $this->drawSingleObject($obj);

        $this->shouldCall('_njDrawSprite3D')->with($obj, 0x18, 0x30);
    }

    public function test_no_groups_draws_nothing()
    {
        $this->setSize('_njDrawSprite3D', 4);

        $this->initUint32($this->addressOf('_var_pedGroupCount_8c228234'), 0);
        $this->initUint32($this->addressOf('_var_pedGroups_8c228230'), 0xbebacafe);

        $this->call('_drawPedestriansMirror_8c028a38')->with(0);
    }

    public function test_inactive_group_is_skipped()
    {
        $this->setSize('_njDrawSprite3D', 4);

        $this->initUint32($this->addressOf('_var_pedGroupCount_8c228234'), 1);

        $groups = $this->alloc(0xc);
        $this->initUint32($groups + 0x00, 0); // inactive
        $this->initUint32($groups + 0x04, 0);
        $this->initUint32($groups + 0x08, 0xbebacafe); // never dereferenced
        $this->initUint32($this->addressOf('_var_pedGroups_8c228230'), $groups);

        $this->call('_drawPedestriansMirror_8c028a38')->with(0);
    }

    public function test_type2_object_draws_sprite_from_own_facing_state()
    {
        $this->setSize('_njDrawSprite3D', 4);

        $this->initUint32($this->addressOf('_var_pedGroupCount_8c228234'), 1);

        $obj = $this->alloc(0x64);
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

        $this->call('_drawPedestriansMirror_8c028a38')->with(0);

        $this->shouldCall('_njDrawSprite3D')->with($obj, 0x21, 0x30);
    }

    public function test_non_type2_object_uses_cached_bucket_and_own_facing_one()
    {
        $this->setSize('_njDrawSprite3D', 4);

        $this->initUint32($this->addressOf('_var_pedGroupCount_8c228234'), 1);

        $obj = $this->alloc(0x64);
        $this->initUint32($obj + 0x3c, 1); // type (not 2)
        $this->initUint32($obj + 0x40, 1); // own facing state
        $this->initUint32($obj + 0x5c, 0x20); // (0x20 >> 2) & 7 == 0
        $this->initUint32($obj + 0x60, 0x20000000); // cached bucket (nNodeFlags scratch)

        $list = $this->alloc(2 * 0x20);
        $this->initUint32($list + 0x00, 1);
        $this->initUint32($list + 0x04, $obj);
        $this->initUint32($list + 0x20, 0); // terminator

        $groups = $this->alloc(0xc);
        $this->initUint32($groups + 0x00, 1);
        $this->initUint32($groups + 0x04, 0);
        $this->initUint32($groups + 0x08, $list);
        $this->initUint32($this->addressOf('_var_pedGroups_8c228230'), $groups);

        $this->call('_drawPedestriansMirror_8c028a38')->with(0);

        // facing == 1, bucket 0x20000000 -> sprite 0x10, + ((0x20 >> 2) & 7) == 0
        $this->shouldCall('_njDrawSprite3D')->with($obj, 0x10, 0x30);
    }

    public function test_non_type2_object_uses_cached_bucket_and_own_facing_zero()
    {
        $this->setSize('_njDrawSprite3D', 4);

        $this->initUint32($this->addressOf('_var_pedGroupCount_8c228234'), 1);

        $obj = $this->alloc(0x64);
        $this->initUint32($obj + 0x3c, 1); // type (not 2)
        $this->initUint32($obj + 0x40, 0); // own facing state
        $this->initUint32($obj + 0x5c, 4); // (4 >> 2) & 7 == 1
        $this->initUint32($obj + 0x60, 0x30000000); // cached bucket (nNodeFlags scratch)

        $list = $this->alloc(2 * 0x20);
        $this->initUint32($list + 0x00, 1);
        $this->initUint32($list + 0x04, $obj);
        $this->initUint32($list + 0x20, 0); // terminator

        $groups = $this->alloc(0xc);
        $this->initUint32($groups + 0x00, 1);
        $this->initUint32($groups + 0x04, 0);
        $this->initUint32($groups + 0x08, $list);
        $this->initUint32($this->addressOf('_var_pedGroups_8c228230'), $groups);

        $this->call('_drawPedestriansMirror_8c028a38')->with(0);

        // facing == 0, bucket 0x30000000 -> sprite 0x10, + ((4 >> 2) & 7) == 1
        $this->shouldCall('_njDrawSprite3D')->with($obj, 0x11, 0x30);
    }
};
