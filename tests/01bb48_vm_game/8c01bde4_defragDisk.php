<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_ok()
    {
        $this->resolveSymbols();

        $drive = random_int(0, 7);

        $this->call('_defragDisk_8c01bde4')->with($drive);

        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldCall('_buDefragDisk')
            ->with($drive, $this->addressOf('_var_defragBuf_8c2261a0'))
            ->andReturn(0);

        $this->shouldReturn(0);
    }

    public function test_error()
    {
        $this->resolveSymbols();

        $drive = random_int(0, 7);

        $this->call('_defragDisk_8c01bde4')->with($drive);

        $this->shouldWriteLong($this->addressOf('_var_bupPhase_8c2260d0'), 0);
        $this->shouldCall('_buDefragDisk')
            ->with($drive, $this->addressOf('_var_defragBuf_8c2261a0'))
            ->andReturn(0xffffffff);

        $this->shouldReturn(0xffffffff);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_buDefragDisk', 4);
    }
};
