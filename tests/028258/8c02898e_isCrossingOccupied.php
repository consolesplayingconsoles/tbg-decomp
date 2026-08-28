<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_reads_entry_at_index()
    {
        $base = $this->addressOf('_var_crossingOccupiedFlags_8c22802c');
        $this->initUint32($base + 3 * 4, 0x88888888);

        $this->call('_isCrossingOccupied_8c02898e')->with(3);

        $this->shouldReturn(0x88888888);
    }
};
