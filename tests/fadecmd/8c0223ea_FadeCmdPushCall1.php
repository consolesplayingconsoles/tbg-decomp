<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_fadeDrawCommandCount_8c226570', 12);
        $this->setSize('_var_fadeDrawCommands_8c22657c', 3 * 0x800);
        $this->setSize('_someCallback1_8c012345', 4);
    }

    public function test_queues_entry(): void {
        $this->resolveSymbols();

        $countBase = $this->addressOf('_var_fadeDrawCommandCount_8c226570');
        $this->initUint32($countBase, 0);
        $this->initUint32($countBase + 4, 3);
        $this->initUint32($countBase + 8, 0);

        $cmdBase = $this->addressOf('_var_fadeDrawCommands_8c22657c') + 1 * 0x800 + 3 * 0x10;

        $this->call('_FadeCmdPushCall1_8c0223ea')
            ->with(1, $this->addressOf('_someCallback1_8c012345'), 42);

        $this->shouldWriteLong($cmdBase, 5);
        $this->shouldWriteLong($cmdBase + 4, $this->addressOf('_someCallback1_8c012345'));
        $this->shouldWriteLong($cmdBase + 8, 42);
        $this->shouldWriteLong($countBase + 4, 4);
    }

    public function test_drops_entry_when_layer_full(): void {
        $this->resolveSymbols();

        $countBase = $this->addressOf('_var_fadeDrawCommandCount_8c226570');
        $this->initUint32($countBase, 0);
        $this->initUint32($countBase + 4, 0x80);
        $this->initUint32($countBase + 8, 0);

        $this->call('_FadeCmdPushCall1_8c0223ea')
            ->with(1, $this->addressOf('_someCallback1_8c012345'), 42);

        $this->forceStop();
    }
};
