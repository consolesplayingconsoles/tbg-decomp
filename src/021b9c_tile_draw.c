/* @unit TileDraw */
#include <shinobi.h>

#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "sectionB.h"
#include "1ba1c8_globals.h"
#include "011120_asset_queues.h" /* LoadedModel */
#include "013ae8_route_load.h"   /* CourseSceneParams */
#include "014a9c_tasks.h"        /* Task */
#include "021b9c_tile_draw.h"
#include "0222dc_fadecmd.h"      /* FadeCmdPushCall1/2 */
#include "02171c_tile_stream.h"  /* TileIndex, TileStreamDrawTile_8c021b34 */
#include "022464_fade.h"         /* FadeCallback1 */

/* ====================
 * Forward Declarations
 * ====================
 */
STATIC void drawTileGrid_8c021b9c(int width, int height);
STATIC void drawTileGridMirror_8c021ec4(int width, int height);

/* ====================
 * Functions
 * ====================
 */

/* Draws the visible window of the tile grid (var_tileLayerSlots_8c226520,
 * width x height tiles) around the bus's position for fade-command layer 0
 * (the primary render). Layers 0, 2 and 3 of the tile grid draw with the
 * Chunk "Easy" light model, layer 1 with "Simple" -- as coded, not a
 * mistake to normalize away. Never called by name from another unit --
 * reachable only via the function-pointer literal TileDrawEnqueueTask_8c0221d0
 * pushes to FadeCmdPushCall2_8c022420 -- so it stays private here. */
STATIC void drawTileGrid_8c021b9c(int width, int height)
{
    int colStart, colEnd, rowStart, rowEnd;
    int row, col, flatIndex, rowBase;
    LoadedModel *slot;

    colStart = (int)var_busState_8c1bb9d0.posX_0x2fc / 150 - var_fogParam_8c226508;
    colEnd = colStart + var_fogParam_8c226504;
    if (colStart < 0) {
        colStart = 0;
    }
    if (colEnd >= width) {
        colEnd = width - 1;
    }

    rowStart = (int)var_busState_8c1bb9d0.posZ_0x304 / 150 - var_fogParam_8c226508;
    rowEnd = rowStart + var_fogParam_8c226504;
    if (rowStart < 0) {
        rowStart = 0;
    }
    if (rowEnd >= height) {
        rowEnd = height - 1;
    }

    njControl3D(NJD_CONTROL_3D_MODEL_CLIP | NJD_CONTROL_3D_SHADOW | NJD_CONTROL_3D_TRANS_MODIFIER);

    rowBase = rowStart * width;
    for (row = rowStart; row <= rowEnd; row++) {
        for (col = colStart; col <= colEnd; col++) {
            flatIndex = rowBase + col;

            slot = &var_tileLayerSlots_8c226520[0][flatIndex];
            if (slot->texlist != NULL) {
                njSetCamera(var_fadeCamera_8c226558);
                njCnkSetEasyLight(var_fadeEasyLightDir_8c226538[0], var_fadeEasyLightDir_8c226538[1], var_fadeEasyLightDir_8c226538[2]);
                njCnkSetEasyLightIntensity(var_fadeEasyLightIntensity_8c226544[0], var_fadeEasyLightIntensity_8c226544[1]);
                njCnkSetEasyLightColor(var_fadeEasyLightColor_8c22654c[0], var_fadeEasyLightColor_8c22654c[1], var_fadeEasyLightColor_8c22654c[2]);
                njSetTexture(slot->texlist);
                njCnkEasyDrawObject(slot->njDest);
            }

            slot = &var_tileLayerSlots_8c226520[1][flatIndex];
            if (slot->texlist != NULL) {
                njSetCamera(var_fadeCamera_8c226558);
                njCnkSetSimpleLight(var_fadeLightDir0_8c2264d8[0], var_fadeLightDir0_8c2264d8[1], var_fadeLightDir0_8c2264d8[2]);
                njCnkSetSimpleLightIntensity(var_fadeLightIntensity_8c2264f0[0], var_fadeLightIntensity_8c2264f0[1]);
                njCnkSetSimpleLightColor(var_fadeLightColor_8c2264f8[0], var_fadeLightColor_8c2264f8[1], var_fadeLightColor_8c2264f8[2]);
                njSetTexture(slot->texlist);
                njCnkSimpleDrawObject(slot->njDest);
            }

            slot = &var_tileLayerSlots_8c226520[2][flatIndex];
            if (slot->texlist != NULL) {
                njSetCamera(var_fadeCamera_8c226558);
                njCnkSetEasyLight(var_fadeEasyLightDir_8c226538[0], var_fadeEasyLightDir_8c226538[1], var_fadeEasyLightDir_8c226538[2]);
                njCnkSetEasyLightIntensity(var_fadeEasyLightIntensity_8c226544[0], var_fadeEasyLightIntensity_8c226544[1]);
                njCnkSetEasyLightColor(var_fadeEasyLightColor_8c22654c[0], var_fadeEasyLightColor_8c22654c[1], var_fadeEasyLightColor_8c22654c[2]);
                njSetTexture(slot->texlist);
                njCnkEasyDrawObject(slot->njDest);
            }

            slot = &var_tileLayerSlots_8c226520[3][flatIndex];
            if (slot->texlist != NULL) {
                njSetCamera(var_fadeCamera_8c226558);
                njCnkSetEasyLight(var_fadeEasyLightDir_8c226538[0], var_fadeEasyLightDir_8c226538[1], var_fadeEasyLightDir_8c226538[2]);
                njCnkSetEasyLightIntensity(var_fadeEasyLightIntensity_8c226544[0], var_fadeEasyLightIntensity_8c226544[1]);
                njCnkSetEasyLightColor(var_fadeEasyLightColor_8c22654c[0], var_fadeEasyLightColor_8c22654c[1], var_fadeEasyLightColor_8c22654c[2]);
                njSetTexture(slot->texlist);
                njCnkEasyDrawObject(slot->njDest);
            }
        }
        rowBase += width;
    }

    njControl3D(NJD_CONTROL_3D_MODEL_CLIP);
}

/* Same as drawTileGrid_8c021b9c, but for fade-command layer 1 (the mirror render):
 * light direction/setup differs (var_fadeLightDir1_8c2264e4 and camera
 * var_mirrorCamera_8c1bb944, set up by TileDrawEnqueueTask_8c0221d0), everything else -- including the
 * per-layer Easy/Simple split -- is identical. Reachable only via the
 * function-pointer literal TileDrawEnqueueTask_8c0221d0 pushes to FadeCmdPushCall2_8c022420,
 * never called directly, so it stays private. */
STATIC void drawTileGridMirror_8c021ec4(int width, int height)
{
    int colStart, colEnd, rowStart, rowEnd;
    int row, col, flatIndex, rowBase;
    LoadedModel *slot;

    colStart = (int)var_busState_8c1bb9d0.posX_0x2fc / 150 - var_fogParam_8c226508;
    colEnd = colStart + var_fogParam_8c226504;
    if (colStart < 0) {
        colStart = 0;
    }
    if (colEnd >= width) {
        colEnd = width - 1;
    }

    rowStart = (int)var_busState_8c1bb9d0.posZ_0x304 / 150 - var_fogParam_8c226508;
    rowEnd = rowStart + var_fogParam_8c226504;
    if (rowStart < 0) {
        rowStart = 0;
    }
    if (rowEnd >= height) {
        rowEnd = height - 1;
    }

    njControl3D(NJD_CONTROL_3D_MODEL_CLIP | NJD_CONTROL_3D_SHADOW | NJD_CONTROL_3D_TRANS_MODIFIER);

    rowBase = rowStart * width;
    for (row = rowStart; row <= rowEnd; row++) {
        for (col = colStart; col <= colEnd; col++) {
            flatIndex = rowBase + col;

            slot = &var_tileLayerSlots_8c226520[0][flatIndex];
            if (slot->texlist != NULL) {
                njSetCamera(var_fadeCamera_8c226558);
                njCnkSetEasyLight(var_fadeEasyLightDir_8c226538[0], var_fadeEasyLightDir_8c226538[1], var_fadeEasyLightDir_8c226538[2]);
                njCnkSetEasyLightIntensity(var_fadeEasyLightIntensity_8c226544[0], var_fadeEasyLightIntensity_8c226544[1]);
                njCnkSetEasyLightColor(var_fadeEasyLightColor_8c22654c[0], var_fadeEasyLightColor_8c22654c[1], var_fadeEasyLightColor_8c22654c[2]);
                njSetTexture(slot->texlist);
                njCnkEasyDrawObject(slot->njDest);
            }

            slot = &var_tileLayerSlots_8c226520[1][flatIndex];
            if (slot->texlist != NULL) {
                njSetCamera(var_fadeCamera_8c226558);
                njCnkSetSimpleLight(var_fadeLightDir1_8c2264e4[0], var_fadeLightDir1_8c2264e4[1], var_fadeLightDir1_8c2264e4[2]);
                njCnkSetSimpleLightIntensity(var_fadeLightIntensity_8c2264f0[0], var_fadeLightIntensity_8c2264f0[1]);
                njCnkSetSimpleLightColor(var_fadeLightColor_8c2264f8[0], var_fadeLightColor_8c2264f8[1], var_fadeLightColor_8c2264f8[2]);
                njSetTexture(slot->texlist);
                njCnkSimpleDrawObject(slot->njDest);
            }

            slot = &var_tileLayerSlots_8c226520[2][flatIndex];
            if (slot->texlist != NULL) {
                njSetCamera(var_fadeCamera_8c226558);
                njCnkSetEasyLight(var_fadeEasyLightDir_8c226538[0], var_fadeEasyLightDir_8c226538[1], var_fadeEasyLightDir_8c226538[2]);
                njCnkSetEasyLightIntensity(var_fadeEasyLightIntensity_8c226544[0], var_fadeEasyLightIntensity_8c226544[1]);
                njCnkSetEasyLightColor(var_fadeEasyLightColor_8c22654c[0], var_fadeEasyLightColor_8c22654c[1], var_fadeEasyLightColor_8c22654c[2]);
                njSetTexture(slot->texlist);
                njCnkEasyDrawObject(slot->njDest);
            }

            slot = &var_tileLayerSlots_8c226520[3][flatIndex];
            if (slot->texlist != NULL) {
                njSetCamera(var_fadeCamera_8c226558);
                njCnkSetEasyLight(var_fadeEasyLightDir_8c226538[0], var_fadeEasyLightDir_8c226538[1], var_fadeEasyLightDir_8c226538[2]);
                njCnkSetEasyLightIntensity(var_fadeEasyLightIntensity_8c226544[0], var_fadeEasyLightIntensity_8c226544[1]);
                njCnkSetEasyLightColor(var_fadeEasyLightColor_8c22654c[0], var_fadeEasyLightColor_8c22654c[1], var_fadeEasyLightColor_8c22654c[2]);
                njSetTexture(slot->texlist);
                njCnkEasyDrawObject(slot->njDest);
            }
        }
        rowBase += width;
    }

    njControl3D(NJD_CONTROL_3D_MODEL_CLIP);
}

/* The TaskAction FadeCmdPushTileDrawTask_8c0222dc installs: computes the
 * three fade light directions (broadcasting one scalar CourseSceneParams
 * component into a vector, then transforming it by whichever camera is
 * current -- as coded, not obviously intentional but preserved), then
 * queues drawTileGrid_8c021b9c/drawTileGridMirror_8c021ec4 as this frame's tile-grid draw calls for
 * fade layers 0/1, and TileStreamDrawTile_8c021b34 (the current tile,
 * `state`) for both layers. */
void TileDrawEnqueueTask_8c0221d0(Task *task, void *state)
{
    njSetCamera(&var_camera_8c1bb904);

    var_fadeEasyLightDir_8c226538[0] = var_sceneParams_8c18ad24->dir1_0x48[0];
    var_fadeEasyLightDir_8c226538[1] = var_sceneParams_8c18ad24->dir1_0x48[0];
    var_fadeEasyLightDir_8c226538[2] = var_sceneParams_8c18ad24->dir1_0x48[0];
    njCalcVector(NULL, (NJS_VECTOR *)var_fadeEasyLightDir_8c226538, (NJS_VECTOR *)var_fadeEasyLightDir_8c226538);

    var_fadeLightDir0_8c2264d8[0] = var_sceneParams_8c18ad24->dir2_0x68[0];
    var_fadeLightDir0_8c2264d8[1] = var_sceneParams_8c18ad24->dir2_0x68[0];
    var_fadeLightDir0_8c2264d8[2] = var_sceneParams_8c18ad24->dir2_0x68[0];
    njCalcVector(NULL, (NJS_VECTOR *)var_fadeLightDir0_8c2264d8, (NJS_VECTOR *)var_fadeLightDir0_8c2264d8);

    njSetCamera(&var_mirrorCamera_8c1bb944);

    var_fadeLightDir1_8c2264e4[0] = var_sceneParams_8c18ad24->dir2_0x68[0];
    var_fadeLightDir1_8c2264e4[1] = var_sceneParams_8c18ad24->dir2_0x68[0];
    var_fadeLightDir1_8c2264e4[2] = var_sceneParams_8c18ad24->dir2_0x68[0];
    njCalcVector(NULL, (NJS_VECTOR *)var_fadeLightDir1_8c2264e4, (NJS_VECTOR *)var_fadeLightDir1_8c2264e4);

    FadeCmdPushCall2_8c022420(0, drawTileGrid_8c021b9c, var_tileLayerIndexes_8c22650c[0]->width, var_tileLayerIndexes_8c22650c[0]->height);
    FadeCmdPushCall2_8c022420(1, drawTileGridMirror_8c021ec4, var_tileLayerIndexes_8c22650c[0]->width, var_tileLayerIndexes_8c22650c[0]->height);
    FadeCmdPushCall1_8c0223ea(0, (FadeCallback1)TileStreamDrawTile_8c021b34, (int)state);
    FadeCmdPushCall1_8c0223ea(1, (FadeCallback1)TileStreamDrawTile_8c021b34, (int)state);
}
