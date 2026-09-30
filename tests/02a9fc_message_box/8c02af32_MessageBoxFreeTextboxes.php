<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // var_messageTextBoxA_8c1bc404/408/40c are consecutive words in the real binary
    // (accessed as (&var_messageTextBoxA_8c1bc404)[idx] elsewhere in this unit); force the
    // same adjacency here since the linker doesn't place separately
    // declared symbols next to each other by default.
    private function resolveSymbols(): void
    {
        $this->setSize('_TxtDestroyTextBox_8c015410', 4);
        $this->setSize('_TxtDestroy_8c01529c', 4);

        $base = $this->addressOf('_var_messageTextBoxA_8c1bc404');
        $this->rellocate('_var_messageTextBoxB_8c1bc408', $base + 4);
        $this->rellocate('_var_messageTextBoxIndex_8c1bc40c', $base + 8);
    }

    public function test_already_freed_is_a_noop(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_messageTextBoxA_8c1bc404'), 0xffffffff);
        $this->initUint32($this->addressOf('_var_messageTextBoxB_8c1bc408'), 0xffffffff);
        $this->initUint32($this->addressOf('_var_messageTextBoxIndex_8c1bc40c'), 0);

        $this->call('_MessageBoxFreeTextboxes_8c02af32');
    }

    public function test_existing_pair_is_destroyed(): void
    {
        $this->resolveSymbols();

        $box0 = $this->alloc(4);
        $box1 = $this->alloc(4);

        $this->initUint32($this->addressOf('_var_messageTextBoxA_8c1bc404'), $box0);
        $this->initUint32($this->addressOf('_var_messageTextBoxB_8c1bc408'), $box1);
        $this->initUint32($this->addressOf('_var_messageTextBoxIndex_8c1bc40c'), 1);

        $this->call('_MessageBoxFreeTextboxes_8c02af32');

        $boxBAddr = $this->addressOf('_var_messageTextBoxB_8c1bc408');
        $indexAddr = $this->addressOf('_var_messageTextBoxIndex_8c1bc40c');

        $this->shouldCall('_TxtDestroyTextBox_8c015410')->with($box0);
        $this->shouldCall('_TxtDestroyTextBox_8c015410')->with($box1);
        $this->shouldCall('_TxtDestroy_8c01529c')
            ->do(function () use ($boxBAddr, $box1, $indexAddr) {
                // Bug-for-bug with the original asm: only A is reset. B and
                // Index are intentionally left stale.
                $b = $this->memory->readUInt32($boxBAddr)->value;
                $index = $this->memory->readUInt32($indexAddr)->value;
                if ($b !== $box1 || $index !== 1) {
                    throw new RuntimeException(sprintf(
                        'var_messageTextBoxB/Index were reset (%08x/%d); only A should be',
                        $b, $index,
                    ));
                }
            });
        $this->shouldWriteLongTo('_var_messageTextBoxA_8c1bc404', 0xffffffff);
    }
};
