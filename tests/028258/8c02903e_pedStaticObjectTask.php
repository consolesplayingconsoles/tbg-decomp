<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_does_nothing(): void
    {
        $this->call('_pedStaticObjectTask_8c02903e')->with(0, 0);
    }
};
