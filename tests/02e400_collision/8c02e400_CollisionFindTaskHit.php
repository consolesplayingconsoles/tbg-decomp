<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// CollisionFindTaskHit_8c02e400 transforms self's oriented bounding box into
// world space, then scans var_tasks_8c1bac28 for another task (skipping
// self and any -1-sentinel task) whose own box collides with it, returning
// that task's state pointer -- or NULL once the scan hits a zero-action
// terminator.
//
// Real game bug, preserved (not "fixed"): the first njCalcPoints call
// transforms &init_variantBoxes_8c04c940[selfIdx] itself -- the table SLOT holding the
// box pointer -- not *init_variantBoxes_8c04c940[selfIdx], the box it points to, like the
// candidate path correctly does.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_njCalcPoints', 4);
        $this->setSize('_njCollisionCheckBB', 4);
        $this->setSize('_init_variantBoxes_8c04c940', 16 * 4);
        $this->setSize('_var_tasks_8c1bac28', 4 * 0x20);
    }

    // entry = {..., matrix @ 0x84 (NJS_MATRIX, 0x40 bytes), ..., variantIdx @ 0x2e0}
    private function makeEntry(int $variantIdx): int {
        $entry = $this->alloc(0x2e4);
        $this->initUint32($entry + 0x2e0, $variantIdx);
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

    // Empty scan: the very first task slot is already the zero-action
    // terminator. Only the self box transform runs; the loop cursor is
    // written once, to the array base, before returning NULL.
    public function test_emptyArray_returnsNull(): void {
        $this->resolveSymbols();

        $self = $this->alloc(4); // any pointer distinct from the task array
        $entry = $this->makeEntry(0);
        $this->setVariantBox(0, $this->alloc(0x60));

        $tasksBase = $this->addressOf('_var_tasks_8c1bac28');
        $this->makeTask(0, 0, 0); // terminator

        $this->call('_CollisionFindTaskHit_8c02e400')->with($self, $entry);

        $this->shouldCall('_njCalcPoints')->with(
            $entry + 0x84,
            $this->addressOf('_init_variantBoxes_8c04c940') + 0, // &init_variantBoxes_8c04c940[0] itself -- the bug
            $this->addressOf('_var_collisionSelfBox_8c228978'),
            8
        );
        $this->shouldWriteLongTo('_var_collisionScanCursor_8c228974', $tasksBase);
        $this->shouldReturn(0);
    }

    // The scanned task equals self: skipped regardless of its action word,
    // as long as that word isn't the zero terminator. The cursor still
    // advances past it before hitting the real terminator.
    public function test_selfTask_isSkipped(): void {
        $this->resolveSymbols();

        $entry = $this->makeEntry(0);
        $this->setVariantBox(0, $this->alloc(0x60));

        $tasksBase = $this->addressOf('_var_tasks_8c1bac28');
        $self = $this->makeTask(0, 1, 0xdeadbeef); // valid, nonzero, non -1 action
        $this->makeTask(1, 0, 0); // terminator

        $this->call('_CollisionFindTaskHit_8c02e400')->with($self, $entry);

        $this->shouldCall('_njCalcPoints')->with(
            $entry + 0x84,
            $this->addressOf('_init_variantBoxes_8c04c940') + 0,
            $this->addressOf('_var_collisionSelfBox_8c228978'),
            8
        );
        $this->shouldWriteLongTo('_var_collisionScanCursor_8c228974', $tasksBase);
        $this->shouldWriteLongTo('_var_collisionScanCursor_8c228974', $tasksBase + 0x20);
        $this->shouldReturn(0);
    }

    // The scanned task's action word is the -1 sentinel: skipped even
    // though it isn't self.
    public function test_negativeOneSentinel_isSkipped(): void {
        $this->resolveSymbols();

        $self = $this->alloc(4);
        $entry = $this->makeEntry(0);
        $this->setVariantBox(0, $this->alloc(0x60));

        $tasksBase = $this->addressOf('_var_tasks_8c1bac28');
        $this->makeTask(0, -1, 0xdeadbeef); // sentinel
        $this->makeTask(1, 0, 0); // terminator

        $this->call('_CollisionFindTaskHit_8c02e400')->with($self, $entry);

        $this->shouldCall('_njCalcPoints')->with(
            $entry + 0x84,
            $this->addressOf('_init_variantBoxes_8c04c940') + 0,
            $this->addressOf('_var_collisionSelfBox_8c228978'),
            8
        );
        $this->shouldWriteLongTo('_var_collisionScanCursor_8c228974', $tasksBase);
        $this->shouldWriteLongTo('_var_collisionScanCursor_8c228974', $tasksBase + 0x20);
        $this->shouldReturn(0);
    }

    // A real candidate is checked and found NOT colliding: its box is
    // computed via a real *load* of init_variantBoxes_8c04c940[idx] (no addressing bug
    // here, unlike the self path), njCollisionCheckBB returns 0, and the
    // scan continues to the terminator.
    public function test_nonCollidingCandidate_isRejected(): void {
        $this->resolveSymbols();

        $self = $this->alloc(4);
        $entry = $this->makeEntry(0);
        $this->setVariantBox(0, $this->alloc(0x60));

        $candidateBoxPtr = $this->alloc(0x60);
        $this->setVariantBox(1, $candidateBoxPtr);
        $candidateEntry = $this->makeEntry(1);

        $tasksBase = $this->addressOf('_var_tasks_8c1bac28');
        $this->makeTask(0, 1, $candidateEntry);
        $this->makeTask(1, 0, 0); // terminator

        $this->call('_CollisionFindTaskHit_8c02e400')->with($self, $entry);

        $this->shouldCall('_njCalcPoints')->with(
            $entry + 0x84,
            $this->addressOf('_init_variantBoxes_8c04c940') + 0,
            $this->addressOf('_var_collisionSelfBox_8c228978'),
            8
        );
        $this->shouldWriteLongTo('_var_collisionScanCursor_8c228974', $tasksBase);
        $this->shouldCall('_njCalcPoints')->with(
            $candidateEntry + 0x84,
            $candidateBoxPtr, // real load this time -- the bug is one-sided
            $this->addressOf('_var_collisionCandidateBox_8c2289d8'),
            8
        );
        $this->shouldCall('_njCollisionCheckBB')->with(
            $this->addressOf('_var_collisionSelfBox_8c228978'),
            $this->addressOf('_var_collisionCandidateBox_8c2289d8')
        )->andReturn(0);
        $this->shouldWriteLongTo('_var_collisionScanCursor_8c228974', $tasksBase + 0x20);
        $this->shouldReturn(0);
    }

    // A real candidate collides: njCollisionCheckBB returns nonzero and the
    // function returns that task's state pointer (the entry), without
    // advancing further or checking any later task.
    public function test_collidingCandidate_returnsItsState(): void {
        $this->resolveSymbols();

        $self = $this->alloc(4);
        $entry = $this->makeEntry(0);
        $this->setVariantBox(0, $this->alloc(0x60));

        $candidateBoxPtr = $this->alloc(0x60);
        $this->setVariantBox(1, $candidateBoxPtr);
        $candidateEntry = $this->makeEntry(1);

        $tasksBase = $this->addressOf('_var_tasks_8c1bac28');
        $this->makeTask(0, 1, $candidateEntry);
        $this->makeTask(1, 0, 0); // terminator; must never be reached

        $this->call('_CollisionFindTaskHit_8c02e400')->with($self, $entry);

        $this->shouldCall('_njCalcPoints')->with(
            $entry + 0x84,
            $this->addressOf('_init_variantBoxes_8c04c940') + 0,
            $this->addressOf('_var_collisionSelfBox_8c228978'),
            8
        );
        $this->shouldWriteLongTo('_var_collisionScanCursor_8c228974', $tasksBase);
        $this->shouldCall('_njCalcPoints')->with(
            $candidateEntry + 0x84,
            $candidateBoxPtr,
            $this->addressOf('_var_collisionCandidateBox_8c2289d8'),
            8
        );
        $this->shouldCall('_njCollisionCheckBB')->with(
            $this->addressOf('_var_collisionSelfBox_8c228978'),
            $this->addressOf('_var_collisionCandidateBox_8c2289d8')
        )->andReturn(1);
        $this->shouldReturn($candidateEntry);
    }
};
