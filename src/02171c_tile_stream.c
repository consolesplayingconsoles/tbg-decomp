/* @unit TileStream */
#include <shinobi.h>
#include "sectionB.h"
#include "1ba1c8_globals.h"
#include "013ae8_route_load.h"
#include "011120_asset_queues.h"
#include "02171c_tile_stream.h"
#include "021b9c_tile_draw.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"

void TileStreamClearSlots_8c02171c(void)
{
    var_tileLayerSlots_8c226520[0] = (void *)-1;
}

/* Releases every loaded tile slot (see TileStreamReleaseAll_8c021a24), then
 * frees the 5 grid buffers and marks the buffer set as unallocated. */
void TileStreamTeardown_8c021724(void)
{
    int i;

    if (var_tileLayerSlots_8c226520[0] != (void *)-1) {
        TileStreamReleaseAll_8c021a24();
        for (i = 0; i < 5; i++) {
            syFree(var_tileLayerSlots_8c226520[i]);
        }
        var_tileLayerSlots_8c226520[0] = (void *)-1;
    }
}

void TileStreamInit_8c02175a(void)
{
    int i;

    for (i = 0; i < 5; i++) {
        var_tileLayerIndexes_8c22650c[i] = var_currentCourse_8c1bb868.tileLayers_0x3c[i];
    }

    for (i = 0; i < 5; i++) {
        TileIndex *index = var_tileLayerIndexes_8c22650c[i];
        int count = index->width * index->height;
        LoadedModel *slot = (LoadedModel *)syMalloc(count * sizeof(LoadedModel));
        var_tileLayerSlots_8c226520[i] = slot;
        for (; count > 0; count--) {
            slot->texlist = NULL;
            slot->njDest = NULL;
            slot++;
        }
    }
}

STATIC Bool lookupTile_8c0217de(int col, int row, void **out, TileIndex *index)
{
    int offset = index->tileOffsets[col + row * index->width];

    if (offset == 0) {
        return FALSE;
    }

    *out = (char *)index + offset;
    return TRUE;
}

/* One texture+model layer per var_datFiles_8c18adb4 entry (layers 0-3 only);
 * frees each datFile once its layer is done. Only fills grid slots that are
 * still unloaded. */
void TileStreamLoad_8c021810(void)
{
    int layer;
    TileRect *rect;
    int row, col;
    int idx;
    LoadedModel *slot;
    void *tileData;
    Uint32 fpos;
    Uint32 rtype;

    if (var_currentTileRegionList_8c226534 == (void *)-1) {
        return;
    }

    for (layer = 0; layer < 4; layer++) {
        for (rect = var_currentTileRegionList_8c226534; rect->endCol != 0 || rect->endRow != 0; rect++) {
            for (row = rect->startRow; row <= rect->endRow; row++) {
                for (col = rect->startCol; col <= rect->endCol; col++) {
                    /* Always layer 0's width, like TileStreamRequestUpload_8c02190a --
                     * not var_tileLayerIndexes_8c22650c[layer]. */
                    idx = row * var_tileLayerIndexes_8c22650c[0]->width + col;
                    fpos = 0;
                    if (lookupTile_8c0217de(col, row, &tileData, var_datFiles_8c18adb4[layer])) {
                        slot = &var_tileLayerSlots_8c226520[layer][idx];
                        if (slot->texlist == NULL) {
                            slot->texlist = njReadBinary(tileData, &fpos, &rtype);
                            slot->njDest = njReadBinary(tileData, &fpos, &rtype);
                        }
                    }
                }
            }
        }
        syFree(var_datFiles_8c18adb4[layer]);
    }
}

/* Marks each already-loaded grid slot in the region as pending upload via
 * AsqRequestTexlist. `requested` is a scratch width*height flag grid so a tile
 * shared by overlapping rects is only requested once. */
void TileStreamRequestUpload_8c02190a(void)
{
    char *requested;
    int width, height, area;
    TileRect *rect;
    int row, col;
    int idx;
    int layer;
    LoadedModel *slot;

    if (var_currentTileRegionList_8c226534 == (void *)-1) {
        return;
    }

    width = var_tileLayerIndexes_8c22650c[0]->width;
    height = var_tileLayerIndexes_8c22650c[0]->height;
    area = width * height;
    requested = (char *)syMalloc(area);
    for (idx = 0; idx < area; idx++) {
        requested[idx] = 0;
    }

    for (rect = var_currentTileRegionList_8c226534; rect->endCol != 0 || rect->endRow != 0; rect++) {
        for (row = rect->startRow; row <= rect->endRow; row++) {
            for (col = rect->startCol; col <= rect->endCol; col++) {
                idx = row * width + col;
                if (requested[idx] == 0) {
                    for (layer = 0; layer < 4; layer++) {
                        slot = &var_tileLayerSlots_8c226520[layer][idx];
                        if (slot->texlist != NULL) {
                            AsqRequestTexlist_8c01181c(var_pvrDir_8c18ad4c, slot->texlist);
                        }
                    }
                    requested[idx] = 1;
                }
            }
        }
    }

    syFree(requested);
}

/* Releases every loaded slot across the full grid: njReleaseTexture+syFree
 * each of the 4 texlist layers, plus syFree the model on the 5th (model-only)
 * layer. Walks the full width*height area (course dims), not a region list. */
void TileStreamReleaseAll_8c021a24(void)
{
    int width, height;
    int row, col, layer;
    int idx;
    LoadedModel *slot;
    TileIndex *courseIndex = var_currentCourse_8c1bb868.tileLayers_0x3c[0];

    width = courseIndex->width;
    height = courseIndex->height;

    for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
            for (layer = 0; layer < 4; layer++) {
                idx = row * var_tileLayerIndexes_8c22650c[layer]->width + col;
                slot = &var_tileLayerSlots_8c226520[layer][idx];
                if (slot->texlist != NULL) {
                    njReleaseTexture(slot->texlist);
                    syFree(slot->texlist);
                    syFree(slot->njDest);
                    slot->texlist = NULL;
                    slot->njDest = NULL;
                }
            }

            idx = row * var_tileLayerIndexes_8c22650c[4]->width + col;
            slot = &var_tileLayerSlots_8c226520[4][idx];
            if (slot->njDest != NULL) {
                syFree(slot->njDest);
                slot->njDest = NULL;
            }
        }
    }
}

/* Draws one tile slot {texlist, model} at the bus's current position, fog
 * disabled for the duration. */
void TileStreamDrawTile_8c021b34(LoadedModel *slot)
{
    njControl3D(0);
    njFogDisable();
    njTranslate(NULL, var_busState_8c1bb9d0.posX_0x2fc, var_busState_8c1bb9d0.posY_0x300, var_busState_8c1bb9d0.posZ_0x304);
    njSetTexture(slot->texlist);
    njCnkSimpleDrawObject(slot->njDest);
    njControl3D(NJD_CONTROL_3D_MODEL_CLIP);
    njFogEnable();
}
