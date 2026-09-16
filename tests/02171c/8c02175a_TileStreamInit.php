<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_currentCourse_8c1bb868', 0x50); // sizeof(CurrentCourse)
        $this->setSize('_var_tileLayerIndexes_8c22650c', 5 * 4);
        $this->setSize('_var_tileLayerSlots_8c226520', 5 * 4);
        $this->setSize('_syMalloc', 4);
    }

    public function test_1(): void {
        $this->resolveSymbols();

        $slotsBase = $this->addressOf('_var_currentCourse_8c1bb868') + 0x3c; // tileLayers_0x3c
        $dims = [];
        for ($i = 0; $i < 5; $i++) {
            $dims[$i] = $this->alloc(8);
            $this->initUint32($dims[$i] + 0, 2);
            $this->initUint32($dims[$i] + 4, 3);
            $this->initUint32($slotsBase + $i * 4, $dims[$i]);
        }

        $this->call('_TileStreamInit_8c02175a');

        $slots = [];
        for ($i = 0; $i < 5; $i++) {
            $this->shouldWriteLong($this->addressOf('_var_tileLayerIndexes_8c22650c') + $i * 4, $dims[$i]);
        }

        for ($i = 0; $i < 5; $i++) {
            $slots[$i] = $this->alloc(2 * 3 * 8);
            $this->shouldCall('_syMalloc')->with(2 * 3 * 8)->andReturn($slots[$i]);
            $this->shouldWriteLong($this->addressOf('_var_tileLayerSlots_8c226520') + $i * 4, $slots[$i]);
            for ($j = 0; $j < 6; $j++) {
                $this->shouldWriteLong($slots[$i] + $j * 8, 0);
                $this->shouldWriteLong($slots[$i] + $j * 8 + 4, 0);
            }
        }
    }

    /* Each layer's grid is sized/zeroed off its own tileLayers_0x3c[i] dims,
     * not a shared one -- give every layer a different width*height so a
     * collapse to one shared size shows up as a wrong syMalloc size. */
    public function test_uses_each_layers_own_dims(): void {
        $this->resolveSymbols();

        $slotsBase = $this->addressOf('_var_currentCourse_8c1bb868') + 0x3c;
        $dims = [];
        $sizes = [];
        for ($i = 0; $i < 5; $i++) {
            $width = $i + 2;
            $height = $i + 1;
            $sizes[$i] = $width * $height;
            $dims[$i] = $this->alloc(8);
            $this->initUint32($dims[$i] + 0, $width);
            $this->initUint32($dims[$i] + 4, $height);
            $this->initUint32($slotsBase + $i * 4, $dims[$i]);
        }

        $this->call('_TileStreamInit_8c02175a');

        for ($i = 0; $i < 5; $i++) {
            $this->shouldWriteLong($this->addressOf('_var_tileLayerIndexes_8c22650c') + $i * 4, $dims[$i]);
        }

        for ($i = 0; $i < 5; $i++) {
            $slot = $this->alloc($sizes[$i] * 8);
            $this->shouldCall('_syMalloc')->with($sizes[$i] * 8)->andReturn($slot);
            $this->shouldWriteLong($this->addressOf('_var_tileLayerSlots_8c226520') + $i * 4, $slot);
            for ($j = 0; $j < $sizes[$i]; $j++) {
                $this->shouldWriteLong($slot + $j * 8, 0);
                $this->shouldWriteLong($slot + $j * 8 + 4, 0);
            }
        }
    }
};
