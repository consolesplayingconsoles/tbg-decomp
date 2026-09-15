<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_resets_state_and_installs_task(): void
    {
        $this->setSize('_var_8c2285c4', 0xa0);
        $base = $this->addressOf('_var_8c2285c4');
        $this->rellocate('_var_stopPhase_8c2285e4', $base + 0x20);
        $this->rellocate('_var_8c2285fc', $base + 0x38);
        $this->rellocate('_var_8c22861c', $base + 0x58);
        $this->rellocate('_var_8c228634', $base + 0x70);

        $this->setSize('_var_tasks_8c1ba5e8', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_practiceRules_8c226410', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x400);
        $this->setSize('_var_cooldownCollision_8c228690', 4);
        $this->setSize('_var_cooldownOffCourse_8c228694', 4);
        $this->setSize('_var_cooldownSignal_8c228698', 4);
        $this->setSize('_var_cooldownLane_8c22869c', 4);
        $this->setSize('_var_cooldownIntersection_8c2286a0', 4);
        $this->setSize('_var_driveMsgQueue_8c228564', 0x60);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x358, 0);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x34c, 0);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x368, 0);

        $this->call('_DrivePointsReset_8c02c46a');

        // DrivePointsReset_8c02c46a has an empty stack frame of its own
        // beyond the two TaskPush out-params (created_task, created_state),
        // pushed right after the STS.L PR/ADD #-8,R15 prologue -- so their
        // addresses are the initial test stack pointer (16MiB-4, see
        // sh4objtest's Run) minus 12 and 8 respectively. Same layout in
        // both objects.
        $sp0 = 1024 * 1024 * 16 - 4;
        $this->shouldCall('_TaskPush_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba5e8'),
                $this->addressOf('_taskCallback_8c02c072'),
                $sp0 - 12, // &created_task
                $sp0 - 8, // &created_state
                0
            )
            ->andReturn(1);

        $this->shouldWriteLong($base + 0x00, 0); // var_8c2285c4[0] phase
        $this->shouldWriteLong($base + 0x04, 0); // var_8c2285c4[1]
        $this->shouldWriteLong($base + 0x20, 1); // var_stopPhase_8c2285e4 (playMode != 1 -> 1)

        $this->shouldWriteLong($base + 0x2c, 0); // var_8c2285c4[11]
        $this->shouldWriteLong($base + 0x30, 0); // var_8c2285c4[12]
        $this->shouldWriteLong($base + 0x34, 0); // var_8c2285c4[13]

        $this->shouldWriteLong($base + 0x38, 0); // var_8c2285fc[0]
        $this->shouldWriteLong($base + 0x40, 0); // var_8c2285fc[2]
        $this->shouldWriteLong($base + 0x44, 0); // var_8c2285fc[3]
        $this->shouldWriteLong($base + 0x48, 0); // var_8c2285fc[4]
        $this->shouldWriteLong($base + 0x4c, 0); // var_8c2285fc[5]
        $this->shouldWriteLong($base + 0x50, 0); // var_8c2285fc[6]
        $this->shouldWriteLong($base + 0x54, -1); // var_8c2285fc[7]

        $this->shouldWriteLong($base + 0x5c, 0); // var_8c22861c[1]
        $this->shouldWriteLong($base + 0x60, 0); // var_8c22861c[2]
        $this->shouldWriteLong($base + 0x64, 1); // var_8c22861c[3]
        $this->shouldWriteLong($base + 0x68, 0); // var_8c22861c[4]
        $this->shouldWriteLong($base + 0x6c, 0); // var_8c22861c[5]
        $this->shouldWriteLong($base + 0x70, 0); // var_8c228634[0]
        $this->shouldWriteLong($base + 0x74, 0); // var_8c228634[1]
        $this->shouldWriteLong($base + 0x78, 0); // var_8c228634[2]

        $this->shouldWriteLong($base + 0x7c, 0); // var_8c2285c4[31] i.e. var_8c228640
        $this->shouldWriteLong($base + 0x88, 0); // var_8c2285c4[34] i.e. var_firstUpshift_8c22864c
        $this->shouldWriteFloat($base + 0x90, 0.0); // var_8c2285c4[36] float
        $this->shouldWriteLong($base + 0x94, 0); // var_8c2285c4[37]
        $this->shouldWriteLong($base + 0x98, 0); // var_8c2285c4[38]

        $this->shouldWriteLong($this->addressOf('_var_cooldownCollision_8c228690'), 0);
        $this->shouldWriteLong($this->addressOf('_var_cooldownOffCourse_8c228694'), 0);
        $this->shouldWriteLong($this->addressOf('_var_cooldownSignal_8c228698'), 0);
        $this->shouldWriteLong($this->addressOf('_var_cooldownLane_8c22869c'), 0);
        $this->shouldWriteLong($this->addressOf('_var_cooldownIntersection_8c2286a0'), 0);

        $this->shouldWriteLong($this->addressOf('_var_driveMsgQueue_8c228564') + 0x14, 0);
        $this->shouldWriteLong($this->addressOf('_var_driveMsgQueue_8c228564') + 0x2c, 0);
        $this->shouldWriteLong($this->addressOf('_var_driveMsgQueue_8c228564') + 0x44, 0);
        $this->shouldWriteLong($this->addressOf('_var_driveMsgQueue_8c228564') + 0x5c, 0);
    }
};
