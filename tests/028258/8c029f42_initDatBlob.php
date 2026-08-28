<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_initializes_fields(): void {
        $obj = $this->alloc(0x20);
        $this->doNotRandomizeMemory();

        $this->call('_initDatBlob_8c029f42')->with($obj, 0x11223344, 0x55667788);

        $this->shouldWriteLong($obj + 0x0c, 0);
        $this->shouldWriteLong($obj + 0x10, 0);
        $this->shouldWriteLong($obj + 0x18, 0x11223344);
        $this->shouldWriteLong($obj + 0x1c, 0x55667788);
        $this->shouldWriteLong($obj + 0x14, $obj + 0x20);
    }
};
