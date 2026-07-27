<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /* Zeroes the 0xcc..0xcf asset flags and sets the 0xd0/0xd1 deadzone to 0x10. */
    public function test_writes_defaults(): void
    {
        $this->setSize('_var_progress_8c1ba1cc', 0xd2);
        $base = $this->addressOf('_var_progress_8c1ba1cc');

        $this->call('_FileMenuResetViewDefaults_8c0188bc');

        $this->shouldWriteByte($base + 0xcc, 0);
        $this->shouldWriteByte($base + 0xcd, 0);
        $this->shouldWriteByte($base + 0xce, 0);
        $this->shouldWriteByte($base + 0xcf, 0);
        $this->shouldWriteByte($base + 0xd0, 0x10);
        $this->shouldWriteByte($base + 0xd1, 0x10);
    }
};
