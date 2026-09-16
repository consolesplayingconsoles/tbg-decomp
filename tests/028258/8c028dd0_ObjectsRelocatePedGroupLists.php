<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_adds_base_to_each_nonzero_entry_until_terminator()
    {
        $table = $this->alloc(4 * 4);
        $this->initUint32($table + 0x00, 0x10);
        $this->initUint32($table + 0x04, 0x20);
        $this->initUint32($table + 0x08, 0x30);
        $this->initUint32($table + 0x0c, 0); // terminator

        $this->call('_ObjectsRelocatePedGroupLists_8c028dd0')->with($table);

        $this->shouldWriteLong($table + 0x00, $table + 0x10);
        $this->shouldWriteLong($table + 0x04, $table + 0x20);
        $this->shouldWriteLong($table + 0x08, $table + 0x30);
    }

    public function test_terminator_only_writes_nothing()
    {
        $table = $this->alloc(4);
        $this->initUint32($table + 0x00, 0); // terminator

        $this->call('_ObjectsRelocatePedGroupLists_8c028dd0')->with($table);
    }
};
