<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_no_groups(): void
    {
        $handle = $this->alloc(0x04);
        $this->initUint32($handle + 0x00, 0);

        $this->call('_relocateMessageText_8c02a9fc')->with($handle);
    }

    public function test_single_group_immediately_empty_entry(): void
    {
        $handle = $this->alloc(0x20);

        // outer table: one group at offset 0x10, then terminator
        $this->initUint32($handle + 0x00, 0x10);
        $this->initUint32($handle + 0x04, 0);

        // group's single entry: string offset 0x18 (empty string)
        $this->initUint32($handle + 0x10, 0x18);
        $this->initUint8($handle + 0x18, 0);

        $this->call('_relocateMessageText_8c02a9fc')->with($handle);

        $this->shouldWriteLong($handle + 0x00, $handle + 0x10);
        $this->shouldWriteLong($handle + 0x10, $handle + 0x18);
    }

    public function test_multiple_groups_multiple_entries(): void
    {
        $handle = $this->alloc(0x34);

        // outer table: group A at 0x10, group B at 0x28, then terminator
        $this->initUint32($handle + 0x00, 0x10);
        $this->initUint32($handle + 0x04, 0x28);
        $this->initUint32($handle + 0x08, 0);

        // group A: two entries -- first non-empty (string at 0x20), second
        // empty (string at 0x24), stopping the inner loop
        $this->initUint32($handle + 0x10, 0x20);
        $this->initUint32($handle + 0x18, 0x24);
        $this->initUint8($handle + 0x20, 0x41); // 'A'
        $this->initUint8($handle + 0x24, 0);

        // group B: one entry, immediately empty (string at 0x30)
        $this->initUint32($handle + 0x28, 0x30);
        $this->initUint8($handle + 0x30, 0);

        $this->call('_relocateMessageText_8c02a9fc')->with($handle);

        $this->shouldWriteLong($handle + 0x00, $handle + 0x10);
        $this->shouldWriteLong($handle + 0x10, $handle + 0x20);
        $this->shouldWriteLong($handle + 0x18, $handle + 0x24);
        $this->shouldWriteLong($handle + 0x04, $handle + 0x28);
        $this->shouldWriteLong($handle + 0x28, $handle + 0x30);
    }
};
