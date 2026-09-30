<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /* Chains the three OPTION sub-screens' DEFAULT resets in order. */
    public function test_calls_all_resets(): void
    {
        $this->call('_FileSelectResetOptionDefaults_8c0189d2');

        $this->shouldCall('_FileSelectResetSettingDefaults_8c018862');
        $this->shouldCall('_FileSelectResetKeyConfigDefaults_8c0188bc');
        $this->shouldCall('_FileSelectResetSoundDefaults_8c0188dc');
    }
};
