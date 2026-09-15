<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Writes one code: a literal byte if < 0x100, or an escalating-width value
// (code - 0x100) after growing the bit-width to match the dictionary size.
return new class extends TestCase {
    public function test_literal_byte(): void
    {
        $this->resolveSymbols();

        $destBuf = $this->alloc(4);
        $destSlot = $this->alloc(4);
        $this->initUint32($destSlot, $destBuf);

        $this->call('_writeCode_8c02f824')->with(0x42, $destSlot);

        $this->shouldCall('_putBit_8c02f49c')->andReturn(0);
        $this->shouldCall('_putBits_8c02f4da')->andReturn(0);
        $this->shouldWriteLong($destSlot, $destBuf);
    }

    public function test_escalated_code_no_width_growth(): void
    {
        $this->resolveSymbols();

        // c7e(0x200) > bae(0x100)-0x100(0) -> no growth.
        $this->initUint16($this->addressOf('_var_nextCode_8c231bae'), 0x100);
        $this->initUint16($this->addressOf('_var_codeLimit_8c235c7e'), 0x200);
        $this->initUint16($this->addressOf('_var_codeBits_8c235c7c'), 9);

        $destBuf = $this->alloc(4);
        $destSlot = $this->alloc(4);
        $this->initUint32($destSlot, $destBuf);

        $this->call('_writeCode_8c02f824')->with(0x150, $destSlot);

        $this->shouldCall('_putBit_8c02f49c')->andReturn(0);
        $this->shouldCall('_putBits_8c02f4da')->andReturn(0);
        $this->shouldWriteLong($destSlot, $destBuf);
    }

    public function test_escalated_code_grows_width(): void
    {
        $this->resolveSymbols();

        // c7e(0x80) <= bae(0x200)-0x100(0x100) -> grows twice: 0x80->0x100 (still <=0x100) ->0x200.
        $this->initUint16($this->addressOf('_var_nextCode_8c231bae'), 0x200);
        $this->initUint16($this->addressOf('_var_codeLimit_8c235c7e'), 0x80);
        $this->initUint16($this->addressOf('_var_codeBits_8c235c7c'), 8);

        $destBuf = $this->alloc(4);
        $destSlot = $this->alloc(4);
        $this->initUint32($destSlot, $destBuf);

        $this->call('_writeCode_8c02f824')->with(0x150, $destSlot);

        $this->shouldWriteWord($this->addressOf('_var_codeBits_8c235c7c'), 9);
        $this->shouldWriteWord($this->addressOf('_var_codeLimit_8c235c7e'), 0x100);
        $this->shouldWriteWord($this->addressOf('_var_codeBits_8c235c7c'), 10);
        $this->shouldWriteWord($this->addressOf('_var_codeLimit_8c235c7e'), 0x200);
        $this->shouldCall('_putBit_8c02f49c')->andReturn(0);
        $this->shouldCall('_putBits_8c02f4da')->andReturn(0);
        $this->shouldWriteLong($destSlot, $destBuf);
    }

    private function resolveSymbols(): void
    {
    }
};
