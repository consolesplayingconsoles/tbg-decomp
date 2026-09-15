<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Writes a multi-bit (MSB-first) value into the packed output byte stream,
// flushing a byte at a time as needed.
return new class extends TestCase {
    public function test_no_flush_fits_in_current_buffer(): void
    {
        $this->resolveSymbols();

        // putlen=8, writing 3 bits value=6 (0b110) -> bac top 3 bits become 110.
        $this->initUint16($this->addressOf('_var_writeBitsLeft_8c228baa'), 8);
        $this->initUint16($this->addressOf('_var_bitBuf_8c228bac'), 0);

        $destBuf = $this->alloc(4);
        $destSlot = $this->alloc(4);
        $this->initUint32($destSlot, $destBuf);

        $this->call('_putBits_8c02f4da')->with(3, 6, $destSlot);

        $this->shouldWriteWord($this->addressOf('_var_writeBitsLeft_8c228baa'), 5);
        $this->shouldWriteWord($this->addressOf('_var_bitBuf_8c228bac'), 0b11000000);
        $this->shouldWriteLong($destSlot, $destBuf);
    }

    public function test_flush_spans_two_bytes(): void
    {
        $this->resolveSymbols();

        // putlen=2, writing 5 bits value=0b01101 (13).
        // First 2 bits (01) fill and flush the current byte;
        // remaining 3 bits (101) start the next byte.
        $this->initUint16($this->addressOf('_var_writeBitsLeft_8c228baa'), 2);
        $this->initUint16($this->addressOf('_var_bitBuf_8c228bac'), 0b01000000);
        $this->initUint32($this->addressOf('_var_replayPackedSize_8c228ba4'), 4);

        $destBuf = $this->alloc(4);
        $destSlot = $this->alloc(4);
        $this->initUint32($destSlot, $destBuf);

        $this->call('_putBits_8c02f4da')->with(5, 0b01101, $destSlot);

        $this->shouldWriteWord($this->addressOf('_var_bitBuf_8c228bac'), 0b01000001);
        $this->shouldWriteByte($destBuf, 0b01000001);
        $this->shouldWriteWord($this->addressOf('_var_bitBuf_8c228bac'), 0);
        $this->shouldWriteWord($this->addressOf('_var_writeBitsLeft_8c228baa'), 8);
        $this->shouldWriteLong($this->addressOf('_var_replayPackedSize_8c228ba4'), 5);
        $this->shouldWriteWord($this->addressOf('_var_writeBitsLeft_8c228baa'), 5);
        $this->shouldWriteWord($this->addressOf('_var_bitBuf_8c228bac'), 0b10100000);
        $this->shouldWriteLong($destSlot, $destBuf + 1);
    }

    private function resolveSymbols(): void
    {
    }
};
