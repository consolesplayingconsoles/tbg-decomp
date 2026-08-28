/* 8c013ae8 */
#ifndef _013AE8_ROUTE_LOAD_H
#define _013AE8_ROUTE_LOAD_H

#include <shinobi.h>
#include "014a9c_tasks.h"
#include "02171c_tile_stream.h" /* TileIndex */

/* =================
 * Type Declarations
 * =================
 */

enum ROUTE {
    ROUTE_SHINJUKU = 0,
    ROUTE_WANGAN   = 1,
    ROUTE_OME      = 2,
};

enum TIME_OF_DAY {
    TIME_OF_DAY_DAY     = 0,
    TIME_OF_DAY_EVENING = 1,
    TIME_OF_DAY_NIGHT   = 2,
};

typedef struct {
    int field_0x00;
    float field_0x04;
    Uint8 field_0x08;
    Uint8 field_0x09;
    Uint8 field_0x0a;
    Uint8 field_0x0b;
    float fogN_0x0c;
    float fogF_0x10;
} FogParams;

/* One entry of var_routeModelSlots_8c1bbddc. */
typedef struct {
    int requested_0x00;
    int needsLoad_0x04;
    NJS_TEXLIST *texlist_0x08; // -1 when unloaded
    void *nj_0x0c;
} ModelSlot;

/* Asset handles for the loaded course, filled by loadRouteModels_8c014088 from
 * the CourseConfig filename field of the same name, and freed by
 * DebugMenuFreeSessionAssets_8c016182. The names are the course's own file
 * names (Shinjuku day: "s_atari_bus.dat", "sd_road_x.dat", ...): atari =
 * collision, line = route path, attr = attributes, mac = machine/actor,
 * hum = pedestrian. */
typedef struct {
    int courseId_0x00;
    void *atariBus_0x04;
    void *lineBus_0x08;
    void *ukn_0x0c;      /* not a filename: a table pointer copied from the config, and not owned */
    void *attrBus_0x10;
    void *attrMark_0x14;
    void *atariCpu_0x18;
    void *lineCpu_0x1c;
    void *attrCpu_0x20;
    void *macCpu1_0x24;
    void *atariHum_0x28;
    void *lineHum_0x2c;  /* the one asset kept as a model rather than a texlist */
    void *macHumG0_0x30;
    void *macHumM0_0x34;
    void *macSignal_0x38;
    /* road_x, machi_x (town), uv_x, shadow_x -- Ome splits uv into uv1/uv2,
     * Shinjuku and Wangan repeat shadow_x for the model-only 5th layer.
     * Consumed by TileStreamInit_8c02175a. */
    TileIndex *tileLayers_0x3c[5];
} CurrentCourse;

/* =====================
 * External Declarations
 * =====================
 */

extern enum ROUTE var_route_8c18ad1c;
extern enum TIME_OF_DAY var_timeOfDay_8c18ad20;
extern FogParams *var_fogParams_8c18ad28;

/* 16-byte game_name / VMS sort key baked into the backup file header. */
extern Sint8 init_8c04410c[16];

extern char var_pvrDir_8c18ad4c[0x20];
extern void *var_datFiles_8c18adb4[4]; /* one per texel layer, freed after TileStreamLoad_8c021810 */

/* =========
 * Functions
 * =========
 */

void RouteLoadPushTask_8c0144fc(void);
void RouteLoadPushTask_8c0144fc(void);
void RouteLoadPushSegmentReloadTask_8c01468e(void);
void RouteLoadSetPvmReady_8c014330(void);
void RouteLoadResetPvmReady_8c014322(void);
void RouteLoadFreeVehicleAssets_8c013b5a(void);
void RouteLoadClearModelSlots_8c013bbc(ModelSlot *slots, int count);
void RouteLoadStartRouteModelLoadPass_8c013d78(void);
void RouteLoadFreeAllRouteModels_8c013dae(void);
void RouteLoadFreePedestrianAssets_8c013ee4(void);
int RouteLoadIsPvmReady_8c01432a(void);
void RouteLoadUnusedTask_8c014784(Task *task, void *state);

#endif // _013AE8_ROUTE_LOAD_H
