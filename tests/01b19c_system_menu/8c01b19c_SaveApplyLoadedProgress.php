<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_copies_progress_fields_into_session_globals()
    {
        $progress = $this->addressOf('_var_progress_8c1ba1cc');
        $this->initUint32($progress + 0xd8, 0x11111111);
        $this->initUint32($progress + 0xdc, 0x22222222);
        $this->initUint32($progress + 0xe0, 0x33333333);
        $this->initUint8($progress + 0xe4, 0x44);

        $this->call('_SystemMenuApplyLoadedProgress_8c01b19c');

        $this->shouldWriteLong($this->addressOf('_var_8c1bb8b8'), 0x11111111);
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8bc'), 0x22222222);
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8dc'), 0x33333333);
        $this->shouldWriteByte($this->addressOf('_var_award_8c1bb8f8'), 0x44);
    }

    public function test_award_byte_is_sign_extended()
    {
        $progress = $this->addressOf('_var_progress_8c1ba1cc');
        $this->initUint32($progress + 0xd8, 0);
        $this->initUint32($progress + 0xdc, 0);
        $this->initUint32($progress + 0xe0, 0);
        $this->initUint8($progress + 0xe4, 0xff);

        $this->call('_SystemMenuApplyLoadedProgress_8c01b19c');

        $this->shouldWriteLong($this->addressOf('_var_8c1bb8b8'), 0);
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8bc'), 0);
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8dc'), 0);
        $this->shouldWriteByte($this->addressOf('_var_award_8c1bb8f8'), 0xff);
    }
};
