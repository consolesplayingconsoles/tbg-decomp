<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_adds_base_to_third_field_of_each_entry_until_terminator()
    {
        $table = $this->alloc(3 * 4 * 2 + 4);
        $this->initUint32($table + 0x00, 1);
        $this->initUint32($table + 0x04, 0xdeadbeef);
        $this->initUint32($table + 0x08, 0x20);

        $this->initUint32($table + 0x0c, 0);
        $this->initUint32($table + 0x10, 0xdeadbeef);
        $this->initUint32($table + 0x14, 0x30);

        $this->initUint32($table + 0x18, -1); // terminator

        $this->call('_FUN_8c028de8')->with($table);

        $this->shouldWriteLong($table + 0x08, $table + 0x20);
        $this->shouldWriteLong($table + 0x14, $table + 0x30);
    }

    public function test_terminator_only_writes_nothing()
    {
        $table = $this->alloc(4);
        $this->initUint32($table + 0x00, -1); // terminator

        $this->call('_FUN_8c028de8')->with($table);
    }
};
