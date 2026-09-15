<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// LZW-encodes `size` bytes into **dest: a 4-byte size header, then one code
// per longest matching dictionary run.
return new class extends TestCase {
    public function test_single_byte_no_dictionary_match(): void
    {
        $this->resolveSymbols();

        $srcBuf = $this->alloc(2);
        $this->initUint8($srcBuf, 0x41);
        $this->initUint8($srcBuf + 1, 0x00);

        $destBuf = $this->alloc(8);
        $destSlot = $this->alloc(4);
        $this->initUint32($destSlot, $destBuf);

        $this->call('_ReplayCodecPack_8c02f934')->with($srcBuf, $destSlot, 1);

        $this->shouldCall('_ReplayCodecInit_8c02f320');
        $this->shouldCall('_memcpy')->andReturn($destBuf);
        $this->shouldCall('_initTables_8c02f704');
        $this->shouldWriteWord($this->addressOf('_var_runBuf_8c235bb4'), 0x41);
        $this->shouldCall('_lzwFindChild_8c02f636')->with(0x41, 0x00)->andReturn(0x1000);
        $this->shouldCall('_writeCode_8c02f824');
        $this->shouldCall('_extendDict_8c02f740');
        $this->shouldCall('_putBits_8c02f4da');
        $this->shouldWriteLong($destSlot, $destBuf + 4);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_memcpy', 4);
    }
};
