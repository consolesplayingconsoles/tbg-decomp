<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// var_bupPhase_8c2260d0 phase codes; BACKUPINFO::Operation is at offset 0x10.
return new class extends TestCase {
    private const OP = 0x10;

    private function bupInfo(int $operation): int
    {
        $info = $this->alloc(0x60);
        $this->initUint32($info + self::OP, $operation);
        return $info;
    }

    public function test_stat_busy_returns_zero()
    {
        // buStat != -1 and != 0 -> idle, phase reset to 0
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $info = $this->bupInfo(0);

        $this->call('_pollBupOp_8c01bc44')->with(0xdead, $drive);
        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($info);
        $this->shouldCall('_buStat')->with($drive)->andReturn(5);
        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldReturn(0);
    }

    public function test_inprogress_saveexec()
    {
        $this->runInProgress(0xd, 0x12); // BUD_OP_SAVEEXECFILE
    }

    public function test_inprogress_defrag()
    {
        $this->runInProgress(0x9, 0x14); // BUD_OP_DEFRAGDISK
    }

    public function test_inprogress_loadfileex()
    {
        $this->runInProgress(0xe, 0x16); // BUD_OP_LOADFILEEX
    }

    public function test_inprogress_rewriteexec()
    {
        $this->runInProgress(0x14, 0x18); // BUD_OP_REWRITEEXECFILE
    }

    private function runInProgress(int $operation, int $phase): void
    {
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $info = $this->bupInfo($operation);

        $this->call('_pollBupOp_8c01bc44')->with(0xdead, $drive);
        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($info);
        $this->shouldCall('_buStat')->with($drive)->andReturn(0xffffffff); // -1
        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), $phase);
        $this->shouldReturn($phase);
    }

    public function test_inprogress_unknown_op_returns_unchanged()
    {
        // busy with an op we don't map: phase left untouched, returned as-is
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $info = $this->bupInfo(0x7); // BUD_OP_DELETEFILE, unmapped
        $this->initUint32($this->addressOf('_var_bupPhase_8c2260d0'), 0x99);

        $this->call('_pollBupOp_8c01bc44')->with(0xdead, $drive);
        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($info);
        $this->shouldCall('_buStat')->with($drive)->andReturn(0xffffffff);
        $this->shouldReturn(0x99);
    }

    public function test_op_nop_polls_again()
    {
        // busy with NOP op: keep polling; second poll reports complete
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $infoNop = $this->bupInfo(0x0);
        $this->initUint32($this->addressOf('_var_bupPhase_8c2260d0'), 0xffffffff); // -1

        $this->call('_pollBupOp_8c01bc44')->with(0xdead, $drive);
        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($infoNop);
        $this->shouldCall('_buStat')->with($drive)->andReturn(0xffffffff); // busy, op NOP
        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($infoNop);
        $this->shouldCall('_buStat')->with($drive)->andReturn(0); // complete
        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldReturn(0);
    }

    public function test_done_error_phase_resets()
    {
        // complete, phase already -1 -> reset to 0
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $info = $this->bupInfo(0);
        $this->initUint32($this->addressOf('_var_bupPhase_8c2260d0'), 0xffffffff);

        $this->call('_pollBupOp_8c01bc44')->with(0xdead, $drive);
        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($info);
        $this->shouldCall('_buStat')->with($drive)->andReturn(0);
        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldReturn(0);
    }

    public function test_done_saveexec_ok()
    {
        $this->runDoneOk(0x12, 0x13);
    }

    public function test_done_defrag_ok()
    {
        $this->runDoneOk(0x14, 0x15);
    }

    public function test_done_loadfileex_ok()
    {
        $this->runDoneOk(0x16, 0x17);
    }

    public function test_done_rewriteexec_ok()
    {
        $this->runDoneOk(0x18, 0x19);
    }

    private function runDoneOk(int $phaseIn, int $phaseOut): void
    {
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $info = $this->bupInfo(0);
        $this->initUint32($this->addressOf('_var_bupPhase_8c2260d0'), $phaseIn);

        $this->call('_pollBupOp_8c01bc44')->with(0xdead, $drive);
        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($info);
        $this->shouldCall('_buStat')->with($drive)->andReturn(0);
        $this->shouldCall('_buGetLastError')->with($drive)->andReturn(0);
        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), $phaseOut);
        $this->shouldReturn($phaseOut);
    }

    public function test_done_saveexec_error()
    {
        $this->runDoneError(0x12);
    }

    public function test_done_rewriteexec_error()
    {
        $this->runDoneError(0x18);
    }

    private function runDoneError(int $phaseIn): void
    {
        // op finished but buGetLastError != 0 -> phase -1
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $info = $this->bupInfo(0);
        $this->initUint32($this->addressOf('_var_bupPhase_8c2260d0'), $phaseIn);

        $this->call('_pollBupOp_8c01bc44')->with(0xdead, $drive);
        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($info);
        $this->shouldCall('_buStat')->with($drive)->andReturn(0);
        $this->shouldCall('_buGetLastError')->with($drive)->andReturn(1);
        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0xffffffff);
        $this->shouldReturn(0xffffffff);
    }

    public function test_done_ok_phase_resets()
    {
        // complete, phase already an "OK" value (0x13) -> reset to 0
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $info = $this->bupInfo(0);
        $this->initUint32($this->addressOf('_var_bupPhase_8c2260d0'), 0x13);

        $this->call('_pollBupOp_8c01bc44')->with(0xdead, $drive);
        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($info);
        $this->shouldCall('_buStat')->with($drive)->andReturn(0);
        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldReturn(0);
    }

    public function test_done_unmapped_phase_unchanged()
    {
        // complete, phase not a tracked value -> returned unchanged, no write
        $this->resolveSymbols();
        $drive = random_int(0, 7);
        $info = $this->bupInfo(0);
        $this->initUint32($this->addressOf('_var_bupPhase_8c2260d0'), 0x55);

        $this->call('_pollBupOp_8c01bc44')->with(0xdead, $drive);
        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($info);
        $this->shouldCall('_buStat')->with($drive)->andReturn(0);
        $this->shouldReturn(0x55);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_BupGetInfo_8c014bba', 4);
        $this->setSize('_buStat', 4);
        $this->setSize('_buGetLastError', 4);
    }
};
