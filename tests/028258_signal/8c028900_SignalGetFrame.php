<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_reads_entry_at_index()
    {
        $table = $this->alloc(4 * 4);
        $this->initUint32($table + 0 * 4, 0x11111111);
        $this->initUint32($table + 1 * 4, 0x22222222);
        $this->initUint32($table + 2 * 4, 0x33333333);
        $this->initUint32($table + 3 * 4, 0x44444444);
        $this->initUint32($this->addressOf('_var_trafficSignalFrames_8c227e24'), $table);

        $this->call('_SignalGetFrame_8c028900')->with(2);

        $this->shouldReturn(0x33333333);
    }
};
