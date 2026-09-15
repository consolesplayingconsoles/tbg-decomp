<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Reads a multi-bit (MSB-first) value from the packed byte stream, refilling
// one byte at a time as needed.
return new class extends TestCase {
    public function test_no_refill_reads_from_current_buffer(): void
    {
        $this->resolveSymbols();

        // getlen=8, want 3 bits -> no refill needed. bac=0b11010110, top 3 bits = 110 = 6.
        $this->initUint16($this->addressOf('_var_readBitsLeft_8c228ba8'), 8);
        $this->initUint16($this->addressOf('_var_bitBuf_8c228bac'), 0xD6);

        $srcBuf = $this->alloc(4);
        $srcSlot = $this->alloc(4);
        $this->initUint32($srcSlot, $srcBuf);

        $countVal = $this->alloc(4);
        $this->initUint32($countVal, 0);
        $countSlot = $this->alloc(4);
        $this->initUint32($countSlot, $countVal);

        $this->call('_getBits_8c02f3e0')->with(3, $srcSlot, $countSlot);

        $this->shouldWriteWord($this->addressOf('_var_readBitsLeft_8c228ba8'), 5);
        $this->shouldWriteLong($srcSlot, $srcBuf);
        $this->shouldWriteLong($countSlot, $countVal);
        $this->shouldReturn(6);
    }

    public function test_refill_spans_two_bytes(): void
    {
        $this->resolveSymbols();

        // getlen=2, want 5 bits -> needs one refill.
        // First byte remaining low 2 bits of bac=0b01 contribute the top 2
        // bits of the result; new byte 0b10110xxx contributes bits.
        $this->initUint16($this->addressOf('_var_readBitsLeft_8c228ba8'), 2);
        $this->initUint16($this->addressOf('_var_bitBuf_8c228bac'), 0x01);

        $srcBuf = $this->alloc(4);
        $this->initUint8($srcBuf, 0xB4); // 1011 0100

        $srcSlot = $this->alloc(4);
        $this->initUint32($srcSlot, $srcBuf);

        $countVal = $this->alloc(4);
        $this->initUint32($countVal, 10);
        $countSlot = $this->alloc(4);
        $this->initUint32($countSlot, $countVal);

        $this->call('_getBits_8c02f3e0')->with(5, $srcSlot, $countSlot);

        $this->shouldWriteWord($this->addressOf('_var_bitBuf_8c228bac'), 0xB4);
        $this->shouldWriteLong($countVal, 11);
        $this->shouldWriteWord($this->addressOf('_var_readBitsLeft_8c228ba8'), 8);
        // getlen(8) -= remaining count(3) -> 5; result = (old low 2 bits '01' << 3) | ((new byte >> 5) & 7)
        $this->shouldWriteWord($this->addressOf('_var_readBitsLeft_8c228ba8'), 5);
        $this->shouldWriteLong($srcSlot, $srcBuf + 1);
        $this->shouldWriteLong($countSlot, $countVal);
        $this->shouldReturn((0x01 << 3) | ((0xB4 >> 5) & 7));
    }

    private function resolveSymbols(): void
    {
    }
};
