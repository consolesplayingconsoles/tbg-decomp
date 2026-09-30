/* 8c227e14: undecompiled data section */
#ifndef _227E14_SECTIONB_H
#define _227E14_SECTIONB_H

#include <shinobi.h>
#include "01614c_replay_menu.h"
#include "013ae8_route_load.h"
#include "02af78_event.h"
#include "028258_traffic_signal.h" /* TrafficSignal */
#include "026710_traffic.h" /* PathRecord */
#include "011120_asset_queues.h"
#include "014a9c_tasks.h"
#include "014b8c_backup.h"
#include "015034_text.h"

/* =================
 * Type Declarations
 * =================
 */

extern Task *var_trafficSignalTasks_8c227e20; /* Task array for trafficSignalTask_8c028258/linkedTrafficSignalTask_8c02833c, sized (count+1) by SignalInit_8c02845a */
extern int *var_trafficSignalFrames_8c227e24; /* per-id current frame index, read by SignalGetFrame_8c028900 */
extern TrafficSignal **var_trafficSignalStates_8c227e28; /* per-id TrafficSignal* */
extern int var_pedCrossingFlags_8c227e2c[128]; /* 64 8-byte entries, zeroed by SignalClearPedCrossingFlags_8c02890c */
extern int var_crossingOccupiedFlags_8c22802c[128]; /* 64 8-byte entries, zeroed by SignalClearCrossingOccupied_8c028958 */
/* 12-byte entries {active, unused, list*}; list is NULL-terminated, holes
 * marked -1. Read by drawPedestrians_8c028b74; var_pedGroupCount_8c228234 is the count. */
extern void* var_pedGroups_8c228230;
extern int var_pedGroupCount_8c228234; /* -1 sentinel means not yet loaded */
/* 12-byte entries {float *first, float *last, float length}, indexed in
 * lockstep with var_pedGroups_8c228230 by pedGroupTask_8c029078. */
extern void* var_pedPaths_8c228238;
/* Crosswalk table walked by pedestrianTask_8c028e00: entries are pairs of
 * path-node pointers, terminated by the end pointer var_crosswalkTableEnd_8c228244. */
extern int* var_crosswalkTableEnd_8c228244;
extern int var_crosswalkTable_8c228248[8];
extern float var_stopLinePointA_8c228268[2]; /* segment-intersection scratch, param1 for IntersectSegments_8c0206f0 */
extern float var_stopLinePointB_8c228270[2]; /* segment-intersection scratch, param2 for IntersectSegments_8c0206f0 */
/* nodes[0] = the route's blinker model (var_routeModels_8c1bc3ec[9]); [1..3]
 * are its child/sibling tree, filled by resolveObjectChildren_8c029868. */
extern NJS_OBJECT *var_routeBlinkerNodes_8c228278[4];
/* Per-group spawn definition, 12-byte entries {int id, float radius, spec
 * list*}, looked up by id in pedestriansTask_8c0293f6. */
extern void* var_pedGroupDefs_8c22823c;
/* int*[] indexed by var_activePedPreset_8c22822c; each list is a -1 terminated array of
 * group ids, consumed by pedestriansTask_8c0293f6. */
extern void* var_pedGroupLists_8c228240;
/* Per-slot destination pointers for a pending object-asset request, one 0x18-byte
 * entry per table row processed by ObjectsStartAssetRequests_8c029ad4 (up to 16
 * rows), also read/freed by ObjectsFreeAssetRequests_8c029cfe and ObjectsPushTasks_8c02a6ac. Raw bytes:
 * which fields are used depends on the row's type. */
extern Uint8 var_assetRequestSlots_8c228288[16 * 0x18];
/* Table currently in flight for ObjectsStartAssetRequests_8c029ad4: an array of
 * {type, dataPtr} pairs terminated by type == -1. -1 when nothing is queued. */
extern int *var_assetRequestTable_8c228408;
/* Fixed asset handles for the type-6 ("FUMI" railway crossing) row, shared by
 * every table that includes one -- there is only ever one railway crossing. */
extern void *var_fumiGateModel_8c22840c;
extern void *var_fumiTexlist_8c228410;
extern void *var_fumiCloseMotion_8c228414;
extern void *var_fumiOpenMotion_8c228418;
extern void *var_fumiLampModel_8c22841c;
extern void *var_fumiLampTexlist_8c228420;
extern void *var_fumiTrainModel_8c228424;
extern void *var_fumiTrainTexlist_8c228428;
extern void *var_fumiTrainMotionA_8c22842c;
extern void *var_fumiTrainMotionB_8c228430;
/* nodes[0] = the FUMI lamp model (var_fumiLampModel_8c22841c); [1..16] are its
 * grandchild tree, filled by resolveObjectGrandchildren_8c02a322. */
extern NJS_OBJECT *var_fumiLampNodes_8c228434[17];
extern int var_messageBoxActive_8c22847c;

/* One entry of var_eventSlides_8c228480[event]/state->slide_0x10, terminated
 * by an entry whose layers_0x00 is (unsigned short *)-1. layers_0x00 is a base
 * image id plus 0-2 overlay ids, all drawn in the same frame in `pr` order.
 * lineListIndex_0x04 selects the line list out of var_messageTextDat_8c228518
 * (an array of EventLine*). */
typedef struct {
    unsigned short *layers_0x00;
    int lineListIndex_0x04;
} EventSlide;

/* Per-event slide table for the route selected by
 * ObjectsRequestMessageAssets_8c02aa36: init_shinjukuEvents_8c049a6c / init_wanganEvents_8c04843c /
 * init_omeEvents_8c04a9c8, indexed by var_selectedEventEntry_8c228478. */
extern EventSlide **var_eventSlides_8c228480;

/* Dedup table of message pvm/dat assets requested by
 * ObjectsRequestMessageAssets_8c02aa36, one entry per distinct id seen
 * across the selected event's slides; count in var_messageAssetCount_8c228514. */
typedef struct {
    int id_0x00;
    void *pvm_0x04;
    void *dat_0x08;
} MessageAssetEntry;
extern MessageAssetEntry var_messageAssets_8c228484[12];

extern int var_messageAssetCount_8c228514;

/* One line of dialogue within a slide, walked by state->line_0x18; terminated
 * by an entry whose text_0x00 points at an empty string. */
typedef struct {
    char *text_0x00;
    int voiceId_0x04;
} EventLine;
extern EventLine **var_messageTextDat_8c228518;

/* unlock-candidate scratch list built by EventScanCandidates_8c02b03c;
 * var_routeEvents_8c22851c points at the active route's EventEntry table */
extern EventEntry* var_routeEvents_8c22851c;
extern int var_eventCandidates_8c228520[];
extern int var_eventCandidateCount_8c228560;

/* One driver-comment banner. `count`/`ids` point into a {count, id...} row
 * of init_penaltyMsgGlyphs_8c04c35c (02b464); the ids are 16x16-atlas glyph
 * indices. [0] is the newest message, [1..3] older ones shifted back as each
 * new one arrives. Typed out one glyph per two frames, then held 60. */
typedef struct {
    int count;
    int *ids;
    float x;          /* row's left edge, centered: (640 - 32 * count) / 2 */
    int revealed;     /* glyphs typed out so far; revealCounter >> 1 */
    int revealCounter;
    int holdFrames;   /* counts down once fully revealed; 0 = slot free */
} DriveMsgSlot;
extern DriveMsgSlot var_driveMsgQueue_8c228564[4];

/* table index (into the EventEntry array pointed to by var_routeEvents_8c22851c)
 * chosen by EventPickForSegment_8c02b170, consumed by
 * EventApplyFlags_8c02b292 */
extern int var_selectedEventEntry_8c228478;
extern int var_activeTrafficPreset_8c227e14;
/* Traffic preset table: indexed by var_busState_8c1bb9d0.scenePresetIds_0x3bc's byte at
 * bits 8-15, yielding that preset's record run in the course's *_MAC_CPU1.DAT;
 * read by trafficUpdateTask_8c0275d4 (026710). */
extern Sint32 *var_trafficPresetTable_8c227e18;
/* Course CPU path-block table (== CurrentCourse.lineCpu_0x1c); 026710_traffic
 * indexes it by a script argument to resolve a traffic entry's path. */
extern PathRecord **var_cpuPathBlocks_8c227e1c;
extern int var_activePedPreset_8c22822c;

#endif // _227E14_SECTIONB_H
