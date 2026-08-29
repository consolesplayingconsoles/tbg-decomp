/* 8c0fcd20: undecompiled data section */
#ifndef _0FCD20_SECTIONB_H
#define _0FCD20_SECTIONB_H

#include <shinobi.h>
#include "01614c_debug_menu.h"
#include "013ae8_route_load.h"
#include "02af78_event.h"
#include "01bb48_vm_game.h" /* LcdAnim */
#include "02171c_tile_stream.h" /* TileIndex, TileRect */
#include "022464_fade.h" /* FadePhase, FadeRequest, FadeMirrorSelect */
#include "028258_objects.h" /* TrafficSignal, TrafficSignalDef */

/* =================
 * Type Declarations
 * =================
 */

typedef struct {
    int field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
    int field_0x10;
    int field_0x14;
    int field_0x18;
} Struct8c2264b8;

typedef struct {
    Uint32 on;   /* 0x00 */
    Sint8  x1;   /* 0x04 */
    Uint8  r;    /* 0x05 */
    Uint8  l;    /* 0x06 */
    Uint8  pad;  /* 0x07 */
} ReplayInput;


#include <shinobi.h>
#include "011120_asset_queues.h"
#include "013ae8_route_load.h"
#include "014a9c_tasks.h"
#include "014b8c_backup.h"
#include "014f54_text.h"

/* =================
 * Type Declarations
 * =================
 */

// TODO:
typedef struct {
    int field_0x000;
    int field_0x004;
    int field_0x008;
    int field_0x00c;
    int field_0x010;
    int field_0x014;
    int field_0x018;
    int field_0x01c;
    int field_0x020;
    int field_0x024;
    int field_0x028;
    int field_0x02c;
    int field_0x030;
    int field_0x034;
    int field_0x038;
    int field_0x03c;
    int field_0x040;
    int field_0x044;
    int field_0x048;
    int field_0x04c;
    int field_0x050;
    int field_0x054;
    int field_0x058;
    int field_0x05c;
    int field_0x060;
    int field_0x064;
    int field_0x068;
    int field_0x06c;

    int distance_traveled_0x070;
    int ang_0x074;
    int acc_0x078;
    int ang_0x07c;
    int blinker_0x080;
    int field_0x084;

    int field_0x088;
    int field_0x08c;
    int field_0x090;
    int field_0x094;
    int field_0x098;
    int field_0x09c;
    int field_0x0a0;
    int field_0x0a4;
    int field_0x0a8;
    int field_0x0ac;
    int field_0x0b0;
    int field_0x0b4;
    int field_0x0b8;
    int field_0x0bc;
    int field_0x0c0;
    int field_0x0c4;
    int field_0x0c8;
    int field_0x0cc;
    int field_0x0d0;
    int field_0x0d4;
    int field_0x0d8;
    int field_0x0dc;
    int field_0x0e0;
    int field_0x0e4;
    int field_0x0e8;
    int field_0x0ec;
    int field_0x0f0;

    /* Averaged with posX_0x2fc/posZ_0x304 by rowMaterialModelTask_8c02a27c to get a
     * distance-fade reference point. */
    float posX_0x0f4;

    int field_0x0f8;
    float posZ_0x0fc;

    float field_0x100;

    int field_0x104;
    float field_0x108; /* also exported standalone as var_8c1bbad8 (02f0c8) */
    int field_0x10c;
    int field_0x110;
    int field_0x114;
    int field_0x118;
    int field_0x11c;
    int field_0x120;
    int field_0x124;
    int field_0x128;
    int field_0x12c;
    int field_0x130;
    int field_0x134;
    int field_0x138;
    int field_0x13c;
    int field_0x140;
    int field_0x144;
    int field_0x148;
    int field_0x14c;
    int field_0x150;
    int field_0x154;
    int field_0x158;
    int field_0x15c;
    int field_0x160;
    int field_0x164;
    int field_0x168;
    int field_0x16c;
    int field_0x170;
    int field_0x174;
    int field_0x178;
    int field_0x17c;
    int field_0x180;
    int field_0x184;
    int field_0x188;
    int field_0x18c;
    int field_0x190;
    int field_0x194;
    int field_0x198;
    int field_0x19c;
    int field_0x1a0;
    int field_0x1a4;
    int field_0x1a8;
    int field_0x1ac;
    int field_0x1b0;
    int field_0x1b4;
    int field_0x1b8;
    int field_0x1bc;
    int field_0x1c0;
    int field_0x1c4;
    int field_0x1c8;
    int field_0x1cc;
    int field_0x1d0;
    int field_0x1d4;
    int field_0x1d8;
    int field_0x1dc;
    int field_0x1e0;
    int field_0x1e4;
    int field_0x1e8;
    int field_0x1ec;
    int field_0x1f0;
    int field_0x1f4;
    int field_0x1f8;
    int field_0x1fc;
    int field_0x200;
    int field_0x204;
    int field_0x208;
    int field_0x20c;
    int field_0x210;
    int field_0x214;
    int field_0x218;
    int field_0x21c;
    int field_0x220;
    int field_0x224;
    int field_0x228;
    int field_0x22c;
    int field_0x230;
    int field_0x234;
    int field_0x238;
    int field_0x23c;
    int field_0x240;
    int field_0x244;
    int field_0x248;
    int field_0x24c;

    int ang_0x250;

    int field_0x254;

    int ang_0x258;

    int field_0x25c;
    int field_0x260;
    int field_0x264;

    int mirror_0x268;

    int field_0x26c;
    int field_0x270;
    int field_0x274;

    float field_0x278;
    float speed_0x27c;
    float acc_hist_0x280[4];

    int field_0x290;
    int field_0x294;
    int field_0x298;

    /* Collision knockback direction (unit vector), set by
     * DrivePointsHandleBump_8c02b6d4 (02b464); field_0x2ac/0x2b0 mirror
     * 0x29c/0x2a0 (role of the duplicate pair unclear). */
    float dir_x_0x29c;
    float dir_z_0x2a0;

    int field_0x2a4;
    int field_0x2a8;
    float dir_x2_0x2ac;
    float dir_z2_0x2b0;

    int bus_state_0x2b4;

    int field_0x2b8;
    int field_0x2bc;
    int field_0x2c0;
    int field_0x2c4;
    int field_0x2c8;
    int field_0x2cc;
    int field_0x2d0;
    int field_0x2d4;
    int field_0x2d8;
    int field_0x2dc;
    int field_0x2e0;
    int field_0x2e4;
    int field_0x2e8;
    int field_0x2ec;
    int field_0x2f0;

    int gear_0x2f4;

    int field_0x2f8;
    /* draw position, read by TileStreamDrawTile_8c021b34 */
    float posX_0x2fc;
    float posY_0x300;
    float posZ_0x304;
    int field_0x308;
    int field_0x30c;
    int field_0x310;
    int field_0x314;
    int field_0x318;
    int field_0x31c;
    int field_0x320;
    int field_0x324;
    int field_0x328;
    int field_0x32c;
    int field_0x330;
    int field_0x334;
    int field_0x338;
    int field_0x33c;
    int field_0x340;
    int field_0x344;
    int field_0x348;
    int field_0x34c;
    int field_0x350;
    int field_0x354;
    int field_0x358;
    int field_0x35c;
    int field_0x360;
    int field_0x364;
    int field_0x368;
    int field_0x36c;
    int field_0x370;
    int field_0x374;
    int field_0x378;
    int field_0x37c;
    int field_0x380;
    int field_0x384;
    int field_0x388;
    int field_0x38c;
    int field_0x390;
    int field_0x394;
    int field_0x398;
    int field_0x39c;
    int field_0x3a0;
    int field_0x3a4;
    int field_0x3a8;
    int field_0x3ac;
    int field_0x3b0;
    int field_0x3b4;
    int field_0x3b8;
    int field_0x3bc;

    int bus_substate_0x3c0;

    int field_0x3c4;
} BusState;

typedef struct {
    Uint8 unlocked_0x00;
    Uint8 new_0x01;
    Uint8 field_0x02;
    Uint8 storySpriteNo_0x03;
    Uint8 freeRunSpriteNo_0x04;
    Uint8 field_0x05[3]; // Padding?
} CourseProgress;

typedef struct {
    int days_0x00;

    /* unlock-flag bitsets set together by setProgressFlag_8c02af78
     * and tested individually by hasProgressFlag_8c02afbe/EventHasProgressFlagAlt_8c02aff0 */
    int field_0x04[5];
    int field_0x18[5];

    int letters_0x2c[6];
    CourseProgress courses_0x44[9];
    int field_0x8c;
    int exp_0x90; // aliased by var_exp_8c1ba25c (01b19c_system_menu addresses it directly; others via this field)
    int field_0x94;
    int field_0x98[11];
    signed char field_0xc4;
    char field_0xc5;
    char field_0xc6;

    /* covers the bytes 011120_asset_queues.c indexes at 0xcc-0xcf */
    char field_0xc7[9];

    /* 0xd0/0xd1 look like saved input deadzone
     * thresholds (see FUN_8c024320/FUN_8c024606) */
    char field_0xd0;
    char field_0xd1;
    char field_0xd2;
    char field_0xd3;

    /* AUDIO MUSIC/SFX/VOICE volume (0-10); reset with the sound mode
     * (see FileMenuResetSoundDefaults_8c0188dc) */
    char field_0xd4;
    char field_0xd5;
    char field_0xd6;
    char field_0xd7; // padding

    /* mirrored to/from var_8c1bb8b8/bc/dc/var_award_8c1bb8f8 by 01b19c_system_menu
     * on load (see SystemMenuApplyLoadedProgress_8c01b19c) and save (SystemMenuWriteToVmu_8c01b26c) */
    int field_0xd8;
    int field_0xdc;
    int field_0xe0;
    char award_0xe4;
    char field_0xe5[3]; // padding
} PlayerProgress;

/*
 * SETTING screen's 5 persisted toggle bytes (DIFFICULTY/DRIVE MODE/DEFAULT
 * VIEW/VIBRATION/SCREEN ROLL); var_8c226074 points here while that screen is
 * active. Sits exactly at &var_progress_8c1ba1cc.field_0xc4 (0x1ba1cc+0xc4),
 * but kept as its own symbol since it's owned by sectionB.src, not decompiled.
 * [0] (DIFFICULTY) is read by BusStopSetup_8c02caba to pick the run's
 * driver-points reset value.
 */
extern char var_8c1ba290[5];

extern int var_exp_8c1ba25c; // EXP shown on the VMU icon status line (see 01b19c_system_menu)

/* single-word bitset, set/tested by setRunEventFlag_8c02b022/hasRunEventFlag_8c02b030; role unclear */
extern int var_runEventFlags_8c1ba2b4;

extern int var_8c1ba2b8[5]; // Maybe progress backup
extern int var_8c1ba2cc[5]; // Maybe progress backup
extern void* var_8c1ba2e0;
extern BUS_BACKUPFILEHEADER var_8c1ba2e4; // 018644: analyzed backup file header
extern void* var_8c1ba33c;
extern void* var_vmuIconFileBuf_8c1ba344;
extern void* var_backupFileImageBuf_8c1ba348;
extern int var_8c1ba350;        // 018644: selected save slot / new-file index
extern TrafficSignalDef *var_trafficSignalDefs_8c1bb8a0; // 028258: signal table, terminated by type_0x00 == 0
extern void* var_groundGridFallback_8c1bb86c;

/* base of an 8-byte-stride table of stop-area records, indexed by a segment
 * record's stopAreaId_0x02 (BusStopGetStopArea_8c02cd7a, 02c884); each slot
 * holds a StopAreaRecord* at +0, the trailing 4 bytes unknown. */
extern void *var_stopAreaTable_8c1bb870;

extern void* var_groundGridPrimary_8c1bb890; // ground query grid, selected into var_activeGroundGrid_8c2264d4

/* pointer to a table of 12-byte stop spawn-area records, indexed by a
 * segment record's field_0x06 (BusStopGetSegment_8c02cd6a, 02c884); a record's field_0x00
 * is a spawn-area pointer (see var_8c22890c below), the rest is unknown.
 * The symbol itself reserves 12 bytes; only the leading 4 (the table
 * pointer) are used by pickWaitingPassengers_8c02c8ae. */
extern void *var_8c1bb894;
extern int var_8c1bb8b8; // Maybe courseMenuHasResult or courseMenuHasDialog
extern float var_groundHeightFallback_8c1bbac8; // fallback ground height when both grid queries miss
/* Sits at var_busState_8c1bb9d0's base+0xfc -- same address as its
 * posZ_0x0fc field -- but exported as its own symbol and referenced that
 * way by FUN_8c02f0c8 (02f0c8), not through the struct. Coincidentally
 * adjacent, not part of it (same pattern as var_busCameraFocusX_8c1bbcd8). */
extern float var_8c1bbacc;
/* Sits at var_busState_8c1bb9d0's base+0x108 (its field_0x108); exported as
 * its own symbol and referenced that way by FUN_8c02f0c8 (02f0c8). */
extern float var_8c1bbad8;
extern int var_8c1bb8bc;
extern int var_8c1bb8c4;
extern int var_pauseActive_8c1bb8cc;
extern int var_8c1bb8d4;
extern int var_runSucceeded_8c1bb8dc;
extern int var_firstClearOfCourse_8c1bb8e0; // course was unlocked
extern int var_passengerCount_8c1bb8e4;
extern int var_eventCount_8c1bb8e8;
extern int var_8c1bb8ec;
extern int var_8c1bb8f0;
extern int var_8c1bb8f4;
extern int var_award_8c1bb8f8;
/* Checked against 0.0 by BusStopUpdateArrival_8c02ce48 (02c884) to gate stop
 * arrival; likely a current-speed value (role/owner elsewhere unclear). */
extern float var_8c1bbc4c;
extern int var_8c1bbc84;
extern Uint32 var_8c1bbcb0;
extern int var_8c1bbcc4;
/* Bus-to-camera-focus vector (x, _, z); written by
 * gameplayRenderBusUpdateCamera_8c025078, read by FUN_8c028b74 via njArcTan2.
 * Immediately follows var_busState_8c1bb9d0 in memory but exported as its
 * own symbols, not struct fields -- coincidentally adjacent, not part of it. */
extern float var_busCameraFocusX_8c1bbcd8;
extern float var_busCameraFocusZ_8c1bbce0;
extern void* var_messageTextBoxA_8c1bc404;
extern void* var_messageTextBoxB_8c1bc408; /* second half of the double-buffered message textbox pair */
extern int var_messageTextBoxIndex_8c1bc40c;   /* active index (0/1) into (&var_messageTextBoxA_8c1bc404)[idx] */
extern void* var_8c1bc410;
extern void* var_8c1bc414;
extern void* var_8c1bc440;
extern void* var_8c1bc444;
/* Current "fuu" stop-marker animation frame, driven by
 * BusStopUpdateArrival_8c02ce48 (02c884): counts up by 1.0 per frame while
 * the bus approaches a stop, wrapping to 0 at var_8c1bc450. */
extern float var_8c1bc44c;
extern float var_8c1bc450;
/* Scratch matrix for the "fuu" stop-marker model, built each frame by
 * drawStopMarker_8c02cd92 (02c884). */
extern NJS_MATRIX var_8c1bc46c;
extern NJS_POINT3 var_groundQueryPoint_8c1bc460; // scratch world point for ground-height queries, e.g. FUN_8c02840c
extern void* var_vmGameBuf_8c1bc454;
extern float var_crossingIntersectPoint_8c1bc458; // IntersectSegments_8c0206f0's intersection-point output (x); [1] (var_8c1bc45c) holds y
extern void* var_busFont_8c1ba1c8;
extern BusState var_busState_8c1bb9d0;
/* Sit at var_busState_8c1bb9d0's base+0x34c/+0x368/+0x384 (its field_0x34c/
 * field_0x368/field_0x384) but are exported as their own symbols and
 * addressed that way by FUN_8c02b986/FUN_8c02bb1c/FUN_8c02bcd8 (02b464),
 * not through the struct. */
extern int var_8c1bbd1c;
extern int var_8c1bbd38;
extern int var_8c1bbd54;
/* Points at the player's own BusState (presumably &var_busState_8c1bb9d0).
 * Used in 026710_traffic.c only as a sentinel "entry" marking the player's
 * bus in traffic-avoidance code that otherwise walks a list of real traffic
 * entries (see TrafficComputeBlockedSpeed_8c026eaa) -- identity-compared,
 * never dereferenced there. 02b464 does dereference it, for the player's
 * side of a collision response. */
extern BusState *var_8c1bbd9c;
/* Bus world matrix; pedestriansTask_8c0293f6 uses it via njCalcPoint to
 * place the crosswalk stop-line scratch points. Immediately follows
 * var_busState_8c1bb9d0 (base+0x84) but exported as its own symbol, not a
 * struct field -- coincidentally adjacent, not part of it. */
extern NJS_MATRIX var_busWorldMatrix_8c1bba54;
/* Three independent byte fields, each an id selecting a preset content set for
 * one subsystem, swapped together:
 *   bits  8-15  traffic     -> var_trafficPresetTable_8c227e18[id]  (026710)
 *   bits 16-23  pedestrians -> var_pedGroupLists_8c228240[id]       (028258)
 *   bits 24-31  set pieces  -> matched against RowEntry.sceneGate_0x14;
 *               nonzero starts the FUMI crossing sequence
 * Named "preset" rather than "demo": nothing here is the attract-mode demo
 * (that is var_demoBuffer_8c1bc828 and friends). Most likely the Japanese
 * sense of "demo" = staged scene, but no writer has been decompiled yet --
 * it lives in whatever owns var_busState_8c1bb9d0, probably 02b464. Revisit
 * the top byte's exact trigger semantics when that lands.
 * Also sits at var_busState_8c1bb9d0's base+0x3bc but is its own symbol, not
 * that struct's field_0x3bc -- coincidentally adjacent, not part of it. */
extern int var_scenePresetIds_8c1bbd8c;
// 026710: cached copies of two CourseSceneParams.rec0_0x0c rows, and their
// per-20-frame deltas, built by TrafficInit_8c02769e when timeOfDay is
// TIME_OF_DAY_NIGHT. Each row is split 2+3 like the struct field itself
// (var_8c1bbdb4/var_8c1bbdac are var_8c1bbdb0[1]/var_8c1bbda8[1],
// separately-imported aliases for the same addresses in 027958).
extern float var_8c1bbda0[2]; // (row2 - row1) / 20, first 2 components
extern float var_8c1bbda8[2]; // cached rec0_0x0c[1][0..1]
extern float var_8c1bbdb0[2]; // cached rec0_0x0c[2][0..1]
extern float var_8c1bbdb8[3]; // (row2 - row1) / 20, last 3 components
extern float var_8c1bbdc4[3]; // cached rec0_0x0c[1][2..4]
extern float var_8c1bbdd0[3]; // cached rec0_0x0c[2][2..4]
extern void* var_busstopDat_8c1bc42c;
extern void* var_busstopPartsDat_8c1bc428;
extern NJS_TEXLIST *var_busStopTexlist_8c1bc424;
extern NJS_CAMERA var_8c1bb904; // 021b9c
extern NJS_CAMERA var_8c1bb944; // 021b9c
extern NJS_CAMERA var_8c1bb984; // 022464
extern FadeMirrorSelect var_mirrorSelect_8c1bbc38; // 022464: which wing mirror FadeUpdate_8c022560 draws; real object is 20 bytes, rest unexplored
extern CurrentCourse var_currentCourse_8c1bb868;
extern int var_cutsceneActive_8c1bb900;
extern int* var_demoBuf_8c1ba3c4;
extern int var_demoIndex_8c1bb8d8;
extern void *var_frontNj_8c1bc434;
extern NJS_TEXLIST *var_frontTexlist_8c1bc430;
extern int var_gameMode_8c1bb8fc;
extern BACKUPINFO var_gBupInfo_8c1bc4ac[8];
extern int var_inputMapSel_8c1bb8c8;
extern void *var_interiorNj_8c1bc43c;
extern NJS_TEXLIST *var_interiorTexlist_8c1bc438;
extern NJS_MOTION* var_loadedFooNjm_8c1bc448;
extern ResourceGroup var_loadingResourceGroup_8c1bc3f8;
extern void* var_markDat_8c1bc420;
extern void* var_markPartsDat_8c1bc41c;
extern NJS_TEXLIST *var_markTexlist_8c1bc418;
extern ModelSlot var_pedestrianAssets_8c1bbfdc[0x41];
extern PDS_PERIPHERAL *var_peripheral_8c1ba358;
extern PDS_PERIPHERAL var_peripherals_8c1ba35c[2];
extern enum PLAY_MODE var_playMode_8c1bb8d0;
extern PlayerProgress var_progress_8c1ba1cc;
extern void* var_routeModels_8c1bc3ec;
extern ModelSlot var_routeModelSlots_8c1bbddc[0x20];
extern LoadedModel *var_segmentModels_8c1bc3f0;
extern int var_selectedVm_8c1ba34c;
extern int var_shouldShowFreeRunIntro_8c1bb8c0;
extern Task var_tasks_8c1ba3c8[];
extern Task var_tasks_8c1ba5e8[];
extern Task var_tasks_8c1ba808[];
extern Task var_tasks_8c1bac28[];
extern Task var_tasks_8c1bb448[];
extern LoadedModel *var_trafficModels_8c1bc3f4;
extern Uint32 var_vibport_8c1ba354;

extern NJS_TEXNAME *var_glyphTexnames_8c1bc78c;
extern NJS_TEXLIST *var_glyphTexlists_8c1bc790;
extern ResourceGroup var_fontResourceGroup_8c1bc794;
extern Sint16 *var_8c1bc7a0;
extern void *var_glyphBuffer_8c1bc7a4;

extern MenuState var_menuState_8c1bc7a8;
extern DebugMenuCourseSel *var_debugMenuCourseSel_8c1bc824;
#define REPLAY_BUFFER_CAPACITY 54000
extern ReplayInput var_demoBuffer_8c1bc828[REPLAY_BUFFER_CAPACITY];
extern ReplayInput *var_demoCursor_8c225fa8;
extern Uint32 var_demoPrevOn_8c225fac;
extern void* var_8c225fe0;      // 018644: BupLoad dest buffer, advances 0x600 per file
extern int var_8c225fe4[10];    // 018644
extern int var_8c22600c;        // 018644: index into var_8c225fe4
extern int var_8c226010;        // 018644: load result (1 = done, 2 = error)
extern int var_8c226014;        // 018644: FILE SELECT leading NEW-FILE card count (0 or 1)
extern int var_8c226018[12];    // 018644: FILE SELECT card list (0xa=NEW FILE, 0xb=empty)

extern int var_vmMountBusy_8c22606c;
extern char var_8c226098[16]; // VMU icon status text, built by 01b19c_system_menu
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
extern void* var_8c226434;
extern void* var_8c226438;
/* Read (!= -1 sentinel) by BusStopUpdateArrival_8c02ce48 (02c884); role/owner
 * (02b464) unclear. */
extern int var_8c226450;
/* Reset to 0 by BusStopUpdateArrival_8c02ce48 (02c884) on a stop-heading
 * transition; role/owner (02b464) unclear. */
extern int var_8c226454;
extern Struct8c2264b8 var_8c2264b8;
/* Reset to 0 by BusStopUpdateArrival_8c02ce48 (02c884) on a stop-heading
 * transition; role/owner (02b464) unclear. */
extern int var_8c2264c4;
extern void* var_activeGroundGrid_8c2264d4; // ground query grid currently selected for GroundQueryFindPolygon_8c020914/GroundProbeInterpolateHeight_8c020f7e
extern float var_fadeLightDir0_8c2264d8[3]; // 021b9c: light direction for fade layer 0
extern float var_fadeLightDir1_8c2264e4[3]; // 021b9c: light direction for fade layer 1 (mirror side)
/* 0222dc: copies of var_sceneParams_8c18ad24->rec2_0x74[0..4]. Two separate
 * symbols (not one float[5]) because each is exported/imported on its own in
 * src/asm/sectionB.src -- coincidentally adjacent, not one C variable. */
extern float var_fadeLightIntensity_8c2264f0[2]; // [0..1]
extern float var_fadeLightColor_8c2264f8[3]; // [2..4]
/* 0222dc: same deal, for var_sceneParams_8c18ad24->rec1_0x54[0..4]. */
extern float var_8c226544[2]; // [0..1]
extern float var_8c22654c[3]; // [2..4]
extern NJS_CAMERA* var_fadeCamera_8c226558; // 022464: camera passed to njSetCamera by draw_8c022464
extern int var_fadeArrivalVariant_8c22655c; // 022464: bus-stop-arrival overlay layout (0-2) drawn by FadeUpdate_8c022560; despite the SDK Bool this held before, values above 1 are reachable (switch in FadeUpdate_8c022560 handles 0-2)
extern int var_fadeArrivalGate_8c226560; // 022464: gates FadeUpdate_8c022560's bus-stop-arrival draw; cleared once its fade-out finishes
extern FadeRequest var_fadeRequest_8c226564; // 022464: requested fade transition, consumed by FadeUpdate_8c022560
extern void (*var_fadeCompleteCallback_8c22656c)(void); // 022464: fade-complete callback; sentinel -1 (0xffffffff) means unset
extern int var_fadeDrawCommandCount_8c226570[3]; // 022464: per-layer draw-command count for var_fadeDrawCommands_8c22657c
extern char var_fadeDrawCommands_8c22657c[3][0x800]; // 022464: per-layer draw-command queue, 16-byte entries {type, arg1, arg2, arg3}
extern FadePhase var_fadePhase_8c227d7c; // 022464: fade state machine phase
extern Uint32 var_fadeProgress_8c227d80; // 022464: fade alpha accumulator for init_fadeQuad_8c0455a8's black overlay, driven by FadeUpdate_8c022560. Two incompatible fixed-point scales are used: FADE_PHASE_OUT/fadeInTask_8c022a54 keep the alpha byte already at bits 24-31 (0xff000000 = opaque, read via a plain & mask); FADE_PHASE_IN/fadeOutTask_8c022ad0 keep it at bits 16-23 (0xff0000 = opaque, read via a <<8 shift)
extern int var_8c227d9c;
extern Uint32 var_8c227da0;
extern int var_8c227da8;
extern float var_busSimpleLightDir_8c227db8[3]; // 028258: light direction (x, y, z), written by gameplayRenderBusUpdateCamera_8c025078
extern float var_8c227dc4[3];
extern int var_8c227dd4;
extern Task *var_trafficSignalTasks_8c227e20; /* Task array for trafficSignalTask_8c028258/linkedTrafficSignalTask_8c02833c, sized (count+1) by ObjectsInitTrafficSignals_8c02845a */
extern int *var_trafficSignalFrames_8c227e24; /* per-id current frame index, read by ObjectsGetTrafficSignalFrame_8c028900 */
extern TrafficSignal **var_trafficSignalStates_8c227e28; /* per-id TrafficSignal* */
extern int var_pedCrossingFlags_8c227e2c[128]; /* 64 8-byte entries, zeroed by clearPedCrossingFlags_8c02890c */
extern int var_crossingOccupiedFlags_8c22802c[128]; /* 64 8-byte entries, zeroed by ObjectsFUN_8c028958 */
/* 12-byte entries {active, unused, list*}; list is NULL-terminated, holes
 * marked -1. Read by FUN_8c028b74; var_pedGroupCount_8c228234 is the count. */
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

/* [0] compared against var_8c228688's 0xf000000 bits in FUN_8c02bb1c
 * (02b464), addressed directly; [1]/[2] (0x228638/0x22863c) are the same
 * two ints but addressed only via var_8c2285c4[29]/[30] -- no code
 * reaches them through this symbol. Role unclear. */
extern int var_8c228634[3];
/* Set to 1 by BusStopUpdateArrival_8c02ce48 (02c884) when var_8c226450 is
 * armed; read by FUN_8c02bcd8 (02b464) only via var_8c2285c4[31] -- no code
 * reaches it through this symbol. */
extern int var_8c228640;

/* PDS_PERIPHERAL.r (see 010e90.h) of var_peripherals_8c1ba35c[0], addressed
 * directly by this symbol rather than through the array/field form. */
extern unsigned short var_8c1ba374;

/* [0]/[1] a duplicated traffic-signal id (FUN_8c02b8b8, 02b464), addressed
 * both directly and via var_8c2285c4[14]/[15]; [3]/[4] a threshold/counter
 * pair graded by FUN_8c02b986 (via var_8c2285c4[17]/[18] there); [6] a
 * driving-state latch toggled by FUN_8c02bb1c. [0] cleared to ready by
 * DrivePointsArmCooldowns_8c02b578 (types 1-3). Other slots unclear.
 *
 * var_8c2285c4[11]/[12]/[13] (0x2285f0/f4/f8, just before this array) have
 * no export of their own; [13] is a one-shot "already graded" latch read
 * by FUN_8c02b8b8. */
extern int var_8c2285fc[8];

/* [5] (0x228630) compared against var_8c228684 in FUN_8c02b986/FUN_8c02bb1c
 * (02b464), addressed only via var_8c2285c4[27] -- no code reaches it
 * through this symbol. Other slots unclear. */
extern int var_8c22861c[6];

/* Bitflags set elsewhere (022bdc, still undecompiled); bits 0x2/0x4 are
 * read by DrivePointsHandleFlags_8c02b7ea (02b464) to grade a driver-points
 * penalty -- both set is worse than either alone. Other bits unclear. */
extern int var_8c228660;

/* The vehicle/pedestrian the player's bus is currently bumping into, set by
 * DrivePointsHandleBump_8c02b6d4 (02b464) from FUN_8c02e2dc's result;
 * var_8c228668 is a redundant copy of the same pointer. */
extern BusState *var_8c228664;
extern BusState *var_8c228668;
/* Player's speed at the moment of the current bump; read to grade the
 * DrivePointsAdjust_8c02b464 penalty and vibration strength. */
extern float var_8c22866c;
/* var_8c228664's speed_0x27c, saved before DrivePointsHandleBump_8c02b6d4
 * (02b464) overwrites it as part of the knockback response. */
extern float var_8c228670;

/* Bump-grading scratch (02b464): [674]/[67c]/[678] a timestamp/threshold
 * trio compared in FUN_8c02b986; [680] the offense-type code driving
 * FUN_8c02b864/886/8b8/986's penalty picks; [684] a timestamp compared
 * against var_8c22861c[5]; [688] a bitflag word tested against 0xf000000;
 * [68c] a small state code (0/1/2) gating several of the above. */
extern int var_8c228674;
extern int var_8c228678;
extern int var_8c22867c;
extern int var_8c228680;
extern int var_8c228684;
extern int var_8c228688;
extern int var_8c22868c;

/* Per-offense-type cooldowns armed by DrivePointsArmCooldowns_8c02b578
 * (02b464) to a large frame count (0x96/0xd2), presumably counted down by
 * a still-undecompiled function; DrivePointsHandleBump_8c02b6d4 treats
 * slot 0 as expired/ready once it goes negative. Role of each slot beyond
 * that unclear. */
extern int var_8c228690;
extern int var_8c228694;
extern int var_8c228698;
extern int var_8c22869c;
extern int var_8c2286a0;

/* One pending driver-comment banner: `count`/`ids` are a {count, id...}
 * list from init_8c04c35c (02b464), `duration` the computed on-screen time,
 * `holdFrames` a fixed 60. [0] is the currently-displayed message;
 * [1..3] are queued behind it, shifted forward as each one finishes. */
typedef struct {
    int count;
    int *ids;
    float duration;
    int field_0x0c;
    int field_0x10;
    int holdFrames;
} DriveMsgSlot;
extern DriveMsgSlot var_driveMsgQueue_8c228564[4];

extern int var_8c2285c4[];

/* Set by trafficUpdateTask_8c0275d4 (026710); role for other consumers
 * (023310, 02e51c, 025b98, 022bdc, 02f0c8, still undecompiled) unclear. */
extern void *var_8c228b3c;
/* One of init_8c04c980/init_8c04caec/init_8c04cd38 (sectionD), picked by
 * route in TrafficInit_8c02769e (026710); role for 02f0c8 (still
 * undecompiled) unclear. */
extern Sint32 *var_8c228b40;
/* Cached pointer into a var_8c228b40 id-group (FUN_8c02f28a, 02f0c8) --
 * -1 means "not cached yet"; reset to -1 by trafficUpdateTask_8c0275d4
 * (026710). */
extern Sint32 *var_8c228b44;
/* Sample-point scratch buffer written by FUN_8c02f0c8 (02f0c8): up to 10
 * (x, z) pairs projected along a traffic entry's upcoming path, walked by
 * FUN_8c02f212 to find nearby occupants. */
extern float var_8c228b48[20];
/* Task-slot pointer to exclude from FUN_8c02f212's scan (02f0c8); never
 * written within 02f0c8 itself -- setter still unclear. */
extern void *var_8c228b98;
/* Read/write cursor into var_8c228b48, consumed by FUN_8c02f212 (02f0c8). */
extern float *var_8c228b9c;
/* One-past-the-end of the valid range in var_8c228b48 (02f0c8). */
extern float *var_8c228ba0;

/* ReplayCodec (02f320) working state and buffers: LZ ring buffer / match
 * tables plus adaptive-code tables used by ReplayCodecInit_8c02f320 and its
 * pack/unpack routines. */
extern Uint32 var_8c228ba4;
extern Sint16 var_8c228ba8; /* signed bit-buffer fill count; goes negative to trigger a refill */
extern Uint16 var_8c228baa;
extern Uint16 var_8c228bac;
extern Uint8 var_8c228bae[0x1000];
extern Uint8 var_8c229bae[0x2000];
extern Uint8 var_8c22bbae[0x2000];
extern Uint8 var_8c22dbae[0x2000];
extern Uint8 var_8c22fbae[0x2000];
extern Uint16 var_8c231bae;
extern Uint8 var_8c231bb0[0x2000];
extern Uint8 var_8c233bb0[0x2000];
extern Uint16 var_8c235bb0;
extern Uint16 var_8c235bb2;
extern Uint8 var_8c235bb4[200];
extern Uint16 var_8c235c7c;
extern Uint16 var_8c235c7e;

extern int var_8c2285c8;
/* Set to 0x1e by BusStopUpdateArrival_8c02ce48 (02c884) on stop completion;
 * role/owner unclear. */
extern int var_8c2285cc;

/* run completion percentage (0-100); read by ResultShowPassedRun_8c01e0b4
 * to pick the award tier and as the score's EXP component */
extern int var_driverPoints_8c2285d0;

/* set alongside var_driverPoints_8c2285d0 by BusStopSetup_8c02caba (same
 * 100/200 value); role elsewhere unclear */
extern int var_8c2285d4;

/* gate for EventPickForSegment_8c02b170: only runs while
 * var_8c2285dc <= var_8c2285d8 (role of each side unclear) */
extern int var_8c2285d8;
extern int var_8c2285dc;

/* Bus-stop arrival state machine driven by BusStopUpdateArrival_8c02ce48
 * (02c884): 0 = cruising, 1 = departed-previous-stop wait, 2 = approaching
 * (mirror-view draw enabled -- gates pedestriansTask_8c0293f6's
 * FUN_8c02d06c registration), 3 = stopped/waiting, 4 = finishing. */
extern int var_mirrorViewLevel_8c2285e4;
/* Set to 0 or 2 by BusStopUpdateArrival_8c02ce48 depending on how the
 * approach (state 2) ended; role elsewhere unclear. */
extern int var_8c2285e8;
/* Running minimum distance-to-stop while approaching (state 2), reset to
 * 9999.0 on arming. */
extern float var_stopMinDistance_8c2285ec;

/* per-segment "has an active stop" flag, one word each, indexed by a segment
 * record's candidate-list entry byte (see BusStopGetSegment_8c02cd6a, 02c884) */
extern int var_8c2286a4[24];

/* index of the stop the run starts from: 0 for a normal course start, or the
 * debug menu's per-entry startStopIndex_0x04 to begin partway along the route */
extern int var_startStopIndex_8c228704;

extern int var_prevStopSegment_8c22870c; // segment index of the previous stop
extern int var_nextStopSegment_8c228710; // segment index of the upcoming stop

/* upcoming stop's heading angle (njArcTan2 of its stop-area record's
 * direction vector, see NinjaApi.h), sign-extended from the low 16 bits by
 * BusStopUpdateStopHeadings_8c02ccc6 */
extern int var_8c228714;

/* number of slots filled in var_8c228798 by pickWaitingPassengers_8c02c8ae
 * (0-16); read by 02d06c/02d968 to spawn that many passenger tasks. */
extern int var_8c228794;

/* one waiting passenger picked for the upcoming stop by
 * pickWaitingPassengers_8c02c8ae */
typedef struct {
    void *spot_0x00;  // chosen candidate-stop entry (segment record's list at +8)
    NJS_POINT3 pos_0x04; // world position; y filled in by the ground snap
    float index_0x10; // 0, 1, 2, ... in pick order
} WaitingPassengerSlot;
extern WaitingPassengerSlot var_8c228798[16];

/* fixed 31-slot table of scripted/special waiting-passenger schedule entries,
 * one per fixed stop position along the route; -1 = unused. Read by
 * FUN_8c02d968 to spawn each slot's passenger task. */
extern int var_8c228718[31];

/* shared NJS_SPRITE used to draw a waiting passenger; only sx/sy/ang/tanim
 * are reset up front, p and tlist are set per-draw. */
extern NJS_SPRITE var_8c2288d8;

/* current (about-to-depart) stop's heading angle, same computation as
 * var_8c228714 but masked unsigned instead of sign-extended; sits right
 * after var_stopTaskGroup_8c2288f8 */
extern int var_8c2288fc;

/* scratch ground-query point for the upcoming stop, snapped to ground by
 * pickWaitingPassengers_8c02c8ae; x/z (only) also set by
 * BusStopUpdateStopHeadings_8c02ccc6 ahead of that ground snap */
extern NJS_POINT3 var_8c228900;
/* Coincidentally sits at var_8c228900's .z (base+8) and is its own exported
 * symbol, used by drawStopMarker_8c02cd92 (02c884). */
extern float var_8c228908;

/* spawn-area record for the upcoming stop's segment (var_8c1bb894 entry
 * selected by its segment record's field_0x06); fields at +4/+8 are the
 * area's (x, z) origin, +0xc/+0x10 its (dx, dz) extent */
extern char *var_8c22890c;

/* Task group for the bus-stop subsystem's waiting-passenger/departure tasks
 * (see FUN_8c02d968); -1 means not currently allocated. */
extern void* var_stopTaskGroup_8c2288f8;
extern Sint8 var_coursesToUnlock_8c225fd4[];
extern int var_currentSegment_8c228708;

/* table index (into the EventEntry array pointed to by var_routeEvents_8c22851c)
 * chosen by EventPickForSegment_8c02b170, consumed by
 * EventApplyFlags_8c02b292 */
extern int var_selectedEventEntry_8c228478;
extern void* var_currentSysResGroupInfo_8c225fb0;
extern TileIndex *var_8c22650c[5]; /* per-layer grid dims, copied from
                                    * var_currentCourse_8c1bb868.slots_0x04[14..18] by TileStreamInit_8c02175a */
extern LoadedModel *var_tileLayerSlots_8c226520[5]; /* per-layer tile grids, width * height slots each;
                                                      * layers 0-3 hold texture+model pairs, layer 4 model only */
extern TileRect *var_currentTileRegionList_8c226534; /* -1 when unset */
extern int var_8c22640c;
extern int var_8c226410;
extern int var_8c226414[6]; /* dialog id queue built by buildDialogQueue_8c01e992, -1 terminated */
extern int var_8c22642c; /* lesson attempt counter, incremented on practice retry */
extern void *var_8c226430; /* selected ending dialog id list, set by selectEndingDialog_8c01f3c0 */
extern int var_activeTrafficPreset_8c227e14;
/* Traffic preset table: indexed by var_scenePresetIds_8c1bbd8c's byte at
 * bits 8-15, yielding that preset's record run in the course's *_MAC_CPU1.DAT;
 * read by trafficUpdateTask_8c0275d4 (026710). */
extern Sint32 *var_trafficPresetTable_8c227e18;
/* Base of a per-entry table; 026710_traffic indexes it by a script argument. */
extern Sint32 *var_8c227e1c;
extern int var_activePedPreset_8c22822c;
extern int var_dialogQueue_8c225fbc[4]; // TODO: Confirm length
extern int var_instructorDialogActive_8c225fb4;
extern int var_fogParam_8c226504;
extern int var_fogParam_8c226508;
extern float var_fogParam_8c227dd0;
extern Bool var_isFading_8c226568;
extern int var_menuTextboxCharLimit_8c225fb8;
extern int var_profileUnlockedCount_8c2263a4; // saved into var_progress_8c1ba1cc.field_0x8c by 01b19c_system_menu
extern char var_profileUnlocked_8c2263b4[56]; // one byte per PROFILE FILE grid slot (55 used, 1 pad byte); set by ProfileFile
extern ResourceGroup* var_resourceGroup_8c2263a8;
extern Sint8 var_soundMode_8c226070;
extern char *var_8c226074; /* SETTING screen: ptr to the 5 gameplay-setting bytes */
/* AUDIO sound-test fields: one int per digit, least significant at index 0. */
extern int var_8c226078[2]; /* MUSIC TEST */
extern int var_8c226080[2]; /* SFX TEST */
extern int var_8c226088[4]; /* VOICE TEST */
extern float var_uknVol_8c226468; // real type is 0100bc_sound.c's local UnknownVolStructB {float}
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

/* CollideFindTaskHit_8c02e400 (02e400): cursor into var_tasks_8c1bac28 during
 * its scan, left pointing at the terminating (action == 0) slot on exit. */
extern Task *var_collideScanCursor_8c228974;
/* CollideFindTaskHit_8c02e400 scratch: world-space oriented bounding box,
 * njCalcPoints'd from the entry passed in (self) and from the current
 * candidate task respectively. */
extern NJS_BOX var_collideSelfBox_8c228978;
extern NJS_BOX var_collideCandidateBox_8c2289d8;
/* CollideQueueAdd_8c02e48e/CollideQueueTest_8c02e4ac: fixed 64-slot queue of
 * pending collision candidates, written and scanned once per frame. */
extern void *var_collideQueue_8c228a38[64];
/* CollideQueueReset_8c02e486/CollideQueueAdd_8c02e48e/CollideQueueTest_8c02e4ac:
 * element count of the fixed 64-slot queue at var_collideQueue_8c228a38. */
extern Sint32 var_collideQueueCount_8c228b38;

#endif // _0FCD20_SECTIONB_H
