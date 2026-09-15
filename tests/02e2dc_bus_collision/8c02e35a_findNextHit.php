<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Dead code -- no caller anywhere in the tree (see docs/next_units.md's
// dead-functions section), kept only for object parity with the original
// binary. A near-copy of FUN_8c02e2dc's scan loop, but it neither
// (re)initializes var_collisionScanCursor_8c228974 nor recomputes the bus's
// own box: it resumes wherever the cursor is already sitting, advancing
// past that entry first (unconditionally, before the terminator check).

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_njCalcPoints', 4);
        $this->setSize('_GeomQuadOverlap_8c020842', 4);
        $this->setSize('_var_tasks_8c1bac28', 4 * 0x20);
        $this->setSize('_var_collisionScanCursor_8c228974', 4);
        $this->setSize('_var_collisionSelfBox_8c228978', 96);
        $this->setSize('_var_collisionCandidateBox_8c2289d8', 96);
    }

    private function initFloat(int $addr, float $value): void {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    private function makeEntry(int $variantIdx, float $distance): int {
        $entry = $this->alloc(0x494);
        $this->initUint32($entry + 0x2e0, $variantIdx);
        $this->initFloat($entry + 0x490, $distance);
        return $entry;
    }

    private function setVariantBox(int $idx, int $boxPtr): void {
        $this->initUint32($this->addressOf('_init_variantBoxes_8c04c940') + $idx * 4, $boxPtr);
    }

    private function makeTask(int $index, int $action, int $state): int {
        $task = $this->addressOf('_var_tasks_8c1bac28') + $index * 0x20;
        $this->initUint32($task + 0x00, $action);
        $this->initUint32($task + 0x04, $state);
        return $task;
    }

    // The cursor starts one slot before a terminator: the unconditional
    // first advance lands directly on it, so the scan finds nothing and no
    // box transform of any kind happens (unlike FUN_8c02e2dc, there is no
    // "self box" setup here at all).
    public function test_advancesPastCurrentSlot_thenEmptyScan_returnsNull(): void {
        $this->resolveSymbols();

        $tasksBase = $this->addressOf('_var_tasks_8c1bac28');
        $this->makeTask(0, 1, 0xdeadbeef); // whatever the cursor is already on
        $this->makeTask(1, 0, 0); // terminator

        $this->initUint32($this->addressOf('_var_collisionScanCursor_8c228974'), $tasksBase);

        $this->call('_findNextHit_8c02e35a');

        $this->shouldWriteLongTo('_var_collisionScanCursor_8c228974', $tasksBase + 0x20);
        $this->shouldReturn(0);
    }

    // Once resumed, a near candidate found colliding returns its state,
    // exactly like FUN_8c02e2dc's tail does.
    public function test_nearCollidingCandidate_returnsItsState(): void {
        $this->resolveSymbols();

        $boxPtr = $this->alloc(0x60);
        $this->setVariantBox(0, $boxPtr);
        $entry = $this->makeEntry(0, 5.0);

        $tasksBase = $this->addressOf('_var_tasks_8c1bac28');
        $this->makeTask(0, 1, 0xdeadbeef); // slot the cursor starts on; only advanced past
        $this->makeTask(1, 1, $entry);
        $this->makeTask(2, 0, 0); // terminator; must never be reached

        $this->initUint32($this->addressOf('_var_collisionScanCursor_8c228974'), $tasksBase);

        $this->call('_findNextHit_8c02e35a');

        $this->shouldWriteLongTo('_var_collisionScanCursor_8c228974', $tasksBase + 0x20);
        $this->shouldCall('_njCalcPoints')->with(
            $entry + 0x84,
            $boxPtr,
            $this->addressOf('_var_collisionCandidateBox_8c2289d8'),
            8
        );
        $this->shouldCall('_GeomQuadOverlap_8c020842')->with(
            $this->addressOf('_var_collisionSelfBox_8c228978'),
            $this->addressOf('_var_collisionCandidateBox_8c2289d8')
        )->andReturn(1);
        $this->shouldReturn($entry);
    }
};
