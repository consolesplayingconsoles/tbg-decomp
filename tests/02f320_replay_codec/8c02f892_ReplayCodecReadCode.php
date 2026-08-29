<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Reads one code from the bitstream: a literal byte if the next bit is 0,
// or 0x100 + an escalating-width value if it's 1. Returns -1 once the byte
// counter exceeds the caller-supplied limit.
return new class extends TestCase {
    public function test_literal_byte_no_threshold(): void
    {
        $this->resolveSymbols();

        // var_8c235c7e(1) > var_8c231bae(0x100)-0x100(0) -> no doubling.
        $this->initUint16($this->addressOf('_var_8c231bae'), 0x100);
        $this->initUint16($this->addressOf('_var_8c235c7e'), 1);
        $this->initUint16($this->addressOf('_var_8c235c7c'), 1);

        $srcBuf = $this->alloc(4);
        $srcSlot = $this->alloc(4);
        $this->initUint32($srcSlot, $srcBuf);

        $countVal = $this->alloc(4);
        $this->initUint32($countVal, 5);

        $this->call('_readCode_8c02f892')->with($srcSlot, 100, $countVal);

        $this->shouldCall('_getBit_8c02f3a0')->andReturn(0);
        $this->shouldCall('_getBits_8c02f3e0')->andReturn(0x77);
        $this->shouldWriteLong($srcSlot, $srcBuf);
        $this->shouldReturn(0x77);
    }

    public function test_doubles_threshold_and_returns_escalated_code(): void
    {
        $this->resolveSymbols();

        // var_8c235c7e(0x80) <= var_8c231bae(0x200)-0x100(0x100) -> doubles.
        $this->initUint16($this->addressOf('_var_8c231bae'), 0x200);
        $this->initUint16($this->addressOf('_var_8c235c7e'), 0x80);
        $this->initUint16($this->addressOf('_var_8c235c7c'), 3);

        $srcBuf = $this->alloc(4);
        $srcSlot = $this->alloc(4);
        $this->initUint32($srcSlot, $srcBuf);

        $countVal = $this->alloc(4);
        $this->initUint32($countVal, 5);

        $this->call('_readCode_8c02f892')->with($srcSlot, 100, $countVal);

        $this->shouldWriteWord($this->addressOf('_var_8c235c7e'), 0x100);
        $this->shouldWriteWord($this->addressOf('_var_8c235c7c'), 4);
        $this->shouldCall('_getBit_8c02f3a0')->andReturn(1);
        $this->shouldCall('_getBits_8c02f3e0')->andReturn(9);
        $this->shouldWriteLong($srcSlot, $srcBuf);
        $this->shouldReturn(0x100 + 9);
    }

    public function test_returns_minus_one_when_over_limit_after_bit(): void
    {
        $this->resolveSymbols();

        $this->initUint16($this->addressOf('_var_8c231bae'), 0x100);
        $this->initUint16($this->addressOf('_var_8c235c7e'), 1);
        $this->initUint16($this->addressOf('_var_8c235c7c'), 1);

        $srcBuf = $this->alloc(4);
        $srcSlot = $this->alloc(4);
        $this->initUint32($srcSlot, $srcBuf);

        // count already over the limit -- no GetBits call.
        $countVal = $this->alloc(4);
        $this->initUint32($countVal, 200);

        $this->call('_readCode_8c02f892')->with($srcSlot, 100, $countVal);

        $this->shouldCall('_getBit_8c02f3a0')->andReturn(0);
        $this->shouldWriteLong($srcSlot, $srcBuf);
        $this->shouldReturn(0xFFFFFFFF);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_8c231bae', 2);
        $this->setSize('_var_8c235c7c', 2);
        $this->setSize('_var_8c235c7e', 2);
    }
};
