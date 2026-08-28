<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    // TileStreamLoad_8c021810's own stack locals: &tileData (out-param of
    // lookupTile_8c0217de) and &fpos/&rtype (out-params of njReadBinary), the
    // same slots reused across every layer/col/row iteration of one call.
    // The two objects order the three slots differently.
    private function isAsm(): bool {
        return str_ends_with($this->objectFile, '_src.obj');
    }

    private function tileDataLocal(): int {
        return $this->isAsm() ? 0xffffbc : 0xffffc4;
    }

    private function fposLocal(): int {
        return $this->isAsm() ? 0xffffc0 : 0xffffc8;
    }

    private function rtypeLocal(): int {
        return $this->isAsm() ? 0xffffc4 : 0xffffcc;
    }

    /** Builds the region-rect list (each rect: [startCol, startRow, endCol,
     * endRow]) terminated by {0,0,0,0}, and points
     * var_currentTileRegionList_8c226534 at it. A rect's endCol/endRow must
     * not both be 0 or it reads as the terminator -- keep endCol >= 1 for
     * any rect touching col 0. */
    private function setUpRegion(array $rects): int {
        $count = count($rects);
        $region = $this->alloc(($count + 1) * 4);

        foreach ($rects as $i => $rect) {
            foreach ($rect as $j => $byte) {
                $this->initUint8($region + $i * 4 + $j, $byte);
            }
        }
        for ($j = 0; $j < 4; $j++) {
            $this->initUint8($region + $count * 4 + $j, 0);
        }

        $this->setSize('_var_currentTileRegionList_8c226534', 4);
        $this->initUint32($this->addressOf('_var_currentTileRegionList_8c226534'), $region);

        return $region;
    }

    /** Sets up all 5 layer-dims slots and 4 grid/datFile slots with a shared
     * {width, height} and zeroed grid buffers. Returns one
     * ['grid' => ..., 'datFile' => ...] entry per layer (datFile is an
     * opaque pointer, only ever passed through to the mocked lookupTile). */
    private function setUpLayers(int $width, int $height): array {
        $this->setSize('_var_8c22650c', 5 * 4);
        $this->setSize('_var_tileLayerSlots_8c226520', 5 * 4);
        $this->setSize('_var_datFiles_8c18adb4', 4 * 4);
        $dimsBase = $this->addressOf('_var_8c22650c');
        $gridBase = $this->addressOf('_var_tileLayerSlots_8c226520');
        $datFilesBase = $this->addressOf('_var_datFiles_8c18adb4');
        $this->setSize('_syFree', 4);
        $this->setSize('_njReadBinary', 4);

        $dims = $this->alloc(8);
        $this->initUint32($dims + 0, $width);
        $this->initUint32($dims + 4, $height);

        $layers = [];
        for ($layer = 0; $layer < 4; $layer++) {
            $grid = $this->alloc($width * $height * 8);
            for ($i = 0; $i < $width * $height; $i++) {
                $this->initUint32($grid + $i * 8 + 0, 0);
                $this->initUint32($grid + $i * 8 + 4, 0);
            }
            $datFile = $this->alloc(4);

            $this->initUint32($dimsBase + $layer * 4, $dims);
            $this->initUint32($gridBase + $layer * 4, $grid);
            $this->initUint32($datFilesBase + $layer * 4, $datFile);

            $layers[$layer] = ['grid' => $grid, 'datFile' => $datFile];
        }

        return $layers;
    }

    private function mockLookupTileFound(int $col, int $row, int $datFile, int $tileData): void {
        $this->shouldCall('_lookupTile_8c0217de')
            ->with($col, $row, $this->tileDataLocal(), $datFile)
            ->do(function ($params) use ($tileData) {
                $this->memory->writeUInt32($params[2], U32::of($tileData));
            })
            ->andReturn(1);
    }

    private function mockLookupTileNotFound(int $col, int $row, int $datFile): void {
        $this->shouldCall('_lookupTile_8c0217de')
            ->with($col, $row, $this->tileDataLocal(), $datFile)
            ->andReturn(0);
    }

    /** Expects the two njReadBinary calls that fill grid slot $idx (col +
     * row*width), and the resulting writes. */
    private function expectTileRead(int $grid, int $idx, int $tileData, int $texlist, int $model): void {
        $slot = $grid + $idx * 8;

        $this->shouldCall('_njReadBinary')
            ->with($tileData, $this->fposLocal(), $this->rtypeLocal())
            ->andReturn($texlist);
        $this->shouldWriteLong($slot + 0, $texlist);

        $this->shouldCall('_njReadBinary')
            ->with($tileData, $this->fposLocal(), $this->rtypeLocal())
            ->andReturn($model);
        $this->shouldWriteLong($slot + 4, $model);
    }

    public function test_no_region_list(): void {
        $this->setSize('_var_currentTileRegionList_8c226534', 4);
        $this->initUint32($this->addressOf('_var_currentTileRegionList_8c226534'), 0xffffffff);

        $this->call('_TileStreamLoad_8c021810');
    }

    public function test_single_tile_all_layers(): void {
        $this->setUpRegion([[0, 0, 1, 0]]); // col 0..1, row 0
        $layers = $this->setUpLayers(2, 1);

        $tileData = $this->alloc(4);
        $texlist = $this->alloc(4);
        $model = $this->alloc(4);

        $this->call('_TileStreamLoad_8c021810');

        foreach ($layers as $layer) {
            $this->mockLookupTileNotFound(0, 0, $layer['datFile']);
            $this->mockLookupTileFound(1, 0, $layer['datFile'], $tileData);
            $this->expectTileRead($layer['grid'], 1, $tileData, $texlist, $model);
            $this->shouldCall('_syFree')->with($layer['datFile']);
        }
    }

    public function test_already_loaded_tile_skips_read(): void {
        // the region-rect list is one global walked identically for every
        // layer, so all 4 layers see the same tile requests each test
        $this->setUpRegion([[1, 0, 1, 0]]); // col 1 only, row 0
        $layers = $this->setUpLayers(2, 1);

        $tileData = $this->alloc(4);

        foreach ($layers as $layer) {
            // slot 1 (col 1) already has a loaded texlist -> must not be re-read
            $this->initUint32($layer['grid'] + 1 * 8 + 0, 0xcafe0001);
            $this->initUint32($layer['grid'] + 1 * 8 + 4, 0xcafe0002);
        }

        $this->call('_TileStreamLoad_8c021810');

        foreach ($layers as $layer) {
            $this->mockLookupTileFound(1, 0, $layer['datFile'], $tileData);
            // no njReadBinary/write expectations: slot is already filled
            $this->shouldCall('_syFree')->with($layer['datFile']);
        }
    }

    public function test_multiple_rects_in_region_list(): void {
        $this->setUpRegion([
            [1, 0, 1, 0], // col 1, row 0
            [2, 0, 2, 0], // col 2, row 0
        ]);
        $layers = $this->setUpLayers(3, 1);

        $tileData1 = $this->alloc(4);
        $tileData2 = $this->alloc(4);
        $texlist = $this->alloc(4);
        $model = $this->alloc(4);

        $this->call('_TileStreamLoad_8c021810');

        foreach ($layers as $layer) {
            $this->mockLookupTileFound(1, 0, $layer['datFile'], $tileData1);
            $this->expectTileRead($layer['grid'], 1, $tileData1, $texlist, $model);

            $this->mockLookupTileFound(2, 0, $layer['datFile'], $tileData2);
            $this->expectTileRead($layer['grid'], 2, $tileData2, $texlist, $model);

            $this->shouldCall('_syFree')->with($layer['datFile']);
        }
    }

    /* Every real course gives all 5 layers the same grid dims, but the asm
     * always strides by var_8c22650c[0]->width regardless of which layer is
     * being loaded -- confirmed by decompiling FUN_8c021810. Give layers 1-3
     * a different width than layer 0 to catch a switch back to per-layer
     * width. */
    public function test_uses_layer_zero_width_for_every_layer(): void {
        $this->setUpRegion([[0, 0, 1, 1]]); // col 0..1, row 0..1
        $layers = $this->setUpLayers(2, 2);

        // layer 0 keeps the shared 2x2 dims; layers 1-3 get their own,
        // differently-strided dims block (grid buffer sized to match)
        for ($layer = 1; $layer < 4; $layer++) {
            $dims = $this->alloc(8);
            $this->initUint32($dims + 0, 5);
            $this->initUint32($dims + 4, 2);
            $this->initUint32($this->addressOf('_var_8c22650c') + $layer * 4, $dims);

            $grid = $this->alloc(5 * 2 * 8);
            for ($i = 0; $i < 5 * 2; $i++) {
                $this->initUint32($grid + $i * 8 + 0, 0);
                $this->initUint32($grid + $i * 8 + 4, 0);
            }
            $this->initUint32($this->addressOf('_var_tileLayerSlots_8c226520') + $layer * 4, $grid);
            $layers[$layer]['grid'] = $grid;
        }

        $tileData = $this->alloc(4);
        $texlist = $this->alloc(4);
        $model = $this->alloc(4);

        $this->call('_TileStreamLoad_8c021810');

        foreach ($layers as $layer) {
            // row-major order using layer 0's width (2), not the layer's own
            $this->mockLookupTileFound(0, 0, $layer['datFile'], $tileData);
            $this->expectTileRead($layer['grid'], 0 * 2 + 0, $tileData, $texlist, $model);

            $this->mockLookupTileFound(1, 0, $layer['datFile'], $tileData);
            $this->expectTileRead($layer['grid'], 0 * 2 + 1, $tileData, $texlist, $model);

            $this->mockLookupTileFound(0, 1, $layer['datFile'], $tileData);
            $this->expectTileRead($layer['grid'], 1 * 2 + 0, $tileData, $texlist, $model);

            $this->mockLookupTileFound(1, 1, $layer['datFile'], $tileData);
            $this->expectTileRead($layer['grid'], 1 * 2 + 1, $tileData, $texlist, $model);

            $this->shouldCall('_syFree')->with($layer['datFile']);
        }
    }

    public function test_multi_row_multi_col_rect(): void {
        $this->setUpRegion([[1, 0, 2, 1]]); // col 1..2, row 0..1
        $layers = $this->setUpLayers(3, 3);

        $tileData = $this->alloc(4);
        $texlist = $this->alloc(4);
        $model = $this->alloc(4);

        $this->call('_TileStreamLoad_8c021810');

        foreach ($layers as $layer) {
            // row-major order: row 0 (col 1, col 2), then row 1 (col 1, col 2)
            $this->mockLookupTileFound(1, 0, $layer['datFile'], $tileData);
            $this->expectTileRead($layer['grid'], 0 * 3 + 1, $tileData, $texlist, $model);

            $this->mockLookupTileFound(2, 0, $layer['datFile'], $tileData);
            $this->expectTileRead($layer['grid'], 0 * 3 + 2, $tileData, $texlist, $model);

            $this->mockLookupTileFound(1, 1, $layer['datFile'], $tileData);
            $this->expectTileRead($layer['grid'], 1 * 3 + 1, $tileData, $texlist, $model);

            $this->mockLookupTileFound(2, 1, $layer['datFile'], $tileData);
            $this->expectTileRead($layer['grid'], 1 * 3 + 2, $tileData, $texlist, $model);

            $this->shouldCall('_syFree')->with($layer['datFile']);
        }
    }
};
