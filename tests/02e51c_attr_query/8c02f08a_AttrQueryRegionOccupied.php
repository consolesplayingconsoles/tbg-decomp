<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// AttrQueryRegionOccupied_8c02f08a scans var_tasks_8c1bac28 for a task (other than self, and
// skipping the -1 sentinel) whose state's signalId_0x410 equals value. If none
// match by the zero-action terminator, it falls back to comparing value
// against var_busState_8c1bb9d0.fallbackTaskMatchId_0x3a0 (imported by the original asm
// under its own symbol, var_8c1bbd70, but that address actually lands
// inside var_busState_8c1bb9d0 -- a linker coincidence).

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_tasks_8c1bac28', 4 * 0x20);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        // The original asm imports BusState.fallbackTaskMatchId_0x3a0 under its own
        // symbol, var_8c1bbd70 -- a linker coincidence, not a real global.
        $this->rellocate('_var_8c1bbd70', $this->addressOf('_var_busState_8c1bb9d0') + 0x3a0);
    }

    private function makeTask(int $index, int $action, int $state): int {
        $task = $this->addressOf('_var_tasks_8c1bac28') + $index * 0x20;
        $this->initUint32($task + 0x00, $action);
        $this->initUint32($task + 0x04, $state);
        return $task;
    }

    private function makeEntry(int $field0x410): int {
        $entry = $this->alloc(0x414);
        $this->initUint32($entry + 0x410, $field0x410);
        return $entry;
    }

    // Empty scan, and the fallback busState field doesn't match either.
    public function test_emptyArray_noFallbackMatch_returnsZero(): void {
        $this->resolveSymbols();

        $self = $this->alloc(4);
        $this->makeTask(0, 0, 0); // terminator
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x3a0, 42);

        $this->call('_AttrQueryRegionOccupied_8c02f08a')->with($self, 7);

        $this->shouldReturn(0);
    }

    // Empty scan, but the fallback busState field matches value.
    public function test_emptyArray_fallbackMatches_returnsOne(): void {
        $this->resolveSymbols();

        $self = $this->alloc(4);
        $this->makeTask(0, 0, 0); // terminator
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x3a0, 7);

        $this->call('_AttrQueryRegionOccupied_8c02f08a')->with($self, 7);

        $this->shouldReturn(1);
    }

    // A matching task is skipped when it is self, even though its
    // signalId_0x410 equals value; the scan continues to the terminator and
    // falls through to the (non-matching) busState comparison.
    public function test_selfTask_isSkipped(): void {
        $this->resolveSymbols();

        $entry = $this->makeEntry(7);
        $self = $this->makeTask(0, 1, $entry);
        $this->makeTask(1, 0, 0); // terminator
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x3a0, 42);

        $this->call('_AttrQueryRegionOccupied_8c02f08a')->with($self, 7);

        $this->shouldReturn(0);
    }

    // A matching task is skipped when its action is the -1 sentinel.
    public function test_negativeOneSentinel_isSkipped(): void {
        $this->resolveSymbols();

        $self = $this->alloc(4);
        $entry = $this->makeEntry(7);
        $this->makeTask(0, -1, $entry); // sentinel
        $this->makeTask(1, 0, 0); // terminator
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x3a0, 42);

        $this->call('_AttrQueryRegionOccupied_8c02f08a')->with($self, 7);

        $this->shouldReturn(0);
    }

    // A real, non-self, non-sentinel task whose state's signalId_0x410
    // matches value: returns 1 without reaching the terminator or the
    // busState fallback.
    public function test_matchingTask_returnsOne(): void {
        $this->resolveSymbols();

        $self = $this->alloc(4);
        $entry = $this->makeEntry(7);
        $this->makeTask(0, 1, $entry);
        $this->makeTask(1, 0, 0); // terminator; must never be reached

        $this->call('_AttrQueryRegionOccupied_8c02f08a')->with($self, 7);

        $this->shouldReturn(1);
    }

    // A real task whose signalId_0x410 does not match value: rejected, scan
    // continues to the terminator, and the busState fallback also fails.
    public function test_nonMatchingTask_fallsThroughToNoMatch(): void {
        $this->resolveSymbols();

        $self = $this->alloc(4);
        $entry = $this->makeEntry(99);
        $this->makeTask(0, 1, $entry);
        $this->makeTask(1, 0, 0); // terminator
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x3a0, 42);

        $this->call('_AttrQueryRegionOccupied_8c02f08a')->with($self, 7);

        $this->shouldReturn(0);
    }
};
