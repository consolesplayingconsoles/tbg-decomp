<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /* Applies the 0xd4/0xd5/0xd6 audio bytes to the sound engine in order. */
    public function test_applies_settings(): void
    {
        $this->setSize('_var_progress_8c1ba1cc', 0xd8);
        $base = $this->addressOf('_var_progress_8c1ba1cc');
        $this->initUint8($base + 0xd4, 7);
        $this->initUint8($base + 0xd5, 3);
        $this->initUint8($base + 0xd6, 5);

        $this->call('_FileMenuApplySoundSettings_8c0189fc');

        $this->shouldCall('_SndSetAdxVol_8c010972')->with(7, 0);
        $this->shouldCall('_SndSetMidiVolAndInitStruct_8c0109f4')->with(3);
        $this->shouldCall('_SndSetAdxVol_8c010972')->with(5, 1);
    }
};
