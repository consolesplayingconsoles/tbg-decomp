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
#include "020914_ground_query.h" /* GroundQueryResult */
#include "023938_bus_drive.h" /* LineBusSegment, LineBusNode */
#include "026710_traffic.h" /* PathRecord */

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

/* var_busState_8c1bb9d0's own storage. sectionB.src also labels 25 addresses
 * inside it and exports them separately, so a fair number of the loose
 * var_8c1bbxxx symbols below are really fields of this struct reached under
 * another name; each one says which field it is. In a test they
 * have to be rellocate()'d onto the struct or the two views drift apart. */
// TODO:
typedef struct {
    int typeCode_0x000;
    int texlistLarge_0x004;
    /* TrafficEntry's texlistSmall_0x08/modelSmall_0x10 sit here; the player
     * bus never sets or reads either, so it has no far/simple LOD. */
    int field_0x008;
    int modelLarge_0x00c;
    int field_0x010;
    int shadowModel_0x014;
    /* Sub-model nodes walked out of modelLarge_0x00c by VehPartsBind_8c02786c
     * and animated by BusDrawUpdateModels_8c027958 (027958_bus_draw). Which of
     * rearWheelB_0x028 and the three turn lamps get bound at all depends on
     * typeCode_0x000; the bus is type 0x1a. */
    NJS_OBJECT *bodyNode_0x018;
    NJS_OBJECT *frontWheelA_0x01c;
    NJS_OBJECT *frontWheelB_0x020;
    NJS_OBJECT *rearWheelA_0x024;
    NJS_OBJECT *rearWheelB_0x028;
    /* The five lamp nodes hanging off bodyNode_0x018, shown/hidden from bits
     * 0-4 of blinker_0x080. Bit 0 is the brake lamp (025b98 sets it while
     * decelerating or stopped); bits 3 and 4 are forced on for the
     * night/evening running lights. */
    NJS_OBJECT *blinkerLights_0x02c[5];
    NJS_OBJECT *turnLampA_0x040;
    NJS_OBJECT *turnLampB_0x044;
    NJS_OBJECT *turnLampC_0x048;
    NJS_OBJECT *bodyModels_0x04c[6];
    /* Zeroed by busInitPlaceBus_8c023310 and never read; TrafficEntry's
     * three at the same offsets are the same way. */
    int field_0x064;
    int field_0x068;
    int field_0x06c;

    /* Animation angles, BAMS. distanceTraveled spins the wheels (one world
     * unit per revolution), steerAngle turns the front pair, and pitch/roll
     * lean the body: pitch off a 4-frame acceleration sum, roll off yaw rate
     * times speed. Both lean angles stay within a few degrees. */
    int distanceTraveled_0x070;
    int steerAngle_0x074;
    int pitchAngle_0x078;
    int rollAngle_0x07c;
    int blinker_0x080;

    /* Bus's world transform matrix, applied by njMultiMatrix/njSetMatrix
     * before drawing and used by njCalcPoint to place the mirror camera. */
    NJS_MATRIX worldMatrix_0x084;

    /* Copy of var_sceneParams_8c18ad24->rec0_0x0c[0] (the primary directional
     * light's coefficient row), set by busInitPlaceBus_8c023310. */
    float lightCoeffRow_0x0c4[5];
    int easyLightIntensityA_0x0d8;
    int easyLightIntensityB_0x0dc;
    int easyLightColorR_0x0e0;
    int easyLightColorG_0x0e4;
    int easyLightColorB_0x0e8;
    /* Last confirmed lane-crossing point (x,z), read by GeomDistanceXZ_8c02081c
     * (022bdc) and written by BusDriveFindLaneTarget_8c023e7e (023938) -- real float fields
     * (FMOV.S stores), not int. */
    float laneTargetX_0x0ec;
    float laneTargetZ_0x0f0;

    /* Averaged with posX_0x2fc/posZ_0x304 by rowMaterialModelTask_8c02a27c to get a
     * distance-fade reference point. posY_0x0f8 (and posHistory_0x100 below)
     * is filled in by busInitPlaceBus_8c023310 via GroundProbeInterpolateHeight_8c020f7e --
     * it treats {posX_0x0f4, posY_0x0f8, posZ_0x0fc} as one contiguous float[3]. */
    float posX_0x0f4;
    float posY_0x0f8;
    float posZ_0x0fc;

    /* Waypoint-position history, 12 entries. busInitPlaceBus_8c023310 seeds
     * rec[0]/rec[1]'s x/z 4.9m/8.0m behind the spawn stop and every rec's y
     * from the just-computed ground height (posY_0x0f8), leaving rec[2..11]'s
     * x/z untouched (stale). */
    NJS_POINT3 posHistory_0x100[12];

    /* 10 ground-polygon samples around the bus, taken by BusDriveSampleGround_8c023938 at
     * corner/lookahead points derived from posHistory_0x100, then read back
     * by BusDriveApplyGround_8c023cba to average a lane-offset ground height. */
    GroundQueryResult groundSamples_0x190[10];

    /* Bus heading unit vector (x,z), dotted and crossed against the move
     * delta by positionCamera_8c024d6c to size/sign its turn. */
    float headingX_0x230;
    int field_0x234;  /* no use site; the pair has no y, so likely padding */
    float headingZ_0x238;
    /* Written as float literals (4.9/2.5/1.25/2.6) by busInitPlaceBus_8c023310;
     * field_0x240 is skipped by that write and its role is unclear. */
    float width_0x23c;
    int field_0x240;
    float height_0x244;
    /* Both written there and read nowhere in decompiled C. */
    float halfHeight_0x248;
    float groundOffset_0x24c;

    int ang_0x250;

    int targetHeadingAngle_0x254;

    int ang_0x258;

    int signalSide_0x25c;
    int turnSignalBlinkCounter_0x260;
    /* No use site. TrafficEntry's same-offset field is unused too. */
    int field_0x264;

    /* Which mirror view is up, a FadeMirrorSelect (022464_fade.h). Set by
     * BusInputUpdate_8c0246b2 from the turn-signal buttons (024280); read by
     * FadeUpdate_8c022560 and BusRenderUpdateMirrorCamera_8c025604. */
    FadeMirrorSelect mirror_0x268;

    /* Divided by pitchCos_0x270 by BusRenderUpdateCamera_8c025078 for the
     * mode-1 camera's Y bob -- a real float field (FMOV.S load), not int. */
    float pitchSin_0x26c;
    float pitchCos_0x270;
    /* dx component of the spawn stop area's direction (StopAreaRecord.dx_0x0c);
     * paired with headingDirZ_0x278 (dz). */
    float headingDirX_0x274;

    float headingDirZ_0x278;
    float speed_0x27c;
    float acc_hist_0x280[4];

    /* No use site. TrafficEntry uses 0x290 as a float in a speed calc
     * (025b98), but the bus never touches it. */
    int field_0x290;
    int field_0x294;
    int field_0x298;

    /* Collision knockback direction (unit vector), set by
     * handleBump_8c02b6d4 (02b464); field_0x2ac/0x2b0 mirror
     * 0x29c/0x2a0 (role of the duplicate pair unclear). */
    float dir_x_0x29c;
    float dir_z_0x2a0;

    int field_0x2a4;  /* no use site */
    int field_0x2a8;  /* no use site */
    float dir_x2_0x2ac;
    float dir_z2_0x2b0;

    /* 0 = stopped at a bus stop (the door sequence in doorState_0x3c0 runs
     * here), 1 = driving, 2 = knockback after a collision, 3 = halted for
     * good, 4 = braking down to 3. All five are dispatched by
     * BusTask_8c022bdc (022bdc). */
    int driveState_0x2b4;

    int currentLinePointPtr_0x2b8; /* the spawn stop area's StopAreaRecord*, cast to int */
    float lineSegmentRemaining_0x2bc;
    float lineSegmentProgress_0x2c0;
    float laneOffset_0x2c4;
    int groundProbeFn_0x2c8;
    int junctionQueryFnCpu_0x2cc;
    int junctionQueryFnRoute_0x2d0;
    int busAheadFlag_0x2d4;
    int lightFadeState_0x2d8;
    int lightFadeGate_0x2dc;
    /* 0 = off, 1 = starting (30-frame crank, idles at 500 rpm), 2 = running.
     * Driven by BusInputUpdate_8c0246b2 (024280); drawHud_8c01fbac (01fa78)
     * eases var_engineRpm_8c226468 toward 0 / 500 / targetRpm_0x2e8
     * accordingly. */
    int engineState_0x2e0;
    /* Throttle ramp phase in BAM; njSin of it times 6000 gives
     * targetRpm_0x2e8. Wound up at the current gear's accelRate_0x00 and
     * clamped to the scaled trigger travel. */
    int rpmRampAngle_0x2e4;
    /* A real float field -- the asm loads it with FMOV.S directly, no
     * int-to-float conversion (Ghidra's decompile shows a spurious int cast
     * here). */
    float targetRpm_0x2e8;
    int idleFrameCounter_0x2ec;
    /* Cleared on the transition into engineState_0x2e0 == 2 (024280) and read
     * nowhere. */
    int field_0x2f0;

    int gear_0x2f4;

    int hazardBlinkCounter_0x2f8;
    /* draw position, read by TileStreamDrawTile_8c021b34 */
    float posX_0x2fc;
    float posY_0x300;
    float posZ_0x304;
    /* Last move-delta vector (x,y,z) applied to posX_0x2fc/posZ_0x304 --
     * rotated toward headingX_0x230/headingZ_0x238 first when the raw bearing
     * change is large -- and its magnitude, all written by
     * positionCamera_8c024d6c. */
    float moveDeltaX_0x308;
    float moveDeltaY_0x30c;
    float moveDeltaZ_0x310;
    float moveDeltaMagnitude_0x314;
    /* Rear-view-mirror camera's local offset (x,y,z), picked by
     * mirror_0x268 and rotated by worldMatrix_0x084 into mirrorWorldOffsetX_0x318/0x32c world
     * offsets, all by BusRenderUpdateMirrorCamera_8c025604. */
    float mirrorWorldOffsetX_0x318;
    float mirrorWorldOffsetY_0x31c;
    float mirrorWorldOffsetZ_0x320;
    float mirrorDirX_0x324;
    int field_0x328;  /* no use site; padding, like field_0x234 */
    float mirrorDirZ_0x32c;
    float mirrorDist_0x330;
    /* Drive BusDriveFindLaneTarget_8c023e7e's search for a lane-change target point (0/1 =
     * search forward/backward from currentLineNodeIdx_0x33c, 2 = idle/done);
     * armed by the same two buttons that toggle signalSide_0x25c, in
     * mapped-route steering mode (024280.c). Despite the name, unrelated to
     * the O_FUMI_* railway level crossing (028258_objects.c) -- "crossing"
     * here means a route-line intersection, not the railway. */
    int laneTargetSearchDone_0x334;
    int laneTargetSearchSide_0x338;
    int currentLineNodeIdx_0x33c;
    int junctionASlot_0x340;
    int field_0x344;
    int field_0x348;
    int junctionARoadFlags_0x34c;
    int field_0x350;
    int field_0x354;
    int junctionARoadFlags2_0x358;
    int junctionBSlot_0x35c;
    int field_0x360;
    int field_0x364;
    int junctionBRoadFlags_0x368;
    int junctionBAttr1_0x36c;
    int field_0x370;
    int junctionBRoadFlags2_0x374;
    int junctionCSlot_0x378;
    int field_0x37c;
    int field_0x380;
    int junctionCRoadFlags_0x384;
    /* Attribute words [1] and [2] of junctionCSlot_0x378's query -- written
     * by 022bdc/023310, read nowhere. */
    int field_0x388;
    int field_0x38c;
    int junctionCRoadFlags2_0x390;
    int cpuAttrOutSlot_0x394;
    int field_0x398;
    int field_0x39c;
    int fallbackTaskMatchId_0x3a0;
    int markAttrOutSlot_0x3a4;
    int field_0x3a8;
    int field_0x3ac;
    int markDriveFlags_0x3b0;
    int markCueByte_0x3b4;
    int markAudioCue_0x3b8;
    int scenePresetIds_0x3bc;

    /* Passenger-door sequence: 0 = shut, 1 = opening, 2 = open, 3 = closing.
     * Only advances while driveState_0x2b4 is 0 (opening) or 1 (closing);
     * var_busDoorFrame_8c227db0 is the animation frame it drives. */
    int doorState_0x3c0;

    /* Asks doorState_0x3c0 for the next step in its cycle -- the A button
     * while stopped, the scripted arrival, or 02b464 after docking the
     * INSTR_DOOR_OPERATION penalty. Cleared once the step is taken. */
    int doorRequest_0x3c4;

    /* Zeroed by BusInitStart_8c023610. Overlaps var_busState_8c1bb9d0.scenePresetIds_0x3bc+0xc
     * (that symbol's reserved span runs 4 bytes past this struct's end) --
     * coincidentally adjacent, not that symbol's field. */
    int cameraYawEase_0x3c8;
} BusState;

/* Per-course badge tier, ratcheted from var_award_8c1bb8f8 (driver points)
 * by ResultShowPassedRun_8c01e0b4. Doubles as a sprite index at one draw
 * site only, where the badge icons happen to sit at 0x18 - tier. */
enum {
    AWARD_TIER_NONE   = 0,
    AWARD_TIER_BRONZE = 1,
    AWARD_TIER_SILVER = 2,
    AWARD_TIER_GOLD   = 3
};

typedef struct {
    Uint8 unlocked_0x00;
    Uint8 new_0x01;
    Uint8 everPlayed_0x02;
    Uint8 storyAward_0x03;
    Uint8 freeRunAward_0x04;
    Uint8 reserved_0x05[3]; // Padding?
} CourseProgress;

typedef struct {
    int days_0x00;

    /* unlock-flag bitsets set together by setProgressFlag_8c02af78
     * and tested individually by hasProgressFlag_8c02afbe/EventHasProgressFlagAlt_8c02aff0 */
    int eventProgressFlags_0x04[5];
    int profileProgressFlags_0x18[5];

    int letters_0x2c[6];
    CourseProgress courses_0x44[9];
    int profileUnlockedCount_0x8c;
    int exp_0x90; // aliased by var_exp_8c1ba25c (01b19c_system_menu addresses it directly; others via this field)
    int field_0x94;
    int practiceLessonBestScores_0x98[11];
    signed char difficulty_0xc4;
    char driveMode_0xc5;
    char defaultView_0xc6;

    /* covers the bytes 011120_asset_queues.c indexes at 0xcc-0xcf */
    char controlAndDisplayFlags_0xc7[9];

    /* 0xd0/0xd1 look like saved input deadzone
     * thresholds (see applyThrottle_8c024320/FUN_8c024606) */
    char accelSensitivity_0xd0;
    char brakeSensitivity_0xd1;
    /* Never touched by any .c; padding between the sensitivity bytes and
     * the volumes at 0xd4. */
    char field_0xd2;
    char field_0xd3;

    /* AUDIO MUSIC/SFX/VOICE volume (0-10); reset with the sound mode
     * (see FileMenuResetSoundDefaults_8c0188dc) */
    char musicVolume_0xd4;
    char sfxVolume_0xd5;
    char voiceVolume_0xd6;
    char reserved_0xd7; // padding

    /* mirrored to/from var_8c1bb8b8/bc/dc/var_award_8c1bb8f8 by 01b19c_system_menu
     * on load (see SystemMenuApplyLoadedProgress_8c01b19c) and save (SystemMenuWriteToVmu_8c01b26c) */
    int introDialogQueued_0xd8;
    int introDialogPending_0xdc;
    int runSucceeded_0xe0;
    char award_0xe4;
    char reserved_0xe5[3]; // padding
} PlayerProgress;

/*
 * SETTING screen's 5 persisted toggle bytes (DIFFICULTY/DRIVE MODE/DEFAULT
 * VIEW/VIBRATION/SCREEN ROLL); var_8c226074 points here while that screen is
 * active. Sits exactly at &var_progress_8c1ba1cc.difficulty_0xc4 (0x1ba1cc+0xc4),
 * but kept as its own symbol since it's owned by sectionB.src, not decompiled.
 * [0] (DIFFICULTY) is read by BusStopSetup_8c02caba to pick the run's
 * driver-points reset value.
 */
extern char var_8c1ba290[5];
/* var_8c1ba290[3], the VIBRATION toggle, with its own export.
 * DriveCueTask_8c020214 reads it that way to stop the pad rumbling once the
 * setting is turned off mid-drive. */
extern char var_vibrationSetting_8c1ba293;

extern int var_exp_8c1ba25c; // EXP shown on the VMU icon status line (see 01b19c_system_menu)

/* single-word bitset, set/tested by setRunEventFlag_8c02b022/hasRunEventFlag_8c02b030; role unclear */
extern int var_runEventFlags_8c1ba2b4;

extern int var_8c1ba2b8[5]; // Maybe progress backup
extern int var_8c1ba2cc[5]; // Maybe progress backup
extern void* var_8c1ba2e0;
extern BUS_BACKUPFILEHEADER var_backupFileHeader_8c1ba2e4; // 018644: analyzed backup file header
extern void* var_vmuIconFileBuf_8c1ba344;
extern void* var_backupFileImageBuf_8c1ba348;
extern int var_8c1ba350;        // 018644: selected save slot / new-file index
extern TrafficSignalDef *var_trafficSignalDefs_8c1bb8a0; // 028258: signal table, terminated by type_0x00 == 0
extern void* var_groundGridFallback_8c1bb86c;

/* base of an 8-byte-stride table of stop-area records, indexed by a segment
 * record's stopAreaId_0x02 (BusStopGetStopArea_8c02cd7a, 02c884); each slot
 * holds a StopAreaRecord* at +0, the trailing 4 bytes unknown. */
extern void *var_stopAreaTable_8c1bb870;

/* var_currentCourse_8c1bb868.atariCpu_0x18 under its own symbol -- the CPU
 * collision grid traffic normally probes against. var_groundGridFallback_8c1bb86c
 * above is the same trick on .atariBus_0x04 and var_groundGridPrimary_8c1bb890
 * below on .atariHum_0x28, so neither of those names says which grid it is
 * either; folding all three into the struct is a move-data job. */
extern void* var_groundGridCpu_8c1bb880;

extern void* var_groundGridPrimary_8c1bb890; // ground query grid, selected into var_activeGroundGrid_8c2264d4

/* pointer to a table of 12-byte stop spawn-area records, indexed by a
 * segment record's field_0x06 (BusStopGetSegment_8c02cd6a, 02c884); a record's field_0x00
 * is a spawn-area pointer (see var_8c22890c below), the rest is unknown.
 * The symbol itself reserves 12 bytes; only the leading 4 (the table
 * pointer) are used by pickWaitingPassengers_8c02c8ae. */
extern void *var_8c1bb894;
extern int var_8c1bb8b8; // Maybe courseMenuHasResult or courseMenuHasDialog
extern int var_8c1bb8bc;
extern int var_8c1bb8c4;
extern int var_pauseActive_8c1bb8cc;
extern int var_8c1bb8d4;
extern int var_runSucceeded_8c1bb8dc;
extern int var_firstClearOfCourse_8c1bb8e0; // course was unlocked
extern int var_passengerCount_8c1bb8e4;
extern int var_eventCount_8c1bb8e8;
extern int var_worstPenaltyMsgSet_8c1bb8ec;
extern int var_worstPenaltyDelta_8c1bb8f0;
extern int var_penaltyCount_8c1bb8f4;
extern Uint8 var_award_8c1bb8f8;

extern void* var_messageTextBoxA_8c1bc404;
extern void* var_messageTextBoxB_8c1bc408; /* second half of the double-buffered message textbox pair */
extern int var_messageTextBoxIndex_8c1bc40c;   /* active index (0/1) into (&var_messageTextBoxA_8c1bc404)[idx] */
extern NJS_MOTION* var_busDoorMotion_8c1bc410;
extern void* var_busDoorShape_8c1bc414;
extern void* var_8c1bc440;
extern void* var_8c1bc444;
/* Current "fuu" stop-marker animation frame, driven by
 * BusStopUpdateArrival_8c02ce48 (02c884): counts up by 1.0 per frame while
 * the bus approaches a stop, wrapping to 0 at var_8c1bc450. */
extern float var_8c1bc44c;
extern float var_8c1bc450;
/* Shared scratch matrix, rebuilt by each user before it reads it back
 * (01fa78, 023938, 024b4c, 02c884). */
extern NJS_MATRIX var_scratchMatrix_8c1bc46c;
extern NJS_POINT3 var_groundQueryPoint_8c1bc460; // scratch world point for ground-height queries, e.g. FUN_8c02840c
extern void* var_vmGameBuf_8c1bc454;
/* IntersectSegments_8c0206f0's intersection-point output, an XZ pair. The two
 * halves are exported separately but must stay adjacent -- callers pass
 * &var_crossingIntersectPoint_8c1bc458 as the whole point. */
extern float var_crossingIntersectPoint_8c1bc458;
extern float var_crossingIntersectPointZ_8c1bc45c;
extern void* var_busFont_8c1ba1c8;
extern BusState var_busState_8c1bb9d0;
/* Points at the player's own BusState (presumably &var_busState_8c1bb9d0).
 * Used in 026710_traffic.c only as a sentinel "entry" marking the player's
 * bus in traffic-avoidance code that otherwise walks a list of real traffic
 * entries (see TrafficComputeBlockedSpeed_8c026eaa) -- identity-compared,
 * never dereferenced there. 02b464 does dereference it, for the player's
 * side of a collision response. */
extern BusState *var_playerBus_8c1bbd9c;
/* The two ends of BusDrawFadeLights_8c028022's (027958_bus_draw) crossfade and
 * the per-frame step between them, cached by TrafficInit_8c02769e from
 * CourseSceneParams.rec0_0x0c rows 1 and 2 -- but only when timeOfDay is
 * TIME_OF_DAY_NIGHT, so by day these hold whatever the last night run left.
 * A row is {intensity0, intensity1, r, g, b}, split 2+3 here exactly as
 * BusState.lightCoeffRow_0x0c4 consumes it. "Off" and "On" are the states of
 * lightFadeGate_0x2dc, a road-polygon attribute word. The ...On1/...Off1 pair
 * is the [1] element of each intensity row under its own symbol, imported
 * separately by 027958_bus_draw's asm. */
extern float var_nightLightIntensityStep_8c1bbda0[2];
extern float var_nightLightIntensityOff_8c1bbda8[2];
extern float var_nightLightIntensityOn_8c1bbdb0[2];
extern float var_nightLightColorStep_8c1bbdb8[3];
extern float var_nightLightColorOff_8c1bbdc4[3];
extern float var_nightLightColorOn_8c1bbdd0[3];
extern float var_nightLightIntensityOn1_8c1bbdb4;
extern float var_nightLightIntensityOff1_8c1bbdac;
extern void* var_busstopDat_8c1bc42c;
extern void* var_busstopPartsDat_8c1bc428;
extern NJS_TEXLIST *var_busStopTexlist_8c1bc424;
extern NJS_CAMERA var_camera_8c1bb904; // 021b9c_tile_draw
extern NJS_CAMERA var_mirrorCamera_8c1bb944; // 021b9c_tile_draw
/* Aimed from the driver's eye down the aisle by DemoBoardingCamera_8c025870
 * (025870); FadeUpdate_8c022560's arrival variant 1 renders it into the inset
 * over the mirror view. */
extern NJS_CAMERA var_cabinCamera_8c1bb984;
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
/* These three are one ResourceGroup (015ab8_title.h) at 8c1bc418: mark.pvm,
 * mark_parts.dat and mark.dat, the same triple 012f44_game.c loads into the
 * declared var_loadingResourceGroup_8c1bc3f8's tlist/tanim/contents. Every
 * TxtDrawSprite_8c014f54 caller reaches them by casting
 * &var_markTexlist_8c1bc418; 8c1bc424 is the same shape for the bus stop.
 * Naming them per-field predates the struct -- folding them into one is a
 * move-data job. */
extern void* var_markDat_8c1bc420;
extern void* var_markPartsDat_8c1bc41c;
extern NJS_TEXLIST *var_markTexlist_8c1bc418;
/* var_routeModelSlots_8c1bbddc[26..31] under their own symbol -- 013ae8 loads
 * all 0x20 slots through that array, which overruns its own label by exactly
 * these 96 bytes. [0] is the player's bus (3s_bus0_l.njd/3t_bus0_l.pvm);
 * BusInitStart_8c023610 takes its texlist/nj for texlistLarge_0x004/
 * modelLarge_0x00c. */
extern ModelSlot var_vehicleModelSlots_8c1bbf7c[6];
extern ModelSlot var_pedestrianAssets_8c1bbfdc[0x41];
extern PDS_PERIPHERAL *var_peripheral_8c1ba358;
extern PDS_PERIPHERAL var_peripherals_8c1ba35c[2];
extern enum PLAY_MODE var_playMode_8c1bb8d0;
extern PlayerProgress var_progress_8c1ba1cc;
extern void* var_routeModels_8c1bc3ec;
/* Its own label only reserves 26 slots; the last 6 are
 * var_vehicleModelSlots_8c1bbf7c (see there). */
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
extern void* var_saveBufCursor_8c225fe0;      // 018644: BupLoad dest buffer, advances 0x600 per file
extern int var_8c225fe4[10];    // 018644
extern int var_loadedSaveCount_8c22600c;        // 018644
extern int var_saveLoadResult_8c226010;        // 018644: load result (1 = done, 2 = error)
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
/* 01fa78_hud scratch; only the last word is live. HudReset_8c02018c zeroes
 * field_0x00/0x04 and nothing reads them back, and field_0x08/0x0c are never
 * touched at all. */
typedef struct {
    int field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
    int driveMarkLatched_0x10;
} HudMarkLatchState;
extern HudMarkLatchState var_hudMarkLatch_8c22643c; // 01fa78
/* HUD sprite id for the map's drive instruction under the bus
 * (markDriveFlags_0x3b0's low 3 bits, + 0x1f), latched by
 * hudUpdateTask_8c01ff48 (01fa78_hud) and only ever -1 before the run's first
 * marked cell. 02b464 and 02c884 read it as "the bus has passed one". */
extern int var_hudDriveMarkIcon_8c226450;
/* Free-running frame counter gating the blink of that HUD slot; reset by
 * hudUpdateTask_8c01ff48 on a new instruction and by
 * BusStopUpdateArrival_8c02ce48 (02c884) on a stop-phase change. */
extern int var_hudBlinkTimer_8c226454;
/* Driver-points meter fill, ramped toward var_driverPoints_8c2285d0 over 20
 * frames (01fa78_hud). field_0x0c is seeded to 1.0f and never read. */
typedef struct {
    float displayedValue_0x00;
    float lastSample_0x04;
    float rampStep_0x08;
    float field_0x0c;
} DriverPointsMeterState;
extern DriverPointsMeterState var_pointsMeter_8c226458; // 01fa78
extern HudVertex var_tachoNeedleVerts_8c226478[3]; // 01fa78
extern HudMarkState var_hudMark_8c2264a8; // 01fa78
extern DriveCueState var_driveCueState_8c2264b8;
extern GroundGrid* var_activeGroundGrid_8c2264d4; // ground query grid currently selected for GroundQueryFindPolygon_8c020914/GroundProbeInterpolateHeight_8c020f7e
extern float var_fadeLightDir0_8c2264d8[3]; // 021b9c_tile_draw: simple-light direction, fade layer 0
extern float var_fadeLightDir1_8c2264e4[3]; // 021b9c_tile_draw: simple-light direction, fade layer 1 (mirror side)
/* 0222dc: copies of var_sceneParams_8c18ad24->rec2_0x74[0..4]. Two separate
 * symbols (not one float[5]) because each is exported/imported on its own in
 * src/asm/sectionB.src -- coincidentally adjacent, not one C variable. */
extern float var_fadeLightIntensity_8c2264f0[2]; // [0..1]
extern float var_fadeLightColor_8c2264f8[3]; // [2..4]
extern float var_fadeEasyLightDir_8c226538[3]; // 021b9c_tile_draw: easy-light direction, shared by both fade layers
/* 0222dc: same deal, for var_sceneParams_8c18ad24->rec1_0x54[0..4]. */
extern float var_fadeEasyLightIntensity_8c226544[2]; // [0..1]
extern float var_fadeEasyLightColor_8c22654c[3]; // [2..4]
extern NJS_CAMERA* var_fadeCamera_8c226558; // 022464: camera passed to njSetCamera by draw_8c022464
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
 * var_currentCourse_8c1bb868.lineBus_0x08/ukn_0x0c by BusInitStart_8c023610,
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
 * change. 012f44_game.c pre-seeds it with the course's opening shot from
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

/* [0] compared against var_prevLaneFlags_8c228688's 0xf000000 bits in gradeIntersection_8c02bb1c
 * (02b464), addressed directly; [1]/[2] (0x228638/0x22863c) are the same
 * two ints but addressed only via var_8c2285c4[29]/[30] -- no code
 * reaches them through this symbol. Role unclear. */
extern int var_8c228634[3];
/* Set to 1 by BusStopUpdateArrival_8c02ce48 (02c884) when var_hudDriveMarkIcon_8c226450 is
 * armed; read by gradeFrame_8c02bcd8 (02b464) only via var_8c2285c4[31] -- no code
 * reaches it through this symbol. */
extern int var_8c228640;

/* PDS_PERIPHERAL.r (see 010e90.h) of var_peripherals_8c1ba35c[0] -- the
 * throttle trigger -- addressed directly by this symbol rather than through
 * the array/field form. */
extern unsigned short var_padTriggerR_8c1ba374;

/* PDS_PERIPHERAL.l of var_peripherals_8c1ba35c[0] -- the brake trigger --
 * addressed directly (see var_padTriggerR_8c1ba374 above for .r). */
extern unsigned short var_padTriggerL_8c1ba376;

/* The progress struct's accelSensitivity_0xd0/brakeSensitivity_0xd1,
 * addressed directly rather than through var_progress_8c1ba1cc. Used as the
 * deadzone thresholds for .r and .l respectively (see
 * var_padTriggerR_8c1ba374/var_padTriggerL_8c1ba376 above). Always consumed
 * as an unsigned byte (every read is followed by EXTU.B in the asm). */
extern unsigned char var_accelSensitivity_8c1ba29c;
extern unsigned char var_brakeSensitivity_8c1ba29d;

/* [0]/[1] a duplicated traffic-signal id (gradeSignals_8c02b8b8, 02b464), addressed
 * both directly and via var_8c2285c4[14]/[15]; [3]/[4] a threshold/counter
 * pair graded by gradeLaneUse_8c02b986 (via var_8c2285c4[17]/[18] there); [6] a
 * driving-state latch toggled by gradeIntersection_8c02bb1c. [0] cleared to ready by
 * armCooldowns_8c02b578 (types 1-3). Other slots unclear.
 *
 * var_8c2285c4[11]/[12]/[13] (0x2285f0/f4/f8, just before this array) have
 * no export of their own; [13] is a one-shot "already graded" latch read
 * by gradeSignals_8c02b8b8. */
extern int var_8c2285fc[8];

/* [5] (0x228630) compared against var_prevLane_8c228684 in gradeLaneUse_8c02b986/gradeIntersection_8c02bb1c
 * (02b464), addressed only via var_8c2285c4[27] -- no code reaches it
 * through this symbol. Other slots unclear. */
extern int var_8c22861c[6];

/* Bitflags set by busDriveDecelerate_8c023bea (023938_bus_drive); bits
 * 0x2/0x4 are read by gradeWallHit_8c02b7ea (02b464) to grade a
 * driver-points penalty -- both set is worse than either alone. Other bits
 * unclear. */
extern int var_wallHitBits_8c228660;

/* Both written by handleBump_8c02b6d4 (02b464) with
 * BusCollisionFindHit_8c02e2dc's result, and read nowhere -- dead stores
 * kept for parity with the original. */
extern BusState *var_8c228664;
extern BusState *var_8c228668;

/* The bus's speed this frame, snapshotted by taskCallback_8c02c072 (02b464)
 * before the graders run so they all see one value. */
extern float var_frameSpeed_8c22866c;

/* The bumped vehicle's speed_0x27c, saved before handleBump_8c02b6d4
 * overwrites it, then given to the player as rebound speed. */
extern float var_bumpSpeed_8c228670;

/* One frame's driving state, all snapshotted together by
 * taskCallback_8c02c072 (02b464) before the graders run.
 *
 * laneA/B/C are junctionARoadFlags2_0x358 / junctionBRoadFlags2_0x374 /
 * junctionCRoadFlags2_0x390 masked with 0xf0000001 -- the bus's three
 * road-probe lanes. prevLane/prevLaneFlags are last frame's, kept in
 * var_8c22861c[5]/var_8c228634[0] between frames. offCourseBits is the
 * larger of junction A's and B's 0x30000 bits: 0x30000 is the severe case
 * gradeOffCourseSevere_8c02b864 grades, 0x20000 and below go to
 * gradeOffCourse_8c02b886. headingVsRoad is 0 when there is no road data,
 * 1 when the bus points along the road and 2 when it points against it --
 * 2 is what gradeSignals_8c02b8b8 reads as wrong-way. */
extern int var_laneA_8c228674;
extern int var_laneB_8c228678;
extern int var_laneC_8c22867c;
extern int var_offCourseBits_8c228680;
extern int var_prevLane_8c228684;
extern int var_prevLaneFlags_8c228688;
extern int var_headingVsRoad_8c22868c;

/* One cooldown per offense category, armed by armCooldowns_8c02b578 (02b464)
 * to 0x96 frames for collision and signal, 0xd2 for the rest, and counted
 * down by taskCallback_8c02c072. They are not independent: the graders run
 * in a chain where each is gated on the cooldown the category above it
 * arms, so a collision silences every lesser offense for 150 frames. A
 * negative value means expired. */
extern int var_cooldownCollision_8c228690;
extern int var_cooldownOffCourse_8c228694;
extern int var_cooldownSignal_8c228698;
extern int var_cooldownLane_8c22869c;
extern int var_cooldownIntersection_8c2286a0;

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

extern int var_8c2285c4[];

/* var_8c2285c4[34] (0x22864c), addressed directly by applyThrottle_8c024320:
 * set to 1 on the upshift out of gear 0, arming gradeFrame_8c02bcd8's
 * rapid-acceleration penalty (02b464), which clears it. */
extern int var_firstUpshift_8c22864c;

extern void *var_8c1bb878;
extern void *var_8c1bb888;
/* Set by BusStopUpdateArrival_8c02ce48 (02c884) when a drive ends with points
 * left and every owed stop served. Never cleared. DriveMsgDraw_8c02b388
 * (02b2f0) is its only reader. */
extern int var_runPassed_8c2285c8;
/* Set to 0x1e by BusStopUpdateArrival_8c02ce48 (02c884) on stop completion;
 * role/owner unclear. */
extern int var_8c2285cc;

/* run completion percentage (0-100); read by ResultShowPassedRun_8c01e0b4
 * to pick the award tier and as the score's EXP component */
extern int var_driverPoints_8c2285d0;

/* The run's starting driver points (100, or 200 on the easiest difficulty
 * outside practice), set with var_driverPoints_8c2285d0 by
 * BusStopSetup_8c02caba. Full-scale value of the HUD points meter. */
extern int var_driverPointsMax_8c2285d4;

/* The timetable slot for the current segment, reloaded from the course's
 * per-segment table by advanceStopSegment_8c02ccae, and the run clock, which
 * BusStopSetup_8c02caba starts 450 frames (15s) before the first slot and
 * gradeFrame_8c02bcd8 advances every frame. Both are 30fps frame counts, both
 * drawn as HH:MM:SS by the HUD. Running past the slot costs points once a
 * second and closes the story-event window (EventPickForSegment_8c02b170).
 * In a practice drill without rule bit 1 the pair is inverted: the slot goes
 * to 0 and the clock counts down to a hard TIME_MANAGEMENT failure. */
extern int var_scheduleTime_8c2285d8;
extern int var_runClock_8c2285dc;

/* Bus-stop arrival state machine driven by BusStopUpdateArrival_8c02ce48
 * (02c884): 0 = cruising, 1 = departed-previous-stop wait, 2 = approaching
 * (mirror-view draw enabled -- gates pedestriansTask_8c0293f6's
 * StopDrawWaitingPassengers_8c02d06c registration), 3 = stopped/waiting, 4 = finishing. */
extern int var_stopPhase_8c2285e4;
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

/* number of slots filled in var_waitingPassengers_8c228798 by pickWaitingPassengers_8c02c8ae
 * (0-16); read by 02d06c/02d968 to spawn that many passenger tasks. */
extern int var_waitingPassengerCount_8c228794;

/* one waiting passenger picked for the upcoming stop by
 * pickWaitingPassengers_8c02c8ae */
typedef struct {
    void *spot_0x00;  // chosen candidate-stop entry (segment record's list at +8)
    NJS_POINT3 pos_0x04; // world position; y filled in by the ground snap
    float index_0x10; // 0, 1, 2, ... in pick order
} WaitingPassengerSlot;
extern WaitingPassengerSlot var_waitingPassengers_8c228798[16];

/* fixed 31-slot table of scripted/special waiting-passenger schedule entries,
 * one per fixed stop position along the route; -1 = unused. Read by
 * StopSpawnInit_8c02d968 to spawn each slot's passenger task. */
extern int var_stopSchedule_8c228718[31];

/* The one NJS_SPRITE every passenger is drawn through, waiting at the stop
 * (02d06c) or inside the bus (02d19c). Only sx/sy/ang/tanim are reset up
 * front; p and tlist are set per draw. */
extern NJS_SPRITE var_passengerSprite_8c2288d8;

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

/* Spawn-area record for the upcoming stop's segment (var_8c1bb894 entry
 * selected by its segment record's field_0x06). Laid out as StopAreaRecord
 * (02c884_bus_stop.h), which pickWaitingPassengers_8c02c8ae casts it to;
 * typed char* here to keep sectionB.h free of that include. */
extern char *var_8c22890c;

/* Six consecutive NJS_POINT3 waypoints (0x228910-0x22894c, 12 bytes apart),
 * filled by StopSpawnInit_8c02d968 at course start and stepped through by
 * 02d19c's passenger tasks, three per sequence in the numbered order. Passengers do
 * not walk between them: each step happens on the one frame
 * var_passengersFadedOut_8c22895c is set, so the sprite vanishes at one spot and
 * reappears at the next.
 *
 * Named by position in the sequence rather than by what is there, because
 * what is there moves: StopSpawnInit_8c02d968 fills the six slots from the
 * same six coordinates in REVERSED order on Ome, swapping which door each
 * sequence uses. (0.15, 0.68, 0.27) is hard against the driver -- the fare
 * box -- and lands in boardSpot2 on flat-fare Shinjuku/Wangan but in
 * exitSpot2 on distance-fare Ome, which is also why 02d19c gates the
 * boarding voice lines on non-Ome and the exiting ones on Ome: the
 * greeting plays wherever the passenger passes the driver.
 *
 * Geometry, against init_seatPositions_8c04c3e4: the +x wall has no seat
 * slot between z = -0.33 and 3.95, and every waypoint falls in that gap, in
 * two z clusters (~0.3-0.9 and ~2.6-3.0) -- two doors. (1.0, 0.35, 2.8) is
 * the only point at half floor height and sits alone in the gap: the
 * doorwell step.
 *
 * Open: the two y=0 points, (-1.6, 0, 0.9) and (-1.39, 0, 2.63), are on -x,
 * the side with the unbroken seat row, though their z matches the doors.
 * They are also the only two rewritten in place by njCalcPoint against
 * var_busState_8c1bb9d0.worldMatrix_0x084, and StopSpawnInit_8c02d968 runs once at
 * course start -- so that transform, not the coordinate, is the thing to
 * check. */
extern NJS_POINT3 var_boardSpot2_8c228910;
extern NJS_POINT3 var_boardSpot3_8c22891c;
extern NJS_POINT3 var_boardSpot1_8c228928;
extern NJS_POINT3 var_exitSpot1_8c228934;
extern NJS_POINT3 var_exitSpot2_8c228940;
extern NJS_POINT3 var_exitSpot3_8c22894c;

/* Set by any 02d19c passenger task that advanced this frame; cleared by the
 * scene task before it pumps the group, then read back to decide whether
 * the stop is still in progress. Int, not float, despite sitting in the
 * middle of the anchor-point float run. */
extern int var_passengerActed_8c228958;

/* Nonzero for the single frame the passenger fade reaches full black.
 * Passenger tasks only advance a step while it is set, so they move unseen. */
extern int var_passengersFadedOut_8c22895c;

/* Constant material for the pass the passenger sprites draw in, passed to
 * njSetConstantMaterial by StopDrawLightBegin_8c02d0fc (02d06c). [0] is the
 * alpha 02d19c's scene task fades in and out at 1/15 per frame, [1..3] the
 * rgb, held at 1.0. [4] is past the NJS_ARGB and is not part of the color.
 * The interior model itself sets no constant material. */
extern float var_passengerFadeColor_8c228960[5];

/* Task group for the bus-stop subsystem's waiting-passenger/departure tasks
 * (see StopSpawnInit_8c02d968); -1 means not currently allocated. */
extern void* var_stopTaskGroup_8c2288f8;
extern Sint8 var_coursesToUnlock_8c225fd4[];
extern int var_currentSegment_8c228708;

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
extern int var_8c226414[6]; /* dialog id queue built by buildDialogQueue_8c01e992, -1 terminated */
extern int var_8c22642c; /* lesson attempt counter, incremented on practice retry */
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
extern char *var_8c226074; /* SETTING screen: ptr to the 5 gameplay-setting bytes */
/* AUDIO sound-test fields: one int per digit, least significant at index 0. */
extern int var_8c226078[2]; /* MUSIC TEST */
extern int var_8c226080[2]; /* SFX TEST */
extern int var_8c226088[4]; /* VOICE TEST */
extern float var_engineRpm_8c226468; // real type is 0100bc_sound.c's local UnknownVolStructB {float}
extern int var_8c22646c; // written (zeroed) by HudReset_8c02018c (01fa78_hud), never read
/* Gear-message / lane-change-message latch for hudUpdateTask_8c01ff48
 * (01fa78): set once the corresponding driver-comment popup has been staged,
 * cleared when the bus-state bit returns to 0. */
extern int var_gearLatch_8c226470;
extern int var_laneLatch_8c226474;
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

/* CollisionFindTaskHit_8c02e400 (02e400) and BusCollisionFindHit_8c02e2dc (02e2dc): cursor
 * into var_tasks_8c1bac28 during its scan, left pointing at the terminating
 * (action == 0) slot on exit. */
extern Task *var_collisionScanCursor_8c228974;
/* CollisionFindTaskHit_8c02e400/BusCollisionFindHit_8c02e2dc scratch: world-space oriented
 * bounding box, njCalcPoints'd from the entry/bus passed in (self) and from
 * the current candidate task respectively. */
extern NJS_BOX var_collisionSelfBox_8c228978;
extern NJS_BOX var_collisionCandidateBox_8c2289d8;
/* CollisionQueueAdd_8c02e48e/CollisionQueueTest_8c02e4ac: fixed 64-slot queue of
 * pending collision candidates, written and scanned once per frame. */
extern void *var_collisionQueue_8c228a38[64];
/* CollisionQueueReset_8c02e486/CollisionQueueAdd_8c02e48e/CollisionQueueTest_8c02e4ac:
 * element count of the fixed 64-slot queue at var_collisionQueue_8c228a38. */
extern Sint32 var_collisionQueueCount_8c228b38;

#endif // _0FCD20_SECTIONB_H
