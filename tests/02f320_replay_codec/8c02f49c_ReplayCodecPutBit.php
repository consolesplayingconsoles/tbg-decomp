<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Writes one bit (MSB-first) into the packed output byte stream.
return new class extends TestCase {
    public function test_bit_set_no_flush(): void
    {
        $this->resolveSymbols();

        // putlen=3 -> decrements to 2, no flush.
        $this->initUint16($this->addressOf('_var_8c228baa'), 3);
        $this->initUint16($this->addressOf('_var_8c228bac'), 0);

        $destBuf = $this->alloc(4);
        $destSlot = $this->alloc(4);
        $this->initUint32($destSlot, $destBuf);

        $this->call('_putBit_8c02f49c')->with(1, $destSlot);

        $this->shouldWriteWord($this->addressOf('_var_8c228baa'), 2);
        $this->shouldWriteWord($this->addressOf('_var_8c228bac'), 0x04);
        $this->shouldWriteLong($destSlot, $destBuf);
    }

    public function test_bit_clear_flushes_byte(): void
    {
        $this->resolveSymbols();

        // putlen=1 -> decrements to 0, flush.
        $this->initUint16($this->addressOf('_var_8c228baa'), 1);
        $this->initUint16($this->addressOf('_var_8c228bac'), 0x36);
        $this->initUint32($this->addressOf('_var_8c228ba4'), 9);

        $destBuf = $this->alloc(4);
        $destSlot = $this->alloc(4);
        $this->initUint32($destSlot, $destBuf);

        $this->call('_putBit_8c02f49c')->with(0, $destSlot);

        $this->shouldWriteWord($this->addressOf('_var_8c228baa'), 0);
        $this->shouldWriteByte($destBuf, 0x36);
        $this->shouldWriteWord($this->addressOf('_var_8c228bac'), 0);
        $this->shouldWriteWord($this->addressOf('_var_8c228baa'), 8);
        $this->shouldWriteLong($this->addressOf('_var_8c228ba4'), 10);
        $this->shouldWriteLong($destSlot, $destBuf + 1);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_8c228ba4', 4);
        $this->setSize('_var_8c228baa', 2);
        $this->setSize('_var_8c228bac', 2);
    }
};
