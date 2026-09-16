<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /* Resets PlayerProgress to the new-game base state. */
    public function test_resets_progress(): void
    {
        $this->setSize('_var_progress_8c1ba1cc', 0xd8);
        $base = $this->addressOf('_var_progress_8c1ba1cc');
        $g1 = $this->addressOf('_var_runReportPending_8c1bb8b8');
        $g2 = $this->addressOf('_var_runWasPractice_8c1bb8bc');

        $this->call('_FileMenuResetProgress_8c01890a');

        // days_0x00
        $this->shouldWriteLong($base + 0x00, 1);
        // eventProgressFlags_0x04[5]
        for ($off = 0x04; $off < 0x18; $off += 4) {
            $this->shouldWriteLong($base + $off, 0);
        }
        // courses_0x44[9]: clear unlocked_0x00, everPlayed_0x02, storyAward_0x03
        for ($i = 0; $i < 9; $i++) {
            $c = $base + 0x44 + $i * 8;
            $this->shouldWriteByte($c + 0, 0);
            $this->shouldWriteByte($c + 2, 0);
            $this->shouldWriteByte($c + 3, 0);
        }
        // unlock courses 0 and 6
        $this->shouldWriteByte($base + 0x44, 1);
        $this->shouldWriteByte($base + 0x74, 1);
        // exp_0x90
        $this->shouldWriteLong($base + 0x90, 0);

        $this->shouldWriteLong($g1, 1);
        $this->shouldWriteLong($g2, 0);
    }
};
