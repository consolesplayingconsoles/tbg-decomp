<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// LZW-decodes from *src (4-byte decompressed-size header + codes) into
// **dest, expanding each code via its dictionary parent chain.
return new class extends TestCase {
    public function test_single_literal_code_completes_immediately(): void
    {
        $this->resolveSymbols();

        $srcBuf = $this->alloc(8);
        $this->initUint32($srcBuf, 1); // decompressed size = 1 byte

        $destBuf = $this->alloc(8);
        $destSlot = $this->alloc(4);
        $this->initUint32($destSlot, $destBuf);

        // code 0x41 is a literal byte (<=0xff), so the parent-walk runs once.
        $this->initUint8($this->addressOf('_var_8c228bae') + 0x41, 0x41);
        $this->initUint16($this->addressOf('_var_8c229bae') + 0x41 * 2, 0x1000);
        $this->initUint16($this->addressOf('_var_8c231bae'), 0x102);

        $this->call('_ReplayCodecUnpack_8c02fa14')->with($srcBuf, $destSlot, 100);

        $this->shouldCall('_memcpy')->do(function () {
            $dst = $this->registers[4]->value;
            $src = $this->registers[5]->value;
            $len = $this->registers[6]->value;
            for ($i = 0; $i < $len; $i++) {
                $this->memory->writeUInt8($dst + $i, $this->memory->readUInt8($src + $i));
            }
        });
        $this->shouldCall('_ReplayCodecInit_8c02f320');
        $this->shouldCall('_initTables_8c02f704');
        $this->shouldCall('_readCode_8c02f892')->andReturn(0x41);
        $this->shouldWriteWord($this->addressOf('_var_8c235bb4') + 99 * 2, 0x41);
        $this->shouldWriteByte($destBuf, 0x41);
        $this->shouldCall('_extendDict_8c02f740');
        $this->shouldWriteLong($destSlot, $destBuf + 1);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_8c228bae', 0x1000);
        $this->setSize('_var_8c229bae', 0x2000);
        $this->setSize('_var_8c231bae', 2);
        $this->setSize('_var_8c235bb0', 2);
        $this->setSize('_var_8c235bb4', 200);
        $this->setSize('_memcpy', 4);
    }
};
