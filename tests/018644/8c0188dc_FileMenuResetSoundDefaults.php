<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /* Non-negative sound mode is cached as-is; 0xd4..0xd6 reset to 9/5/9. */
    public function test_positive_sound_mode(): void
    {
        $this->setSize('_var_progress_8c1ba1cc', 0xd8);
        $base = $this->addressOf('_var_progress_8c1ba1cc');

        $this->call('_FileMenuResetSoundDefaults_8c0188dc');

        $this->shouldCall('_SndGetSoundMode_8c010924')->andReturn(3);
        $this->shouldWriteByte($this->addressOf('_var_soundMode_8c226070'), 3);
        $this->shouldWriteByte($base + 0xd4, 9);
        $this->shouldWriteByte($base + 0xd5, 5);
        $this->shouldWriteByte($base + 0xd6, 9);
    }

    /* Negative sound mode is clamped to 0. */
    public function test_negative_sound_mode(): void
    {
        $this->setSize('_var_progress_8c1ba1cc', 0xd8);
        $base = $this->addressOf('_var_progress_8c1ba1cc');
        $soundMode = $this->addressOf('_var_soundMode_8c226070');

        $this->call('_FileMenuResetSoundDefaults_8c0188dc');

        $this->shouldCall('_SndGetSoundMode_8c010924')->andReturn(-1);
        $this->shouldWriteByte($soundMode, -1);
        $this->shouldWriteByte($soundMode, 0);
        $this->shouldWriteByte($base + 0xd4, 9);
        $this->shouldWriteByte($base + 0xd5, 5);
        $this->shouldWriteByte($base + 0xd6, 9);
    }
};
