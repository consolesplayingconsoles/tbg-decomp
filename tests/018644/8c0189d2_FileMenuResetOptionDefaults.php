<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /* Chains the control/view/sound default resets in order. */
    public function test_calls_all_resets(): void
    {
        $this->call('_FileMenuResetOptionDefaults_8c0189d2');

        $this->shouldCall('_FileMenuResetControlDefaults_8c018862');
        $this->shouldCall('_FileMenuResetViewDefaults_8c0188bc');
        $this->shouldCall('_FileMenuResetSoundDefaults_8c0188dc');
    }
};
