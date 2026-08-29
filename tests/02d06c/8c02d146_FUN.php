<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_restores_control3d_flags(): void
    {
        $this->setSize('_njControl3D', 4);

        $this->call('_FUN_8c02d146')->with(0);

        $this->shouldCall('_njControl3D')->with(0x100);
    }

    public function test_ignores_its_arg(): void
    {
        $this->setSize('_njControl3D', 4);

        $this->call('_FUN_8c02d146')->with(1);

        $this->shouldCall('_njControl3D')->with(0x100);
    }
};
