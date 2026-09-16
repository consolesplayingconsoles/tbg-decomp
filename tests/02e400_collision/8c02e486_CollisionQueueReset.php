<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// CollisionQueueReset_8c02e486 zeroes the collision queue's element count.

return new class extends TestCase {
    public function test_zeroesQueueCount(): void {
        $this->initUint32($this->addressOf('_var_collisionQueueCount_8c228b38'), 5);

        $this->call('_CollisionQueueReset_8c02e486')->with();

        $this->shouldWriteLongTo('_var_collisionQueueCount_8c228b38', 0);
    }
};
