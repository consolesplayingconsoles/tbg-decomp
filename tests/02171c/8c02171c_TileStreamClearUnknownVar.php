<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_tileLayerSlots_8c226520', 4);
    }

    public function test_1(): void {
        $this->resolveSymbols();

        $this->call('_TileStreamClearUnknownVar_8c02171c');

        $this->shouldWriteLong($this->addressOf('_var_tileLayerSlots_8c226520'), 0xffffffff);
    }
};
