<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // saveExecFile_8c01bd30's own stack locals (findbuf[16], SYS_RTC_DATE
    // rtc). This is the only frame on the simulator's stack (test entry is
    // a single call()), so their addresses are the deterministic offsets
    // from the simulator's fixed initial SP (0xFFFFFC), verified against
    // each object's own prologue -- and they differ per object because the
    // C compiler's register-save set/frame size differs from the archived
    // asm's.
    private function findBufAddr(): int
    {
        return $this->isAsmObject() ? 0xFFFFD4 : 0xFFFFCC;
    }

    private function rtcAddr(): int
    {
        return $this->isAsmObject() ? 0xFFFFE4 : 0xFFFFC0;
    }

    protected function isAsmObject(): bool
    {
        return str_contains($this->objectFile, '/asm/');
    }

    public function test_no_existing_file()
    {
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $fname = $this->allocString('PDAQUIZ.BIN');
        $buf = $this->alloc(4);
        $nblock = random_int(1, 100);

        $this->call('_saveExecFile_8c01bd30')->with($buf, $fname, $nblock, $drive);
        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldCall('_buFindExecFile')
            ->with($drive, $this->findBufAddr())
            ->andReturn(0);
        $this->shouldReturn(0xb);
    }

    public function test_find_bad_disk()
    {
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $fname = $this->allocString('PDAQUIZ.BIN');
        $buf = $this->alloc(4);
        $nblock = random_int(1, 100);

        $this->call('_saveExecFile_8c01bd30')->with($buf, $fname, $nblock, $drive);
        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldCall('_buFindExecFile')
            ->with($drive, $this->findBufAddr())
            ->andReturn(0xffffff03); // -0xfd
        $this->shouldReturn(0xd);
    }

    public function test_find_no_card()
    {
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $fname = $this->allocString('PDAQUIZ.BIN');
        $buf = $this->alloc(4);
        $nblock = random_int(1, 100);

        $this->call('_saveExecFile_8c01bd30')->with($buf, $fname, $nblock, $drive);
        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldCall('_buFindExecFile')
            ->with($drive, $this->findBufAddr())
            ->andReturn(0xffffffff); // -1
        $this->shouldReturn(0xe);
    }

    public function test_find_bad_card()
    {
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $fname = $this->allocString('PDAQUIZ.BIN');
        $buf = $this->alloc(4);
        $nblock = random_int(1, 100);

        $this->call('_saveExecFile_8c01bd30')->with($buf, $fname, $nblock, $drive);
        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldCall('_buFindExecFile')
            ->with($drive, $this->findBufAddr())
            ->andReturn(0xffffff01); // -0xff
        $this->shouldReturn(0xf);
    }

    public function test_find_other_error()
    {
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $fname = $this->allocString('PDAQUIZ.BIN');
        $buf = $this->alloc(4);
        $nblock = random_int(1, 100);

        $this->call('_saveExecFile_8c01bd30')->with($buf, $fname, $nblock, $drive);
        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldCall('_buFindExecFile')
            ->with($drive, $this->findBufAddr())
            ->andReturn(0xfffffffb); // -5, unhandled
        $this->shouldReturn(10);
    }

    public function test_save_ok()
    {
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $fname = $this->allocString('PDAQUIZ.BIN');
        $buf = $this->alloc(4);
        $nblock = random_int(1, 100);

        $this->call('_saveExecFile_8c01bd30')->with($buf, $fname, $nblock, $drive);
        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldCall('_buFindExecFile')
            ->with($drive, $this->findBufAddr())
            ->andReturn(0xffffff05); // -0xfb (no such file, room to write)
        $this->shouldCall('_buGetDiskFree')
            ->with($drive, 1)
            ->andReturn($nblock); // enough free blocks
        $this->shouldCall('_syRtcGetDate')
            ->with($this->rtcAddr());
        $this->shouldCall('_buSaveExecFile')
            ->with($drive, $fname, $buf, $nblock, $this->rtcAddr(), 0x800000ff)
            ->andReturn(0);
        $this->shouldReturn(0);
    }

    public function test_save_fail()
    {
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $fname = $this->allocString('PDAQUIZ.BIN');
        $buf = $this->alloc(4);
        $nblock = random_int(1, 100);

        $this->call('_saveExecFile_8c01bd30')->with($buf, $fname, $nblock, $drive);
        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldCall('_buFindExecFile')
            ->with($drive, $this->findBufAddr())
            ->andReturn(0xffffff05); // -0xfb
        $this->shouldCall('_buGetDiskFree')
            ->with($drive, 1)
            ->andReturn($nblock);
        $this->shouldCall('_syRtcGetDate')
            ->with($this->rtcAddr());
        $this->shouldCall('_buSaveExecFile')
            ->with($drive, $fname, $buf, $nblock, $this->rtcAddr(), 0x800000ff)
            ->andReturn(0xffffffff);
        $this->shouldReturn(0xc);
    }

    public function test_full_after_defrag()
    {
        // free(with system area) < nblock, defrag-able free (0) still >= nblock -> 0x10
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $fname = $this->allocString('PDAQUIZ.BIN');
        $buf = $this->alloc(4);
        $nblock = 50;

        $this->call('_saveExecFile_8c01bd30')->with($buf, $fname, $nblock, $drive);
        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldCall('_buFindExecFile')
            ->with($drive, $this->findBufAddr())
            ->andReturn(0xffffff05); // -0xfb
        $this->shouldCall('_buGetDiskFree')
            ->with($drive, 1)
            ->andReturn(10); // < nblock
        $this->shouldCall('_buGetDiskFree')
            ->with($drive, 0)
            ->andReturn(60); // >= nblock
        $this->shouldReturn(0x10);
    }

    public function test_full_hard()
    {
        // even total free (0) < nblock -> 0x11
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $fname = $this->allocString('PDAQUIZ.BIN');
        $buf = $this->alloc(4);
        $nblock = 50;

        $this->call('_saveExecFile_8c01bd30')->with($buf, $fname, $nblock, $drive);
        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldCall('_buFindExecFile')
            ->with($drive, $this->findBufAddr())
            ->andReturn(0xffffff05); // -0xfb
        $this->shouldCall('_buGetDiskFree')
            ->with($drive, 1)
            ->andReturn(10);
        $this->shouldCall('_buGetDiskFree')
            ->with($drive, 0)
            ->andReturn(20);
        $this->shouldReturn(0x11);
    }

    public function test_diskfree_error_reclassifies()
    {
        // -0xfb path, buGetDiskFree(1) returns a negative error -> loop re-classifies it
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $fname = $this->allocString('PDAQUIZ.BIN');
        $buf = $this->alloc(4);
        $nblock = random_int(1, 100);

        $this->call('_saveExecFile_8c01bd30')->with($buf, $fname, $nblock, $drive);
        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldCall('_buFindExecFile')
            ->with($drive, $this->findBufAddr())
            ->andReturn(0xffffff05); // -0xfb
        $this->shouldCall('_buGetDiskFree')
            ->with($drive, 1)
            ->andReturn(0xffffffff); // -1 -> reclassified at loop top
        $this->shouldReturn(0xe);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_buFindExecFile', 4);
        $this->setSize('_buGetDiskFree', 4);
        $this->setSize('_syRtcGetDate', 4);
        $this->setSize('_buSaveExecFile', 4);
    }
};
