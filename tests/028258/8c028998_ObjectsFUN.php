<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_reads_entry_at_index()
    {
        $base = $this->addressOf('_var_pedCrossingFlags_8c227e2c');
        $this->initUint32($base + 3 * 4, 0x77777777);

        $this->call('_ObjectsFUN_8c028998')->with(3);

        $this->shouldReturn(0x77777777);
    }
};
