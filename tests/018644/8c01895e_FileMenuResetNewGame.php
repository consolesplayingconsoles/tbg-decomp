<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /* Full new-game reset: base progress plus letters/new-flags/scratch fields. */
    public function test_resets_new_game(): void
    {
        $this->setSize('_var_progress_8c1ba1cc', 0xd8);
        $base = $this->addressOf('_var_progress_8c1ba1cc');

        $this->call('_FileMenuResetNewGame_8c01895e');

        $this->shouldCall('_FileMenuResetProgress_8c01890a');

        // field_0x18[5]
        for ($off = 0x18; $off < 0x2c; $off += 4) {
            $this->shouldWriteLong($base + $off, 0);
        }
        // letters_0x2c[6]
        for ($off = 0x2c; $off < 0x44; $off += 4) {
            $this->shouldWriteLong($base + $off, 0);
        }
        // courses_0x44[9]: clear new_0x01, freeRunSpriteNo_0x04
        for ($i = 0; $i < 9; $i++) {
            $c = $base + 0x44 + $i * 8;
            $this->shouldWriteByte($c + 1, 0);
            $this->shouldWriteByte($c + 4, 0);
        }
        // mark courses 0 and 6 as new
        $this->shouldWriteByte($base + 0x45, 1);
        $this->shouldWriteByte($base + 0x75, 1);
        // field_0x8c, field_0x94
        $this->shouldWriteLong($base + 0x8c, 0);
        $this->shouldWriteLong($base + 0x94, 0);
        // field_0x98[11]
        for ($off = 0x98; $off < 0xc4; $off += 4) {
            $this->shouldWriteLong($base + $off, 0);
        }
    }
};
