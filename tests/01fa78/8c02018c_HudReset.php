<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function taskPushCheck(int $tasksAddr, int $actionAddr): callable
    {
        return function () use ($tasksAddr, $actionAddr) {
            $r4 = $this->registers[4]->value;
            $r5 = $this->registers[5]->value;
            $allocSize = $this->memory->readUInt32($this->registers[15]->value)->value;
            if ($r4 !== $tasksAddr || $r5 !== $actionAddr || $allocSize !== 0) {
                throw new \Exception(sprintf(
                    "Unexpected TaskPush args: tasks=0x%x action=0x%x allocSize=%d",
                    $r4, $r5, $allocSize
                ));
            }
        };
    }

    public function test_installs_task_and_resets_scratch_state(): void
    {
        $this->setSize('_var_tasks_8c1ba5e8', 4);
        // The whole 0x22643c..0x2264a8 scratch region is addressed by this
        // function via one base pointer (var_8c22643c) plus fixed offsets,
        // not via separate relocations -- anchor every write off that base
        // so the layout matches the real (contiguous) one.
        $this->setSize('_var_8c22643c', 0x6c);
        $base = $this->addressOf('_var_8c22643c');
        // The C object addresses var_8c226450/454/458/engineRpm_8c226468/
        // 8c22646c/gearLatch_8c226470/laneLatch_8c226474/8c226478 as their
        // own (separately-relocated) globals rather than through
        // var_8c22643c's base pointer; place them at their real, contiguous
        // offsets so both objects' writes land at the same test addresses.
        $this->rellocate('_var_8c226450', $base + 0x14);
        $this->rellocate('_var_8c226454', $base + 0x18);
        $this->rellocate('_var_8c226458', $base + 0x1c);
        $this->rellocate('_var_engineRpm_8c226468', $base + 0x2c);
        $this->rellocate('_var_8c22646c', $base + 0x30);
        $this->rellocate('_var_gearLatch_8c226470', $base + 0x34);
        $this->rellocate('_var_laneLatch_8c226474', $base + 0x38);
        $this->rellocate('_var_8c226478', $base + 0x3c);

        $this->initUint32($this->addressOf('_var_driverPoints_8c2285d0'), 42);

        $this->call('_HudReset_8c02018c');

        $this->shouldCall('_TaskPush_8c014ae8')->do($this->taskPushCheck(
            $this->addressOf('_var_tasks_8c1ba5e8'),
            $this->addressOf('_hudUpdateTask_8c01ff48'),
        ));

        $this->shouldWriteLong($base + 0x00, 0);
        $this->shouldWriteLong($base + 0x04, 0);
        $this->shouldWriteLong($base + 0x10, 0);
        $this->shouldWriteLong($base + 0x14, -1); // var_8c226450
        $this->shouldWriteLong($base + 0x18, 0); // var_8c226454
        $this->shouldWriteFloat($base + 0x20, 42.0); // var_8c226458.lastSample_0x04
        $this->shouldWriteFloat($base + 0x1c, 42.0); // var_8c226458.displayedValue_0x00
        $this->shouldWriteFloat($base + 0x28, 1.0); // var_8c226458.field_0x0c
        $this->shouldWriteFloat($base + 0x2c, 0.0); // var_engineRpm_8c226468
        $this->shouldWriteLong($base + 0x30, 0); // var_8c22646c
        $this->shouldWriteLong($base + 0x34, 0); // var_gearLatch_8c226470
        $this->shouldWriteLong($base + 0x38, 0); // var_laneLatch_8c226474
        $this->shouldWriteLong($base + 0x48, 0xffff0000); // var_8c226478[0].color
        $this->shouldWriteLong($base + 0x58, 0xffff0000); // var_8c226478[1].color
        $this->shouldWriteLong($base + 0x68, 0xffff0000); // var_8c226478[2].color
    }
};
