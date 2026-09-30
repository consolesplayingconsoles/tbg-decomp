<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_is_noop_before_the_run_starts(): void
    {
        $this->initUint32($this->addressOf('_var_runState_8c2285c4'), 0);

        $this->call('_execRowTaskGroupTask_8c02a60e');
    }

    public function test_driving_pushes_draw_callbacks_and_execs_row_tasks(): void
    {
        $this->initUint32($this->addressOf('_var_runState_8c2285c4'), 1);

        $this->call('_execRowTaskGroupTask_8c02a60e');

        $this->shouldCall('_RenderPushCall1_8c0223ea')
            ->with(0, $this->addressOf('_setSimpleLightCallback_8c02a5d0'), 0);
        $this->shouldCall('_RenderPushCall1_8c0223ea')
            ->with(1, $this->addressOf('_setSimpleLightCallback_8c02a5d0'), 1);
        $this->shouldCall('_TaskRunGroup_8c014b42')
            ->with($this->addressOf('_var_tasks_8c1bb448'));
    }
};
