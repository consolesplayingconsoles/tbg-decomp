/* 8c225fb0: undecompiled data section */
#ifndef _225FB0_SECTIONB_H
#define _225FB0_SECTIONB_H

#include <shinobi.h>
#include "01614c_replay_menu.h"
#include "013ae8_route_load.h"
#include "02af78_event.h"
#include "01bb48_vm_game.h" /* LcdAnim */
#include "02171c_tile_stream.h" /* TileIndex, TileRect */
#include "022464_fade.h" /* FadePhase, FadeRequest, FadeDrawCommand */
#include "028258_objects.h" /* TrafficSignal */
#include "020914_ground_query.h" /* GroundGrid */
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

/* The driver-comment popup currently on screen (01fa78_hud). showMark_8c01fa78
 * sets markSpriteId_0x00 and displayTimer_0x08 (frames left); blinkIconId_0x04
 * is a second sprite hudUpdateTask_8c01ff48 blinks on top of it, from the map
 * cell's scenePresetIds_0x3bc, and only while the mark id is in [0x1e, 0x50].
 * Both are drawn from var_markTexlist_8c1bc418. */
typedef struct {
    int markSpriteId_0x00;
    int blinkIconId_0x04;
    int displayTimer_0x08;
    int blinkCounter_0x0c;
} HudMarkState;

/* A screen-space vertex for njDrawPolygon: position plus an ARGB word. The
 * HUD's static quads (01fa78_hud) are raw byte arrays of these. */
typedef struct {
    float x, y, z;
    Uint32 color;
} HudVertex;

/* Ambient drive-cue state, owned by 020214_drive_cue_task (see its .h).
 * Three fields are written from outside that unit: BusTask_8c022bdc (022bdc)
 * sets nearStopLatch_0x0c on the first A press of a drive and
 * BusStopUpdateArrival_8c02ce48 (02c884) clears it on a stop-heading
 * transition; gradeFrame_8c02bcd8 (02b464) also sets it when it docks points
 * for a missing announcement; TrafficDriveVehicle_8c025b98 (025b98) sets
 * firstChimeArmed_0x18 when a CPU vehicle sits stopped at a junction. */
typedef struct {
    int idleChimeState_0x00;
    int idleChimeTimer_0x04;
    int stopAnnounceState_0x08;
    int nearStopLatch_0x0c;
    int stopAnnounceTimer_0x10;
    int nearStopChimeLatch_0x14;
    int firstChimeArmed_0x18;
} DriveCueState;

extern void* var_saveBufCursor_8c225fe0;      // 018644: BupLoad dest buffer, advances 0x600 per file
extern int var_loadedSaveSlots_8c225fe4[10];    // 018644: VMU file index of each loaded save, in load order
extern int var_loadedSaveCount_8c22600c;        // 018644
extern int var_saveLoadResult_8c226010;        // 018644: load result (1 = done, 2 = error)
extern int var_fileCardCount_8c226014;        // 018644: FILE SELECT cards in use
extern int var_fileCards_8c226018[12];    // 018644: FILE SELECT card list; VMU file index, or 0xa=NEW FILE / 0xb=empty

extern int var_vmMountBusy_8c22606c;
extern char var_vmsComment_8c226098[16]; // 01b19c: VMS file comment, "9/<day> EXP <points>"
extern int var_lcdAnimActive_8c2260a8;
extern LcdAnim var_lcdAnimBus_8c2260ac; // 01bb48: vm_bus.lcd anim
extern LcdAnim var_lcdAnimDanger_8c2260b8; // 01bb48: vm_danger.lcd anim
extern LcdAnim var_lcdAnimLoading_8c2260c4; // 01bb48: now_loading.lcd anim
extern enum VmGameBupPhase var_bupPhase_8c2260d0; // 01bb48: last-seen bu* async op status/phase
extern char var_lcdClearBuf_8c2260d4[0xc0]; // 01bb48: VMS LCD framebuffer (48x32 mono)
extern Uint32 var_lcdFrameDelay_8c226198;    // 01bb48: LCD anim step counter
extern LcdFrame* var_lcdFramePtr_8c22619c; // 01bb48: current LCD anim frame ptr
extern char var_defragBuf_8c2261a0[512]; // 01bb48: buDefragDisk work buffer
extern int var_lcdSlot_8c2263a0;   // 01bb48
/* Both only ever written, to -1, by GameInit_8c0134ec; nothing in the image
 * reads either. */
extern void* var_8c226434;
extern void* var_8c226438;
/* Driver-points meter fill, ramped toward
 * var_runState_8c2285c4.driverPoints_0x0c over 20 frames (01fa78_hud).
 * field_0x0c is seeded to 1.0f and never read. */
typedef struct {
    float displayedValue_0x00;
    float lastSample_0x04;
    float rampStep_0x08;
    float field_0x0c;
} DriverPointsMeterState;

/* The HUD's per-frame state. The whole 60-byte block is one object: every
 * field is reached as a constant displacement off a single base register, and
 * the next address (var_tachoNeedleVerts_8c226478) is loaded as its own. */
typedef struct {
    /* HudReset_8c02018c zeroes field_0x00/0x04 and nothing reads them back;
     * field_0x08/0x0c are never touched at all. */
    int field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
    int driveMarkLatched_0x10;

    /* HUD sprite id for the map's drive instruction under the bus
     * (markDriveFlags_0x3b0's low 3 bits, + 0x1f), latched by
     * hudUpdateTask_8c01ff48 (01fa78_hud) and only ever -1 before the run's
     * first marked cell. 02b464 and 02c884 read it as "the bus has passed
     * one". */
    int driveMarkIcon_0x14;

    /* Free-running frame counter gating the blink of that HUD slot; reset by
     * hudUpdateTask_8c01ff48 on a new instruction and by
     * BusStopUpdateArrival_8c02ce48 (02c884) on a stop-phase change. */
    int blinkTimer_0x18;

    DriverPointsMeterState pointsMeter_0x1c;

    float engineRpm_0x2c;

    /* Written (zeroed) by HudReset_8c02018c (01fa78_hud), never read. */
    int field_0x30;

    /* Gear-message / lane-change-message latch for hudUpdateTask_8c01ff48
     * (01fa78): set once the corresponding driver-comment popup has been
     * staged, cleared when the bus-state bit returns to 0. */
    int gearLatch_0x34;
    int laneLatch_0x38;
} HudState;
extern HudState var_hudState_8c22643c; // 01fa78
extern HudVertex var_tachoNeedleVerts_8c226478[3]; // 01fa78
extern HudMarkState var_hudMark_8c2264a8; // 01fa78
extern DriveCueState var_driveCueState_8c2264b8;
extern GroundGrid* var_activeGroundGrid_8c2264d4; // ground query grid currently selected for GroundQueryFindPolygon_8c020914/GroundProbeInterpolateHeight_8c020f7e
extern float var_fadeLightDir0_8c2264d8[3]; // 021b9c_tile_draw: simple-light direction, fade layer 0
extern float var_fadeLightDir1_8c2264e4[3]; // 021b9c_tile_draw: simple-light direction, fade layer 1 (mirror side)
/* 0222dc: copies of var_sceneParams_8c18ad24->rec2_0x74[0..4]. Every access is
 * a bare single-word pool load, so the archive says nothing about whether the
 * five floats are one array or two; split here because the intensity pair and
 * the colour triple go to different SDK calls. */
extern float var_fadeLightIntensity_8c2264f0[2]; // [0..1]
extern float var_fadeLightColor_8c2264f8[3]; // [2..4]
extern float var_fadeEasyLightDir_8c226538[3]; // 021b9c_tile_draw: easy-light direction, shared by both fade layers
/* 0222dc: same deal, for var_sceneParams_8c18ad24->rec1_0x54[0..4]. */
extern float var_fadeEasyLightIntensity_8c226544[2]; // [0..1]
extern float var_fadeEasyLightColor_8c22654c[3]; // [2..4]
extern NJS_CAMERA* var_fadeCamera_8c226558; // 022464: camera passed to njSetCamera by fadeDraw_8c022464
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

extern Sint8 var_coursesToUnlock_8c225fd4[];

/* table index (into the EventEntry array pointed to by var_routeEvents_8c22851c)
 * chosen by EventPickForSegment_8c02b170, consumed by
 * EventApplyFlags_8c02b292 */
extern int var_selectedEventEntry_8c228478;
extern void* var_currentSysResGroupInfo_8c225fb0;
/* Copied from var_currentCourse_8c1bb868.tileLayers_0x3c by
 * TileStreamInit_8c02175a; only the dims are read, the offset tables come
 * from var_datFiles_8c18adb4. */
extern TileIndex *var_tileLayerIndexes_8c22650c[5];

/* Per-layer tile grids, width * height slots each; layers 0-3 hold
 * texture+model pairs, layer 4 model only. [0] is -1 while unallocated. */
extern LoadedModel *var_tileLayerSlots_8c226520[5];

extern TileRect *var_currentTileRegionList_8c226534; /* -1 when unset */
/* Selected PRACTICE lesson, 0-10 (01e27c_practice_menu): indexes
 * init_practiceRules_8c0451c0, the description-page table, and
 * var_progress_8c1ba1cc.practiceLessonBestScores_0x98. */
extern int var_practiceLesson_8c22640c;
/* Which parts of a normal run still apply to the selected practice drill,
 * from init_practiceRules_8c0451c0 (01e27c). A set bit keeps the normal
 * behaviour; a clear one takes the drill shortcut, and every reader pairs it
 * with var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE. 1 = stop announcements and
 * stop-arrival grading (020214, 02b464), 2 = the schedule (02c884, 02b464's
 * INSTR_TIME_MANAGEMENT), 4 = the stop sequence and run completion (02b464),
 * 8 = passengers and bus stops at all (02d968, 02d19c, 022464, and 023310's
 * blinker/mirror start). */
extern int var_practiceRules_8c226410;
extern int var_lessonDialogQueue_8c226414[6]; /* built by buildDialogQueue_8c01e992, -1 terminated */
/* Practice drives finished since the lesson list was opened; still 0 means the
 * player never drove, and the day does not advance. */
extern int var_lessonAttempts_8c22642c;
extern int *var_endingVoiceList_8c226430; /* selected ending voice-id list, set by selectEndingDialog_8c01f3c0 */
extern int var_activeTrafficPreset_8c227e14;
/* Traffic preset table: indexed by var_busState_8c1bb9d0.scenePresetIds_0x3bc's byte at
 * bits 8-15, yielding that preset's record run in the course's *_MAC_CPU1.DAT;
 * read by trafficUpdateTask_8c0275d4 (026710). */
extern Sint32 *var_trafficPresetTable_8c227e18;
/* Course CPU path-block table (== CurrentCourse.lineCpu_0x1c); 026710_traffic
 * indexes it by a script argument to resolve a traffic entry's path. */
extern PathRecord **var_cpuPathBlocks_8c227e1c;
extern int var_activePedPreset_8c22822c;
extern int var_dialogQueue_8c225fbc[4]; // TODO: Confirm length
extern int var_instructorDialogActive_8c225fb4;
extern int var_fogParam_8c226504;
extern int var_fogParam_8c226508;
extern float var_fogParam_8c227dd0;
extern Bool var_isFading_8c226568;
extern int var_menuTextboxCharLimit_8c225fb8;
extern int var_profileUnlockedCount_8c2263a4; // saved into var_progress_8c1ba1cc.profileUnlockedCount_0x8c by 01b19c_system_menu
extern char var_profileUnlocked_8c2263b4[56]; // one byte per PROFILE FILE grid slot (55 used, 1 pad byte); set by ProfileFile
extern ResourceGroup* var_resourceGroup_8c2263a8;
extern Sint8 var_soundMode_8c226070;
/* The SETTING rows at var_progress_8c1ba1cc.difficulty_0xc4; pointed there
 * only by OptionSwitchToTopMenu_8c01b122. */
extern char *var_settingValues_8c226074;
/* AUDIO sound-test entry fields: one int per decimal digit, ones at index 0.
 * Laid out contiguously, and switchToAudio_8c01afd8 clears them relying on it. */
extern int var_musicTestDigits_8c226078[2];
extern int var_sfxTestDigits_8c226080[2];
extern int var_voiceTestDigits_8c226088[4];
extern int var_vmuStatus_8c226048[9];
/* RESULTS screen score category totals, drawn digit-by-digit by
 * drawScoreDigits_8c01d7fc (01d7fc). */
extern int var_scoreCourseClearBonus_8c2263ec;
extern int var_scoreFirstClearBonus_8c2263f0;
extern int var_scoreDriverPointsBonus_8c2263f4;
extern int var_scoreBadgeBonus_8c2263f8;
extern int var_scorePassengerBonus_8c2263fc;
extern int var_scoreEventBonus_8c226400;
extern int var_scoreTotal_8c226404;
/* set by ResultShowFailedRun_8c01e24e, cleared by ResultShowPassedRun_8c01e0b4 */
extern int var_runFailed_8c226408;

#endif // _225FB0_SECTIONB_H
