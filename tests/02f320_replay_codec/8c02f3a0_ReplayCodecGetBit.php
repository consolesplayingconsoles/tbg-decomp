<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Reads one bit (MSB-first) from the packed byte stream.
return new class extends TestCase {
    public function test_no_refill_returns_bit_from_buffer(): void
    {
        $this->resolveSymbols();

        // getlen=4 -> decrements to 3, no refill; bac bit 3 extracted.
        $this->initUint16($this->addressOf('_var_readBitsLeft_8c228ba8'), 4);
        $this->initUint16($this->addressOf('_var_bitBuf_8c228bac'), 0x0008); // bit 3 set

        $srcBuf = $this->alloc(4);
        $srcSlot = $this->alloc(4);
        $this->initUint32($srcSlot, $srcBuf);

        $countVal = $this->alloc(4);
        $this->initUint32($countVal, 0);
        $countSlot = $this->alloc(4);
        $this->initUint32($countSlot, $countVal);

        $this->call('_getBit_8c02f3a0')->with($srcSlot, $countSlot);

        $this->shouldWriteWord($this->addressOf('_var_readBitsLeft_8c228ba8'), 3);
        $this->shouldReturn(1);
    }

    public function test_refill_reads_next_byte(): void
    {
        $this->resolveSymbols();

        // getlen=0 -> decrements to -1, triggers refill.
        $this->initUint16($this->addressOf('_var_readBitsLeft_8c228ba8'), 0);
        $this->initUint16($this->addressOf('_var_bitBuf_8c228bac'), 0);

        $srcBuf = $this->alloc(4);
        $this->initUint8($srcBuf, 0x80); // MSB set

        $srcSlot = $this->alloc(4);
        $this->initUint32($srcSlot, $srcBuf);

        $countVal = $this->alloc(4);
        $this->initUint32($countVal, 5);
        $countSlot = $this->alloc(4);
        $this->initUint32($countSlot, $countVal);

        $this->call('_getBit_8c02f3a0')->with($srcSlot, $countSlot);

        $this->shouldWriteWord($this->addressOf('_var_readBitsLeft_8c228ba8'), 0xFFFF);
        $this->shouldWriteWord($this->addressOf('_var_readBitsLeft_8c228ba8'), 7);
        $this->shouldWriteWord($this->addressOf('_var_bitBuf_8c228bac'), 0x80);
        $this->shouldWriteLong($countVal, 6);
        $this->shouldWriteLong($srcSlot, $srcBuf + 1);
        $this->shouldWriteLong($countSlot, $countVal);
        $this->shouldReturn(1);
    }

    private function resolveSymbols(): void
    {
    }
};
