/* 8c226558: undecompiled data section */
#ifndef _226558_SECTIONB_H
#define _226558_SECTIONB_H

#include <shinobi.h>
#include "01614c_replay_menu.h"
#include "013ae8_route_load.h"
#include "02af78_event.h"
#include "022464_fade.h" /* FadePhase, FadeRequest, FadeDrawCommand */
#include "028258_objects.h" /* TrafficSignal */
#include "023938_bus_drive.h" /* LineBusSegment, LineBusNode */
#include "026710_traffic.h" /* PathRecord */
#include "011120_asset_queues.h"
#include "014a9c_tasks.h"
#include "014b8c_backup.h"
#include "014f54_text.h"

/* =================
 * Type Declarations
 * =================
 */

extern NJS_CAMERA* var_drawCamera_8c226558; // 022464: camera for the layer being drawn; FadeUpdate_8c022560 picks main/mirror/cabin
extern int var_fadeArrivalVariant_8c22655c; // 022464: bus-stop-arrival overlay layout (0-2) drawn by FadeUpdate_8c022560; despite the SDK Bool this held before, values above 1 are reachable (switch in FadeUpdate_8c022560 handles 0-2)
extern int var_fadeArrivalGate_8c226560; // 022464: gates FadeUpdate_8c022560's bus-stop-arrival draw; cleared once its fade-out finishes
extern FadeRequest var_fadeRequest_8c226564; // 022464: requested fade transition, consumed by FadeUpdate_8c022560
extern void (*var_fadeCompleteCallback_8c22656c)(void); // 022464: fade-complete callback; sentinel -1 (0xffffffff) means unset
extern int var_fadeDrawCommandCount_8c226570[3]; // 022464: per-layer draw-command count for var_fadeDrawCommands_8c22657c
extern FadeDrawCommand var_fadeDrawCommands_8c22657c[3][128]; // 022464: per-layer draw-command queue
extern FadePhase var_fadePhase_8c227d7c; // 022464: fade state machine phase
extern Uint32 var_fadeProgress_8c227d80; // 022464: fade alpha accumulator for init_fadeQuad_8c0455a8's black overlay, driven by FadeUpdate_8c022560. Two incompatible fixed-point scales are used: FADE_PHASE_OUT/fadeInTask_8c022a54 keep the alpha byte already at bits 24-31 (0xff000000 = opaque, read via a plain & mask); FADE_PHASE_IN/fadeOutTask_8c022ad0 keep it at bits 16-23 (0xff0000 = opaque, read via a <<8 shift)
/* The active course's route line: its segments (each a LinePoint list) and
 * the node table that links them. Copied from
 * var_currentCourse_8c1bb868.lineBus_0x08/lineNodes_0x0c by BusInitStart_8c023610,
 * alongside var_activeGroundGrid_8c2264d4/var_activeAttrGrid_8c228b3c. */
extern LineBusSegment *var_lineSegments_8c227d84;
extern LineBusNode *var_lineNodes_8c227d88;
/* Peak brake-pedal travel of the current press, scaled to 0..255 and never
 * walked back down; applyBrakingSfx_8c024606 (024280) picks the release note
 * from it. */
extern int var_brakePressPeak_8c227d8c;
/* Fixed camera-interest point for BusRenderUpdateCamera_8c025078's
 * var_cameraMode_8c227d9c==4 mode. */
extern float var_fixedCameraTarget_8c227d90[3];
extern int var_cameraMode_8c227d9c;
extern Uint32 var_savedCameraMode_8c227da0;
extern int var_cameraCueState_8c227da4; /* 02d19c/024b4c: shifted into var_savedCameraCueState_8c227da8 each frame alongside var_cameraMode_8c227d9c->var_savedCameraMode_8c227da0 */
extern int var_savedCameraCueState_8c227da8;
extern int var_cameraCueBusy_8c227dac; /* zeroed alongside var_cameraMode_8c227d9c by busInitPlaceBus_8c023310 for a normal run */
/* Door-timer counter driven by BusTask_8c022bdc (022bdc): counts up by 0.5/frame
 * while boarding (doorState_0x3c0==1), capped at var_busDoorLastFrame_8c227db4, then counts
 * back down by 0.5/frame once departing (doorState_0x3c0==3) until it hits 0. */
extern float var_busDoorFrame_8c227db0;
/* var_busDoorMotion_8c1bc410->nbFrame - 1.0, set by BusInitStart_8c023610, read by BusTask_8c022bdc
 * (022bdc). */
extern float var_busDoorLastFrame_8c227db4;
extern float var_busSimpleLightDir_8c227db8[3]; // 028258: light direction (x, y, z), written by BusRenderUpdateCamera_8c025078
extern float var_mirrorLightDir_8c227dc4[3];
/* Which attract-mode shot is showing, so demoShotTask_8c0259e8 only cuts on a
 * change. 0129cc_game.c pre-seeds it with the course's opening shot from
 * init_demoFirstShot_8c0460b0 (sectionD.h). */
extern int var_demoShotId_8c227dd4;
/* Five prev/current float pairs, each shifted (dd8->ddc, de0->de4, de8->dec,
 * df0->df4, df8->dfc) every frame by BusRenderSaveCameraState_8c024b4c (024b4c). */
extern float var_cameraHeightFrom_8c227dd8;
extern float var_savedCameraHeightFrom_8c227ddc;
extern float var_cameraHeightTo_8c227de0;
extern float var_savedCameraHeightTo_8c227de4;
extern float var_cameraHeightDelta_8c227de8;
extern float var_savedCameraHeightDelta_8c227dec;
/* The chase camera's height above the bus, passed to
 * positionCamera_8c024d6c (024b4c) as its dyOffset. 5.0 near / 18.0 far by
 * default; scripted cues ramp it to another value and back. */
extern float var_cameraHeight_8c227df0;
extern float var_savedCameraHeight_8c227df4;
/* BusRenderUpdateCamera_8c025078's quarter-sine ease-angle accumulator
 * (BAMS units, 0 to 0x4000) -- a real int (MOV.L/ADD/CMP/GE), not float. */
extern Sint32 var_cameraHeightPhase_8c227df8;
extern Sint32 var_savedCameraHeightPhase_8c227dfc;
/* The current demo shot's pos_0x04, copied here by demoShotTask_8c0259e8
 * (025870_demo). Read as a world point for camera mode 5 and as a bus-space
 * offset for 6 and 7; the first two are resolved once by
 * applyShotPosition_8c0258ba, mode 7 every frame by DemoUpdateCamera_8c025906. */
extern NJS_POINT3 var_demoShotPos_8c227e00;
/* This route's attract-mode camera tour: a DemoShot[] (025870_demo.c), one of
 * init_demoShotsShinjuku_8c045674/...Wangan_8c045b60/...Ome_8c045ee4, selected
 * by DemoStartTour_8c025af4 from var_route_8c18ad1c and indexed by the stop
 * marker in var_busState_8c1bb9d0.markDriveFlags_0x3b0. */
extern int *var_demoShots_8c227e0c;
/* Makes demoShotTask_8c0259e8 cut to var_demoShotId_8c227dd4's shot on its
 * first frame instead of waiting for the marker to change. */
extern int var_demoShotRearm_8c227e10;
extern Task *var_trafficSignalTasks_8c227e20; /* Task array for trafficSignalTask_8c028258/linkedTrafficSignalTask_8c02833c, sized (count+1) by ObjectsInitTrafficSignals_8c02845a */
extern int *var_trafficSignalFrames_8c227e24; /* per-id current frame index, read by ObjectsGetTrafficSignalFrame_8c028900 */
extern TrafficSignal **var_trafficSignalStates_8c227e28; /* per-id TrafficSignal* */
extern int var_pedCrossingFlags_8c227e2c[128]; /* 64 8-byte entries, zeroed by clearPedCrossingFlags_8c02890c */
extern int var_crossingOccupiedFlags_8c22802c[128]; /* 64 8-byte entries, zeroed by ObjectsClearCrossingOccupied_8c028958 */
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
extern float var_farClipDepth_8c227dd0;
extern Bool var_isFading_8c226568;

#endif // _226558_SECTIONB_H
