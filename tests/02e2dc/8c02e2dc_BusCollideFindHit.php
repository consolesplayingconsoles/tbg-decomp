<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// BusCollideFindHit_8c02e2dc: finds the vehicle/pedestrian the player's bus is currently
// bumping into. Transforms the bus's own fixed local box (init_8c04c820)
// into world space via the bus's world matrix, then scans
// var_tasks_8c1bac28 from the start (skipping the -1 sentinel), gating each
// candidate on TrafficEntry.field_0x490 (distance to the bus) being under
// 12.0 world units before paying for the box transform + collision test.
// Returns the first hit's state, or NULL once the scan hits a zero-action
// terminator.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_njCalcPoints', 4);
        $this->setSize('_GeomQuadOverlap_8c020842', 4);
        $this->setSize('_var_busWorldMatrix_8c1bba54', 0x40);
        $this->setSize('_var_tasks_8c1bac28', 4 * 0x20);
        $this->setSize('_var_collideScanCursor_8c228974', 4);
        $this->setSize('_var_collideSelfBox_8c228978', 96);
        $this->setSize('_var_collideCandidateBox_8c2289d8', 96);
    }

    // entry = {..., matrix @ 0x84 (NJS_MATRIX, 0x40 bytes), ..., variantIdx @ 0x2e0, ..., distance @ 0x490}
    private function makeEntry(int $variantIdx, float $distance): int {
        $entry = $this->alloc(0x494);
        $this->initUint32($entry + 0x2e0, $variantIdx);
        $this->initFloat($entry + 0x490, $distance);
        return $entry;
    }

    private function initFloat(int $addr, float $value): void {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    private function setVariantBox(int $idx, int $boxPtr): void {
        $this->initUint32($this->addressOf('_init_8c04c940') + $idx * 4, $boxPtr);
    }

    private function makeTask(int $index, int $action, int $state): int {
        $task = $this->addressOf('_var_tasks_8c1bac28') + $index * 0x20;
        $this->initUint32($task + 0x00, $action);
        $this->initUint32($task + 0x04, $state);
        return $task;
    }

    private function shouldCalcSelfBox(): void {
        $this->shouldCall('_njCalcPoints')->with(
            $this->addressOf('_var_busWorldMatrix_8c1bba54'),
            $this->addressOf('_init_8c04c820'),
            $this->addressOf('_var_collideSelfBox_8c228978'),
            8
        );
    }

    // Empty scan: the very first task slot is already the zero-action
    // terminator. Only the bus's own box transform runs; the cursor is
    // written once, to the array base, before returning NULL.
    public function test_emptyArray_returnsNull(): void {
        $this->resolveSymbols();

        $tasksBase = $this->addressOf('_var_tasks_8c1bac28');
        $this->makeTask(0, 0, 0); // terminator

        $this->call('_BusCollideFindHit_8c02e2dc');

        $this->shouldCalcSelfBox();
        $this->shouldWriteLongTo('_var_collideScanCursor_8c228974', $tasksBase);
        $this->shouldReturn(0);
    }

    // The scanned task's action word is the -1 sentinel: skipped without
    // even reading its distance, then the cursor advances to the real
    // terminator.
    public function test_negativeOneSentinel_isSkipped(): void {
        $this->resolveSymbols();

        $tasksBase = $this->addressOf('_var_tasks_8c1bac28');
        $this->makeTask(0, -1, 0xdeadbeef); // sentinel
        $this->makeTask(1, 0, 0); // terminator

        $this->call('_BusCollideFindHit_8c02e2dc');

        $this->shouldCalcSelfBox();
        $this->shouldWriteLongTo('_var_collideScanCursor_8c228974', $tasksBase);
        $this->shouldWriteLongTo('_var_collideScanCursor_8c228974', $tasksBase + 0x20);
        $this->shouldReturn(0);
    }

    // A candidate at exactly the 12.0 threshold (not strictly under it) is
    // gated out just like a farther one -- the comparison is a strict "<",
    // not "<=" (asm: FCMP/GT tests 12.0 > distance). No candidate box
    // transform or collision test happens for it.
    public function test_candidateAtThreshold_isSkipped(): void {
        $this->resolveSymbols();

        $entry = $this->makeEntry(0, 12.0);
        $this->setVariantBox(0, $this->alloc(0x60));

        $tasksBase = $this->addressOf('_var_tasks_8c1bac28');
        $this->makeTask(0, 1, $entry);
        $this->makeTask(1, 0, 0); // terminator

        $this->call('_BusCollideFindHit_8c02e2dc');

        $this->shouldCalcSelfBox();
        $this->shouldWriteLongTo('_var_collideScanCursor_8c228974', $tasksBase);
        $this->shouldWriteLongTo('_var_collideScanCursor_8c228974', $tasksBase + 0x20);
        $this->shouldReturn(0);
    }

    // A candidate farther than the 12.0 threshold is gated out before any
    // box transform or collision test -- the whole point of the distance
    // pre-filter.
    public function test_farCandidate_isSkipped(): void {
        $this->resolveSymbols();

        $entry = $this->makeEntry(0, 50.0);
        $this->setVariantBox(0, $this->alloc(0x60));

        $tasksBase = $this->addressOf('_var_tasks_8c1bac28');
        $this->makeTask(0, 1, $entry);
        $this->makeTask(1, 0, 0); // terminator

        $this->call('_BusCollideFindHit_8c02e2dc');

        $this->shouldCalcSelfBox();
        $this->shouldWriteLongTo('_var_collideScanCursor_8c228974', $tasksBase);
        $this->shouldWriteLongTo('_var_collideScanCursor_8c228974', $tasksBase + 0x20);
        $this->shouldReturn(0);
    }

    // A near candidate (under the threshold) is checked and found NOT
    // colliding: its box is transformed and GeomQuadOverlap_8c020842 returns FALSE, so
    // the scan continues to the terminator.
    public function test_nearNonCollidingCandidate_isRejected(): void {
        $this->resolveSymbols();

        $boxPtr = $this->alloc(0x60);
        $this->setVariantBox(0, $boxPtr);
        $entry = $this->makeEntry(0, 5.0);

        $tasksBase = $this->addressOf('_var_tasks_8c1bac28');
        $this->makeTask(0, 1, $entry);
        $this->makeTask(1, 0, 0); // terminator

        $this->call('_BusCollideFindHit_8c02e2dc');

        $this->shouldCalcSelfBox();
        $this->shouldWriteLongTo('_var_collideScanCursor_8c228974', $tasksBase);
        $this->shouldCall('_njCalcPoints')->with(
            $entry + 0x84,
            $boxPtr,
            $this->addressOf('_var_collideCandidateBox_8c2289d8'),
            8
        );
        $this->shouldCall('_GeomQuadOverlap_8c020842')->with(
            $this->addressOf('_var_collideSelfBox_8c228978'),
            $this->addressOf('_var_collideCandidateBox_8c2289d8')
        )->andReturn(0);
        $this->shouldWriteLongTo('_var_collideScanCursor_8c228974', $tasksBase + 0x20);
        $this->shouldReturn(0);
    }

    // A near candidate collides: GeomQuadOverlap_8c020842 returns TRUE and the function
    // returns that task's state pointer (the entry) immediately, without
    // advancing further or checking any later task.
    public function test_nearCollidingCandidate_returnsItsState(): void {
        $this->resolveSymbols();

        $boxPtr = $this->alloc(0x60);
        $this->setVariantBox(0, $boxPtr);
        $entry = $this->makeEntry(0, 5.0);

        $tasksBase = $this->addressOf('_var_tasks_8c1bac28');
        $this->makeTask(0, 1, $entry);
        $this->makeTask(1, 0, 0); // terminator; must never be reached

        $this->call('_BusCollideFindHit_8c02e2dc');

        $this->shouldCalcSelfBox();
        $this->shouldWriteLongTo('_var_collideScanCursor_8c228974', $tasksBase);
        $this->shouldCall('_njCalcPoints')->with(
            $entry + 0x84,
            $boxPtr,
            $this->addressOf('_var_collideCandidateBox_8c2289d8'),
            8
        );
        $this->shouldCall('_GeomQuadOverlap_8c020842')->with(
            $this->addressOf('_var_collideSelfBox_8c228978'),
            $this->addressOf('_var_collideCandidateBox_8c2289d8')
        )->andReturn(1);
        $this->shouldReturn($entry);
    }
};
