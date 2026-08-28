<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// CollideQueueTest_8c02e4ac scans the queue at var_collideQueue_8c228a38,
// from index 0 up to (exclusive) var_collideQueueCount_8c228b38, calling
// njCollisionCheckBS(&var_collideSelfBox_8c228978, queue[i]) on each entry.
// It returns the first entry for which that call is nonzero, or NULL once
// the scan runs out without a hit.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_njCollisionCheckBS', 4);
        $this->setSize('_var_collideSelfBox_8c228978', 96);
        $this->setSize('_var_collideQueueCount_8c228b38', 4);
        $this->setSize('_var_collideQueue_8c228a38', 256);
    }

    private function setQueue(array $entries): void {
        $this->initUint32($this->addressOf('_var_collideQueueCount_8c228b38'), count($entries));
        foreach ($entries as $i => $entry) {
            $this->initUint32($this->addressOf('_var_collideQueue_8c228a38') + $i * 4, $entry);
        }
    }

    // Empty queue: no calls at all, returns NULL.
    public function test_emptyQueue_returnsNull(): void {
        $this->resolveSymbols();

        $this->setQueue([]);

        $this->call('_CollideQueueTest_8c02e4ac');

        $this->shouldReturn(0);
    }

    // Single entry, hit on the very first check: returned immediately.
    public function test_hitAtIndexZero_returnsIt(): void {
        $this->resolveSymbols();

        $entry0 = $this->alloc(4);
        $this->setQueue([$entry0]);

        $this->call('_CollideQueueTest_8c02e4ac');

        $this->shouldCall('_njCollisionCheckBS')
            ->with($this->addressOf('_var_collideSelfBox_8c228978'), $entry0)
            ->andReturn(1);
        $this->shouldReturn($entry0);
    }

    // Full scan, every check rejects: returns NULL after checking every
    // queued entry.
    public function test_allRejected_returnsNull(): void {
        $this->resolveSymbols();

        $entry0 = $this->alloc(4);
        $entry1 = $this->alloc(4);
        $entry2 = $this->alloc(4);
        $this->setQueue([$entry0, $entry1, $entry2]);

        $this->call('_CollideQueueTest_8c02e4ac');

        $this->shouldCall('_njCollisionCheckBS')
            ->with($this->addressOf('_var_collideSelfBox_8c228978'), $entry0)
            ->andReturn(0);
        $this->shouldCall('_njCollisionCheckBS')
            ->with($this->addressOf('_var_collideSelfBox_8c228978'), $entry1)
            ->andReturn(0);
        $this->shouldCall('_njCollisionCheckBS')
            ->with($this->addressOf('_var_collideSelfBox_8c228978'), $entry2)
            ->andReturn(0);
        $this->shouldReturn(0);
    }

    // Hit at a later index: earlier entries are checked and rejected first,
    // pinning the index arithmetic used to reach queue[2].
    public function test_hitAtLaterIndex_earlierRejected(): void {
        $this->resolveSymbols();

        $entry0 = $this->alloc(4);
        $entry1 = $this->alloc(4);
        $entry2 = $this->alloc(4);
        $this->setQueue([$entry0, $entry1, $entry2]);

        $this->call('_CollideQueueTest_8c02e4ac');

        $this->shouldCall('_njCollisionCheckBS')
            ->with($this->addressOf('_var_collideSelfBox_8c228978'), $entry0)
            ->andReturn(0);
        $this->shouldCall('_njCollisionCheckBS')
            ->with($this->addressOf('_var_collideSelfBox_8c228978'), $entry1)
            ->andReturn(0);
        $this->shouldCall('_njCollisionCheckBS')
            ->with($this->addressOf('_var_collideSelfBox_8c228978'), $entry2)
            ->andReturn(1);
        $this->shouldReturn($entry2);
    }
};
