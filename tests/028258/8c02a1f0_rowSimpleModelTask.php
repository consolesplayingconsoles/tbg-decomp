<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_pushes_single_opaque_draw_call(): void
    {
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0x33000000);
        $state = $this->alloc(0x7c);

        $this->call('_rowSimpleModelTask_8c02a1f0')->with($task, $state);

        $this->shouldCall('_RenderPushCall2_8c022420')
            ->with(0, $this->addressOf('_drawRowSimpleModel_8c02a1b2'), $state, 0x33000000);
    }
};
