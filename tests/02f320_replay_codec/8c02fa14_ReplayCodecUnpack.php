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
        $this->initUint8($this->addressOf('_var_dictByte_8c228bae') + 0x41, 0x41);
        $this->initUint16($this->addressOf('_var_dictParent_8c229bae') + 0x41 * 2, 0x1000);
        $this->initUint16($this->addressOf('_var_nextCode_8c231bae'), 0x102);

        $this->call('_ReplayCodecUnpack_8c02fa14')->with($srcBuf, $destSlot, 100);

        // dst (r4) is &headerSize, a callee-local stack slot whose address
        // isn't knowable ahead of the call -- capture it at runtime instead
        // of asserting it. src/len are known, so those are checked directly.
        $this->shouldCall('_memcpy')->do(function () use ($srcBuf) {
            $dst = $this->registers[4]->value;
            $src = $this->registers[5]->value;
            $len = $this->registers[6]->value;

            if ($src !== $srcBuf || $len !== 4) {
                throw new RuntimeException(sprintf(
                    '_memcpy: expected src=0x%08x len=4, got src=0x%08x len=%d',
                    $srcBuf, $src, $len
                ));
            }

            for ($i = 0; $i < $len; $i++) {
                $this->memory->writeUInt8($dst + $i, $this->memory->readUInt8($src + $i));
            }
        });
        $this->shouldCall('_ReplayCodecInit_8c02f320');
        $this->shouldCall('_initTables_8c02f704');
        $this->shouldCall('_readCode_8c02f892')->andReturn(0x41);
        $this->shouldWriteWord($this->addressOf('_var_runBuf_8c235bb4') + 99 * 2, 0x41);
        $this->shouldWriteByte($destBuf, 0x41);
        $this->shouldCall('_extendDict_8c02f740');
        $this->shouldWriteLong($destSlot, $destBuf + 1);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_memcpy', 4);
    }
};
