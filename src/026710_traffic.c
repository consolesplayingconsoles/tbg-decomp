/* @unit Traffic */

#include <shinobi.h>

#include "011120_asset_queues.h"
#include "013ae8_route_load.h"
#include "014a9c_tasks.h"
#include "0222dc_fadecmd.h"
#include "020914_ground_query.h" /* GroundQueryFindPolygon_8c020914, GroundQueryResult */
#include "020b6c_ground_probe.h"
#include "025b98_traffic_drive.h"
#include "026710_traffic.h"
#include "028258_objects.h"
#include "02786c_vehicle_parts.h"
#include "02c884_bus_stop.h"
#include "02df3c.h"
#include "02e51c.h"
#include "02f0c8.h"
#include "sectionB.h"
#include "sectionD.h"
#include "serial_debug.h"

/* ====================
 * Initialized Globals
 * ====================
 */

/* Opcode word lengths for TrafficRunEntryScript_8c027012, indexed by opcode. */
STATIC Uint8 init_8c0460bc[] = {
    0x01, 0x03, 0x03, 0x03, 0x03, 0x02, 0x03, 0x02, 0x04, 0x01, 0x04, 0x00,
};

/* Per-variant body dimensions, indexed by the 0..15 variant index: one record
 * of {width, height, groundOffset, length}. Ghidra split the first record's
 * leading three floats into separate symbols because the code addresses each
 * field directly, and the .src still carries those labels -- but only
 * init_8c0460c8 is a real symbol ("from defines"; the others are marked "from
 * ghidra"), and the table is one contiguous 0x100 block ending exactly where
 * init_8c0461c8 begins. Grouped by record here, which is why the rows differ
 * from the .src's layout. Paired variants share dimensions. */
STATIC float init_8c0460c8[16][4] = {
    { 2.42f, 1.67f, 0.65f, 3.0500002f }, /* 2do0 */
    { 2.42f, 1.67f, 0.65f, 3.0500002f }, /* 2do1 */
    { 2.2f,  1.73f, 0.76f, 3.04f      }, /* 4wd  */
    { 2.8f,  1.8f,  0.9f,  3.8f       }, /* sed0 */
    { 2.8f,  1.8f,  0.9f,  3.8f       }, /* sed1 */
    { 2.8f,  1.8f,  0.9f,  3.8f       }, /* sed2 */
    { 2.8f,  1.7f,  0.7f,  3.9f       }, /* tax  */
    { 6.1f,  2.03f, 1.5f,  8.9f       }, /* tor0 */
    { 6.1f,  2.03f, 1.5f,  8.9f       }, /* tor1 */
    { 2.7f,  1.8f,  1.0f,  4.0f       }, /* kto  */
    { 4.0f,  2.07f, 2.0f,  5.3f       }, /* dan0 */
    { 4.0f,  2.07f, 2.0f,  5.3f       }, /* dan1 */
    { 2.6f,  1.53f, 0.9f,  3.5f       }, /* wag  */
    { 4.9f,  2.33f, 2.6f,  8.0f       }, /* bus0 */
    { 2.8f,  1.69f, 0.74f, 3.96f      }, /* pat  */
    { 2.6f,  1.68f, 1.0f,  3.8999999f }, /* kyu  */
};

STATIC float init_8c0461c8[] = {
    0.01f, 0.01f, 0.004f, 0.007f,
    0.007f, 0.007f, 0.007f, 0.002f,
    0.002f, 0.004f, 0.002f, 0.002f,
    0.004f, 0.001f, 0.01f, 0.01f,
};

STATIC Uint8 init_8c046208[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x00,
};

STATIC Uint8 init_8c04622c[] = {
    0x00, 0x00, 0x01, 0x02, 0x02, 0x02, 0x03, 0x04, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x09, 0x0A,
};

/* ====================
 * Functions
 * ====================
 */

/* Decodes a stage script into the entry's resolved-args array (offset 0x304),
 */
void TrafficReadScriptArgs_8c026710(TrafficEntry *entry, Uint16 *script)
{
    Sint32 *out;
    Uint16 *ip;

    out = (Sint32 *)entry->resolvedArgs_0x304;
    for (ip = (Uint16 *)((Uint8 *)script + 2); *ip != 9; ip += init_8c0460bc[*ip]) {
        if (*ip == 1) {
            *out = var_8c227e1c[ip[1]];
            out++;
        }
    }
    *out = -1;
}

/* Runs one "spawn" or "place decoration" instruction of the entry's script
 * (opcodes 0 and 10 in the caller, FUN_8c027012): walks the path to the
 * entry's starting distance, resolves its position and heading, fills in
 * the vehicle's dimension/animation state from its variant table, and pushes
 * its driving task. entry+0x2f8 holds the script's own base pointer, so
 * *entry->0x2f8 is the script's header word: 10 marks a fixed-angle static
 * decoration (traffic light, sign, ...) rather than a path-following
 * vehicle. entry+0x304 onward is itself an inline array of per-block path
 * pointers (the same region TrafficReadScriptArgs_8c026710 resolves into
 * from the entry's script), indexed by the segment index at entry+0x300;
 * each block is a run of {len,x,y,dx,dy} records terminated by a len==0
 * sentinel record.
 */
STATIC void initEntryState_8c026748(TrafficEntry *entry, int *scriptIp)
{
    TrafficEntry *e = entry;
    Uint16 type = *e->scriptCursor_0x2f8;
    PathRecord *seg = (PathRecord *)0;
    int scriptCursor = 0;
    float dist = 0.0f;
    Sint32 segIdx;
    Sint32 variantIdx;
    const float *dims;
    void *junction;
    /* 4 bytes larger than GroundQueryResult; the extra word keeps the frame
     * layout the original build had. */
    Uint8 groundBuf[20];
    float *zeroPtr;
    Uint16 speedRandom;

    if (type != 10) {
        seg = e->resolvedArgs_0x304[0];
        scriptCursor = *scriptIp + 2;
        e->field_0x300 = 0;
        dist = e->field_0x2e8 + 2.0f;
        while (seg->length_0x00 <= dist) {
            dist -= seg->length_0x00;
            seg++;
            if (seg->length_0x00 == 0.0f) {
                segIdx = e->field_0x300 + 1;
                e->field_0x300 = segIdx;
                seg = e->resolvedArgs_0x304[segIdx];
                scriptCursor += 6;
            }
        }
    }

    e->field_0x408 = 0;
    e->field_0x40c = 0;
    e->field_0x504 = 0;
    e->field_0x508 = 0;
    e->field_0x50c = 0;
    variantIdx = e->variantIndex_0x2e0;
    e->field_0x064 = 0;
    e->field_0x068 = 0;
    e->field_0x06c = 0;
    e->field_0x070 = 0;
    e->field_0x074 = 0;
    e->field_0x078 = 0;
    e->field_0x07c = 0;
    e->field_0x080 = 0;

    if (var_timeOfDay_8c18ad20 == TIME_OF_DAY_NIGHT && type == 10 &&
        (junction = FUN_8c02e51c(e->posX_0xf4, e->posY_0xf8,
                                  e->posZ_0xfc, &e->field_0x404)) != (void *)0 &&
        *(Sint32 *)((Uint8 *)junction + 4) != 0) {
        e->field_0x0c4 = var_8c1bbdb0[0];
        e->field_0x0c8 = var_8c1bbdb0[1];
        e->field_0x0cc = var_8c1bbdd0[0];
        e->field_0x0d0 = var_8c1bbdd0[1];
        e->field_0x0d4 = var_8c1bbdd0[2];
    } else {
        e->field_0x0d8 = var_sceneParams_8c18ad24->rec0_0x0c[2][0];
        e->field_0x0dc = var_sceneParams_8c18ad24->rec0_0x0c[2][1];
        e->field_0x0e0 = var_sceneParams_8c18ad24->rec0_0x0c[2][2];
        e->field_0x0e4 = var_sceneParams_8c18ad24->rec0_0x0c[2][3];
        e->field_0x0e8 = var_sceneParams_8c18ad24->rec0_0x0c[2][4];
        e->field_0x0c4 = var_sceneParams_8c18ad24->rec0_0x0c[1][0];
        e->field_0x0c8 = var_sceneParams_8c18ad24->rec0_0x0c[1][1];
        e->field_0x0cc = var_sceneParams_8c18ad24->rec0_0x0c[1][2];
        e->field_0x0d0 = var_sceneParams_8c18ad24->rec0_0x0c[1][3];
        e->field_0x0d4 = var_sceneParams_8c18ad24->rec0_0x0c[1][4];
    }

    if (type != 10) {
        e->posX_0xf4 = (dist - 2.0f) * seg->dirX_0x0c + seg->x_0x04 + e->originJitterX_0x2ec;
        e->posZ_0xfc = (dist - 2.0f) * seg->dirZ_0x10 + seg->z_0x08 + e->originJitterZ_0x2f0;
    }

    GroundQueryFindPolygon_8c020914(e->posX_0xf4, 0.0f, e->posZ_0xfc,
                                    (GroundQueryResult *)groundBuf);
    GroundProbeInterpolateHeight_8c020f7e((GroundQueryResult *)groundBuf, &e->posX_0xf4);

    dims = init_8c0460c8[variantIdx];
    e->width_0x23c = dims[0];
    e->length_0x240 = dims[3];
    e->height_0x244 = dims[1];
    e->halfHeight_0x248 = dims[1] / 2.0f;
    e->groundOffset_0x24c = dims[2];
    e->field_0x258 = 0;
    e->field_0x268 = 0;
    e->field_0x270 = 1.0f;
    e->field_0x26c = 0;

    if (type == 10) {
        e->field_0x278 = njCos(e->heading_0x250);
        e->field_0x274 = njSin(e->heading_0x250);
    } else {
        e->field_0x278 = seg->dirZ_0x10;
        e->field_0x274 = seg->dirX_0x0c;
    }

    e->field_0x100 = e->posX_0xf4 - e->width_0x23c * e->field_0x274;
    e->field_0x108 = e->posZ_0xfc - e->width_0x23c * e->field_0x278;
    e->field_0x104 = e->posY_0xf8;
    e->field_0x11c = e->posY_0xf8;
    e->field_0x128 = e->posY_0xf8;
    e->groundProbe_0x190[0].vertexIds_0x08 = 0;
    e->groundProbe_0x190[0].count_0x0c = 0;
    e->groundProbe_0x190[1].vertexIds_0x08 = 0;
    e->groundProbe_0x190[1].count_0x0c = 0;
    e->groundProbe_0x190[2].vertexIds_0x08 = 0;
    e->groundProbe_0x190[2].count_0x0c = 0;
    e->groundProbe_0x190[3].vertexIds_0x08 = 0;
    e->groundProbe_0x190[3].count_0x0c = 0;
    e->driveState_0x2b4 = 0;
    e->pathRecord_0x2b8 = seg;
    e->pathDistanceCopy_0x2c0 = dist;
    e->pathDistance_0x2bc = dist;
    e->projectDistance_0x2c4 = 2.0f;
    e->field_0x2d4 = 0;
    e->field_0x2d8 = 0;
    e->field_0x2dc = 0;

    if (type == 10) {
        e->speed_0x27c = 0;
    } else {
        zeroPtr = &e->field_0x280[1];
        e->field_0x414 = (float)*(Uint16 *)(scriptCursor + 4) / 65536.0f;
        e->speed_0x27c = e->field_0x414 / 2.0f;
        do {
            *zeroPtr = 0;
            zeroPtr++;
        } while (zeroPtr < &e->field_0x290);
        e->field_0x290 = init_8c0461c8[variantIdx];
        e->field_0x418 = 9999.0f;
        e->field_0x424 = 0;
        e->field_0x428 = 0;
        speedRandom = AsqGetRandomA_8c012166();
        e->field_0x41c = (float)speedRandom / 65536.0f + 1.0f;
        e->field_0x42c = 0;
        e->field_0x448 = 0;
        e->field_0x450 = 0xffffffff;
        e->field_0x458 = 0;
        e->field_0x498 = 0;
        e->field_0x468 = 0;
        e->field_0x474 = 0;
        *scriptIp = scriptCursor + 6;
        e->field_0x490 = 9999.0f;
        e->field_0x4ec = 0.0f;
        e->field_0x4f4 = seg;
        e->field_0x4f8 = e->field_0x300;
        e->field_0x4fc = dist + 2.5f;
        TrafficLookaheadInit_8c02df3c(entry);
    }
}

/* Re-derives heading and the vehicle's 4 body-corner points after the entry
 * reaches a new waypoint (e+0x100/0x108): direction is (target - current)
 * normalized by its own length (njHypot, not the incoming float argument --
 * confirmed via dual-object test that the first parameter goes completely
 * unused by this function), matching the sin/dx=e+0x274, cos/dy=e+0x278
 * convention from initEntryState_8c026748. e+0x23c/0x240 place the
 * front/rear reference points along the new heading; e+0x248 (half-width)
 * offsets those into the 4 corners at e+0x118/0x120/0x124/0x12c. The heading
 * angle (e+0x250) is njArcCos(dy) with its sign flipped when dx < 0 --
 * confirmed via test that acosf's argument is the original normalized dy,
 * not the half-width-scaled value that overwrites the same Ghidra SSA
 * variable just before the call.
 */
void TrafficUpdateHeading_8c026bc4(float unused, TrafficEntry *entry)
{
    TrafficEntry *e = entry;
    float dx = e->field_0x100 - e->posX_0xf4;
    float dy = e->field_0x108 - e->posZ_0xfc;
    float dist = njSqrt(dx * dx + dy * dy);
    float halfDx, halfDy;
    float angle;
    int iAngle;

    dx = dx / dist;
    dy = dy / dist;
    e->field_0x274 = dx;
    e->field_0x278 = dy;

    e->field_0x100 = dx * e->width_0x23c + e->posX_0xf4;
    e->field_0x108 = dy * e->width_0x23c + e->posZ_0xfc;
    e->field_0x10c = dx * e->length_0x240 + e->posX_0xf4;
    e->field_0x114 = dy * e->length_0x240 + e->posZ_0xfc;

    halfDy = e->halfHeight_0x248 * dy;
    halfDx = e->halfHeight_0x248 * dx;
    e->field_0x118 = e->posX_0xf4 - halfDy;
    e->field_0x120 = e->posZ_0xfc + halfDx;
    e->field_0x124 = e->posX_0xf4 + halfDy;
    e->field_0x12c = e->posZ_0xfc - halfDx;

    angle = acosf(dy);
    iAngle = (int)((angle * 65536.0) / 6.283184);
    if (dx < 0.0f) {
        iAngle = -iAngle;
    }
    e->heading_0x250 = iAngle;
}


/* Advances the entry's path-segment cursor (entry+0x2b8 record pointer,
 * entry+0x2bc distance already accumulated into the current record) to the
 * {len,x,y,dx,dy} record containing that distance, then re-derives the
 * vehicle's world position by projecting a fixed distance (entry+0x2c4) from
 * the record's point (entry+0xec/0xf0) towards the current position, along
 * the direction from that record point back to the current position
 * (njHypot, not the incoming float argument -- confirmed unused via probing,
 * same as TrafficUpdateHeading_8c026bc4). Also refreshes the secondary
 * heading angle at entry+0x254 (acosf(dy), sign-flipped when the normalized
 * dx <= 0). Returns 1 when a containing record was found; 0 when the
 * segment run is exhausted (len==0 sentinel), in which case only the
 * segment pointer/distance fields are updated for the caller to supply the
 * next path block.
 */
Sint32 TrafficAdvanceOnPath_8c026ca2(float unused, TrafficEntry *entry)
{
    TrafficEntry *e = entry;
    PathRecord *seg = e->pathRecord_0x2b8;
    float dist = e->pathDistance_0x2bc;
    float segX, segY;
    float dx, dy, len;
    Sint32 angle;

    (void)unused;

    while (dist >= seg->length_0x00) {
        dist -= seg->length_0x00;
        seg++;
        if (seg->length_0x00 == 0.0f) {
            e->pathRecord_0x2b8 = seg;
            e->pathDistance_0x2bc = dist;
            e->pathDistanceCopy_0x2c0 = dist;
            return 0;
        }
    }

    e->pathRecord_0x2b8 = seg;
    e->pathDistance_0x2bc = dist;

    segX = dist * seg->dirX_0x0c + seg->x_0x04 + e->originJitterX_0x2ec;
    segY = dist * seg->dirZ_0x10 + seg->z_0x08 + e->originJitterZ_0x2f0;
    e->field_0x0ec = segX;
    e->field_0x0f0 = segY;

    dx = e->posX_0xf4 - segX;
    dy = e->posZ_0xfc - segY;
    len = njSqrt(dx * dx + dy * dy);
    dx /= len;
    dy /= len;

    e->posX_0xf4 = dx * e->projectDistance_0x2c4 + segX;
    e->posZ_0xfc = dy * e->projectDistance_0x2c4 + segY;

    angle = (Sint32)((acosf(dy) * 65536.0) / 6.283184);
    if (dx <= 0.0f) {
        angle = -angle;
    }
    e->headingAlt_0x254 = angle;

    return 1;
}

/* Relocation fixup for a freshly-loaded blob (called generically from
 * routeLoadTask_8c014338/RouteLoadUnusedTask_8c014784 after loading course
 * data): an array of self-relative offsets from the blob's own base
 * (handle), terminated by 0, each pointing to a record run. Each record is
 * 0xc bytes; the offset array's own slot is overwritten with the record's
 * absolute address, and the record's dword at +4 is likewise turned from a
 * self-relative offset into an absolute pointer, walking 0xc-byte strides
 * until a record's +4 field is 0. Same shape as FUN_8c028dd0/FUN_8c028de8
 * in 028258_objects.c. */
void TrafficRelocatePlacementTable_8c026da4(void *handle)
{
    Sint32 *outer;
    TrafficPlacement *rec;

    for (outer = (Sint32 *)handle; *outer != 0; outer++) {
        *outer += (Sint32)handle;
        rec = (TrafficPlacement *)*outer;
        for (; rec->script_0x04 != NULL; rec++) {
            rec->script_0x04 = (Uint16 *)((Uint8 *)rec->script_0x04 + (Sint32)handle);
        }
    }
}

/* Ghidra needed its prototype forced to
 * `float FUN_8c026eaa(void *entry, void *other)`, matching its only caller's
 * two-pointer call sites and float use of the result. Its CONCAT44/ulonglong
 * dance is an artifact of modelling that return as a paired double register:
 * the high dword is always 0 and the real return is a plain float in FR0, so
 * the store of it to other's 0x418 field is just storing 0.0f (both
 * interpretations share the same all-zero bit pattern). */

/* Scans the global list of traffic entries starting at "other" (advanced via
 * TrafficPathScanNext_8c02f212, an accessor with no arguments -- the list cursor is
 * maintained elsewhere) looking for one whose path projection is ahead of
 * "entry" (own current entry), to derive a speed limit "entry" must obey to
 * avoid it. var_8c1bbd9c is a sentinel pointer value standing in for the
 * player's bus in this list; when "other" equals it, the check instead reads
 * the bus's own waypoint fields directly (bus has no ordinary entry struct)
 * and the scan always stops there. Returns 9999.0f (no constraint) unless a
 * blocking entry lowers it. As a side effect, an entry found already ahead of
 * "entry" (0x2c0 <= entry's own 0x2c0) has its own 0x418 field refreshed with
 * "entry"'s current speed margin (own use unclear here; initEntryState_8c026748
 * initializes the same field to 9999.0f).
 */
float TrafficComputeBlockedSpeed_8c026eaa(TrafficEntry *entry, TrafficEntry *other)
{
    TrafficEntry *e = entry;
    TrafficEntry *o = other;
    float result = 9999.0f;
    float margin = -0.18518518f;
    float dot;
    float candidate;

    for (; o != NULL; o = (TrafficEntry *)TrafficPathScanNext_8c02f212()) {
        if (o == (TrafficEntry *)var_8c1bbd9c) {
            dot = (o->field_0x100 - var_busState_8c1bb9d0.posX_0x0f4) *
                      (o->field_0x100 - o->posX_0xf4) +
                  (o->field_0x108 - o->posZ_0xfc) *
                      (o->field_0x108 - var_busState_8c1bb9d0.posZ_0x0fc);
            if (dot < 0.0f) {
                candidate = var_busState_8c1bb9d0.speed_0x27c + margin;
                result = candidate;
                if (candidate < 0.0f) {
                    result = 0.0f;
                }
            }
            break;
        }

        if (o->pathDistanceCopy_0x2c0 <= e->pathDistanceCopy_0x2c0) {
            o->field_0x418 = e->speed_0x27c + margin;
            if (o->field_0x418 < 0.0f) {
                o->field_0x418 = 0.0f;
            }
        } else {
            candidate = o->speed_0x27c + margin;
            result = candidate;
            if (candidate < 0.0f) {
                result = 0.0f;
            }
        }
    }

    return result;
}

/* Rebuilds the "traffic signal frame in use" table (var_trafficSignalFrames_8c227e24,
 * 0..maxId) ahead of loading a new segment's decoration scripts, so a
 * newly-spawned signal's FUN_8c026748 init path can pick an id nothing else
 * already owns. When the segment has no scene-object-type list at all
 * (sceneObjectTypeIds_0x14 == NULL), every id up to maxId is conservatively
 * marked in-use. Otherwise every id starts free, then every already-placed
 * decoration script (opcode 5 = fixed id, same opcode table as
 * TrafficReadScriptArgs_8c026710) across every scene-object type listed for
 * this segment marks its id in-use -- one type's placed-object list is a run
 * of TrafficRelocatePlacementTable_8c026da4-fixed-up 0xc-byte records, each holding its own script
 * pointer at +4, terminated by a 0 there. The per-type table base is
 * var_currentCourse_8c1bb868.macCpu1_0x24 (Ghidra showed it as a standalone
 * global at that address's coincidental offset into the CurrentCourse
 * struct -- confirmed via dual-object probe, "unresolved relocation
 * _var_currentCourse_8c1bb868" on a guess that didn't read that struct). */
void TrafficMarkSignalIdsInUse_8c026dcc(int maxId)
{
    CourseSegment *seg;
    Uint8 *typeIds;
    void **table;
    TrafficPlacement *rec;
    Uint16 *ip;
    int i;

    seg = BusStopGetSegment_8c02cd6a(var_currentSegment_8c228708);
    typeIds = seg->sceneObjectTypeIds_0x14;

    if (typeIds == (Uint8 *)0) {
        for (i = 0; i <= maxId; i++) {
            var_trafficSignalFrames_8c227e24[i] = 1;
        }
        return;
    }

    for (i = 0; i < maxId; i++) {
        var_trafficSignalFrames_8c227e24[i] = 0;
    }

    table = (void **)var_currentCourse_8c1bb868.macCpu1_0x24;
    for (; *typeIds != 0xff; typeIds++) {
        for (rec = (TrafficPlacement *)table[*typeIds]; rec->script_0x04 != NULL; rec++) {
            for (ip = rec->script_0x04; *ip != 9; ip += init_8c0460bc[*ip]) {
                if (*ip == 5) {
                    var_trafficSignalFrames_8c227e24[ip[1]] = 1;
                }
            }
        }
    }
}

void TrafficUpdateFrameFlags_8c026f7e(TrafficEntry *entry)
{
    TrafficEntry *e = entry;
    Uint32 counter = e->field_0x260;

    e->field_0x260 = counter + 1;
    if ((counter & 0x10) == 0) {
        if (e->field_0x47c == 0) {
            e->field_0x080 = e->field_0x080 | 2;
        } else {
            e->field_0x080 = e->field_0x080 | 4;
        }
    }
}

/* Discounting the CONCAT44/in_fr1 artifact --
 * same float-as-double-return misreading noted in
 * TrafficComputeBlockedSpeed_8c026eaa's header comment; the real return is a
 * plain float in FR0.
 */

/* Sums the {len,...} record lengths from the entry's current path record
 * (entry+0x2b8) to the end of the current path block (len==0 terminator),
 * minus the distance already consumed into the current record
 * (entry+0x2bc) -- i.e. the remaining distance to the end of this path
 * block. Read-only: unlike TrafficAdvanceOnPath_8c026ca2, it does not update
 * the entry's segment pointer or distance fields.
 */
float TrafficRemainingPathDistance_8c026fb0(TrafficEntry *entry)
{
    TrafficEntry *e = entry;
    PathRecord *seg = e->pathRecord_0x2b8;
    float remaining = -e->pathDistance_0x2bc;

    do {
        remaining += seg->length_0x00;
        seg++;
    } while (seg->length_0x00 != 0.0f);

    return remaining;
}

void TrafficSeekPathRecord_8c026fcc(TrafficEntry *entry, PathRecord *seg)
{
    TrafficEntry *e = entry;
    Sint32 segIdx;
    float dist = e->pathDistanceCopy_0x2c0;

    while (seg->length_0x00 <= dist) {
        dist -= seg->length_0x00;
        seg++;
        if (seg->length_0x00 == 0.0f) {
            segIdx = e->field_0x300 + 1;
            e->field_0x300 = segIdx;
            seg = e->resolvedArgs_0x304[segIdx];
        }
    }

    e->resolvedArgs_0x304[e->field_0x300] = seg;
    e->pathRecord_0x2b8 = seg;
    e->pathDistance_0x2bc = dist;
}


/* Per-frame(ish) script interpreter for one traffic entry: walks opcodes
 * starting at the entry's saved cursor (entry+0x2fc) until it hits a
 * stopping condition, then writes the cursor back. Opcode 0 (spawn) is
 * consumed transparently in a loop of its own -- initEntryState_8c026748
 * is called for every run of opcode-0 words, and the cursor it leaves behind
 * (via its scriptIp out-param) is re-read for the next opcode immediately,
 * without yielding. Every other opcode advances the cursor by its own fixed
 * word length (init_8c0460bc) and is handled in one pass:
 *
 *   1 - the first time this run: marks the entry spawned (same flag opcode 0
 *       sets), advances to the next resolved path block (entry+0x300 index
 *       into the entry+0x304 pointer array), and refreshes entry+0x414
 *       (lane-offset ratio, same field initEntryState_8c026748
 *       seeds). Once already spawned (whether by an earlier opcode 0 or by
 *       this very opcode on a previous call), it instead acts as a yield
 *       point: it halts the interpreter *without* consuming itself, so the
 *       same instruction is reprocessed on the next call.
 *   2/3 - configure entries at 0x42c/0x430/0x434/0x438 (0x430 distinguishes
 *       the two opcodes), then continue.
 *   4 - skip (3 words), no state change.
 *   5/6/7 - configure decoration-ish slots (0x448/0x458/0x468 families),
 *       tagging each with the entry's current block index (0x300), then
 *       continue.
 *   8 - configure the 0x474/0x478/0x47c/0x480 family, resolving its id
 *       argument through var_8c227e1c the same way TrafficReadScriptArgs_8c026710
 *       does, then continue.
 *   9 - end of script: if the entry never spawned this run, report failure
 *       (0); otherwise halt (cursor stays put -- op 9 does not advance).
 *   10 - place a fixed-position/fixed-heading decoration (entry+0xf4/0xfc/
 *       0x250/0x254), call initEntryState_8c026748 to finish spawning
 *       it, then halt.
 *
 * Returns 1 for every halt except an opcode-9 with no prior spawn, which
 * returns 0.
 */
Sint32 TrafficRunEntryScript_8c027012(TrafficEntry *entry)
{
    TrafficEntry *e = entry;
    Uint16 *ip = e->scriptBase_0x2fc;
    Uint16 *cur;
    Uint16 op;
    Sint32 spawned = 0;
    Sint32 blockIdx;

    for (;;) {
        cur = ip;
        op = *ip;

        if (op == 0) {
            initEntryState_8c026748(entry, (int *)&ip);
            e->field_0x2e8 = 0;
            spawned = 1;
            continue;
        }

        if (op == 1) {
            if (spawned) {
                break;
            }
            spawned = 1;
            blockIdx = e->field_0x300 + 1;
            e->field_0x300 = blockIdx;
            e->pathRecord_0x2b8 = e->resolvedArgs_0x304[blockIdx];
            ip = cur + 3;
            e->field_0x414 = (float)cur[2] / 65536.0f;
            continue;
        }

        switch (op) {
        case 2:
        case 3:
            e->field_0x42c = 1;
            e->field_0x430 = (op == 3) ? 1 : 0;
            e->field_0x434 = cur[1];
            e->field_0x438 = cur[2];
            ip = cur + 3;
            continue;
        case 4:
            ip = cur + 3;
            continue;
        case 5:
            e->field_0x448 = 2;
            e->field_0x450 = cur[1];
            e->field_0x454 = e->field_0x300;
            ip = cur + 2;
            continue;
        case 6:
            e->field_0x458 = 2;
            e->field_0x45c = cur[1];
            e->field_0x460 = cur[2];
            e->field_0x464 = e->field_0x300;
            ip = cur + 3;
            continue;
        case 7:
            e->field_0x468 = 2;
            e->field_0x46c = cur[1];
            e->field_0x470 = e->field_0x300;
            ip = cur + 2;
            continue;
        case 8:
            e->field_0x474 = 1;
            e->field_0x478 = cur[1];
            e->field_0x47c = cur[2];
            e->field_0x484 = var_8c227e1c[cur[3]];
            e->field_0x480 = e->field_0x300;
            ip = cur + 4;
            continue;
        case 9:
            if (!spawned) {
                return 0;
            }
            goto done;
        case 10:
            e->posX_0xf4 = (float)cur[1] / 10.0f;
            e->posZ_0xfc = (float)cur[2] / 10.0f;
            e->heading_0x250 = cur[3];
            e->headingAlt_0x254 = e->heading_0x250 + 0x8000;
            ip = cur + 3;
            initEntryState_8c026748(entry, (int *)&ip);
            goto done;
        default:
            /* Unreachable with well-formed scripts: init_8c0460bc only
             * defines lengths for opcodes 0-10. Preserved as real asm
             * behavior -- an unrecognized opcode leaves the cursor
             * untouched and loops forever. */
            continue;
        }
    }

done:
    e->scriptBase_0x2fc = ip;
    return 1;
}

/* Spawns one traffic entry for the script's opcode-0 header word (typeCode),
 * script pointer, and route progress (only ever forwarded to
 * TrafficAdvanceOnPath_8c026ca2/TrafficUpdateHeading_8c026bc4, both of which
 * ignore it -- see their own comments).
 *
 * typeCode's low byte is first checked against a per-(route,timeOfDay) day
 * bitmask (init_8c046208): for typeCode 0x1c/0x1e, a clear bit means "not
 * today" and the entry is skipped entirely (still returns 1, as if spawned).
 *
 * The script's header word (*script) selects the driving task: 10 marks a
 * fixed decoration (TrafficDriveDecoration_8c02656a, entry+0x2e4=1), anything else a
 * path-following vehicle (TrafficDriveVehicle_8c025b98, entry+0x2e4=0). typeCode's 0x8000/
 * 0x4000 bits pick the animation kind (entry+0x48c) and the ground/junction
 * query callback pair stored at entry+0x2c8/0x2cc (elevated-road vs. normal).
 *
 * typeCode's low 12 bits then index a pair of consecutive
 * var_routeModelSlots_8c1bbddc entries (front/rear texlist+model, entry+4/
 * 0x8/0xc/0x10); halved, they give the variant index at entry+0x2e0, used to
 * resolve the variant's body-type animation data (init_8c04622c ->
 * var_trafficModels_8c1bc3f4, entry+0x14). An unloaded model slot
 * (texlist_0x08 == -1) frees the task and skips the rest of the setup below
 * -- but real asm behavior: it still returns 1, same as a completed spawn.
 *
 * For a moving vehicle (*script != 10), the script's block-relative args are
 * resolved (TrafficReadScriptArgs_8c026710), then TrafficPathScanBuild_8c02f0c8 gets a chance
 * to reject the entry outright (TaskFree, return 0 -- this is the only path
 * that actually reports failure after allocation); on acceptance, the
 * entry's start progress and a small random per-entry path-origin jitter
 * (entry+0x2ec/0x2f0) are recorded.
 *
 * Finally the entry's script cursor is armed (entry+0x2f8/0x2fc), VehPartsBind_8c02786c
 * runs any remaining per-type setup, and the entry's script is run once
 * (TrafficRunEntryScript_8c027012) before its path/heading are derived for
 * the first frame. Returns 1 on every completed spawn (day-mask skip and
 * unloaded model slot included), 0 only when TaskPush itself fails or
 * TrafficPathScanBuild_8c02f0c8 rejects the entry.
 */
STATIC Sint32 spawnEntry_8c0272b8(Uint32 typeCode, float progress, Uint16 *script)
{
    Task *task;
    void *entryVoid;
    TrafficEntry *e;
    Sint32 dayShift;
    Uint32 dayMask;
    Uint32 tableWord;
    Sint32 variantIdx;
    Uint8 bodyTypeIdx;
    Uint16 rnd;

    if ((typeCode & 0xff) == 0x1c || (typeCode & 0xff) == 0x1e) {
        dayShift = var_progress_8c1ba1cc.days_0x00 - 1;
        dayMask = (dayShift < 0) ? (1u >> ((Uint32)(-dayShift) & 0x1f))
                                  : (1u << ((Uint32)dayShift & 0x1f));
        tableWord = *(Uint32 *)(init_8c046208 + var_timeOfDay_8c18ad20 * 4 +
                                 var_route_8c18ad1c * 0xc);
        if ((dayMask & tableWord) == 0) {
            return 1;
        }
    }

    if (*script == 10) {
        if (!TaskPush_8c014ae8(var_tasks_8c1bac28, TrafficDriveDecoration_8c02656a, &task, &entryVoid, 0x514)) {
            return 0;
        }
        e = (TrafficEntry *)entryVoid;
        e->field_0x2e4 = 1;
    } else {
        if (!TaskPush_8c014ae8(var_tasks_8c1bac28, TrafficDriveVehicle_8c025b98, &task, &entryVoid, 0x514)) {
            return 0;
        }
        e = (TrafficEntry *)entryVoid;
        e->field_0x2e4 = 0;
    }

    e->field_0x48c = (typeCode & 0x8000) ? 4 : 3;

    if (typeCode & 0x4000) {
        e->field_0x2c8 = (void *)GroundProbeTrackPolygonAtHeight_8c021290;
        e->field_0x2cc = (void *)FUN_8c02eab4;
    } else {
        e->field_0x2c8 = (void *)GroundProbeTrackPolygon_8c020b6c;
        e->field_0x2cc = (void *)FUN_8c02e51c;
    }

    typeCode &= 0xfff;
    variantIdx = (Sint32)typeCode >> 1;
    e->variantIndex_0x2e0 = variantIdx;

    if (var_routeModelSlots_8c1bbddc[typeCode].texlist_0x08 == (NJS_TEXLIST *)-1) {
        /* Real asm quirk: an unloaded model slot still frees the just-
         * allocated task, but falls through to the same "success" return
         * as a completed spawn -- it does not report failure here. */
        TaskFree_8c014b66(task);
    } else {
        e->field_0x510 = 0;
        if ((typeCode == 0x14 || typeCode == 0x16) && (AsqGetRandomA_8c012166() & 1) != 0) {
            e->field_0x510 |= 0x40;
        }

        if (*script != 10) {
            TrafficReadScriptArgs_8c026710(e, script);

            if (TrafficPathScanBuild_8c02f0c8(task, e, (Sint32)e->resolvedArgs_0x304[0], 0, 8.0f, 8.0f) != 0) {
                TaskFree_8c014b66(task);
                return 0;
            }

            e->field_0x2e8 = progress;
            rnd = AsqGetRandomA_8c012166();
            e->originJitterX_0x2ec = (float)rnd / 65536.0f - 0.5f;
            rnd = AsqGetRandomA_8c012166();
            e->originJitterZ_0x2f0 = (float)rnd / 65536.0f - 0.5f;
        }

        e->field_0x2f4 = var_activeTrafficPreset_8c227e14;
        e->scriptCursor_0x2f8 = script;
        e->scriptBase_0x2fc = script;

        e->texlistLarge_0x04 = var_routeModelSlots_8c1bbddc[typeCode].texlist_0x08;
        e->modelLarge_0x0c = var_routeModelSlots_8c1bbddc[typeCode].nj_0x0c;
        e->texlistSmall_0x08 = var_routeModelSlots_8c1bbddc[typeCode + 1].texlist_0x08;
        e->modelSmall_0x10 = var_routeModelSlots_8c1bbddc[typeCode + 1].nj_0x0c;

        bodyTypeIdx = init_8c04622c[variantIdx];
        e->bodyModel_0x14 = var_trafficModels_8c1bc3f4[bodyTypeIdx].njDest;

        VehPartsBind_8c02786c(e, typeCode);
        TrafficRunEntryScript_8c027012(e);

        if (*script != 10) {
            TrafficAdvanceOnPath_8c026ca2(progress, e);
        }
        TrafficUpdateHeading_8c026bc4(progress, e);
    }

    return 1;
}

/* Sets Chunk simple/easy light direction from one of two fixed light-dir
 * vectors, selected by flag. Installed as a FadeCallback1 (arg passed
 * through as flag) by spawnEntry_8c0272b8's caller. */
STATIC void applyTrafficLighting_8c02756a(int flag)
{
    float *dir;

    if (flag != 0) {
        dir = var_8c227dc4;
    } else {
        dir = var_busSimpleLightDir_8c227db8;
    }

    njCnkSetSimpleLight(dir[0], dir[1], dir[2]);
    njCnkSetEasyLight(dir[0], dir[1], dir[2]);
}
/* Not detected as a function by Ghidra at this hidden
 * address -- nothing calls it directly, only the .DATA.L pool entry
 * TaskPush_8c014ae8 uses.
 *
 * same unused-first-float-argument pattern as TrafficUpdateHeading_8c026bc4
 * and TrafficAdvanceOnPath_8c026ca2. var_8c1bb880/var_8c1bb888 are
 * var_currentCourse_8c1bb868's atariCpu_0x18/attrCpu_0x20 fields (see
 * 013ae8_route_load.h); var_8c2264d4 is the already-named
 * var_activeGroundGrid_8c2264d4. lightCallback_8c02756a is this unit's own
 * applyTrafficLighting_8c02756a.
 */

/* Per-frame TaskAction driving the traffic subsystem: pushed once (with no
 * extra state -- its own Task struct doubles as the state, see field_0x08/
 * 0x0c/queuedItem_0x18 below) into var_tasks_8c1ba5e8. No-ops entirely
 * while traffic is disabled (var_8c2285c4[0] == 0).
 *
 * Selects the CPU-vehicle collision/attribute meshes as the active
 * ground-query grid for the GroundQueryFindPolygon_8c020914/GroundProbeInterpolateHeight_8c020f7e queries run while
 * spawning/updating entries below (var_activeGroundGrid_8c2264d4 <-
 * .atariCpu_0x18, var_8c228b3c <- .attrCpu_0x20), and resets var_8c228b44
 * (role for its other, still-undecompiled consumer unclear).
 *
 * Tracks a traffic-preset switch via var_scenePresetIds_8c1bbd8c's
 * bits 8-15 (a different byte lane than pedestriansTask_8c0293f6's own use
 * of the same packed word): the first time through with field_0x0c == 0 and
 * no preset selected, arms field_0x0c = 2; once a *different* preset id appears,
 * it's latched into var_activeTrafficPreset_8c227e14 and the task's script
 * cursor (queuedItem_0x18) is refreshed from var_trafficPresetTable_8c227e18[presetId], with
 * field_0x0c reset to 0.
 *
 * Then processes the current script record pointed to by queuedItem_0x18
 * (an inline array of 0xc-byte {typeCode, threshold, script, progress}
 * records): while the record has a script (its dword at +4 != 0), a
 * counter (field_0x08) advances every call (compared against the
 * record's threshold, its Uint16 at +2, *before* the increment); once the
 * threshold is exceeded, the record is "consumed" -- unless
 * field_0x0c is not 1 and the record's own progress float is itself
 * nonzero, this spawns the entry via
 * spawnEntry_8c0272b8(typeCode, progress, script). Real asm quirk:
 * when the (field_0x0c != 1 && progress != 0.0f) branch is taken instead,
 * spawnEntry_8c0272b8 is never called at all, but the record is
 * still advanced as if it had succeeded. Either way "succeeding" advances
 * the cursor to the next record (+0xc) and resets the counter.
 *
 * Finally re-applies the two fixed light directions (applyTrafficLighting_8c02756a,
 * one FadeCmdPushCall1 per fade layer) and runs every vehicle task pushed
 * above to completion (TaskExecGroup_8c014b42 on var_tasks_8c1bac28).
 */
STATIC void trafficUpdateTask_8c0275d4(Task *task, void *state)
{
    TrafficPlacement *rec;
    Uint32 presetMask;
    Sint32 presetId;
    Sint32 counter;

    (void)state;

    if (var_8c2285c4[0] == 0) {
        return;
    }

    var_activeGroundGrid_8c2264d4 = var_currentCourse_8c1bb868.atariCpu_0x18;
    var_8c228b3c = var_currentCourse_8c1bb868.attrCpu_0x20;
    ObjectsFUN_8c028958();
    var_8c228b44 = (Sint32 *)-1;

    presetMask = var_scenePresetIds_8c1bbd8c & 0xff00;
    if (task->field_0x0c == (void *)0) {
        if (presetMask == 0) {
            task->field_0x0c = (void *)2;
        }
    } else if (presetMask != 0) {
        presetId = (Sint32)presetMask >> 8;
        if (var_activeTrafficPreset_8c227e14 != presetId) {
            var_activeTrafficPreset_8c227e14 = presetId;
            task->queuedItem_0x18 = (void *)var_trafficPresetTable_8c227e18[presetId];
            task->field_0x0c = (void *)0;
        }
    }

    rec = (TrafficPlacement *)task->queuedItem_0x18;
    if (rec->script_0x04 != NULL) {
        counter = task->field_0x08;
        task->field_0x08 = counter + 1;
        if ((Sint32)rec->threshold_0x02 < counter) {
            if ((task->field_0x0c != (void *)1 && rec->progress_0x08 != 0.0f) ||
                spawnEntry_8c0272b8(rec->typeCode_0x00, rec->progress_0x08,
                                    rec->script_0x04) != 0) {
                task->queuedItem_0x18 = (void *)(rec + 1);
                task->field_0x08 = 0;
            }
        }
    }

    FadeCmdPushCall1_8c0223ea(0, applyTrafficLighting_8c02756a, 0);
    FadeCmdPushCall1_8c0223ea(1, applyTrafficLighting_8c02756a, 1);
    TaskExecGroup_8c014b42(var_tasks_8c1bac28);
}

/* Reading this needs var_8c227e1c/var_trafficPresetTable_8c227e18 typed as the
 * already-known CurrentCourse.lineCpu_0x1c/macCpu1_0x24 field values instead
 * of the invented PTR_PTR_8c1bb884/PTR_PTR_8c1bb88c globals -- their
 * addresses are var_currentCourse_8c1bb868 + 0x1c/0x24 -- and var_8c228b40 as
 * Sint32*.
 *
 * Pushes the entry point for trafficUpdateTask_8c0275d4 into var_tasks_8c1ba5e8,
 * caching two per-course table pointers (route path array, per-preset script
 * table) and, on the night route/day table, a copy of two adjacent
 * CourseSceneParams.rec0_0x0c rows plus their per-20-frame deltas -- consumed
 * by initEntryState_8c026748's junction-light path and (rows only,
 * still undecompiled) 027958.
 */
void TrafficInit_8c02769e(void)
{
    Task *task;
    void *state;

    var_8c227e1c = (Sint32 *)var_currentCourse_8c1bb868.lineCpu_0x1c;
    var_trafficPresetTable_8c227e18 = (Sint32 *)var_currentCourse_8c1bb868.macCpu1_0x24;

    if (var_route_8c18ad1c == ROUTE_WANGAN) {
        var_8c228b40 = init_8c04c980;
    } else if (var_route_8c18ad1c == ROUTE_SHINJUKU) {
        var_8c228b40 = init_8c04caec;
    } else if (var_route_8c18ad1c == ROUTE_OME) {
        var_8c228b40 = init_8c04cd38;
    }

    if (var_timeOfDay_8c18ad20 == TIME_OF_DAY_NIGHT) {
        var_8c1bbdb0[0] = var_sceneParams_8c18ad24->rec0_0x0c[2][0];
        var_8c1bbdb0[1] = var_sceneParams_8c18ad24->rec0_0x0c[2][1];
        var_8c1bbdd0[0] = var_sceneParams_8c18ad24->rec0_0x0c[2][2];
        var_8c1bbdd0[1] = var_sceneParams_8c18ad24->rec0_0x0c[2][3];
        var_8c1bbdd0[2] = var_sceneParams_8c18ad24->rec0_0x0c[2][4];
        var_8c1bbda8[0] = var_sceneParams_8c18ad24->rec0_0x0c[1][0];
        var_8c1bbda8[1] = var_sceneParams_8c18ad24->rec0_0x0c[1][1];
        var_8c1bbdc4[0] = var_sceneParams_8c18ad24->rec0_0x0c[1][2];
        var_8c1bbdc4[1] = var_sceneParams_8c18ad24->rec0_0x0c[1][3];
        var_8c1bbdc4[2] = var_sceneParams_8c18ad24->rec0_0x0c[1][4];

        var_8c1bbda0[0] = (var_8c1bbdb0[0] - var_8c1bbda8[0]) / 20.0f;
        var_8c1bbda0[1] = (var_8c1bbdb0[1] - var_8c1bbda8[1]) / 20.0f;
        var_8c1bbdb8[0] = (var_8c1bbdd0[0] - var_8c1bbdc4[0]) / 20.0f;
        var_8c1bbdb8[1] = (var_8c1bbdd0[1] - var_8c1bbdc4[1]) / 20.0f;
        var_8c1bbdb8[2] = (var_8c1bbdd0[2] - var_8c1bbdc4[2]) / 20.0f;
    }

    TaskPush_8c014ae8(var_tasks_8c1ba5e8, trafficUpdateTask_8c0275d4, &task, &state, 0);
    task->queuedItem_0x18 = (void *)var_trafficPresetTable_8c227e18[var_activeTrafficPreset_8c227e14];
    task->field_0x08 = 0;
    task->field_0x0c = (void *)1;

    ObjectsFUN_8c028958();
}
