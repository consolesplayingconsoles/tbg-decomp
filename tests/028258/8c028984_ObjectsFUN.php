<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_sets_entry_at_index()
    {
        $base = $this->addressOf('_var_crossingOccupiedFlags_8c22802c');
        $this->initUint32($base + 3 * 4, 0);

        $this->call('_ObjectsFUN_8c028984')->with(3);

        $this->shouldWriteLong($base + 3 * 4, 1);
    }
};
