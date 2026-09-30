<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_drawCommandCount_8c226570', 12);
    }

    public function test_clears_draw_command_counts(): void {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_drawCommandCount_8c226570');
        $this->initUint32($base, 1);
        $this->initUint32($base + 4, 2);
        $this->initUint32($base + 8, 3);

        $this->call('_RenderResetQueues_8c02239c');

        $this->shouldWriteLong($base, 0);
        $this->shouldWriteLong($base + 4, 0);
        $this->shouldWriteLong($base + 8, 0);
    }
};
