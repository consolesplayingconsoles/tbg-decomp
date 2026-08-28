<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_found(): void {
        // header {width=4, height=3} + 4*3 offset table
        $datFile = $this->alloc((2 + 4 * 3) * 4);
        $this->initUint32($datFile + 0, 4);
        $this->initUint32($datFile + 4, 3);
        $this->doNotRandomizeMemory();
        for ($i = 0; $i < 4 * 3; $i++) {
            $this->initUint32($datFile + 8 + $i * 4, 0);
        }
        // col=2, row=1 -> word index col + row*width + 2 = 2 + 1*4 + 2 = 8
        $this->initUint32($datFile + 8 * 4, 0x40);

        $out = $this->alloc(4);

        $this->call('_lookupTile_8c0217de')->with(2, 1, $out, $datFile);

        $this->shouldWriteLong($out, $datFile + 0x40);
        $this->shouldReturn(1);
    }

    public function test_not_found(): void {
        $datFile = $this->alloc((2 + 4 * 3) * 4);
        $this->initUint32($datFile + 0, 4);
        $this->initUint32($datFile + 4, 3);
        $this->doNotRandomizeMemory();
        for ($i = 0; $i < 4 * 3; $i++) {
            $this->initUint32($datFile + 8 + $i * 4, 0);
        }

        $out = $this->alloc(4);

        $this->call('_lookupTile_8c0217de')->with(2, 1, $out, $datFile);

        $this->shouldReturn(0);
    }
};
