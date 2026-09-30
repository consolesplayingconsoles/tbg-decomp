<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_fadeDrawCommandCount_8c226570', 12);
        $this->setSize('_var_fadeDrawCommands_8c22657c', 3 * 0x800);
        $this->setSize('_someCallback2_8c012345', 4);
    }

    public function test_queues_entry(): void {
        $this->resolveSymbols();

        $countBase = $this->addressOf('_var_fadeDrawCommandCount_8c226570');
        $this->initUint32($countBase, 0);
        $this->initUint32($countBase + 4, 0);
        $this->initUint32($countBase + 8, 5);

        $cmdBase = $this->addressOf('_var_fadeDrawCommands_8c22657c') + 2 * 0x800 + 5 * 0x10;

        $this->call('_FadePushCall2_8c022420')
            ->with(2, $this->addressOf('_someCallback2_8c012345'), 11, 22);

        $this->shouldWriteLong($cmdBase, 6);
        $this->shouldWriteLong($cmdBase + 4, $this->addressOf('_someCallback2_8c012345'));
        $this->shouldWriteLong($cmdBase + 8, 11);
        $this->shouldWriteLong($cmdBase + 12, 22);
        $this->shouldWriteLong($countBase + 8, 6);
    }

    public function test_drops_entry_when_layer_full(): void {
        $this->resolveSymbols();

        $countBase = $this->addressOf('_var_fadeDrawCommandCount_8c226570');
        $this->initUint32($countBase, 0);
        $this->initUint32($countBase + 4, 0);
        $this->initUint32($countBase + 8, 0x80);

        $this->call('_FadePushCall2_8c022420')
            ->with(2, $this->addressOf('_someCallback2_8c012345'), 11, 22);

        $this->forceStop();
    }
};
