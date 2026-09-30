<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_ok()
    {
        $this->resolveSymbols();

        $drive = random_int(0, 7);
        $fname = $this->allocString('TOKYOBUS._VM');
        $buf = $this->alloc(4);
        $start = random_int(0, 100);
        $nblock = random_int(1, 100);

        $this->call('_rewriteExecFile_8c01be60')->with($drive, $fname, $buf, $start, $nblock);

        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldCall('_buRewriteExecFile')
            ->with($drive, $fname, $buf, $start, $nblock)
            ->andReturn(0);

        $this->shouldReturn(0);
    }

    public function test_error()
    {
        $this->resolveSymbols();

        $drive = random_int(0, 7);
        $fname = $this->allocString('TOKYOBUS._VM');
        $buf = $this->alloc(4);
        $start = random_int(0, 100);
        $nblock = random_int(1, 100);

        $this->call('_rewriteExecFile_8c01be60')->with($drive, $fname, $buf, $start, $nblock);

        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldCall('_buRewriteExecFile')
            ->with($drive, $fname, $buf, $start, $nblock)
            ->andReturn(0xffffffff);

        $this->shouldReturn(0xffffffff);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_buRewriteExecFile', 4);
    }
};
