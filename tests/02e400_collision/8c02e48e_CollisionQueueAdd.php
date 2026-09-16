<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// CollisionQueueAdd_8c02e48e appends obj to the fixed 64-slot queue at the
// pre-increment index of the count, once the count itself has already been
// bumped. Past 0x40 entries it silently drops the write.

return new class extends TestCase {
    private function resolveSymbols(): void {
    }

    public function test_appendsAtZero(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_collisionQueueCount_8c228b38'), 0);

        $obj = $this->alloc(4);

        $this->call('_CollisionQueueAdd_8c02e48e')->with($obj);

        $this->shouldWriteLongTo('_var_collisionQueueCount_8c228b38', 1);
        $this->shouldWriteLong($this->addressOf('_var_collisionQueue_8c228a38') + 0 * 4, $obj);
    }

    public function test_appendsAtMidRange(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_collisionQueueCount_8c228b38'), 5);

        $obj = $this->alloc(4);

        $this->call('_CollisionQueueAdd_8c02e48e')->with($obj);

        $this->shouldWriteLongTo('_var_collisionQueueCount_8c228b38', 6);
        $this->shouldWriteLong($this->addressOf('_var_collisionQueue_8c228a38') + 5 * 4, $obj);
    }

    public function test_rejectsWhenFull(): void {
        $this->resolveSymbols();

        // Count already at the 0x40 cap: nothing is written at all.
        $this->initUint32($this->addressOf('_var_collisionQueueCount_8c228b38'), 0x40);

        $obj = $this->alloc(4);

        $this->call('_CollisionQueueAdd_8c02e48e')->with($obj);
    }

    public function test_rejectsWhenOverFull(): void {
        $this->resolveSymbols();

        // Count past the cap (shouldn't normally happen, but the guard is
        // a plain >= so it must still reject).
        $this->initUint32($this->addressOf('_var_collisionQueueCount_8c228b38'), 0x41);

        $obj = $this->alloc(4);

        $this->call('_CollisionQueueAdd_8c02e48e')->with($obj);
    }
};
