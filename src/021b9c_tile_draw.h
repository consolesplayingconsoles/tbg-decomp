/* 8c021b9c */
#ifndef _021B9C_TILE_DRAW_H
#define _021B9C_TILE_DRAW_H

#include "011120_asset_queues.h" /* LoadedModel */
#include "02171c_tile_stream.h"  /* TileIndex, TileRect */

/* =======================
 * Non-initialized Globals
 * =======================
 */

extern float var_simpleLightDir_8c2264d8[3]; // njCnkSetSimpleLight direction, main camera
extern float var_mirrorSimpleLightDir_8c2264e4[3]; // njCnkSetSimpleLight direction, mirror camera
/* Copies of var_sceneParams_8c18ad24->rec2_0x74[0..4]. Every access is
 * a bare single-word pool load, so the archive says nothing about whether the
 * five floats are one array or two; split here because the intensity pair and
 * the colour triple go to different SDK calls. */
extern float var_simpleLightIntensity_8c2264f0[2]; // [0..1]
extern float var_simpleLightColor_8c2264f8[3]; // [2..4]
extern int var_tileDrawSpan_8c226504;
extern int var_tileDrawRadius_8c226508;
/* Copied from var_currentCourse_8c1bb868.tileLayers_0x3c by
 * TileStreamInit_8c02175a; only the dims are read, the offset tables come
 * from var_datFiles_8c18adb4. */
extern TileIndex *var_tileLayerIndexes_8c22650c[5];
/* Per-layer tile grids, width * height slots each; layers 0-3 hold
 * texture+model pairs, layer 4 model only. [0] is -1 while unallocated. */
extern LoadedModel *var_tileLayerSlots_8c226520[5];
extern TileRect *var_currentTileRegionList_8c226534; /* -1 when unset */

/* =========
 * Functions
 * =========
 */

void TileDrawSpawnTask_8c0222dc(void);

#endif // _021B9C_TILE_DRAW_H
