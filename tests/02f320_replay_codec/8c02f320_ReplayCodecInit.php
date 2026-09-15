<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Resets ReplayCodec's scratch state before a pack/unpack pass.
return new class extends TestCase {
    public function test_resets_state(): void
    {
        $this->resolveSymbols();

        $this->call('_ReplayCodecInit_8c02f320');

        $this->shouldWriteLong($this->addressOf('_var_replayPackedSize_8c228ba4'), 0);
        $this->shouldWriteWord($this->addressOf('_var_readBitsLeft_8c228ba8'), 0);
        $this->shouldWriteWord($this->addressOf('_var_writeBitsLeft_8c228baa'), 8);
        $this->shouldWriteWord($this->addressOf('_var_bitBuf_8c228bac'), 0);
        $this->shouldWriteWord($this->addressOf('_var_lruHead_8c235bb0'), 0x1000);
        $this->shouldWriteWord($this->addressOf('_var_lruTail_8c235bb2'), 0x1000);
        $this->shouldWriteWord($this->addressOf('_var_codeBits_8c235c7c'), 1);
        $this->shouldWriteWord($this->addressOf('_var_codeLimit_8c235c7e'), 2);
        $this->shouldWriteWord($this->addressOf('_var_nextCode_8c231bae'), 0x100);
        $this->shouldCall('_memset')
            ->with($this->addressOf('_var_dictByte_8c228bae'), 0, 0x1000)
            ->andReturn($this->addressOf('_var_dictByte_8c228bae'));
        $this->shouldCall('_memset')
            ->with($this->addressOf('_var_dictParent_8c229bae'), 0, 0x1000)
            ->andReturn($this->addressOf('_var_dictParent_8c229bae'));
        $this->shouldCall('_memset')
            ->with($this->addressOf('_var_dictChild_8c22bbae'), 0, 0x1000)
            ->andReturn($this->addressOf('_var_dictChild_8c22bbae'));
        $this->shouldCall('_memset')
            ->with($this->addressOf('_var_dictNext_8c22dbae'), 0, 0x1000)
            ->andReturn($this->addressOf('_var_dictNext_8c22dbae'));
        $this->shouldCall('_memset')
            ->with($this->addressOf('_var_dictPrev_8c22fbae'), 0, 0x1000)
            ->andReturn($this->addressOf('_var_dictPrev_8c22fbae'));
        $this->shouldCall('_memset')
            ->with($this->addressOf('_var_lruPrev_8c231bb0'), 0, 0x1000)
            ->andReturn($this->addressOf('_var_lruPrev_8c231bb0'));
        $this->shouldCall('_memset')
            ->with($this->addressOf('_var_lruNext_8c233bb0'), 0, 0x1000)
            ->andReturn($this->addressOf('_var_lruNext_8c233bb0'));
        $this->shouldCall('_memset')
            ->with($this->addressOf('_var_runBuf_8c235bb4'), 0, 100)
            ->andReturn($this->addressOf('_var_runBuf_8c235bb4'));
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_memset', 4);
    }
};
