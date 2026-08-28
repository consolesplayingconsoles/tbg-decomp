<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
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

    /** Sets up the 5 layer-dims slots (all sharing one {width, height}) and
     * the 4 texel-layer grid buffers, zeroed. Returns the 4 grid base
     * addresses. Layer 4 (model-only) is out of this function's range and
     * left unset. */
    private function setUpLayers(int $width, int $height): array {
        $this->setSize('_var_8c22650c', 5 * 4);
        $this->setSize('_var_tileLayerSlots_8c226520', 5 * 4);
        $dimsBase = $this->addressOf('_var_8c22650c');
        $gridBase = $this->addressOf('_var_tileLayerSlots_8c226520');

        $dims = $this->alloc(8);
        $this->initUint32($dims + 0, $width);
        $this->initUint32($dims + 4, $height);

        $grids = [];
        for ($layer = 0; $layer < 4; $layer++) {
            $grid = $this->alloc($width * $height * 8);
            for ($i = 0; $i < $width * $height; $i++) {
                $this->initUint32($grid + $i * 8 + 0, 0);
                $this->initUint32($grid + $i * 8 + 4, 0);
            }

            $this->initUint32($dimsBase + $layer * 4, $dims);
            $this->initUint32($gridBase + $layer * 4, $grid);

            $grids[$layer] = $grid;
        }

        return $grids;
    }

    private function setPvrDir(string $dir): void {
        $this->setSize('_var_pvrDir_8c18ad4c', 0x20);
        $address = $this->addressOf('_var_pvrDir_8c18ad4c');
        foreach (str_split($dir) as $i => $char) {
            $this->initUint8($address + $i, ord($char));
        }
        $this->initUint8($address + strlen($dir), 0);
    }

    public function test_no_region_list(): void {
        $this->setSize('_var_currentTileRegionList_8c226534', 4);
        $this->initUint32($this->addressOf('_var_currentTileRegionList_8c226534'), 0xffffffff);

        $this->call('_TileStreamRequestUpload_8c02190a');
    }

    public function test_uploads_loaded_tiles_only(): void {
        $this->setUpRegion([[0, 0, 1, 0]]); // col 0..1, row 0
        $grids = $this->setUpLayers(2, 1);
        $this->setPvrDir('\\SD_PVR');

        $texlists = [];
        foreach ($grids as $layer => $grid) {
            // col 1 (idx 1) already has a loaded texlist -> gets requested
            $texlists[$layer] = $this->alloc(4);
            $this->initUint32($grid + 1 * 8 + 0, $texlists[$layer]);
            // col 0 (idx 0) stays unloaded (0) -> never requested
        }

        $scratch = $this->alloc(2 * 1);
        $this->setSize('_syMalloc', 4);
        $this->setSize('_syFree', 4);
        $this->setSize('_AsqRequestTexlist_8c01181c', 4);

        $this->call('_TileStreamRequestUpload_8c02190a');

        $this->shouldCall('_syMalloc')->with(2 * 1)->andReturn($scratch);
        for ($i = 0; $i < 2 * 1; $i++) {
            $this->shouldWriteByte($scratch + $i, 0);
        }
        // col 0 (idx 0): no loaded texlists, marked requested anyway
        $this->shouldWriteByte($scratch + 0, 1);
        // col 1 (idx 1): one upload request per layer, then marked requested
        foreach ($grids as $layer => $grid) {
            $this->shouldCall('_AsqRequestTexlist_8c01181c')->with('\\SD_PVR', $texlists[$layer]);
        }
        $this->shouldWriteByte($scratch + 1, 1);
        $this->shouldCall('_syFree')->with($scratch);
    }
};
