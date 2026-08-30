<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_FadeCmdPushCall1_8c0223ea', 4);
    }

    public function test_registers_draw_callback_on_layer2(): void
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x38);

        $this->call('_BusRiderSeatedTask_8c02d5ca')->with($task, $state);

        $this->shouldCall('_FadeCmdPushCall1_8c0223ea')->with(2, $this->addressOf('_drawRiderSprite_8c02d19c'), $state);
    }
};
