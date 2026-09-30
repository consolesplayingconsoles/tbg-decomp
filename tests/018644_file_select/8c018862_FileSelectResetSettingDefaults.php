<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /* SETTING screen DEFAULT: DIFFICULTY, DRIVE MODE, DEFAULT VIEW, VIBRATION, SCREEN ROLL. */
    public function test_writes_defaults(): void
    {
        $this->setSize('_var_progress_8c1ba1cc', 0xd2);
        $base = $this->addressOf('_var_progress_8c1ba1cc');

        $this->call('_FileSelectResetSettingDefaults_8c018862');

        $this->shouldWriteByte($base + 0xc4, 1);
        $this->shouldWriteByte($base + 0xc5, 0);
        $this->shouldWriteByte($base + 0xc6, 2);
        $this->shouldWriteByte($base + 0xc7, 0);
        $this->shouldWriteByte($base + 0xc8, 0);
    }
};
