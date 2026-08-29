<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_starts_fade_out_flow(): void
    {
        $this->setSize('_var_tasks_8c1ba3c8', 4);

        $task = $this->alloc(0x1c);

        $this->call('_FUN_8c02c738');

        $this->shouldCall('_FUN_8c01614c');
        $this->shouldCall('_TaskPush_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                new WildcardArgument(), // FUN_8c02c69a's address
                new WildcardArgument(), // &created_task (local var, address not predictable here)
                new WildcardArgument(), // &created_state
                0
            )
            ->do(function () use ($task) {
                // R6 is TaskPush's 3rd arg (created_task), a stack local
                // in FUN_8c02c738 whose address isn't known ahead of time.
                $this->writeUInt32($this->getRegister(6)->value, 0, U32::of($task));
            })
            ->andReturn(1);

        $this->shouldWriteLong($task + 8, 0);
        $this->shouldCall('_FadePushIn_8c022a9c')->with(10);
    }
};
