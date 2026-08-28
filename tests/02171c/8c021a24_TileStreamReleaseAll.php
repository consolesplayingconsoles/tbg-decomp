<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_releases_loaded_and_skips_empty_slots(): void {
        // int courseId_0x00; void *slots_0x04[19];
        $this->setSize('_var_currentCourse_8c1bb868', 4 + 19 * 4);
        $this->setSize('_var_8c22650c', 5 * 4);
        $this->setSize('_var_tileLayerSlots_8c226520', 5 * 4);
        $this->setSize('_njReleaseTexture', 4);
        $this->setSize('_syFree', 4);

        // 1x1 grid so there is exactly one (row, col) to walk.
        $dims = $this->alloc(8);
        $this->initUint32($dims + 0, 1);
        $this->initUint32($dims + 4, 1);
        $slot14 = $this->addressOf('_var_currentCourse_8c1bb868') + 4 + 14 * 4;
        $this->initUint32($slot14, $dims);
        // var_8c1bb8a4 aliases var_currentCourse_8c1bb868.slots_0x04[14].
        $this->rellocate('_var_8c1bb8a4', $slot14);

        $dimsBase = $this->addressOf('_var_8c22650c');
        $gridBase = $this->addressOf('_var_tileLayerSlots_8c226520');
        // var_8c22651c aliases var_8c22650c[4].
        $this->rellocate('_var_8c22651c', $dimsBase + 4 * 4);

        $grids = [];
        for ($layer = 0; $layer < 5; $layer++) {
            $this->initUint32($dimsBase + $layer * 4, $dims);
            $grids[$layer] = $this->alloc(8);
            $this->initUint32($gridBase + $layer * 4, $grids[$layer]);
        }

        // Layer 0: loaded texlist+model -> released.
        $texlist0 = $this->alloc(4);
        $model0 = $this->alloc(4);
        $this->initUint32($grids[0] + 0, $texlist0);
        $this->initUint32($grids[0] + 4, $model0);

        // Layers 1-3: unloaded -> skipped.
        for ($layer = 1; $layer < 4; $layer++) {
            $this->initUint32($grids[$layer] + 0, 0);
            $this->initUint32($grids[$layer] + 4, 0);
        }

        // Layer 4 (model-only): loaded model -> freed.
        $model4 = $this->alloc(4);
        $this->initUint32($grids[4] + 0, 0);
        $this->initUint32($grids[4] + 4, $model4);

        $this->call('_TileStreamReleaseAll_8c021a24');

        $this->shouldCall('_njReleaseTexture')->with($texlist0);
        $this->shouldCall('_syFree')->with($texlist0);
        $this->shouldCall('_syFree')->with($model0);
        $this->shouldWriteLong($grids[0] + 0, 0);
        $this->shouldWriteLong($grids[0] + 4, 0);

        $this->shouldCall('_syFree')->with($model4);
        $this->shouldWriteLong($grids[4] + 4, 0);
    }

    /* Every real course gives all 5 layers the same grid dims, but the asm
     * strides each layer (0-3) by its own var_8c22650c[layer]->width, and the
     * model-only layer by var_8c22650c[4]->width -- confirmed by decompiling
     * FUN_8c021a24. Give every layer a different width than the outer
     * row/col bound (from courseIndex = tileLayers_0x3c[0]) to catch a
     * collapse back to one shared width. */
    public function test_uses_each_layers_own_width(): void {
        $this->setSize('_var_currentCourse_8c1bb868', 4 + 19 * 4);
        $this->setSize('_var_8c22650c', 5 * 4);
        $this->setSize('_var_tileLayerSlots_8c226520', 5 * 4);
        $this->setSize('_njReleaseTexture', 4);
        $this->setSize('_syFree', 4);

        // outer bound: 2x2 (courseIndex = tileLayers_0x3c[0]) -- need row >= 1
        // so a layer's stride actually affects idx = row*width + col
        $courseDims = $this->alloc(8);
        $this->initUint32($courseDims + 0, 2);
        $this->initUint32($courseDims + 4, 2);
        $slot14 = $this->addressOf('_var_currentCourse_8c1bb868') + 4 + 14 * 4;
        $this->initUint32($slot14, $courseDims);
        $this->rellocate('_var_8c1bb8a4', $slot14);

        $dimsBase = $this->addressOf('_var_8c22650c');
        $gridBase = $this->addressOf('_var_tileLayerSlots_8c226520');
        $this->rellocate('_var_8c22651c', $dimsBase + 4 * 4);

        // each layer gets its own width (3, 4, 5, 6, 7)
        $widths = [];
        $grids = [];
        for ($layer = 0; $layer < 5; $layer++) {
            $width = $layer + 3;
            $widths[$layer] = $width;
            $dims = $this->alloc(8);
            $this->initUint32($dims + 0, $width);
            $this->initUint32($dims + 4, 2);
            $this->initUint32($dimsBase + $layer * 4, $dims);

            $grid = $this->alloc($width * 2 * 8);
            for ($i = 0; $i < $width * 2; $i++) {
                $this->initUint32($grid + $i * 8 + 0, 0);
                $this->initUint32($grid + $i * 8 + 4, 0);
            }
            $this->initUint32($gridBase + $layer * 4, $grid);
            $grids[$layer] = $grid;
        }

        // one loaded texlist+model per layer 0-3, at (row=1, col=0) ->
        // idx = 1*layerWidth+0 = layerWidth, distinct per layer
        $texlists = [];
        $models = [];
        for ($layer = 0; $layer < 4; $layer++) {
            $idx = $widths[$layer];
            $texlists[$layer] = $this->alloc(4);
            $models[$layer] = $this->alloc(4);
            $this->initUint32($grids[$layer] + $idx * 8 + 0, $texlists[$layer]);
            $this->initUint32($grids[$layer] + $idx * 8 + 4, $models[$layer]);
        }
        // layer 4 (model-only): loaded model at (row=1, col=0) -> idx = layerWidth
        $idx4 = $widths[4];
        $model4 = $this->alloc(4);
        $this->initUint32($grids[4] + $idx4 * 8 + 4, $model4);

        $this->call('_TileStreamReleaseAll_8c021a24');

        for ($layer = 0; $layer < 4; $layer++) {
            $idx = $widths[$layer];
            $this->shouldCall('_njReleaseTexture')->with($texlists[$layer]);
            $this->shouldCall('_syFree')->with($texlists[$layer]);
            $this->shouldCall('_syFree')->with($models[$layer]);
            $this->shouldWriteLong($grids[$layer] + $idx * 8 + 0, 0);
            $this->shouldWriteLong($grids[$layer] + $idx * 8 + 4, 0);
        }
        $this->shouldCall('_syFree')->with($model4);
        $this->shouldWriteLong($grids[4] + $idx4 * 8 + 4, 0);
    }
};
