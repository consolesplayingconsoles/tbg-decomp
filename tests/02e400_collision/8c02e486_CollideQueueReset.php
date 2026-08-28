<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// CollideQueueReset_8c02e486 zeroes the collision queue's element count.

return new class extends TestCase {
    public function test_zeroesQueueCount(): void {
        $this->setSize('_var_collideQueueCount_8c228b38', 4);
        $this->initUint32($this->addressOf('_var_collideQueueCount_8c228b38'), 5);

        $this->call('_CollideQueueReset_8c02e486')->with();

        $this->shouldWriteLongTo('_var_collideQueueCount_8c228b38', 0);
    }
};
