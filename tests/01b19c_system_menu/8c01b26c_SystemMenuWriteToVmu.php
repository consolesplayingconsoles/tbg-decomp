<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\CallingConventions\RoriCallingConvention;

return new class extends TestCase {
    protected function isAsmObject(): bool
    {
        return str_contains($this->objectFile, '/asm/');
    }

    /* SHC lowers strcpy to __slow_strcpy (dst/src in R0/R1, Rori). */
    private function expectStrcpy(int $dst, $src): void
    {
        if ($this->isAsmObject()) {
            $this->shouldCall('_strcpy')->with($dst, $src);
        } else {
            $this->shouldCall('__slow_strcpy')->with($dst, $src)
                ->using(new RoriCallingConvention());
        }
    }

    public function test_builds_and_saves_backup_image(): void
    {
        $this->setSize('_var_progress_8c1ba1cc', 0xe8);
        $this->setSize('_var_backupFileHeader_8c1ba2e4', 0x60);       // BUS_BACKUPFILEHEADER
        $this->setSize('_init_saveNames_8c044d50', 0x2c); // char*[11]

        $progress = $this->addressOf('_var_progress_8c1ba1cc');
        $header = $this->addressOf('_var_backupFileHeader_8c1ba2e4');
        $saveNames = $this->addressOf('_init_saveNames_8c044d50');
        $icon = $this->addressOf('_var_vmuIconFileBuf_8c1ba344');

        $this->initUint32($this->addressOf('_var_runReportPending_8c1bb8b8'), 0x11111111);
        $this->initUint32($this->addressOf('_var_runWasPractice_8c1bb8bc'), 0x22222222);
        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 0x33333333);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0x44);
        $this->initUint32($this->addressOf('_var_profileUnlockedCount_8c2263a4'), 0x55555555);
        $this->initUint32($icon, 0x8c400000);
        $this->initUint16($header + 0x54, 0);        // visual_type (memset is mocked)
        $this->initUint32($saveNames + 2 * 4, 0x8c440000);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 2);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 1);

        $this->call('_SystemMenuWriteToVmu_8c01b26c');

        // Stage the session values into the progress struct.
        $this->shouldWriteLong($progress + 0xd8, 0x11111111);
        $this->shouldWriteLong($progress + 0xdc, 0x22222222);
        $this->shouldWriteLong($progress + 0xe0, 0x33333333);
        $this->shouldWriteByte($progress + 0xe4, 0x44);
        $this->shouldCall('_ProfileFileUpdateUnlocks_8c01c980');
        $this->shouldWriteLong($progress + 0x8c, 0x55555555);
        $this->shouldCall('_updateVmsComment_8c01b206');

        // Build the backup file header.
        $this->shouldCall('_memset')->with($header, 0, 0x60);
        $this->shouldCall('_njMemCopy')->with($header, $this->addressOf('_var_vmsComment_8c226098'), 0x10);
        $this->expectStrcpy($header + 0x12, "東京バス案内　データ");
        $this->shouldCall('_njMemCopy')->with($header + 0x34, $this->addressOf('_init_bupGameName_8c04410c'), 0x10);
        $this->shouldWriteLong($header + 0x44, 0x8c400000);       // icon_palette
        $this->shouldWriteLong($header + 0x48, 0x8c400020);       // icon_data
        $this->shouldWriteWord($header + 0x4c, 1);                // icon_num
        $this->shouldWriteWord($header + 0x4e, 1);                // icon_speed
        $this->shouldWriteLong($header + 0x58, $progress);        // save_data
        $this->shouldWriteLong($header + 0x5c, 0xe8);             // save_size

        $this->shouldCall('_buCalcBackupFileSize')->with(1, 0, 0xe8)->andReturn(3);
        $this->shouldCall('_syMalloc')->with(3 << 9)->andReturn(0x8c500000);
        $this->shouldWriteLong($this->addressOf('_var_backupFileImageBuf_8c1ba348'), 0x8c500000);
        $this->shouldCall('_buMakeBackupFileImage')->with(0x8c500000, $header);
        $this->shouldCall('_BupSave_8c014bcc')->with(1, 0x8c440000, 0x8c500000, 3);
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 1);
    }
};
