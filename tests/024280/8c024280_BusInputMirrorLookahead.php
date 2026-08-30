<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// BusInputMirrorLookahead_8c024280: called once per frame from BusTask_8c022bdc when
// BusState.mirror_0x268 is set. Computes lookahead = max(busSpeed -
// 0.18518517911434174, 0), then scans var_tasks_8c1bac28 (skipping the -1
// sentinel, stopping at the zero-action terminator) and, for each traffic
// entry with mirrorVisible_0x268 set whose heading_0x250 is within 0x4000 (~90
// degrees) of the bus's own ang_0x250, writes lookahead to that entry's
// field_0x418.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_busState_8c1bb9d0', 0x3c8);
        $this->setSize('_var_tasks_8c1bac28', 4 * 0x20);
    }

    private function makeTask(int $index, int $action, int $state): int {
        $task = $this->addressOf('_var_tasks_8c1bac28') + $index * 0x20;
        $this->initUint32($task + 0x00, $action);
        $this->initUint32($task + 0x04, $state);
        return $task;
    }

    // entry = { ..., heading_0x250 @ 0x250, mirrorVisible_0x268 @ 0x268, field_0x418 @ 0x418 }
    private function makeEntry(int $field0x268, int $heading0x250): int {
        $entry = $this->alloc(0x420);
        $this->initUint32($entry + 0x268, $field0x268);
        $this->initUint32($entry + 0x250, $heading0x250);
        return $entry;
    }

    private function setBusSpeedAndHeading(float $speed, int $heading): void {
        $bus = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($bus + 0x27c, unpack('L', pack('f', $speed))[1]);
        $this->initUint32($bus + 0x250, $heading);
    }

    public function test_emptyArray_doesNothing(): void {
        $this->resolveSymbols();
        $this->setBusSpeedAndHeading(1.0, 0);
        $this->makeTask(0, 0, 0); // terminator

        $this->call('_BusInputMirrorLookahead_8c024280');
    }

    public function test_negativeOneSentinel_isSkipped(): void {
        $this->resolveSymbols();
        $this->setBusSpeedAndHeading(1.0, 0);
        $this->makeTask(0, -1, 0xdeadbeef);
        $this->makeTask(1, 0, 0);

        $this->call('_BusInputMirrorLookahead_8c024280');
    }

    public function test_entryWithFlagClear_isSkipped(): void {
        $this->resolveSymbols();
        $this->setBusSpeedAndHeading(1.0, 0);

        $entry = $this->makeEntry(0, 0);
        $this->makeTask(0, 1, $entry);
        $this->makeTask(1, 0, 0);

        $this->call('_BusInputMirrorLookahead_8c024280');
    }

    // Heading exactly 0x4000 away is not "within" the cone (strict <).
    public function test_entryAtHeadingThreshold_isSkipped(): void {
        $this->resolveSymbols();
        $this->setBusSpeedAndHeading(1.0, 0);

        $entry = $this->makeEntry(1, 0x4000);
        $this->makeTask(0, 1, $entry);
        $this->makeTask(1, 0, 0);

        $this->call('_BusInputMirrorLookahead_8c024280');
    }

    public function test_entryWithinCone_getsLookahead(): void {
        $this->resolveSymbols();
        $this->setBusSpeedAndHeading(1.0, 0x100);

        $entry = $this->makeEntry(1, 0);
        $this->makeTask(0, 1, $entry);
        $this->makeTask(1, 0, 0);

        $this->call('_BusInputMirrorLookahead_8c024280');

        $this->shouldWriteFloat($entry + 0x418, 1.0 - 0.18518517911434174);
    }

    // Negative heading diff also hits the cone (abs value).
    public function test_entryWithinCone_negativeDiff_getsLookahead(): void {
        $this->resolveSymbols();
        $this->setBusSpeedAndHeading(1.0, -0x100);

        $entry = $this->makeEntry(1, 0);
        $this->makeTask(0, 1, $entry);
        $this->makeTask(1, 0, 0);

        $this->call('_BusInputMirrorLookahead_8c024280');

        $this->shouldWriteFloat($entry + 0x418, 1.0 - 0.18518517911434174);
    }

    // Low speed clamps lookahead to 0.
    public function test_lowSpeed_clampsToZero(): void {
        $this->resolveSymbols();
        $this->setBusSpeedAndHeading(0.05, 0);

        $entry = $this->makeEntry(1, 0);
        $this->makeTask(0, 1, $entry);
        $this->makeTask(1, 0, 0);

        $this->call('_BusInputMirrorLookahead_8c024280');

        $this->shouldWriteFloat($entry + 0x418, 0.0);
    }
};
