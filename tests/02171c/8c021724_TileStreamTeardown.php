<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_already_torn_down_is_a_noop(): void {
        $this->setSize('_var_tileLayerSlots_8c226520', 5 * 4);
        $this->initUint32($this->addressOf('_var_tileLayerSlots_8c226520'), 0xffffffff);

        $this->call('_TileStreamTeardown_8c021724');
    }

    public function test_releases_all_then_frees_the_5_grid_buffers(): void {
        $this->setSize('_var_tileLayerSlots_8c226520', 5 * 4);
        $this->setSize('_syFree', 4);

        $base = $this->addressOf('_var_tileLayerSlots_8c226520');
        $slots = [];
        for ($i = 0; $i < 5; $i++) {
            $slots[$i] = $this->alloc(4);
            $this->initUint32($base + $i * 4, $slots[$i]);
        }

        $this->call('_TileStreamTeardown_8c021724');

        $this->shouldCall('_TileStreamReleaseAll_8c021a24');
        for ($i = 0; $i < 5; $i++) {
            $this->shouldCall('_syFree')->with($slots[$i]);
        }
        $this->shouldWriteLong($base, 0xffffffff);
    }
};
