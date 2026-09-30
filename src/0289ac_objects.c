/* @unit Objects */
/* 8c0289ac */
#include <shinobi.h>

#include "0289ac_objects.h"
#include "011120_asset_queues.h" /* AsqRequestNj_8c011492, AsqRequestPvm_8c011ac0, AsqRequestDat_8c011182 */
#include "013ae8_route_load.h" /* var_commonDirCopy_8c18ad8c, var_commonDir_8c18ad6c, enum ROUTE */
#include "014a9c_tasks.h" /* Task */
#include "015034_text.h" /* enum PLAY_MODE */
#include "016d2c_course_menu.h" /* var_menuTextboxCharLimit_8c225fb8 */
#include "0100bc_sound.h" /* SndStopAdx_8c010ca6, SndPlayAdx_8c010cd6, SndPollVoiceEnd_8c0106ac */
#include "0206f0_intersect.h" /* IntersectSegments_8c0206f0 */
#include "020914_ground_query.h" /* GroundQueryFindPolygon_8c020914, GroundQueryResult */
#include "020b6c_ground_probe.h"
#include "021b9c_tile_draw.h"
#include "022464_render.h" /* FadeRequest, var_fadeRequest_8c226564, var_arrivalOverlayGate_8c226560 */
#include "028258_traffic_signal.h" /* SignalGetFrame_8c028900, SignalClearPedCrossingFlags_8c02890c */
#include "02d06c_stop_draw.h" /* StopDrawWaitingPassengers_8c02d06c */
#include "02af78_event.h" /* EventApplyFlags_8c02b292 */
#include "02e400_collision.h" /* CollisionQueueReset_8c02e486, CollisionQueueAdd_8c02e48e */
#include "02fb50_sh4nlfzn.h" /* rand */
#include "02b464_drive_points.h"
#include "024b4c_bus_render.h"
#include "sectionB.h" /* var_busState_8c1bb9d0, ground query globals,
                        * var_pedGroups_8c228230, var_pedPaths_8c228238,
                        * var_pedestrianAssets_8c1bbfdc, AsqGetRandomA_8c012166,
                        * var_eventSlides_8c228480, MessageAssetEntry, var_messageAssets_8c228484,
                        * var_selectedEventEntry_8c228478 */
#include "1ba1c8_globals.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"

/* ====================
 * Type Declarations
 * ====================
 */

/* One blinker spawn point, terminated by an all-zero entry. */
typedef struct {
    float x_0x00;
    float z_0x04;
    int angleDeg_0x08;
} RouteMarkerPoint;


/* One node of a pedestrian path, terminated by a node whose flLength_0x00 is 0.
 * A pedestrian flPathPos_0x54 units along the segment sits at
 * flBase + flPathPos * flStep. */
typedef struct {
    float flLength_0x00;
    float flBaseX_0x04;
    float flBaseZ_0x08;
    float flStepX_0x0c;
    float flStepZ_0x10;
    /* Bits 0-11/16-27: crosswalk signal ids, split into nSignalIdB_0x6c/A_0x68.
     * Bits 28-29: mirror-view facing bucket (drawPedestriansMirror_8c028a38). */
    int nFlags_0x14;
} PedPathNode;

/* Task state for a walking pedestrian, driven by pedestrianTask_8c028e00. */
typedef struct {
    NJS_SPRITE sprite_0x00;
    float flBaseX_0x20;
    float flBaseZ_0x24;
    GroundQueryResult aGround_0x28;
    int nKindId_0x38;
    int nKind_0x3c;
    int nReverse_0x40; /* also this pedestrian's sprite-facing quadrant */
    PedPathNode *pPathFirst_0x44;
    PedPathNode *pPathLast_0x48;
    PedPathNode *pPathNode_0x4c;
    float flPathLength_0x50;
    float flPathPos_0x54;
    float flSpeed_0x58;
    int nAnimPhase_0x5c;
    int nNodeFlags_0x60;   /* copy of pPathNode_0x4c's nFlags_0x14 */
    int nState_0x64;       /* 0 walking, 1 checking the signal, 2 waiting, 4 crossing */
    int nSignalIdA_0x68;   /* nNodeFlags_0x60's high 12 bits */
    int nSignalIdB_0x6c;   /* nNodeFlags_0x60's low 12 bits */
} PedestrianState;

/* Capacity of drawPedestrians_8c028b74's per-frame facing cache; matches the
 * original's 0x400-byte allocation. */
#define DIR_CACHE_CAPACITY 0x80

/* Live-pedestrian cap for one group. */
#define PED_GROUP_SLOTS 0x10

typedef struct {
    PedPathNode *key;
    int bucket; /* 0, 0x10000000, 0x20000000 or 0x30000000 */
} DirCacheEntry;

/* One per pedestrian-group slot; var_pedGroups_8c228230's entry type. */
typedef struct {
    int active_0x00;   /* whether list_0x08 has been allocated */
    /* refreshed each frame by pedestriansTask_8c0293f6; drives pedGroupTask_8c029078's
     * teardown decision */
    int wanted_0x04;
    Task *list_0x08;    /* the group's own Task subgroup; NULL-terminated, freed slots -1 */
} PedGroupEntry;

/* One spawn entry from a group's spec list (PedGroupDef.specs_0x08), walked by
 * pedGroupTask_8c029078. */
typedef struct {
    Uint16 nKindId_0x00; /* list terminator: 0xffff */
    Uint8 nKind_0x02;
    Uint8 nReverse_0x03;
    float flX_0x04; /* nKind_0x02 0/1: along-path spawn distance; nKind_0x02 2: world X */
    float flZ_0x08; /* nKind_0x02 0/1: unused; nKind_0x02 2: world Z */
} PedGroupSpawnSpec;

/* var_pedGroupDefs_8c22823c entry, looked up by id when pedestriansTask_8c0293f6
 * spins a group's Task up for the first time. */
typedef struct {
    int id_0x00;
    float radius_0x04;             /* seeds the group's PedGroupTask.radius_0x10 */
    PedGroupSpawnSpec *specs_0x08;
} PedGroupDef;

/* var_pedPaths_8c228238 entry, indexed in lockstep with var_pedGroups_8c228230
 * by pedGroupTask_8c029078. */
typedef struct {
    PedPathNode *pFirst_0x00;
    PedPathNode *pLast_0x04;
    float flLength_0x08;
} PedPathInfo;

/* Task private state for pedGroupTask_8c029078, set up by pedestriansTask_8c0293f6. */
typedef struct {
    TaskAction action;
    void *state;
    int pageListIndex_0x08;
    void *field_0x0c;
    float radius_0x10; /* overlays Task's int field_0x10; no reinterpret cast needed at the read site */
    int field_0x14;
    Task *subTasks_0x18;
    PedGroupSpawnSpec *spec_0x1c;
} PedGroupTask;

/* Task private state for pedestriansTask_8c0293f6. */
typedef struct {
    TaskAction action;
    void *state;
    int lastPreset_0x08;   /* currently-synced var_activePedPreset_8c22822c snapshot */
    int debounce_0x0c;      /* pending-preset-change flag, held across the frame it reads 0 */
    int field_0x10;
    int field_0x14;
    void *queuedItem_0x18;
    int field_0x1c;
} PedestriansTask;

/* Trailing 8 bytes of a type-5 row's source struct (see
 * ObjectsStartAssetRequests_8c029ad4), copied verbatim into the dest slot;
 * compiles to a call to the SHC struct-copy helper __quick_evn_mvn. Read by
 * rowMaterialModelTask_8c02a27c as the row's world X/Z position for its distance-fade calc. */
typedef struct {
    float posX_0x00;
    float posZ_0x04;
} ObjectAssetType5Extra;


/* One row's slot in var_assetRequestSlots_8c228288: the handles
 * ObjectsStartAssetRequests_8c029ad4 asks the asset queues to fill in, read back
 * by ObjectsPushTasks_8c02a6ac once loaded. Which members a row uses depends on
 * its type. */
typedef struct {
    NJS_TEXLIST *pvm_0x00;
    NJS_CNK_OBJECT *nj_0x04;
    void *dat_0x08;                 /* NJS_OBJECT for type 0, NJS_MOTION for types 0/3, DatBlob for type 1 */
    ObjectAssetType5Extra pos_0x0c; /* type 5 only */
    /* types 2/3/4; matched against var_busState_8c1bb9d0.scenePresetIds_0x3bc's top byte, see
     * rowModelTask_8c02a08a */
    Sint8 sceneGate_0x14;
    /* types 2/3/4; routed into the spawned Task's field_0x0c (2/3) or field_0x08 (4) */
    Sint8 taskFlag_0x15;
    Uint8 fogEnable_0x16;            /* types 2/3/4 */
    Uint8 control3DEnable_0x17;      /* types 2/3/4 */
} AssetRequestSlot;

/* A type-1 row's loaded DAT blob: a header followed in the same allocation by
 * one visibility bitmask per animation step. evalflags bit convention: clear
 * hides a child (0x3e), set shows it (0x36). */
typedef struct {
    int nNodes_0x00;                   /* bounds the child walk; counts from bit 1, bit 0 unused */
    int framesPerStep_0x04;
    int stepCount_0x08;
    int step_0x0c;
    int tick_0x10;
    Uint32 *masks_0x14;                /* == masks_0x20 */
    NJS_TEXLIST *texlist_0x18;
    NJS_CNK_OBJECT *model_0x1c;
    Uint32 masks_0x20[1];              /* stepCount_0x08 entries */
} DatBlob;

/* Shared TaskPush_8c014ae8 state for every row-type task ObjectsPushTasks_8c02a6ac
 * spawns -- it pushes sizeof(RowTaskState) (0x7c) for all of them: -- rowFlyByTask_8c029e94/flyByModelTask_8c029e68,
 * rowDatTask_8c029fcc, rowModelTask_8c02a08a, rowMotionModelTask_8c02a120,
 * rowSimpleModelTask_8c02a1f0, rowMaterialModelTask_8c02a27c and
 * fumiCrossingTask_8c02a4f8. Each row type only touches the subset of fields
 * it needs; bytes below 0x40 are never read or written anywhere in this unit.
 *
 * `dat_0x48` mirrors ObjectsPushTasks_8c02a6ac's own row-local `dat` variable: an
 * NJS_MOTION* for types 0/3, a DatBlob* for type 1, or a randomly-chosen train
 * NJS_MOTION* for type 6, hence declared as a bare void*. */
typedef struct RowTaskState {
    char field_0x00[0x40];
    NJS_TEXLIST *texlist_0x40;
    NJS_CNK_OBJECT *model_0x44;
    void *dat_0x48;
    ObjectAssetType5Extra pos_0x4c;    /* type-5 only */
    int phase_0x54;                    /* types 2/3/6 */
    float frame_0x58;
    float frameLimit_0x5c;
    float trainFrame_0x60;             /* type-6 only */
    float trainActive_0x64;            /* type-6 only: frame threshold, zeroed once the train has passed */
    NJS_ARGB material_0x68;            /* type-5 only */
    Uint8 fogEnable_0x78;              /* types 2/3 */
    Uint8 control3DEnable_0x79;        /* types 2/3 */
    Uint8 field_0x7a[2];
} RowTaskState;

/* Where messageBoxTask_8c02ab7a picks up on the current frame. The stages run in
 * this order and fall through to the next one; the phase in state->phase_0x00 only decides
 * where to enter. */
typedef enum {
    MSGBOX_STAGE_SWAP,
    MSGBOX_STAGE_WAIT,
    MSGBOX_STAGE_DRAW
} MsgBoxStage;

/* messageBoxTask_8c02ab7a's TaskPush_8c014ae8 state, exactly the 0x1c it asks
 * for (see tests/0289ac_objects/8c02ab7a_messageBoxTask.php's ST_* offsets). */
typedef struct {
    int phase_0x00;
    int pageCount_0x04;
    int pageIndex_0x08;
    int frameCounter_0x0c;
    EventSlide *slide_0x10;
    unsigned short *ids_0x14;
    EventLine *line_0x18;
} MessageBoxState;

/* One entry of init_objectAssetFiles_8c046758, the .map/.pvm filename pair
 * for a message-box asset id. */
typedef struct {
    char *map_0x00;
    char *pvm_0x04;
} MessageAssetFiles;

/* ====================
 * Initialized Globals
 * ====================
 */

/* Mechanically dumped from src/asm/decompiled/0289ac_objects.src sections C/D
 * via scripts/dump_src_data.py, in original file (== address) order; a few
 * symbols below are hand-typed where a semantic type earns its keep, verified
 * byte-identical against the archived asm via scripts/dcdiff.py. */

/* Sprite frames for the pedestrian billboards; indexed off the ped kind. */
NJS_TEXANIM init_pedestrianTexAnims_8c04623c[] = {
    {   /* [0] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 128, 0,
        /* u2, v2   */ 191, 63,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [1] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 192, 0,
        /* u2, v2   */ 255, 63,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [2] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 0, 64,
        /* u2, v2   */ 63, 127,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [3] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 192, 0,
        /* u2, v2   */ 255, 63,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [4] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 128, 0,
        /* u2, v2   */ 191, 63,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [5] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 64, 0,
        /* u2, v2   */ 127, 63,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [6] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 0, 0,
        /* u2, v2   */ 63, 63,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [7] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 64, 0,
        /* u2, v2   */ 127, 63,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [8] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 192, 64,
        /* u2, v2   */ 255, 127,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [9] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 0, 128,
        /* u2, v2   */ 63, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [10] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 64, 128,
        /* u2, v2   */ 127, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [11] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 0, 128,
        /* u2, v2   */ 63, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [12] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 192, 64,
        /* u2, v2   */ 255, 127,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [13] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 128, 64,
        /* u2, v2   */ 191, 127,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [14] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 64, 64,
        /* u2, v2   */ 127, 127,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [15] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 128, 64,
        /* u2, v2   */ 191, 127,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [16] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 0, 192,
        /* u2, v2   */ 63, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [17] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 64, 192,
        /* u2, v2   */ 127, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [18] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 128, 192,
        /* u2, v2   */ 191, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [19] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 64, 192,
        /* u2, v2   */ 127, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [20] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 0, 192,
        /* u2, v2   */ 63, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [21] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 192, 128,
        /* u2, v2   */ 255, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [22] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 128, 128,
        /* u2, v2   */ 191, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [23] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 192, 128,
        /* u2, v2   */ 255, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [24] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 63, 192,
        /* u2, v2   */ 0, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [25] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 127, 192,
        /* u2, v2   */ 64, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [26] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 191, 192,
        /* u2, v2   */ 128, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [27] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 127, 192,
        /* u2, v2   */ 64, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [28] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 63, 192,
        /* u2, v2   */ 0, 255,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [29] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 255, 128,
        /* u2, v2   */ 192, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [30] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 191, 128,
        /* u2, v2   */ 128, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [31] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 255, 128,
        /* u2, v2   */ 192, 191,
        /* texid    */ 0,
        /* attr     */ 0,
    },
    {   /* [32] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 0, 0,
        /* u2, v2   */ 127, 127,
        /* texid    */ 1,
        /* attr     */ 0,
    },
    {   /* [33] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 128, 0,
        /* u2, v2   */ 255, 127,
        /* texid    */ 1,
        /* attr     */ 0,
    },
    {   /* [34] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 0, 128,
        /* u2, v2   */ 127, 255,
        /* texid    */ 1,
        /* attr     */ 0,
    },
    {   /* [35] */
        /* sx, sy   */ 64, 128,
        /* cx, cy   */ 32, 128,
        /* u1, v1   */ 127, 128,
        /* u2, v2   */ 0, 255,
        /* texid    */ 1,
        /* attr     */ 0,
    },
};

/* Endpoints of the bus's stop line in bus-local space; njCalcPoint'd into
 * var_stopLinePointA/B_8c228268 each time the route state is rebuilt. */
STATIC NJS_POINT3 init_stopLineLocalA_8c04650c = { 0.0f, 0.0f, -4.1f };

STATIC NJS_POINT3 init_stopLineLocalB_8c046518 = { 0.0f, 0.0f, 9.5f };

/* Route-specific blinker spawn point tables, keyed by var_route_8c18ad1c. */
STATIC RouteMarkerPoint init_shinjukuBlinkerPoints_8c046524[] = {
    {   /* [0] */
        /* x_0x00, z_0x04 */ 4301.0f, 4493.0f,
        /* angleDeg_0x08  */ 68,
    },
    {   /* [1] */
        /* x_0x00, z_0x04 */ 3898.0f, 4387.0f,
        /* angleDeg_0x08  */ 6,
    },
    {   /* [2] */
        /* x_0x00, z_0x04 */ 3867.0f, 3877.0f,
        /* angleDeg_0x08  */ -18,
    },
    {   /* [3] */
        /* x_0x00, z_0x04 */ 3979.0f, 3709.0f,
        /* angleDeg_0x08  */ 70,
    },
    {   /* [4] */
        /* x_0x00, z_0x04 */ 3022.0f, 3051.0f,
        /* angleDeg_0x08  */ 18,
    },
    {   /* [5] */
        /* x_0x00, z_0x04 */ 2905.0f, 2484.0f,
        /* angleDeg_0x08  */ 109,
    },
    {   /* [6] */
        /* x_0x00, z_0x04 */ 2788.0f, 2552.0f,
        /* angleDeg_0x08  */ 20,
    },
    {   /* [7] */
        /* x_0x00, z_0x04 */ 2311.0f, 1604.0f,
        /* angleDeg_0x08  */ 30,
    },
    {   /* [8] */
        /* x_0x00, z_0x04 */ 2426.0f, 772.0f,
        /* angleDeg_0x08  */ 89,
    },
    {   /* [9] */
        /* x_0x00, z_0x04 */ 2406.0f, 850.0f,
        /* angleDeg_0x08  */ -3,
    },
    {   /* [10] */
        /* x_0x00, z_0x04 */ 1999.0f, 848.0f,
        /* angleDeg_0x08  */ 20,
    },
    {   /* [11] */
        /* x_0x00, z_0x04 */ 640.0f, 154.0f,
        /* angleDeg_0x08  */ 90,
    },
    {   /* [12] */
        /* x_0x00, z_0x04 */ 500.0f, 154.0f,
        /* angleDeg_0x08  */ 180,
    },
    {   /* [13] */
        /* x_0x00, z_0x04 */ 537.0f, 349.0f,
        /* angleDeg_0x08  */ 90,
    },
    {   /* [14] */
        /* x_0x00, z_0x04 */ 0.0f, 0.0f,
        /* angleDeg_0x08  */ 0,
    },
};

STATIC RouteMarkerPoint init_wanganBlinkerPoints_8c0465d8[] = {
    {   /* [0] */
        /* x_0x00, z_0x04 */ 3948.0f, 2871.0f,
        /* angleDeg_0x08  */ -90,
    },
    {   /* [1] */
        /* x_0x00, z_0x04 */ 4024.0f, 2825.0f,
        /* angleDeg_0x08  */ -90,
    },
    {   /* [2] */
        /* x_0x00, z_0x04 */ 4171.0f, 3055.0f,
        /* angleDeg_0x08  */ 122,
    },
    {   /* [3] */
        /* x_0x00, z_0x04 */ 2938.0f, 3860.0f,
        /* angleDeg_0x08  */ 135,
    },
    {   /* [4] */
        /* x_0x00, z_0x04 */ 2823.0f, 3169.0f,
        /* angleDeg_0x08  */ 10,
    },
    {   /* [5] */
        /* x_0x00, z_0x04 */ 2630.0f, 4066.0f,
        /* angleDeg_0x08  */ -160,
    },
    {   /* [6] */
        /* x_0x00, z_0x04 */ 2894.0f, 4580.0f,
        /* angleDeg_0x08  */ 124,
    },
    {   /* [7] */
        /* x_0x00, z_0x04 */ 2195.0f, 4344.0f,
        /* angleDeg_0x08  */ 14,
    },
    {   /* [8] */
        /* x_0x00, z_0x04 */ 2603.0f, 2848.0f,
        /* angleDeg_0x08  */ 15,
    },
    {   /* [9] */
        /* x_0x00, z_0x04 */ 748.0f, 1974.0f,
        /* angleDeg_0x08  */ -23,
    },
    {   /* [10] */
        /* x_0x00, z_0x04 */ 741.0f, 1824.0f,
        /* angleDeg_0x08  */ -14,
    },
    {   /* [11] */
        /* x_0x00, z_0x04 */ 764.0f, 1601.0f,
        /* angleDeg_0x08  */ 90,
    },
    {   /* [12] */
        /* x_0x00, z_0x04 */ 604.0f, 1610.0f,
        /* angleDeg_0x08  */ 25,
    },
    {   /* [13] */
        /* x_0x00, z_0x04 */ 728.0f, 1302.0f,
        /* angleDeg_0x08  */ -112,
    },
    {   /* [14] */
        /* x_0x00, z_0x04 */ 796.0f, 1335.0f,
        /* angleDeg_0x08  */ -20,
    },
    {   /* [15] */
        /* x_0x00, z_0x04 */ 1174.0f, 486.0f,
        /* angleDeg_0x08  */ 70,
    },
    {   /* [16] */
        /* x_0x00, z_0x04 */ 0.0f, 0.0f,
        /* angleDeg_0x08  */ 0,
    },
};

STATIC RouteMarkerPoint init_omeBlinkerPoints_8c0466a4[] = {
    {   /* [0] */
        /* x_0x00, z_0x04 */ 6992.0f, 6229.0f,
        /* angleDeg_0x08  */ -123,
    },
    {   /* [1] */
        /* x_0x00, z_0x04 */ 7179.0f, 6394.0f,
        /* angleDeg_0x08  */ -48,
    },
    {   /* [2] */
        /* x_0x00, z_0x04 */ 7307.0f, 6233.0f,
        /* angleDeg_0x08  */ 7,
    },
    {   /* [3] */
        /* x_0x00, z_0x04 */ 7282.0f, 6159.0f,
        /* angleDeg_0x08  */ 104,
    },
    {   /* [4] */
        /* x_0x00, z_0x04 */ 7189.0f, 6189.0f,
        /* angleDeg_0x08  */ 63,
    },
    {   /* [5] */
        /* x_0x00, z_0x04 */ 6871.0f, 6076.0f,
        /* angleDeg_0x08  */ 5,
    },
    {   /* [6] */
        /* x_0x00, z_0x04 */ 6781.0f, 5526.0f,
        /* angleDeg_0x08  */ 45,
    },
    {   /* [7] */
        /* x_0x00, z_0x04 */ 6492.0f, 5214.0f,
        /* angleDeg_0x08  */ 38,
    },
    {   /* [8] */
        /* x_0x00, z_0x04 */ 5486.0f, 3643.0f,
        /* angleDeg_0x08  */ 55,
    },
    {   /* [9] */
        /* x_0x00, z_0x04 */ 4581.0f, 3102.0f,
        /* angleDeg_0x08  */ 7,
    },
    {   /* [10] */
        /* x_0x00, z_0x04 */ 4915.0f, 2539.0f,
        /* angleDeg_0x08  */ 17,
    },
    {   /* [11] */
        /* x_0x00, z_0x04 */ 794.0f, 1025.0f,
        /* angleDeg_0x08  */ 29,
    },
    {   /* [12] */
        /* x_0x00, z_0x04 */ 746.0f, 979.0f,
        /* angleDeg_0x08  */ 160,
    },
    {   /* [13] */
        /* x_0x00, z_0x04 */ 5902.0f, 4057.0f,
        /* angleDeg_0x08  */ 50,
    },
    {   /* [14] */
        /* x_0x00, z_0x04 */ 0.0f, 0.0f,
        /* angleDeg_0x08  */ 0,
    },
};

STATIC MessageAssetFiles init_objectAssetFiles_8c046758[] = {
    {"", ""},
    {"B001.map", "B001.pvm"},
    {"B002.map", "B002.pvm"},
    {"B003.map", "B003.pvm"},
    {"B004.map", "B004.pvm"},
    {"B005.map", "B005.pvm"},
    {"B006.map", "B006.pvm"},
    {"B007.map", "B007.pvm"},
    {"B008.map", "B008.pvm"},
    {"B009.map", "B009.pvm"},
    {"B010.map", "B010.pvm"},
    {"B011.map", "B011.pvm"},
    {"B012.map", "B012.pvm"},
    {"B013.map", "B013.pvm"},
    {"B014.map", "B014.pvm"},
    {"B015.map", "B015.pvm"},
    {"B016.map", "B016.pvm"},
    {"B017.map", "B017.pvm"},
    {"B018.map", "B018.pvm"},
    {"B019.map", "B019.pvm"},
    {"B020.map", "B020.pvm"},
    {"B021.map", "B021.pvm"},
    {"B022.map", "B022.pvm"},
    {"B023.map", "B023.pvm"},
    {"B024.map", "B024.pvm"},
    {"B025.map", "B025.pvm"},
    {"B026.map", "B026.pvm"},
    {"B027.map", "B027.pvm"},
    {"B028.map", "B028.pvm"},
    {"B029.map", "B029.pvm"},
    {"B030.map", "B030.pvm"},
    {"B031.map", "B031.pvm"},
    {"B032.map", "B032.pvm"},
    {"B033.map", "B033.pvm"},
    {"B034.map", "B034.pvm"},
    {"B035.map", "B035.pvm"},
    {"B036.map", "B036.pvm"},
    {"B037.map", "B037.pvm"},
    {"B038.map", "B038.pvm"},
    {"B039.map", "B039.pvm"},
    {"B040.map", "B040.pvm"},
    {"B041.map", "B041.pvm"},
    {"B042.map", "B042.pvm"},
    {"B043.map", "B043.pvm"},
    {"B044.map", "B044.pvm"},
    {"B045.map", "B045.pvm"},
    {"B046.map", "B046.pvm"},
    {"B047.map", "B047.pvm"},
    {"B048.map", "B048.pvm"},
    {"B049.map", "B049.pvm"},
    {"B050.map", "B050.pvm"},
    {"B051.map", "B051.pvm"},
    {"B052.map", "B052.pvm"},
    {"B053.map", "B053.pvm"},
    {"B054.map", "B054.pvm"},
    {"B055.map", "B055.pvm"},
    {"B056.map", "B056.pvm"},
    {"B057.map", "B057.pvm"},
    {"B058.map", "B058.pvm"},
    {"B059.map", "B059.pvm"},
    {"B060.map", "B060.pvm"},
    {"B061.map", "B061.pvm"},
    {"B062.map", "B062.pvm"},
    {"B063.map", "B063.pvm"},
    {"B064.map", "B064.pvm"},
    {"B065.map", "B065.pvm"},
    {"B066.map", "B066.pvm"},
    {"B067.map", "B067.pvm"},
    {"B068.map", "B068.pvm"},
    {"B069.map", "B069.pvm"},
    {"B070.map", "B070.pvm"},
    {"B071.map", "B071.pvm"},
    {"B072.map", "B072.pvm"},
    {"B073.map", "B073.pvm"},
    {"B074.map", "B074.pvm"},
    {"B075.map", "B075.pvm"},
    {"B076.map", "B076.pvm"},
    {"B077.map", "B077.pvm"},
    {"B078.map", "B078.pvm"},
    {"B079.map", "B079.pvm"},
    {"B080.map", "B080.pvm"},
    {"B081.map", "B081.pvm"},
    {"B082.map", "B082.pvm"},
    {"B083.map", "B083.pvm"},
    {"B084.map", "B084.pvm"},
    {"B085.map", "B085.pvm"},
    {"B086.map", "B086.pvm"},
    {"B087.map", "B087.pvm"},
    {"B088.map", "B088.pvm"},
    {"B089.map", "B089.pvm"},
    {"B090.map", "B090.pvm"},
    {"B091.map", "B091.pvm"},
    {"B092.map", "B092.pvm"},
    {"B093.map", "B093.pvm"},
    {"B094.map", "B094.pvm"},
    {"B095.map", "B095.pvm"},
    {"B096.map", "B096.pvm"},
    {"B097.map", "B097.pvm"},
    {"B098.map", "B098.pvm"},
    {"B099.map", "B099.pvm"},
    {"B100.map", "B100.pvm"},
    {"B101.map", "B101.pvm"},
    {"B102.map", "B102.pvm"},
    {"B103.map", "B103.pvm"},
    {"B104.map", "B104.pvm"},
    {"B105.map", "B105.pvm"},
    {"B106.map", "B106.pvm"},
    {"B107.map", "B107.pvm"},
    {"B108.map", "B108.pvm"},
    {"B109.map", "B109.pvm"},
    {"B110.map", "B110.pvm"},
    {"B111.map", "B111.pvm"},
    {"B112.map", "B112.pvm"},
    {"B113.map", "B113.pvm"},
    {"B114.map", "B114.pvm"},
    {"B115.map", "B115.pvm"},
    {"B116.map", "B116.pvm"},
    {"B117.map", "B117.pvm"},
    {"B118.map", "B118.pvm"},
    {"B119.map", "B119.pvm"},
    {"B120.map", "B120.pvm"},
    {"B121.map", "B121.pvm"},
    {"B122.map", "B122.pvm"},
    {"B123.map", "B123.pvm"},
    {"B124.map", "B124.pvm"},
    {"B125.map", "B125.pvm"},
    {"B126.map", "B126.pvm"},
    {"B127.map", "B127.pvm"},
    {"B128.map", "B128.pvm"},
    {"B129.map", "B129.pvm"},
    {"B130.map", "B130.pvm"},
    {"B131.map", "B131.pvm"},
    {"B132.map", "B132.pvm"},
    {"B133.map", "B133.pvm"},
    {"B134.map", "B134.pvm"},
    {"B135.map", "B135.pvm"},
    {"B136.map", "B136.pvm"},
    {"B137.map", "B137.pvm"},
    {"B138.map", "B138.pvm"},
    {"B139.map", "B139.pvm"},
    {"B140.map", "B140.pvm"},
    {"B141.map", "B141.pvm"},
    {"B142.map", "B142.pvm"},
    {"B143.map", "B143.pvm"},
    {"B144.map", "B144.pvm"},
    {"B145.map", "B145.pvm"},
    {"B146.map", "B146.pvm"},
    {"B147.map", "B147.pvm"},
    {"B148.map", "B148.pvm"},
    {"B149.map", "B149.pvm"},
    {"B150.map", "B150.pvm"},
    {"B151.map", "B151.pvm"},
    {"B152.map", "B152.pvm"},
    {"B153.map", "B153.pvm"},
    {"B154.map", "B154.pvm"},
    {"B155.map", "B155.pvm"},
    {"B156.map", "B156.pvm"},
    {"B157.map", "B157.pvm"},
    {"B158.map", "B158.pvm"},
    {"B159.map", "B159.pvm"},
    {"B160.map", "B160.pvm"},
    {"B161.map", "B161.pvm"},
    {"B162.map", "B162.pvm"},
    {"B163.map", "B163.pvm"},
    {"B164.map", "B164.pvm"},
    {"B165.map", "B165.pvm"},
    {"B166.map", "B166.pvm"},
    {"B167.map", "B167.pvm"},
    {"B168.map", "B168.pvm"},
    {"B169.map", "B169.pvm"},
    {"B170.map", "B170.pvm"},
    {"B171.map", "B171.pvm"},
    {"B172.map", "B172.pvm"},
    {"B173.map", "B173.pvm"},
    {"B174.map", "B174.pvm"},
    {"B175.map", "B175.pvm"},
    {"B176.map", "B176.pvm"},
    {"B177.map", "B177.pvm"},
    {"B178.map", "B178.pvm"},
    {"B179.map", "B179.pvm"},
    {"B180.map", "B180.pvm"},
    {"B181.map", "B181.pvm"},
    {"B182.map", "B182.pvm"},
    {"B183.map", "B183.pvm"},
    {"B184.map", "B184.pvm"},
    {"B185.map", "B185.pvm"},
    {"B186.map", "B186.pvm"},
    {"B187.map", "B187.pvm"},
    {"B188.map", "B188.pvm"},
    {"B189.map", "B189.pvm"},
    {"B190.map", "B190.pvm"},
    {"B191.map", "B191.pvm"},
    {"B192.map", "B192.pvm"},
    {"B193.map", "B193.pvm"},
    {"B194.map", "B194.pvm"},
    {"B195.map", "B195.pvm"},
    {"B196.map", "B196.pvm"},
    {"B197.map", "B197.pvm"},
    {"B198.map", "B198.pvm"},
    {"B199.map", "B199.pvm"},
    {"B200.map", "B200.pvm"},
    {"P001.map", "P001.pvm"},
    {"P002.map", "P002.pvm"},
    {"P003.map", "P003.pvm"},
    {"P004.map", "P004.pvm"},
    {"P005.map", "P005.pvm"},
    {"P006.map", "P006.pvm"},
    {"P007.map", "P007.pvm"},
    {"P008.map", "P008.pvm"},
    {"P009.map", "P009.pvm"},
    {"P010.map", "P010.pvm"},
    {"P011.map", "P011.pvm"},
    {"P012.map", "P012.pvm"},
    {"P013.map", "P013.pvm"},
    {"P014.map", "P014.pvm"},
    {"P015.map", "P015.pvm"},
    {"P016.map", "P016.pvm"},
    {"P017.map", "P017.pvm"},
    {"P018.map", "P018.pvm"},
    {"P019.map", "P019.pvm"},
    {"P020.map", "P020.pvm"},
    {"P021.map", "P021.pvm"},
    {"P022.map", "P022.pvm"},
    {"P023.map", "P023.pvm"},
    {"P024.map", "P024.pvm"},
    {"P025.map", "P025.pvm"},
    {"P026.map", "P026.pvm"},
    {"P027.map", "P027.pvm"},
    {"P028.map", "P028.pvm"},
    {"P029.map", "P029.pvm"},
    {"P030.map", "P030.pvm"},
    {"P031.map", "P031.pvm"},
    {"P032.map", "P032.pvm"},
    {"P033.map", "P033.pvm"},
    {"P034.map", "P034.pvm"},
    {"P035.map", "P035.pvm"},
    {"P036.map", "P036.pvm"},
    {"P037.map", "P037.pvm"},
    {"P038.map", "P038.pvm"},
    {"P039.map", "P039.pvm"},
    {"P040.map", "P040.pvm"},
    {"P041.map", "P041.pvm"},
    {"P042.map", "P042.pvm"},
    {"P043.map", "P043.pvm"},
    {"P044.map", "P044.pvm"},
    {"P045.map", "P045.pvm"},
    {"P046.map", "P046.pvm"},
    {"P047.map", "P047.pvm"},
    {"P048.map", "P048.pvm"},
    {"P049.map", "P049.pvm"},
    {"P050.map", "P050.pvm"},
    {"P051.map", "P051.pvm"},
    {"P052.map", "P052.pvm"},
    {"P053.map", "P053.pvm"},
    {"P054.map", "P054.pvm"},
};

/* Unreferenced; same {id..., 0xFFFF} shape as the id lists below. */
STATIC Uint16 init_slideLayers_8c046f50[] = {
    0x0000, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f54[] = {
    0x0001, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f58[] = {
    0x0001, 0x00C9, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f5e[] = {
    0x0002, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f62[] = {
    0x0003, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f66[] = {
    0x0004, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f6a[] = {
    0x0005, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f6e[] = {
    0x0005, 0x00CA, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f74[] = {
    0x0006, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f78[] = {
    0x0007, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f7c[] = {
    0x0007, 0x00CB, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f82[] = {
    0x0007, 0x00CB, 0x00CD, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f8a[] = {
    0x0007, 0x00CC, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f90[] = {
    0x0007, 0x00CC, 0x00CD, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f98[] = {
    0x0007, 0x00CD, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046f9e[] = {
    0x0008, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fa2[] = {
    0x0008, 0x00CE, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fa8[] = {
    0x0009, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fac[] = {
    0x000A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fb0[] = {
    0x000A, 0x00CF, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fb6[] = {
    0x000B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fba[] = {
    0x000C, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fbe[] = {
    0x000D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fc2[] = {
    0x000D, 0x00D0, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fc8[] = {
    0x000E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fcc[] = {
    0x000F, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fd0[] = {
    0x0010, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fd4[] = {
    0x0011, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fd8[] = {
    0x0012, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fdc[] = {
    0x0013, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fe0[] = {
    0x0013, 0x00D1, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fe6[] = {
    0x0013, 0x00D1, 0x00D3, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046fee[] = {
    0x0013, 0x00D2, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046ff4[] = {
    0x0013, 0x00D2, 0x00D3, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c046ffc[] = {
    0x0013, 0x00D3, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047002[] = {
    0x0014, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047006[] = {
    0x0015, 0xFFFF, 0x0015, 0x00D4, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047010[] = {
    0x0016, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047014[] = {
    0x0016, 0x00D5, 0xFFFF, 0x0016, 0x00D5, 0x00D7, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047022[] = {
    0x0016, 0x00D6, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047028[] = {
    0x0016, 0x00D6, 0x00D7, 0xFFFF, 0x0016, 0x00D7, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047036[] = {
    0x0017, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04703a[] = {
    0x0018, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04703e[] = {
    0x0018, 0x00D8, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047044[] = {
    0x0019, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047048[] = {
    0x001A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04704c[] = {
    0x001B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047050[] = {
    0x001C, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047054[] = {
    0x001D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047058[] = {
    0x001E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04705c[] = {
    0x001E, 0x00D9, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047062[] = {
    0x001F, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047066[] = {
    0x0020, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04706a[] = {
    0x0020, 0x00DA, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047070[] = {
    0x0021, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047074[] = {
    0x0022, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047078[] = {
    0x0022, 0x00DB, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04707e[] = {
    0x0022, 0x00DB, 0x00DD, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047086[] = {
    0x0022, 0x00DC, 0xFFFF, 0x0154, 0x00DC, 0x00DD, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047094[] = {
    0x0022, 0x00DD, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04709a[] = {
    0x0023, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04709e[] = {
    0x0024, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470a2[] = {
    0x0024, 0x00DE, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470a8[] = {
    0x0025, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470ac[] = {
    0x0026, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470b0[] = {
    0x0026, 0x00DF, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470b6[] = {
    0x0027, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470ba[] = {
    0x0028, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470be[] = {
    0x0029, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470c2[] = {
    0x002A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470c6[] = {
    0x002B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470ca[] = {
    0x002B, 0x00E0, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470d0[] = {
    0x002C, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470d4[] = {
    0x002D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470d8[] = {
    0x002E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470dc[] = {
    0x002F, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470e0[] = {
    0x0030, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470e4[] = {
    0x0031, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470e8[] = {
    0x0031, 0x00E1, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470ee[] = {
    0x0032, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470f2[] = {
    0x0033, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470f6[] = {
    0x0034, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0470fa[] = {
    0x0034, 0x00E2, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047100[] = {
    0x0035, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047104[] = {
    0x0036, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047108[] = {
    0x0037, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04710c[] = {
    0x0038, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047110[] = {
    0x0039, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047114[] = {
    0x003A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047118[] = {
    0x003B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04711c[] = {
    0x003C, 0xFFFF, 0x003C, 0x00E3, 0xFFFF, 0x003C, 0x00E3, 0x00E5,
    0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04712e[] = {
    0x003C, 0x00E4, 0xFFFF, 0x003C, 0x00E4, 0x00E5, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04713c[] = {
    0x003C, 0x00E5, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047142[] = {
    0x003D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047146[] = {
    0x003E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04714a[] = {
    0x003E, 0x00E6, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047150[] = {
    0x003F, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047154[] = {
    0x003F, 0x00E7, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04715a[] = {
    0x0040, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04715e[] = {
    0x0040, 0x00E8, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047164[] = {
    0x0041, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047168[] = {
    0x0042, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04716c[] = {
    0x0042, 0x00E9, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047172[] = {
    0x0042, 0x00E9, 0x00EA, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04717a[] = {
    0x0042, 0x00EA, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047180[] = {
    0x0043, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047184[] = {
    0x0044, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047188[] = {
    0x0045, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04718c[] = {
    0x0046, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047190[] = {
    0x0047, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047194[] = {
    0x0048, 0xFFFF, 0x0049, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04719c[] = {
    0x004A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471a0[] = {
    0x004B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471a4[] = {
    0x004C, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471a8[] = {
    0x004D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471ac[] = {
    0x004E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471b0[] = {
    0x004F, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471b4[] = {
    0x0050, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471b8[] = {
    0x0050, 0x00EB, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471be[] = {
    0x0051, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471c2[] = {
    0x0052, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471c6[] = {
    0x0052, 0x00EC, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471cc[] = {
    0x0053, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471d0[] = {
    0x0054, 0xFFFF, 0x0054, 0x00ED, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471da[] = {
    0x0055, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471de[] = {
    0x0056, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471e2[] = {
    0x0057, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471e6[] = {
    0x0058, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471ea[] = {
    0x0059, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471ee[] = {
    0x005A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471f2[] = {
    0x005B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471f6[] = {
    0x005C, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471fa[] = {
    0x005D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0471fe[] = {
    0x005E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047202[] = {
    0x005F, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047206[] = {
    0x0060, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04720a[] = {
    0x0061, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04720e[] = {
    0x0062, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047212[] = {
    0x0063, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047216[] = {
    0x0064, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04721a[] = {
    0x0065, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04721e[] = {
    0x0066, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047222[] = {
    0x0066, 0x00EE, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047228[] = {
    0x0067, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04722c[] = {
    0x0068, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047230[] = {
    0x0069, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047234[] = {
    0x0069, 0x00EF, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04723a[] = {
    0x006A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04723e[] = {
    0x006B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047242[] = {
    0x006B, 0x00F0, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047248[] = {
    0x006B, 0x00F0, 0x00F1, 0xFFFF, 0x006B, 0x00F1, 0xFFFF, 0x006C,
    0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04725a[] = {
    0x006D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04725e[] = {
    0x006C, 0x00F2, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047264[] = {
    0x006E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047268[] = {
    0x006F, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04726c[] = {
    0x0070, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047270[] = {
    0x0070, 0x00F3, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047276[] = {
    0x0071, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04727a[] = {
    0x0072, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04727e[] = {
    0x0073, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047282[] = {
    0x0074, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047286[] = {
    0x0074, 0x00F4, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04728c[] = {
    0x0075, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047290[] = {
    0x0076, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047294[] = {
    0x0077, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047298[] = {
    0x0077, 0x00F5, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04729e[] = {
    0x0078, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472a2[] = {
    0x0079, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472a6[] = {
    0x0079, 0x00F6, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472ac[] = {
    0x007A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472b0[] = {
    0x007B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472b4[] = {
    0x007C, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472b8[] = {
    0x007D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472bc[] = {
    0x007D, 0x00F7, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472c2[] = {
    0x007E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472c6[] = {
    0x007F, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472ca[] = {
    0x007F, 0x00F8, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472d0[] = {
    0x0080, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472d4[] = {
    0x0081, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472d8[] = {
    0x0082, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472dc[] = {
    0x0083, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472e0[] = {
    0x0084, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472e4[] = {
    0x0085, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472e8[] = {
    0x0085, 0x00F9, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472ee[] = {
    0x0086, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472f2[] = {
    0x0086, 0x00FA, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472f8[] = {
    0x0087, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c0472fc[] = {
    0x0088, 0xFFFF, 0x0015, 0x00FB, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047306[] = {
    0x0016, 0x00FC, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04730c[] = {
    0x0016, 0x00D5, 0x00FC, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047314[] = {
    0x0016, 0x00D6, 0x00FC, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04731c[] = {
    0x0017, 0x00FD, 0xFFFF, 0x0066, 0x00FE, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047328[] = {
    0x0089, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04732c[] = {
    0x008A, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047330[] = {
    0x008B, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047334[] = {
    0x008C, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047338[] = {
    0x008D, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04733c[] = {
    0x0008, 0x00CD, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c047342[] = {
    0x0008, 0x00CD, 0x00CE, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04734a[] = {
    0x008E, 0xFFFF,
};

STATIC Uint16 init_slideLayers_8c04734e[] = {
    0x008F, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
    0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000,
};

STATIC EventSlide init_eventSlides_8c04740c[] = {
    { init_slideLayers_8c04715a, 0x00 },
    { init_slideLayers_8c047150, 0x01 },
    { init_slideLayers_8c047164, 0x02 },
    { init_slideLayers_8c04715a, 0x03 },
    { init_slideLayers_8c047150, 0x04 },
    { init_slideLayers_8c04715a, 0x05 },
    { init_slideLayers_8c047150, 0x06 },
    { init_slideLayers_8c04715a, 0x07 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047454[] = {
    { init_slideLayers_8c047150, 0x08 },
    { init_slideLayers_8c047164, 0x09 },
    { init_slideLayers_8c047150, 0x0a },
    { init_slideLayers_8c04715a, 0x0b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04747c[] = {
    { init_slideLayers_8c04715a, 0x0c },
    { init_slideLayers_8c047150, 0x0d },
    { init_slideLayers_8c047164, 0x0e },
    { init_slideLayers_8c04715a, 0x0f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0474a4[] = {
    { init_slideLayers_8c047150, 0x10 },
    { init_slideLayers_8c04715a, 0x11 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0474bc[] = {
    { init_slideLayers_8c047150, 0x12 },
    { init_slideLayers_8c04715a, 0x13 },
    { init_slideLayers_8c047164, 0x14 },
    { init_slideLayers_8c047150, 0x15 },
    { init_slideLayers_8c04715a, 0x16 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0474ec[] = {
    { init_slideLayers_8c047150, 0x17 },
    { init_slideLayers_8c047164, 0x18 },
    { init_slideLayers_8c047150, 0x19 },
    { init_slideLayers_8c04715a, 0x1a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047514[] = {
    { init_slideLayers_8c047154, 0x1b },
    { init_slideLayers_8c047164, 0x1c },
    { init_slideLayers_8c04715e, 0x1d },
    { init_slideLayers_8c047154, 0x1e },
    { init_slideLayers_8c04715e, 0x1f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047544[] = {
    { init_slideLayers_8c04715e, 0x20 },
    { init_slideLayers_8c047154, 0x21 },
    { init_slideLayers_8c04715e, 0x22 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047564[] = {
    { init_slideLayers_8c047154, 0x23 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047574[] = {
    { init_slideLayers_8c047172, 0x26 },
    { init_slideLayers_8c04717a, 0x27 },
    { init_slideLayers_8c047168, 0x28 },
    { init_slideLayers_8c047188, 0x29 },
    { init_slideLayers_8c047168, 0x2a },
    { init_slideLayers_8c04718c, 0x2b },
    { init_slideLayers_8c047188, 0x2c },
    { init_slideLayers_8c04718c, 0x2d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0475bc[] = {
    { init_slideLayers_8c047168, 0x2e },
    { init_slideLayers_8c047188, 0x2f },
    { init_slideLayers_8c04718c, 0x30 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0475dc[] = {
    { init_slideLayers_8c047168, 0x31 },
    { init_slideLayers_8c047188, 0x32 },
    { init_slideLayers_8c047168, 0x33 },
    { init_slideLayers_8c04716c, 0x34 },
    { init_slideLayers_8c04718c, 0x35 },
    { init_slideLayers_8c047168, 0x36 },
    { init_slideLayers_8c04718c, 0x37 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04761c[] = {
    { init_slideLayers_8c047168, 0x38 },
    { init_slideLayers_8c047188, 0x39 },
    { init_slideLayers_8c04718c, 0x3a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04763c[] = {
    { init_slideLayers_8c04717a, 0x3b },
    { init_slideLayers_8c047188, 0x3c },
    { init_slideLayers_8c047168, 0x3d },
    { init_slideLayers_8c04718c, 0x3e },
    { init_slideLayers_8c047168, 0x3f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04766c[] = {
    { init_slideLayers_8c047168, 0x40 },
    { init_slideLayers_8c047188, 0x41 },
    { init_slideLayers_8c04718c, 0x42 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04768c[] = {
    { init_slideLayers_8c047184, 0x43 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04769c[] = {
    { init_slideLayers_8c047184, 0x44 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0476ac[] = {
    { init_slideLayers_8c047180, 0x45 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0476bc[] = {
    { init_slideLayers_8c047180, 0x46 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0476cc[] = {
    { init_slideLayers_8c04715e, 0x47 },
    { init_slideLayers_8c047154, 0x48 },
    { init_slideLayers_8c047164, 0x49 },
    { init_slideLayers_8c047154, 0x4a },
    { init_slideLayers_8c047190, 0x4b },
    { init_slideLayers_8c047190, 0x4c },
    { init_slideLayers_8c047194, 0x4d },
    { init_slideLayers_8c047190, 0x4e },
    { init_slideLayers_8c047190, 0x4f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04771c[] = {
    { init_slideLayers_8c047190, 0x50 },
    { init_slideLayers_8c047194, 0x51 },
    { init_slideLayers_8c047190, 0x52 },
    { init_slideLayers_8c047190, 0x53 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047744[] = {
    { init_slideLayers_8c047190, 0x54 },
    { init_slideLayers_8c047194, 0x55 },
    { init_slideLayers_8c047190, 0x56 },
    { init_slideLayers_8c047190, 0x57 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04776c[] = {
    { init_slideLayers_8c04719c, 0x58 },
    { init_slideLayers_8c0471a4, 0x59 },
    { init_slideLayers_8c0471a0, 0x5a },
    { init_slideLayers_8c04719c, 0x5b },
    { init_slideLayers_8c0471a8, 0x5c },
    { init_slideLayers_8c0471a0, 0x5d },
    { init_slideLayers_8c0471a8, 0x5e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0477ac[] = {
    { init_slideLayers_8c04719c, 0x5f },
    { init_slideLayers_8c0471a4, 0x60 },
    { init_slideLayers_8c0471a8, 0x61 },
    { init_slideLayers_8c04719c, 0x62 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0477d4[] = {
    { init_slideLayers_8c04719c, 0x63 },
    { init_slideLayers_8c0471a4, 0x64 },
    { init_slideLayers_8c0471a0, 0x65 },
    { init_slideLayers_8c0471a8, 0x66 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0477fc[] = {
    { init_slideLayers_8c04719c, 0x67 },
    { init_slideLayers_8c0471a4, 0x68 },
    { init_slideLayers_8c0471a0, 0x69 },
    { init_slideLayers_8c0471a8, 0x6a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047824[] = {
    { init_slideLayers_8c0471a0, 0x6b },
    { init_slideLayers_8c04719c, 0x6c },
    { init_slideLayers_8c0471a4, 0x6d },
    { init_slideLayers_8c04719c, 0x6e },
    { init_slideLayers_8c0471a0, 0x6f },
    { init_slideLayers_8c0471a8, 0x70 },
    { init_slideLayers_8c04719c, 0x71 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047864[] = {
    { init_slideLayers_8c0471a0, 0x72 },
    { init_slideLayers_8c0471a4, 0x73 },
    { init_slideLayers_8c04719c, 0x74 },
    { init_slideLayers_8c0471a8, 0x75 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04788c[] = {
    { init_slideLayers_8c0471a0, 0x76 },
    { init_slideLayers_8c0471a4, 0x77 },
    { init_slideLayers_8c04719c, 0x78 },
    { init_slideLayers_8c0471a8, 0x79 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0478b4[] = {
    { init_slideLayers_8c0471a0, 0x7a },
    { init_slideLayers_8c0471a4, 0x7b },
    { init_slideLayers_8c04719c, 0x7c },
    { init_slideLayers_8c0471a8, 0x7d },
    { init_slideLayers_8c04719c, 0x7e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0478e4[] = {
    { init_slideLayers_8c0471ac, 0x7f },
    { init_slideLayers_8c04719c, 0x80 },
    { init_slideLayers_8c0471a0, 0x81 },
    { init_slideLayers_8c0471a4, 0x82 },
    { init_slideLayers_8c04719c, 0x83 },
    { init_slideLayers_8c0471a8, 0x84 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04791c[] = {
    { init_slideLayers_8c0471a0, 0x85 },
    { init_slideLayers_8c0471a4, 0x86 },
    { init_slideLayers_8c04719c, 0x87 },
    { init_slideLayers_8c0471a8, 0x88 },
    { init_slideLayers_8c04719c, 0x89 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04794c[] = {
    { init_slideLayers_8c04719c, 0x8a },
    { init_slideLayers_8c0471a4, 0x8b },
    { init_slideLayers_8c0471a8, 0x8c },
    { init_slideLayers_8c0471a0, 0x8d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047974[] = {
    { init_slideLayers_8c046fd4, 0x8e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047984[] = {
    { init_slideLayers_8c046fd4, 0x8f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047994[] = {
    { init_slideLayers_8c046fe6, 0x90 },
    { init_slideLayers_8c046ffc, 0x91 },
    { init_slideLayers_8c047006, 0x92 },
    { init_slideLayers_8c046fdc, 0x93 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0479bc[] = {
    { init_slideLayers_8c046fdc, 0x94 },
    { init_slideLayers_8c046fee, 0x95 },
    { init_slideLayers_8c047006, 0x96 },
    { init_slideLayers_8c046fe0, 0x97 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0479e4[] = {
    { init_slideLayers_8c046fe0, 0x98 },
    { init_slideLayers_8c047002, 0x99 },
    { init_slideLayers_8c047006, 0x9a },
    { init_slideLayers_8c046fee, 0x9b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047a0c[] = {
    { init_slideLayers_8c046fdc, 0x9c },
    { init_slideLayers_8c046fee, 0x9d },
    { init_slideLayers_8c047006, 0x9e },
    { init_slideLayers_8c047002, 0x9f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047a34[] = {
    { init_slideLayers_8c046fdc, 0xa0 },
    { init_slideLayers_8c047002, 0xa1 },
    { init_slideLayers_8c047006, 0xa2 },
    { init_slideLayers_8c046fdc, 0xa3 },
    { init_slideLayers_8c047002, 0xa4 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047a64[] = {
    { init_slideLayers_8c046fdc, 0xa5 },
    { init_slideLayers_8c047006, 0xa6 },
    { init_slideLayers_8c046fdc, 0xa7 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047a84[] = {
    { init_slideLayers_8c046fe6, 0xa8 },
    { init_slideLayers_8c047002, 0xa9 },
    { init_slideLayers_8c047006, 0xaa },
    { init_slideLayers_8c046fe0, 0xab },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047aac[] = {
    { init_slideLayers_8c046ffc, 0xac },
    { init_slideLayers_8c047006, 0xad },
    { init_slideLayers_8c047002, 0xae },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047acc[] = {
    { init_slideLayers_8c046fe0, 0xaf },
    { init_slideLayers_8c047002, 0xb0 },
    { init_slideLayers_8c047006, 0xb1 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047aec[] = {
    { init_slideLayers_8c046fee, 0xb2 },
    { init_slideLayers_8c047002, 0xb3 },
    { init_slideLayers_8c047006, 0xb4 },
    { init_slideLayers_8c046fdc, 0xb5 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047b14[] = {
    { init_slideLayers_8c046fe6, 0xb6 },
    { init_slideLayers_8c046fd4, 0xb7 },
    { init_slideLayers_8c046fe0, 0xb8 },
    { init_slideLayers_8c047002, 0xb9 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047b3c[] = {
    { init_slideLayers_8c046fe6, 0xba },
    { init_slideLayers_8c046fd4, 0xbb },
    { init_slideLayers_8c047002, 0xbc },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047b5c[] = {
    { init_slideLayers_8c047328, 0xbd },
    { init_slideLayers_8c047334, 0xbe },
    { init_slideLayers_8c047330, 0xbf },
    { init_slideLayers_8c047328, 0xc0 },
    { init_slideLayers_8c04732c, 0xc1 },
    { init_slideLayers_8c047328, 0xc2 },
    { init_slideLayers_8c047334, 0xc3 },
    { init_slideLayers_8c047330, 0xc4 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047ba4[] = {
    { init_slideLayers_8c047330, 0xc5 },
    { init_slideLayers_8c04732c, 0xc6 },
    { init_slideLayers_8c047334, 0xc7 },
    { init_slideLayers_8c047328, 0xc8 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047bcc[] = {
    { init_slideLayers_8c047328, 0xc9 },
    { init_slideLayers_8c047330, 0xca },
    { init_slideLayers_8c047328, 0xcb },
    { init_slideLayers_8c047330, 0xcc },
    { init_slideLayers_8c047328, 0xcd },
    { init_slideLayers_8c047334, 0xce },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047c04[] = {
    { init_slideLayers_8c047334, 0xcf },
    { init_slideLayers_8c047328, 0xd0 },
    { init_slideLayers_8c047334, 0xd1 },
    { init_slideLayers_8c047328, 0xd2 },
    { init_slideLayers_8c04732c, 0xd3 },
    { init_slideLayers_8c047328, 0xd4 },
    { init_slideLayers_8c04732c, 0xd5 },
    { init_slideLayers_8c047328, 0xd6 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047c4c[] = {
    { init_slideLayers_8c047330, 0xd7 },
    { init_slideLayers_8c047334, 0xd8 },
    { init_slideLayers_8c047328, 0xd9 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047c6c[] = {
    { init_slideLayers_8c047338, 0xda },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047c7c[] = {
    { init_slideLayers_8c047338, 0xdb },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047c8c[] = {
    { init_slideLayers_8c047338, 0xdc },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047c9c[] = {
    { init_slideLayers_8c047338, 0xdd },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047cac[] = {
    { init_slideLayers_8c047338, 0xde },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047cbc[] = {
    { init_slideLayers_8c047338, 0xdf },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047ccc[] = {
    { init_slideLayers_8c047338, 0xe0 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047cdc[] = {
    { init_slideLayers_8c047338, 0xe1 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047cec[] = {
    { init_slideLayers_8c047078, 0xe2 },
    { init_slideLayers_8c04709a, 0xe3 },
    { init_slideLayers_8c04709e, 0xe4 },
    { init_slideLayers_8c047074, 0xe5 },
    { init_slideLayers_8c047086, 0xe6 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047d1c[] = {
    { init_slideLayers_8c047074, 0xe7 },
    { init_slideLayers_8c04709e, 0xe8 },
    { init_slideLayers_8c04709a, 0xe9 },
    { init_slideLayers_8c04709e, 0xea },
    { init_slideLayers_8c047086, 0xeb },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047d4c[] = {
    { init_slideLayers_8c047078, 0xec },
    { init_slideLayers_8c04709a, 0xed },
    { init_slideLayers_8c04709e, 0xee },
    { init_slideLayers_8c047074, 0xef },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047d74[] = {
    { init_slideLayers_8c047074, 0xf0 },
    { init_slideLayers_8c04709e, 0xf1 },
    { init_slideLayers_8c04709a, 0xf2 },
    { init_slideLayers_8c0470a2, 0xf3 },
    { init_slideLayers_8c047086, 0xf4 },
    { init_slideLayers_8c047094, 0xf5 },
    { init_slideLayers_8c047074, 0xf6 },
    { init_slideLayers_8c04709a, 0xf7 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047dbc[] = {
    { init_slideLayers_8c047074, 0xf8 },
    { init_slideLayers_8c04709a, 0xf9 },
    { init_slideLayers_8c04709e, 0xfa },
    { init_slideLayers_8c047086, 0xfb },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047de4[] = {
    { init_slideLayers_8c0471ee, 0xfc },
    { init_slideLayers_8c0471f2, 0xfd },
    { init_slideLayers_8c0471b0, 0xfe },
    { init_slideLayers_8c0471b4, 0xff },
    { init_slideLayers_8c0471b8, 0x100 },
    { init_slideLayers_8c0471b4, 0x101 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047e1c[] = {
    { init_slideLayers_8c0471f6, 0x102 },
    { init_slideLayers_8c0471ee, 0x103 },
    { init_slideLayers_8c0471f6, 0x104 },
    { init_slideLayers_8c0471f2, 0x105 },
    { init_slideLayers_8c0471b0, 0x106 },
    { init_slideLayers_8c0471b4, 0x107 },
    { init_slideLayers_8c0471b0, 0x108 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047e5c[] = {
    { init_slideLayers_8c0471be, 0x109 },
    { init_slideLayers_8c0471c2, 0x10a },
    { init_slideLayers_8c0471c6, 0x10b },
    { init_slideLayers_8c0471be, 0x10c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047e84[] = {
    { init_slideLayers_8c0471be, 0x10d },
    { init_slideLayers_8c0471c2, 0x10e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047e9c[] = {
    { init_slideLayers_8c0471cc, 0x10f },
    { init_slideLayers_8c0471d0, 0x110 },
    { init_slideLayers_8c0471cc, 0x111 },
    { init_slideLayers_8c0471d0, 0x112 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047ec4[] = {
    { init_slideLayers_8c0471cc, 0x113 },
    { init_slideLayers_8c0471d0, 0x114 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047edc[] = {
    { init_slideLayers_8c0471ee, 0x115 },
    { init_slideLayers_8c0471f6, 0x116 },
    { init_slideLayers_8c0471b0, 0x117 },
    { init_slideLayers_8c0471b4, 0x118 },
    { init_slideLayers_8c0471b8, 0x119 },
    { init_slideLayers_8c0471b0, 0x11a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047f14[] = {
    { init_slideLayers_8c0471ee, 0x11b },
    { init_slideLayers_8c0471f2, 0x11c },
    { init_slideLayers_8c0471f6, 0x11d },
    { init_slideLayers_8c0471ee, 0x11e },
    { init_slideLayers_8c0471b0, 0x11f },
    { init_slideLayers_8c0471b4, 0x120 },
    { init_slideLayers_8c0471b8, 0x121 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047f54[] = {
    { init_slideLayers_8c0471be, 0x122 },
    { init_slideLayers_8c0471c6, 0x123 },
    { init_slideLayers_8c0471c2, 0x124 },
    { init_slideLayers_8c0471be, 0x125 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047f7c[] = {
    { init_slideLayers_8c0471be, 0x126 },
    { init_slideLayers_8c0471c2, 0x127 },
    { init_slideLayers_8c0471be, 0x128 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047f9c[] = {
    { init_slideLayers_8c0471da, 0x129 },
    { init_slideLayers_8c0471de, 0x12a },
    { init_slideLayers_8c0471e6, 0x12b },
    { init_slideLayers_8c0471e2, 0x12c },
    { init_slideLayers_8c0471ea, 0x12d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047fcc[] = {
    { init_slideLayers_8c0471fa, 0x12e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c047fdc[] = {
    { init_slideLayers_8c0471fe, 0x12f },
    { init_slideLayers_8c047202, 0x130 },
    { init_slideLayers_8c047206, 0x131 },
    { init_slideLayers_8c0471fe, 0x132 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048004[] = {
    { init_slideLayers_8c0471fe, 0x133 },
    { init_slideLayers_8c047202, 0x134 },
    { init_slideLayers_8c0471fe, 0x135 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048024[] = {
    { init_slideLayers_8c0471fe, 0x136 },
    { init_slideLayers_8c047202, 0x137 },
    { init_slideLayers_8c047206, 0x138 },
    { init_slideLayers_8c0471fe, 0x139 },
    { init_slideLayers_8c047202, 0x13a },
    { init_slideLayers_8c047206, 0x13b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04805c[] = {
    { init_slideLayers_8c0471fe, 0x13c },
    { init_slideLayers_8c047206, 0x13d },
    { init_slideLayers_8c047202, 0x13e },
    { init_slideLayers_8c0471fe, 0x13f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048084[] = {
    { init_slideLayers_8c0471fe, 0x140 },
    { init_slideLayers_8c047202, 0x141 },
    { init_slideLayers_8c047206, 0x142 },
    { init_slideLayers_8c0471fe, 0x143 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0480ac[] = {
    { init_slideLayers_8c0471fe, 0x144 },
    { init_slideLayers_8c047206, 0x145 },
    { init_slideLayers_8c047202, 0x146 },
    { init_slideLayers_8c0471fe, 0x147 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0480d4[] = {
    { init_slideLayers_8c047216, 0x148 },
    { init_slideLayers_8c047118, 0x149 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0480ec[] = {
    { init_slideLayers_8c047118, 0x14a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0480fc[] = {
    { init_slideLayers_8c04720a, 0x14b },
    { init_slideLayers_8c047212, 0x14c },
    { init_slideLayers_8c04720a, 0x14d },
    { init_slideLayers_8c04720e, 0x14e },
    { init_slideLayers_8c04720a, 0x14f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04812c[] = {
    { init_slideLayers_8c04711c, 0x150 },
    { init_slideLayers_8c047142, 0x151 },
    { init_slideLayers_8c047146, 0x152 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04814c[] = {
    { init_slideLayers_8c047118, 0x153 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04815c[] = {
    { init_slideLayers_8c04711c, 0x154 },
    { init_slideLayers_8c047142, 0x155 },
    { init_slideLayers_8c047146, 0x156 },
    { init_slideLayers_8c04712e, 0x157 },
    { init_slideLayers_8c047146, 0x158 },
    { init_slideLayers_8c047142, 0x159 },
    { init_slideLayers_8c04711c, 0x15a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04819c[] = {
    { init_slideLayers_8c04711c, 0x15b },
    { init_slideLayers_8c047142, 0x15c },
    { init_slideLayers_8c047146, 0x15d },
    { init_slideLayers_8c04712e, 0x15e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0481c4[] = {
    { init_slideLayers_8c04711c, 0x15f },
    { init_slideLayers_8c04713c, 0x160 },
    { init_slideLayers_8c04711c, 0x161 },
    { init_slideLayers_8c047142, 0x162 },
    { init_slideLayers_8c047146, 0x163 },
    { init_slideLayers_8c04712e, 0x164 },
    { init_slideLayers_8c04714a, 0x165 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048204[] = {
    { init_slideLayers_8c047180, 0x166 },
    { init_slideLayers_8c04715e, 0x167 },
    { init_slideLayers_8c047154, 0x168 },
    { init_slideLayers_8c047164, 0x169 },
    { init_slideLayers_8c047154, 0x16a },
    { init_slideLayers_8c047190, 0x16b },
    { init_slideLayers_8c047190, 0x16c },
    { init_slideLayers_8c047194, 0x16d },
    { init_slideLayers_8c047190, 0x16e },
    { init_slideLayers_8c047190, 0x16f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04825c[] = {
    { init_slideLayers_8c047180, 0x170 },
    { init_slideLayers_8c047190, 0x171 },
    { init_slideLayers_8c047194, 0x172 },
    { init_slideLayers_8c047190, 0x173 },
    { init_slideLayers_8c047190, 0x174 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04828c[] = {
    { init_slideLayers_8c047180, 0x175 },
    { init_slideLayers_8c047190, 0x176 },
    { init_slideLayers_8c047194, 0x177 },
    { init_slideLayers_8c047190, 0x178 },
    { init_slideLayers_8c047190, 0x179 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0482bc[] = {
    { init_slideLayers_8c046fe0, 0x17a },
    { init_slideLayers_8c047002, 0x17b },
    { init_slideLayers_8c047006, 0x17c },
    { init_slideLayers_8c046fd4, 0x17d },
    { init_slideLayers_8c046fe6, 0x17e },
    { init_slideLayers_8c046fd4, 0x17f },
    { init_slideLayers_8c046fe0, 0x180 },
    { init_slideLayers_8c047002, 0x181 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048304[] = {
    { init_slideLayers_8c046fee, 0x182 },
    { init_slideLayers_8c047002, 0x183 },
    { init_slideLayers_8c047006, 0x184 },
    { init_slideLayers_8c046fdc, 0x185 },
    { init_slideLayers_8c046fd4, 0x186 },
    { init_slideLayers_8c046fe6, 0x187 },
    { init_slideLayers_8c046fd4, 0x188 },
    { init_slideLayers_8c047002, 0x189 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04834c[] = {
    { init_slideLayers_8c047216, 0x18a },
    { init_slideLayers_8c047118, 0x18b },
    { init_slideLayers_8c04720a, 0x18c },
    { init_slideLayers_8c047212, 0x18d },
    { init_slideLayers_8c04720a, 0x18e },
    { init_slideLayers_8c04720e, 0x18f },
    { init_slideLayers_8c04720a, 0x190 },
    { init_slideLayers_8c04711c, 0x191 },
    { init_slideLayers_8c047142, 0x192 },
    { init_slideLayers_8c047146, 0x193 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0483a4[] = {
    { init_slideLayers_8c047118, 0x194 },
    { init_slideLayers_8c04720a, 0x195 },
    { init_slideLayers_8c047212, 0x196 },
    { init_slideLayers_8c04720a, 0x197 },
    { init_slideLayers_8c04720e, 0x198 },
    { init_slideLayers_8c04720a, 0x199 },
    { init_slideLayers_8c04711c, 0x19a },
    { init_slideLayers_8c047142, 0x19b },
    { init_slideLayers_8c047146, 0x19c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0483f4[] = {
    { init_slideLayers_8c047118, 0x19d },
    { init_slideLayers_8c04711c, 0x19e },
    { init_slideLayers_8c047142, 0x19f },
    { init_slideLayers_8c047146, 0x1a0 },
    { init_slideLayers_8c04712e, 0x1a1 },
    { init_slideLayers_8c047146, 0x1a2 },
    { init_slideLayers_8c047142, 0x1a3 },
    { init_slideLayers_8c04711c, 0x1a4 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide *init_wanganEvents_8c04843c[] = {
    init_eventSlides_8c04740c, init_eventSlides_8c047454, init_eventSlides_8c04747c, init_eventSlides_8c0474a4,
    init_eventSlides_8c0474bc, init_eventSlides_8c0474ec, init_eventSlides_8c047514, init_eventSlides_8c047544,
    init_eventSlides_8c047564, init_eventSlides_8c047574, init_eventSlides_8c0475bc, init_eventSlides_8c0475dc,
    init_eventSlides_8c04761c, init_eventSlides_8c04763c, init_eventSlides_8c04766c, init_eventSlides_8c04768c,
    init_eventSlides_8c04769c, init_eventSlides_8c0476ac, init_eventSlides_8c0476bc, init_eventSlides_8c0476cc,
    init_eventSlides_8c04771c, init_eventSlides_8c047744, init_eventSlides_8c04776c, init_eventSlides_8c0477ac,
    init_eventSlides_8c0477d4, init_eventSlides_8c0477fc, init_eventSlides_8c047824, init_eventSlides_8c047864,
    init_eventSlides_8c04788c, init_eventSlides_8c0478b4, init_eventSlides_8c0478e4, init_eventSlides_8c04791c,
    init_eventSlides_8c04794c, init_eventSlides_8c047974, init_eventSlides_8c047984, init_eventSlides_8c047994,
    init_eventSlides_8c0479bc, init_eventSlides_8c0479e4, init_eventSlides_8c047a0c, init_eventSlides_8c047a34,
    init_eventSlides_8c047a64, init_eventSlides_8c047a84, init_eventSlides_8c047aac, init_eventSlides_8c047acc,
    init_eventSlides_8c047aec, init_eventSlides_8c047b14, init_eventSlides_8c047b3c, init_eventSlides_8c047b5c,
    init_eventSlides_8c047ba4, init_eventSlides_8c047bcc, init_eventSlides_8c047c04, init_eventSlides_8c047c4c,
    init_eventSlides_8c047c6c, init_eventSlides_8c047c7c, init_eventSlides_8c047c8c, init_eventSlides_8c047c9c,
    init_eventSlides_8c047cac, init_eventSlides_8c047cbc, init_eventSlides_8c047ccc, init_eventSlides_8c047cdc,
    init_eventSlides_8c047cec, init_eventSlides_8c047d1c, init_eventSlides_8c047d4c, init_eventSlides_8c047d74,
    init_eventSlides_8c047dbc, init_eventSlides_8c047de4, init_eventSlides_8c047e1c, init_eventSlides_8c047e5c,
    init_eventSlides_8c047e84, init_eventSlides_8c047e9c, init_eventSlides_8c047ec4, init_eventSlides_8c047edc,
    init_eventSlides_8c047f14, init_eventSlides_8c047f54, init_eventSlides_8c047f7c, init_eventSlides_8c047f9c,
    init_eventSlides_8c047fcc, init_eventSlides_8c047fdc, init_eventSlides_8c048004, init_eventSlides_8c048024,
    init_eventSlides_8c04805c, init_eventSlides_8c048084, init_eventSlides_8c0480ac, init_eventSlides_8c0480d4,
    init_eventSlides_8c0480ec, init_eventSlides_8c0480fc, init_eventSlides_8c04812c, init_eventSlides_8c04814c,
    init_eventSlides_8c04815c, init_eventSlides_8c04819c, init_eventSlides_8c0481c4, init_eventSlides_8c048204,
    init_eventSlides_8c04825c, init_eventSlides_8c04828c, init_eventSlides_8c0482bc, init_eventSlides_8c048304,
    init_eventSlides_8c04834c, init_eventSlides_8c0483a4, init_eventSlides_8c0483f4, NULL,
};

STATIC EventSlide init_eventSlides_8c0485cc[] = {
    { init_slideLayers_8c046f54, 0x00 },
    { init_slideLayers_8c046f58, 0x01 },
    { init_slideLayers_8c046f5e, 0x02 },
    { init_slideLayers_8c046f54, 0x03 },
    { init_slideLayers_8c046f58, 0x04 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0485fc[] = {
    { init_slideLayers_8c046f5e, 0x05 },
    { init_slideLayers_8c046f54, 0x06 },
    { init_slideLayers_8c046f62, 0x07 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04861c[] = {
    { init_slideLayers_8c046f54, 0x08 },
    { init_slideLayers_8c046f5e, 0x09 },
    { init_slideLayers_8c046f62, 0x0a },
    { init_slideLayers_8c046f5e, 0x0b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048644[] = {
    { init_slideLayers_8c046f54, 0x0d },
    { init_slideLayers_8c046f5e, 0x0e },
    { init_slideLayers_8c046f62, 0x0f },
    { init_slideLayers_8c046f58, 0x10 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04866c[] = {
    { init_slideLayers_8c046f54, 0x11 },
    { init_slideLayers_8c046f5e, 0x12 },
    { init_slideLayers_8c046f62, 0x13 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04868c[] = {
    { init_slideLayers_8c046f54, 0x16 },
    { init_slideLayers_8c046f5e, 0x17 },
    { init_slideLayers_8c046f62, 0x18 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0486ac[] = {
    { init_slideLayers_8c046f5e, 0x19 },
    { init_slideLayers_8c046f54, 0x1a },
    { init_slideLayers_8c046f62, 0x1b },
    { init_slideLayers_8c046f5e, 0x1c },
    { init_slideLayers_8c046f58, 0x1d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0486dc[] = {
    { init_slideLayers_8c046f54, 0x1e },
    { init_slideLayers_8c046f62, 0x1f },
    { init_slideLayers_8c046f5e, 0x20 },
    { init_slideLayers_8c046f58, 0x21 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048704[] = {
    { init_slideLayers_8c046f66, 0x22 },
    { init_slideLayers_8c046f6e, 0x23 },
    { init_slideLayers_8c046f66, 0x24 },
    { init_slideLayers_8c046f6a, 0x25 },
    { init_slideLayers_8c046f66, 0x26 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048734[] = {
    { init_slideLayers_8c046f66, 0x27 },
    { init_slideLayers_8c046f6a, 0x28 },
    { init_slideLayers_8c046f66, 0x29 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048754[] = {
    { init_slideLayers_8c046f66, 0x2a },
    { init_slideLayers_8c046f6e, 0x2b },
    { init_slideLayers_8c046f74, 0x2c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048774[] = {
    { init_slideLayers_8c046f66, 0x2d },
    { init_slideLayers_8c046f6a, 0x2e },
    { init_slideLayers_8c046f66, 0x2f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048794[] = {
    { init_slideLayers_8c046f66, 0x30 },
    { init_slideLayers_8c046f6e, 0x31 },
    { init_slideLayers_8c046f74, 0x32 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0487b4[] = {
    { init_slideLayers_8c046f66, 0x33 },
    { init_slideLayers_8c046f6e, 0x34 },
    { init_slideLayers_8c046f74, 0x35 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0487d4[] = {
    { init_slideLayers_8c046f7c, 0x36 },
    { init_slideLayers_8c046f82, 0x37 },
    { init_slideLayers_8c046f78, 0x38 },
    { init_slideLayers_8c046fb6, 0x39 },
    { init_slideLayers_8c046f7c, 0x3a },
    { init_slideLayers_8c046f98, 0x3b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04880c[] = {
    { init_slideLayers_8c046f7c, 0x3c },
    { init_slideLayers_8c046fb6, 0x3d },
    { init_slideLayers_8c046f78, 0x3e },
    { init_slideLayers_8c046f98, 0x3f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048834[] = {
    { init_slideLayers_8c046fba, 0x40 },
    { init_slideLayers_8c046f78, 0x41 },
    { init_slideLayers_8c046fa8, 0x42 },
    { init_slideLayers_8c046f82, 0x43 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04885c[] = {
    { init_slideLayers_8c046f82, 0x44 },
    { init_slideLayers_8c046fa8, 0x45 },
    { init_slideLayers_8c046fb6, 0x46 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04887c[] = {
    { init_slideLayers_8c046f9e, 0x47 },
    { init_slideLayers_8c046fac, 0x48 },
    { init_slideLayers_8c04733c, 0x49 },
    { init_slideLayers_8c046fb0, 0x4a },
    { init_slideLayers_8c04733c, 0x4b },
    { init_slideLayers_8c046fb6, 0x4c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0488b4[] = {
    { init_slideLayers_8c046fa2, 0x4d },
    { init_slideLayers_8c046fb0, 0x4e },
    { init_slideLayers_8c046fb6, 0x4f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0488d4[] = {
    { init_slideLayers_8c046f9e, 0x50 },
    { init_slideLayers_8c046fb6, 0x51 },
    { init_slideLayers_8c046fb0, 0x52 },
    { init_slideLayers_8c047342, 0x53 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0488fc[] = {
    { init_slideLayers_8c046f9e, 0x54 },
    { init_slideLayers_8c046fac, 0x55 },
    { init_slideLayers_8c04733c, 0x56 },
    { init_slideLayers_8c046fb0, 0x57 },
    { init_slideLayers_8c047342, 0x58 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04892c[] = {
    { init_slideLayers_8c046fb0, 0x59 },
    { init_slideLayers_8c046fb6, 0x5a },
    { init_slideLayers_8c046f9e, 0x5b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04894c[] = {
    { init_slideLayers_8c046fd0, 0x5c },
    { init_slideLayers_8c046fbe, 0x5d },
    { init_slideLayers_8c046fc8, 0x5e },
    { init_slideLayers_8c046fc2, 0x5f },
    { init_slideLayers_8c046fc8, 0x60 },
    { init_slideLayers_8c046fc2, 0x61 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048984[] = {
    { init_slideLayers_8c046fd0, 0x62 },
    { init_slideLayers_8c046fbe, 0x63 },
    { init_slideLayers_8c046fc2, 0x64 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0489a4[] = {
    { init_slideLayers_8c046fd0, 0x65 },
    { init_slideLayers_8c046fbe, 0x66 },
    { init_slideLayers_8c046fc2, 0x67 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0489c4[] = {
    { init_slideLayers_8c046fd0, 0x68 },
    { init_slideLayers_8c046fbe, 0x69 },
    { init_slideLayers_8c046fc2, 0x6a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0489e4[] = {
    { init_slideLayers_8c046fc8, 0x6b },
    { init_slideLayers_8c046fc2, 0x6c },
    { init_slideLayers_8c046f8a, 0x6d },
    { init_slideLayers_8c046f90, 0x6e },
    { init_slideLayers_8c046fa8, 0x6f },
    { init_slideLayers_8c046f98, 0x70 },
    { init_slideLayers_8c046fa8, 0x71 },
    { init_slideLayers_8c046f98, 0x72 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048a2c[] = {
    { init_slideLayers_8c046fc8, 0x73 },
    { init_slideLayers_8c046fc2, 0x74 },
    { init_slideLayers_8c046f8a, 0x75 },
    { init_slideLayers_8c046f90, 0x76 },
    { init_slideLayers_8c046f78, 0x77 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048a5c[] = {
    { init_slideLayers_8c046fcc, 0x78 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048a6c[] = {
    { init_slideLayers_8c046fcc, 0x79 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048a7c[] = {
    { init_slideLayers_8c046f9e, 0x7a },
    { init_slideLayers_8c046fa2, 0x7b },
    { init_slideLayers_8c046fb6, 0x7c },
    { init_slideLayers_8c046fb0, 0x7d },
    { init_slideLayers_8c047342, 0x7e },
    { init_slideLayers_8c046fb0, 0x7f },
    { init_slideLayers_8c046fb6, 0x80 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048abc[] = {
    { init_slideLayers_8c046fa2, 0x81 },
    { init_slideLayers_8c046fb6, 0x82 },
    { init_slideLayers_8c046fb0, 0x83 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048adc[] = {
    { init_slideLayers_8c046fd4, 0x84 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048aec[] = {
    { init_slideLayers_8c046fd4, 0x85 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048afc[] = {
    { init_slideLayers_8c046fe6, 0x86 },
    { init_slideLayers_8c046fdc, 0x87 },
    { init_slideLayers_8c047006, 0x88 },
    { init_slideLayers_8c046ff4, 0x89 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048b24[] = {
    { init_slideLayers_8c046fe6, 0x8a },
    { init_slideLayers_8c047002, 0x8b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048b3c[] = {
    { init_slideLayers_8c046fd8, 0x8c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048b4c[] = {
    { init_slideLayers_8c046fd8, 0x8d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048b5c[] = {
    { init_slideLayers_8c046fe6, 0x8e },
    { init_slideLayers_8c046fdc, 0x8f },
    { init_slideLayers_8c047006, 0x90 },
    { init_slideLayers_8c046fee, 0x91 },
    { init_slideLayers_8c047006, 0x92 },
    { init_slideLayers_8c047002, 0x93 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048b94[] = {
    { init_slideLayers_8c046fe6, 0x94 },
    { init_slideLayers_8c047002, 0x95 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048bac[] = {
    { init_slideLayers_8c046fdc, 0x96 },
    { init_slideLayers_8c046fe0, 0x97 },
    { init_slideLayers_8c046fe6, 0x98 },
    { init_slideLayers_8c047002, 0x99 },
    { init_slideLayers_8c046fdc, 0x9a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048bdc[] = {
    { init_slideLayers_8c046fdc, 0x9b },
    { init_slideLayers_8c047002, 0x9c },
    { init_slideLayers_8c047006, 0x9d },
    { init_slideLayers_8c046ffc, 0x9e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048c04[] = {
    { init_slideLayers_8c047002, 0x9f },
    { init_slideLayers_8c046fdc, 0xa0 },
    { init_slideLayers_8c046fee, 0xa1 },
    { init_slideLayers_8c047006, 0xa2 },
    { init_slideLayers_8c046fdc, 0xa3 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048c34[] = {
    { init_slideLayers_8c046fdc, 0xa4 },
    { init_slideLayers_8c046fee, 0xa5 },
    { init_slideLayers_8c047006, 0xa6 },
    { init_slideLayers_8c046fdc, 0xa7 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048c5c[] = {
    { init_slideLayers_8c046ffc, 0xa8 },
    { init_slideLayers_8c046fdc, 0xa9 },
    { init_slideLayers_8c047002, 0xaa },
    { init_slideLayers_8c046fdc, 0xab },
    { init_slideLayers_8c046fee, 0xac },
    { init_slideLayers_8c047006, 0xad },
    { init_slideLayers_8c046fdc, 0xae },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048c9c[] = {
    { init_slideLayers_8c047006, 0xaf },
    { init_slideLayers_8c047002, 0xb0 },
    { init_slideLayers_8c046fdc, 0xb1 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048cbc[] = {
    { init_slideLayers_8c046fe0, 0xb2 },
    { init_slideLayers_8c047002, 0xb3 },
    { init_slideLayers_8c047006, 0xb4 },
    { init_slideLayers_8c046fdc, 0xb5 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048ce4[] = {
    { init_slideLayers_8c046fe0, 0xb6 },
    { init_slideLayers_8c046fdc, 0xb7 },
    { init_slideLayers_8c047006, 0xb8 },
    { init_slideLayers_8c046fe0, 0xb9 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048d0c[] = {
    { init_slideLayers_8c047010, 0xba },
    { init_slideLayers_8c047306, 0xbb },
    { init_slideLayers_8c047022, 0xbc },
    { init_slideLayers_8c047306, 0xbd },
    { init_slideLayers_8c04703e, 0xbe },
    { init_slideLayers_8c04703a, 0xbf },
    { init_slideLayers_8c04730c, 0xc0 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048d4c[] = {
    { init_slideLayers_8c047022, 0xc1 },
    { init_slideLayers_8c047036, 0xc2 },
    { init_slideLayers_8c04703a, 0xc3 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048d6c[] = {
    { init_slideLayers_8c047010, 0xc4 },
    { init_slideLayers_8c047036, 0xc5 },
    { init_slideLayers_8c047014, 0xc6 },
    { init_slideLayers_8c04703a, 0xc7 },
    { init_slideLayers_8c04730c, 0xc8 },
    { init_slideLayers_8c04703e, 0xc9 },
    { init_slideLayers_8c047036, 0xca },
    { init_slideLayers_8c047306, 0xcb },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048db4[] = {
    { init_slideLayers_8c047022, 0xcc },
    { init_slideLayers_8c047306, 0xcd },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048dcc[] = {
    { init_slideLayers_8c047010, 0xce },
    { init_slideLayers_8c047306, 0xcf },
    { init_slideLayers_8c04703a, 0xd0 },
    { init_slideLayers_8c04703e, 0xd1 },
    { init_slideLayers_8c047306, 0xd2 },
    { init_slideLayers_8c04703e, 0xd3 },
    { init_slideLayers_8c04731c, 0xd4 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048e0c[] = {
    { init_slideLayers_8c047022, 0xd5 },
    { init_slideLayers_8c047314, 0xd6 },
    { init_slideLayers_8c04703e, 0xd7 },
    { init_slideLayers_8c047314, 0xd8 },
    { init_slideLayers_8c047010, 0xd9 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048e3c[] = {
    { init_slideLayers_8c047010, 0xda },
    { init_slideLayers_8c04703a, 0xdb },
    { init_slideLayers_8c047306, 0xdc },
    { init_slideLayers_8c047010, 0xdd },
    { init_slideLayers_8c04703e, 0xde },
    { init_slideLayers_8c047028, 0xdf },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048e74[] = {
    { init_slideLayers_8c047022, 0xe0 },
    { init_slideLayers_8c047036, 0xe1 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048e8c[] = {
    { init_slideLayers_8c047044, 0xe2 },
    { init_slideLayers_8c047050, 0xe3 },
    { init_slideLayers_8c047044, 0xe4 },
    { init_slideLayers_8c04704c, 0xe5 },
    { init_slideLayers_8c047048, 0xe6 },
    { init_slideLayers_8c04704c, 0xe7 },
    { init_slideLayers_8c047050, 0xe8 },
    { init_slideLayers_8c047044, 0xe9 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048ed4[] = {
    { init_slideLayers_8c047044, 0xea },
    { init_slideLayers_8c047048, 0xeb },
    { init_slideLayers_8c047050, 0xec },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048ef4[] = {
    { init_slideLayers_8c047044, 0xed },
    { init_slideLayers_8c04704c, 0xee },
    { init_slideLayers_8c047048, 0xef },
    { init_slideLayers_8c047044, 0xf0 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048f1c[] = {
    { init_slideLayers_8c047044, 0xf1 },
    { init_slideLayers_8c047048, 0xf2 },
    { init_slideLayers_8c047050, 0xf3 },
    { init_slideLayers_8c047044, 0xf4 },
    { init_slideLayers_8c04704c, 0xf5 },
    { init_slideLayers_8c047050, 0xf6 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048f54[] = {
    { init_slideLayers_8c04704c, 0xf7 },
    { init_slideLayers_8c047048, 0xf8 },
    { init_slideLayers_8c04704c, 0xf9 },
    { init_slideLayers_8c047044, 0xfa },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048f7c[] = {
    { init_slideLayers_8c047050, 0xfb },
    { init_slideLayers_8c047044, 0xfc },
    { init_slideLayers_8c04704c, 0xfd },
    { init_slideLayers_8c047044, 0xfe },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048fa4[] = {
    { init_slideLayers_8c047054, 0xff },
    { init_slideLayers_8c047062, 0x100 },
    { init_slideLayers_8c047054, 0x101 },
    { init_slideLayers_8c047062, 0x102 },
    { init_slideLayers_8c047054, 0x103 },
    { init_slideLayers_8c04705c, 0x104 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048fdc[] = {
    { init_slideLayers_8c04705c, 0x105 },
    { init_slideLayers_8c047062, 0x106 },
    { init_slideLayers_8c047054, 0x107 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c048ffc[] = {
    { init_slideLayers_8c047066, 0x108 },
    { init_slideLayers_8c047070, 0x109 },
    { init_slideLayers_8c047066, 0x10a },
    { init_slideLayers_8c047070, 0x10b },
    { init_slideLayers_8c047066, 0x10c },
    { init_slideLayers_8c04706a, 0x10d },
    { init_slideLayers_8c047074, 0x10e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04903c[] = {
    { init_slideLayers_8c047066, 0x10f },
    { init_slideLayers_8c047070, 0x110 },
    { init_slideLayers_8c047066, 0x111 },
    { init_slideLayers_8c047070, 0x112 },
    { init_slideLayers_8c047066, 0x113 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04906c[] = {
    { init_slideLayers_8c04705c, 0x114 },
    { init_slideLayers_8c047054, 0x115 },
    { init_slideLayers_8c047058, 0x116 },
    { init_slideLayers_8c047062, 0x117 },
    { init_slideLayers_8c047054, 0x118 },
    { init_slideLayers_8c047058, 0x119 },
    { init_slideLayers_8c047054, 0x11a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0490ac[] = {
    { init_slideLayers_8c047054, 0x11b },
    { init_slideLayers_8c047062, 0x11c },
    { init_slideLayers_8c04705c, 0x11d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0490cc[] = {
    { init_slideLayers_8c04707e, 0x11e },
    { init_slideLayers_8c047074, 0x11f },
    { init_slideLayers_8c047086, 0x120 },
    { init_slideLayers_8c04709e, 0x121 },
    { init_slideLayers_8c04709a, 0x122 },
    { init_slideLayers_8c047074, 0x123 },
    { init_slideLayers_8c04709a, 0x124 },
    { init_slideLayers_8c0470a2, 0x125 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049114[] = {
    { init_slideLayers_8c047074, 0x126 },
    { init_slideLayers_8c04709a, 0x127 },
    { init_slideLayers_8c047094, 0x128 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049134[] = {
    { init_slideLayers_8c04707e, 0x129 },
    { init_slideLayers_8c047086, 0x12a },
    { init_slideLayers_8c04709e, 0x12b },
    { init_slideLayers_8c047074, 0x12c },
    { init_slideLayers_8c04709a, 0x12d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049164[] = {
    { init_slideLayers_8c047074, 0x12e },
    { init_slideLayers_8c04709e, 0x12f },
    { init_slideLayers_8c047086, 0x130 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049184[] = {
    { init_slideLayers_8c04707e, 0x131 },
    { init_slideLayers_8c047078, 0x132 },
    { init_slideLayers_8c04709a, 0x133 },
    { init_slideLayers_8c04709e, 0x134 },
    { init_slideLayers_8c047078, 0x135 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0491b4[] = {
    { init_slideLayers_8c047078, 0x136 },
    { init_slideLayers_8c047074, 0x137 },
    { init_slideLayers_8c04709e, 0x138 },
    { init_slideLayers_8c04709a, 0x139 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0491dc[] = {
    { init_slideLayers_8c047078, 0x13a },
    { init_slideLayers_8c0470a2, 0x13b },
    { init_slideLayers_8c047086, 0x13c },
    { init_slideLayers_8c047074, 0x13d },
    { init_slideLayers_8c04709a, 0x13e },
    { init_slideLayers_8c04709e, 0x13f },
    { init_slideLayers_8c047086, 0x140 },
    { init_slideLayers_8c0470a2, 0x141 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049224[] = {
    { init_slideLayers_8c047074, 0x142 },
    { init_slideLayers_8c0470a2, 0x143 },
    { init_slideLayers_8c047074, 0x144 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049244[] = {
    { init_slideLayers_8c0470ba, 0x145 },
    { init_slideLayers_8c0470c2, 0x146 },
    { init_slideLayers_8c0470ba, 0x147 },
    { init_slideLayers_8c0470be, 0x148 },
    { init_slideLayers_8c0470c2, 0x149 },
    { init_slideLayers_8c0470a8, 0x14a },
    { init_slideLayers_8c0470b6, 0x14b },
    { init_slideLayers_8c0470ac, 0x14c },
    { init_slideLayers_8c0470a8, 0x14d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049294[] = {
    { init_slideLayers_8c0470c6, 0x14e },
    { init_slideLayers_8c0470d4, 0x14f },
    { init_slideLayers_8c0470d8, 0x150 },
    { init_slideLayers_8c0470c6, 0x151 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0492bc[] = {
    { init_slideLayers_8c0470c6, 0x152 },
    { init_slideLayers_8c0470d8, 0x153 },
    { init_slideLayers_8c0470c6, 0x154 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0492dc[] = {
    { init_slideLayers_8c0470d0, 0x155 },
    { init_slideLayers_8c0470c6, 0x156 },
    { init_slideLayers_8c0470b0, 0x157 },
    { init_slideLayers_8c0470ca, 0x158 },
    { init_slideLayers_8c0470a8, 0x159 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04930c[] = {
    { init_slideLayers_8c0470dc, 0x15a },
    { init_slideLayers_8c0470e0, 0x15b },
    { init_slideLayers_8c0470e4, 0x15c },
    { init_slideLayers_8c0470dc, 0x15d },
    { init_slideLayers_8c0470e8, 0x15e },
    { init_slideLayers_8c0470ee, 0x15f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049344[] = {
    { init_slideLayers_8c0470f2, 0x160 },
    { init_slideLayers_8c047100, 0x161 },
    { init_slideLayers_8c0470f6, 0x162 },
    { init_slideLayers_8c0470f2, 0x163 },
    { init_slideLayers_8c0470fa, 0x164 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049374[] = {
    { init_slideLayers_8c0470f2, 0x165 },
    { init_slideLayers_8c0470fa, 0x166 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04938c[] = {
    { init_slideLayers_8c0470f2, 0x167 },
    { init_slideLayers_8c047100, 0x168 },
    { init_slideLayers_8c0470f6, 0x169 },
    { init_slideLayers_8c0470f2, 0x16a },
    { init_slideLayers_8c0470f6, 0x16b },
    { init_slideLayers_8c047100, 0x16c },
    { init_slideLayers_8c0470f2, 0x16d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0493cc[] = {
    { init_slideLayers_8c0470f2, 0x16e },
    { init_slideLayers_8c0470fa, 0x16f },
    { init_slideLayers_8c047100, 0x170 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0493ec[] = {
    { init_slideLayers_8c047104, 0x171 },
    { init_slideLayers_8c047108, 0x172 },
    { init_slideLayers_8c047104, 0x173 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04940c[] = {
    { init_slideLayers_8c047104, 0x174 },
    { init_slideLayers_8c047108, 0x175 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049424[] = {
    { init_slideLayers_8c04711c, 0x176 },
    { init_slideLayers_8c047110, 0x177 },
    { init_slideLayers_8c04710c, 0x178 },
    { init_slideLayers_8c047110, 0x179 },
    { init_slideLayers_8c04710c, 0x17a },
    { init_slideLayers_8c047114, 0x17b },
    { init_slideLayers_8c047110, 0x17c },
    { init_slideLayers_8c04710c, 0x17d },
    { init_slideLayers_8c047110, 0x17e },
    { init_slideLayers_8c047108, 0x17f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04947c[] = {
    { init_slideLayers_8c047110, 0x180 },
    { init_slideLayers_8c047108, 0x181 },
    { init_slideLayers_8c047110, 0x182 },
    { init_slideLayers_8c04710c, 0x183 },
    { init_slideLayers_8c047110, 0x184 },
    { init_slideLayers_8c047108, 0x185 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0494b4[] = {
    { init_slideLayers_8c047118, 0x186 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0494c4[] = {
    { init_slideLayers_8c047118, 0x187 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0494d4[] = {
    { init_slideLayers_8c04711c, 0x188 },
    { init_slideLayers_8c04712e, 0x189 },
    { init_slideLayers_8c047146, 0x18a },
    { init_slideLayers_8c047142, 0x18b },
    { init_slideLayers_8c04711c, 0x18c },
    { init_slideLayers_8c047146, 0x18d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04950c[] = {
    { init_slideLayers_8c04711c, 0x18e },
    { init_slideLayers_8c047146, 0x18f },
    { init_slideLayers_8c047142, 0x190 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04952c[] = {
    { init_slideLayers_8c04711c, 0x191 },
    { init_slideLayers_8c047142, 0x192 },
    { init_slideLayers_8c047146, 0x193 },
    { init_slideLayers_8c04712e, 0x194 },
    { init_slideLayers_8c047146, 0x195 },
    { init_slideLayers_8c047142, 0x196 },
    { init_slideLayers_8c04711c, 0x197 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04956c[] = {
    { init_slideLayers_8c04711c, 0x198 },
    { init_slideLayers_8c047142, 0x199 },
    { init_slideLayers_8c047146, 0x19a },
    { init_slideLayers_8c04711c, 0x19b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049594[] = {
    { init_slideLayers_8c04711c, 0x19c },
    { init_slideLayers_8c04714a, 0x19d },
    { init_slideLayers_8c047142, 0x19e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0495b4[] = {
    { init_slideLayers_8c046fd0, 0x19f },
    { init_slideLayers_8c046fbe, 0x1a0 },
    { init_slideLayers_8c046fc2, 0x1a1 },
    { init_slideLayers_8c046fc8, 0x1a2 },
    { init_slideLayers_8c046fc2, 0x1a3 },
    { init_slideLayers_8c046f8a, 0x1a4 },
    { init_slideLayers_8c046f90, 0x1a5 },
    { init_slideLayers_8c046fa8, 0x1a6 },
    { init_slideLayers_8c046f98, 0x1a7 },
    { init_slideLayers_8c046fa8, 0x1a8 },
    { init_slideLayers_8c046f98, 0x1a9 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049614[] = {
    { init_slideLayers_8c046fd0, 0x1aa },
    { init_slideLayers_8c046fbe, 0x1ab },
    { init_slideLayers_8c046fc2, 0x1ac },
    { init_slideLayers_8c046fc8, 0x1ad },
    { init_slideLayers_8c046fc2, 0x1ae },
    { init_slideLayers_8c046f8a, 0x1af },
    { init_slideLayers_8c046f90, 0x1b0 },
    { init_slideLayers_8c046fa8, 0x1b1 },
    { init_slideLayers_8c046f98, 0x1b2 },
    { init_slideLayers_8c046fa8, 0x1b3 },
    { init_slideLayers_8c046f98, 0x1b4 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049674[] = {
    { init_slideLayers_8c046fd0, 0x1b5 },
    { init_slideLayers_8c046fbe, 0x1b6 },
    { init_slideLayers_8c046fc2, 0x1b7 },
    { init_slideLayers_8c046fc8, 0x1b8 },
    { init_slideLayers_8c046fc2, 0x1b9 },
    { init_slideLayers_8c046f8a, 0x1ba },
    { init_slideLayers_8c046f90, 0x1bb },
    { init_slideLayers_8c046f78, 0x1bc },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0496bc[] = {
    { init_slideLayers_8c046fcc, 0x1bd },
    { init_slideLayers_8c046f9e, 0x1be },
    { init_slideLayers_8c046fa2, 0x1bf },
    { init_slideLayers_8c046fb6, 0x1c0 },
    { init_slideLayers_8c046fb0, 0x1c1 },
    { init_slideLayers_8c047342, 0x1c2 },
    { init_slideLayers_8c046fb0, 0x1c3 },
    { init_slideLayers_8c046fb6, 0x1c4 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049704[] = {
    { init_slideLayers_8c046fcc, 0x1c5 },
    { init_slideLayers_8c046f9e, 0x1c6 },
    { init_slideLayers_8c046fa2, 0x1c7 },
    { init_slideLayers_8c046fb6, 0x1c8 },
    { init_slideLayers_8c046fb0, 0x1c9 },
    { init_slideLayers_8c047342, 0x1ca },
    { init_slideLayers_8c046fb0, 0x1cb },
    { init_slideLayers_8c046fb6, 0x1cc },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04974c[] = {
    { init_slideLayers_8c046fcc, 0x1cd },
    { init_slideLayers_8c046fa2, 0x1ce },
    { init_slideLayers_8c046fb6, 0x1cf },
    { init_slideLayers_8c046fb0, 0x1d0 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049774[] = {
    { init_slideLayers_8c046fd4, 0x1d1 },
    { init_slideLayers_8c046fe6, 0x1d2 },
    { init_slideLayers_8c046fdc, 0x1d3 },
    { init_slideLayers_8c047006, 0x1d4 },
    { init_slideLayers_8c046ff4, 0x1d5 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0497a4[] = {
    { init_slideLayers_8c046fd4, 0x1d6 },
    { init_slideLayers_8c046fe6, 0x1d7 },
    { init_slideLayers_8c047002, 0x1d8 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0497c4[] = {
    { init_slideLayers_8c046fd8, 0x1d9 },
    { init_slideLayers_8c046fe6, 0x1da },
    { init_slideLayers_8c046fdc, 0x1db },
    { init_slideLayers_8c047006, 0x1dc },
    { init_slideLayers_8c046fee, 0x1dd },
    { init_slideLayers_8c047006, 0x1de },
    { init_slideLayers_8c047002, 0x1df },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049804[] = {
    { init_slideLayers_8c046fd8, 0x1e0 },
    { init_slideLayers_8c046fe6, 0x1e1 },
    { init_slideLayers_8c047002, 0x1e2 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049824[] = {
    { init_slideLayers_8c047066, 0x1e3 },
    { init_slideLayers_8c047070, 0x1e4 },
    { init_slideLayers_8c047066, 0x1e5 },
    { init_slideLayers_8c047070, 0x1e6 },
    { init_slideLayers_8c047066, 0x1e7 },
    { init_slideLayers_8c04706a, 0x1e8 },
    { init_slideLayers_8c047074, 0x1e9 },
    { init_slideLayers_8c047054, 0x1ea },
    { init_slideLayers_8c047062, 0x1eb },
    { init_slideLayers_8c047054, 0x1ec },
    { init_slideLayers_8c047062, 0x1ed },
    { init_slideLayers_8c047054, 0x1ee },
    { init_slideLayers_8c04705c, 0x1ef },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049894[] = {
    { init_slideLayers_8c047066, 0x1f0 },
    { init_slideLayers_8c047070, 0x1f1 },
    { init_slideLayers_8c047066, 0x1f2 },
    { init_slideLayers_8c047070, 0x1f3 },
    { init_slideLayers_8c047066, 0x1f4 },
    { init_slideLayers_8c04705c, 0x1f5 },
    { init_slideLayers_8c047058, 0x1f6 },
    { init_slideLayers_8c047054, 0x1f7 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0498dc[] = {
    { init_slideLayers_8c0470c6, 0x1f8 },
    { init_slideLayers_8c0470d4, 0x1f9 },
    { init_slideLayers_8c0470d0, 0x1fa },
    { init_slideLayers_8c0470d8, 0x1fb },
    { init_slideLayers_8c0470c6, 0x1fc },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04990c[] = {
    { init_slideLayers_8c0470c6, 0x1fd },
    { init_slideLayers_8c0470d4, 0x1fe },
    { init_slideLayers_8c0470d8, 0x1ff },
    { init_slideLayers_8c0470c6, 0x200 },
    { init_slideLayers_8c0470d0, 0x201 },
    { init_slideLayers_8c0470c6, 0x202 },
    { init_slideLayers_8c0470b0, 0x203 },
    { init_slideLayers_8c0470ca, 0x204 },
    { init_slideLayers_8c0470a8, 0x205 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04995c[] = {
    { init_slideLayers_8c0470c6, 0x206 },
    { init_slideLayers_8c0470d4, 0x207 },
    { init_slideLayers_8c0470d0, 0x208 },
    { init_slideLayers_8c0470d8, 0x209 },
    { init_slideLayers_8c0470c6, 0x20a },
    { init_slideLayers_8c0470d0, 0x20b },
    { init_slideLayers_8c0470c6, 0x20c },
    { init_slideLayers_8c0470b0, 0x20d },
    { init_slideLayers_8c0470ca, 0x20e },
    { init_slideLayers_8c0470a8, 0x20f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c0499b4[] = {
    { init_slideLayers_8c047104, 0x210 },
    { init_slideLayers_8c047108, 0x211 },
    { init_slideLayers_8c047104, 0x212 },
    { init_slideLayers_8c04711c, 0x213 },
    { init_slideLayers_8c047110, 0x214 },
    { init_slideLayers_8c04710c, 0x215 },
    { init_slideLayers_8c047110, 0x216 },
    { init_slideLayers_8c04710c, 0x217 },
    { init_slideLayers_8c047114, 0x218 },
    { init_slideLayers_8c047110, 0x219 },
    { init_slideLayers_8c04710c, 0x21a },
    { init_slideLayers_8c047110, 0x21b },
    { init_slideLayers_8c047108, 0x21c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049a24[] = {
    { init_slideLayers_8c047104, 0x21d },
    { init_slideLayers_8c047108, 0x21e },
    { init_slideLayers_8c047110, 0x21f },
    { init_slideLayers_8c047108, 0x220 },
    { init_slideLayers_8c047110, 0x221 },
    { init_slideLayers_8c04710c, 0x222 },
    { init_slideLayers_8c047110, 0x223 },
    { init_slideLayers_8c047108, 0x224 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide *init_shinjukuEvents_8c049a6c[] = {
    init_eventSlides_8c0485cc, init_eventSlides_8c0485fc, init_eventSlides_8c04861c, init_eventSlides_8c048644,
    init_eventSlides_8c04866c, init_eventSlides_8c04868c, init_eventSlides_8c0486ac, init_eventSlides_8c0486dc,
    init_eventSlides_8c048704, init_eventSlides_8c048734, init_eventSlides_8c048754, init_eventSlides_8c048774,
    init_eventSlides_8c048794, init_eventSlides_8c0487b4, init_eventSlides_8c0487d4, init_eventSlides_8c04880c,
    init_eventSlides_8c048834, init_eventSlides_8c04885c, init_eventSlides_8c04887c, init_eventSlides_8c0488b4,
    init_eventSlides_8c0488d4, init_eventSlides_8c0488fc, init_eventSlides_8c04892c, init_eventSlides_8c04894c,
    init_eventSlides_8c048984, init_eventSlides_8c0489a4, init_eventSlides_8c0489c4, init_eventSlides_8c0489e4,
    init_eventSlides_8c048a2c, init_eventSlides_8c048a5c, init_eventSlides_8c048a6c, init_eventSlides_8c048a7c,
    init_eventSlides_8c048abc, init_eventSlides_8c048adc, init_eventSlides_8c048aec, init_eventSlides_8c048afc,
    init_eventSlides_8c048b24, init_eventSlides_8c048b3c, init_eventSlides_8c048b4c, init_eventSlides_8c048b5c,
    init_eventSlides_8c048b94, init_eventSlides_8c048bac, init_eventSlides_8c048bdc, init_eventSlides_8c048c04,
    init_eventSlides_8c048c34, init_eventSlides_8c048c5c, init_eventSlides_8c048c9c, init_eventSlides_8c048cbc,
    init_eventSlides_8c048ce4, init_eventSlides_8c048d0c, init_eventSlides_8c048d4c, init_eventSlides_8c048d6c,
    init_eventSlides_8c048db4, init_eventSlides_8c048dcc, init_eventSlides_8c048e0c, init_eventSlides_8c048e3c,
    init_eventSlides_8c048e74, init_eventSlides_8c048e8c, init_eventSlides_8c048ed4, init_eventSlides_8c048ef4,
    init_eventSlides_8c048f1c, init_eventSlides_8c048f54, init_eventSlides_8c048f7c, init_eventSlides_8c048fa4,
    init_eventSlides_8c048fdc, init_eventSlides_8c048ffc, init_eventSlides_8c04903c, init_eventSlides_8c04906c,
    init_eventSlides_8c0490ac, init_eventSlides_8c0490cc, init_eventSlides_8c049114, init_eventSlides_8c049134,
    init_eventSlides_8c049164, init_eventSlides_8c049184, init_eventSlides_8c0491b4, init_eventSlides_8c0491dc,
    init_eventSlides_8c049224, init_eventSlides_8c049244, init_eventSlides_8c049294, init_eventSlides_8c0492bc,
    init_eventSlides_8c0492dc, init_eventSlides_8c04930c, init_eventSlides_8c049344, init_eventSlides_8c049374,
    init_eventSlides_8c04938c, init_eventSlides_8c0493cc, init_eventSlides_8c0493ec, init_eventSlides_8c04940c,
    init_eventSlides_8c049424, init_eventSlides_8c04947c, init_eventSlides_8c0494b4, init_eventSlides_8c0494c4,
    init_eventSlides_8c0494d4, init_eventSlides_8c04950c, init_eventSlides_8c04952c, init_eventSlides_8c04956c,
    init_eventSlides_8c049594, init_eventSlides_8c0495b4, init_eventSlides_8c049614, init_eventSlides_8c049674,
    init_eventSlides_8c0496bc, init_eventSlides_8c049704, init_eventSlides_8c04974c, init_eventSlides_8c049774,
    init_eventSlides_8c0497a4, init_eventSlides_8c0497c4, init_eventSlides_8c049804, init_eventSlides_8c049824,
    init_eventSlides_8c049894, init_eventSlides_8c0498dc, init_eventSlides_8c04990c, init_eventSlides_8c04995c,
    init_eventSlides_8c0499b4, init_eventSlides_8c049a24, NULL,
};

STATIC EventSlide init_eventSlides_8c049c38[] = {
    { init_slideLayers_8c04721a, 0x00 },
    { init_slideLayers_8c04722c, 0x01 },
    { init_slideLayers_8c04721e, 0x02 },
    { init_slideLayers_8c047228, 0x03 },
    { init_slideLayers_8c04721a, 0x04 },
    { init_slideLayers_8c04722c, 0x05 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049c70[] = {
    { init_slideLayers_8c04722c, 0x06 },
    { init_slideLayers_8c04721e, 0x07 },
    { init_slideLayers_8c04721a, 0x08 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049c90[] = {
    { init_slideLayers_8c04734a, 0x09 },
    { init_slideLayers_8c04721a, 0x0a },
    { init_slideLayers_8c04721e, 0x0b },
    { init_slideLayers_8c04721a, 0x0c },
    { init_slideLayers_8c04722c, 0x0d },
    { init_slideLayers_8c047228, 0x0e },
    { init_slideLayers_8c04722c, 0x0f },
    { init_slideLayers_8c047222, 0x10 },
    { init_slideLayers_8c04721a, 0x11 },
    { init_slideLayers_8c04722c, 0x12 },
    { init_slideLayers_8c047222, 0x13 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049cf0[] = {
    { init_slideLayers_8c04721a, 0x14 },
    { init_slideLayers_8c04722c, 0x15 },
    { init_slideLayers_8c04721e, 0x16 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049d10[] = {
    { init_slideLayers_8c04721a, 0x17 },
    { init_slideLayers_8c047228, 0x18 },
    { init_slideLayers_8c04722c, 0x19 },
    { init_slideLayers_8c04721e, 0x1a },
    { init_slideLayers_8c04721a, 0x1b },
    { init_slideLayers_8c04722c, 0x1c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049d48[] = {
    { init_slideLayers_8c04722c, 0x1d },
    { init_slideLayers_8c04721e, 0x1e },
    { init_slideLayers_8c04721a, 0x1f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049d68[] = {
    { init_slideLayers_8c047270, 0x20 },
    { init_slideLayers_8c04727a, 0x21 },
    { init_slideLayers_8c047270, 0x22 },
    { init_slideLayers_8c04727a, 0x23 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049d90[] = {
    { init_slideLayers_8c04721a, 0x24 },
    { init_slideLayers_8c047228, 0x25 },
    { init_slideLayers_8c04721e, 0x26 },
    { init_slideLayers_8c04721a, 0x27 },
    { init_slideLayers_8c04722c, 0x28 },
    { init_slideLayers_8c047228, 0x29 },
    { init_slideLayers_8c047222, 0x2a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049dd0[] = {
    { init_slideLayers_8c047230, 0x2b },
    { init_slideLayers_8c047234, 0x2c },
    { init_slideLayers_8c04723a, 0x2d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049df0[] = {
    { init_slideLayers_8c047230, 0x2e },
    { init_slideLayers_8c04723a, 0x2f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049e08[] = {
    { init_slideLayers_8c047230, 0x30 },
    { init_slideLayers_8c047234, 0x31 },
    { init_slideLayers_8c04723a, 0x32 },
    { init_slideLayers_8c047234, 0x33 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049e30[] = {
    { init_slideLayers_8c047230, 0x34 },
    { init_slideLayers_8c047234, 0x35 },
    { init_slideLayers_8c04723a, 0x36 },
    { init_slideLayers_8c047230, 0x37 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049e58[] = {
    { init_slideLayers_8c04723a, 0x38 },
    { init_slideLayers_8c047234, 0x39 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049e70[] = {
    { init_slideLayers_8c04723e, 0x3a },
    { init_slideLayers_8c047264, 0x3b },
    { init_slideLayers_8c04723e, 0x3c },
    { init_slideLayers_8c04725a, 0x3d },
    { init_slideLayers_8c047268, 0x3e },
    { init_slideLayers_8c04725a, 0x3f },
    { init_slideLayers_8c04723e, 0x40 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049eb0[] = {
    { init_slideLayers_8c04723e, 0x41 },
    { init_slideLayers_8c047264, 0x42 },
    { init_slideLayers_8c04725a, 0x43 },
    { init_slideLayers_8c047268, 0x44 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049ed8[] = {
    { init_slideLayers_8c04723e, 0x45 },
    { init_slideLayers_8c04725a, 0x46 },
    { init_slideLayers_8c047268, 0x47 },
    { init_slideLayers_8c04725e, 0x48 },
    { init_slideLayers_8c04723e, 0x49 },
    { init_slideLayers_8c047264, 0x4a },
    { init_slideLayers_8c047242, 0x4b },
    { init_slideLayers_8c047248, 0x4c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049f20[] = {
    { init_slideLayers_8c04726c, 0x4d },
    { init_slideLayers_8c04727a, 0x4e },
    { init_slideLayers_8c047276, 0x4f },
    { init_slideLayers_8c04726c, 0x50 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049f48[] = {
    { init_slideLayers_8c04726c, 0x51 },
    { init_slideLayers_8c04727a, 0x52 },
    { init_slideLayers_8c04726c, 0x53 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049f68[] = {
    { init_slideLayers_8c04726c, 0x54 },
    { init_slideLayers_8c04727a, 0x55 },
    { init_slideLayers_8c047276, 0x56 },
    { init_slideLayers_8c047270, 0x57 },
    { init_slideLayers_8c04727a, 0x58 },
    { init_slideLayers_8c04726c, 0x59 },
    { init_slideLayers_8c04727a, 0x5a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049fa8[] = {
    { init_slideLayers_8c04726c, 0x5b },
    { init_slideLayers_8c04727a, 0x5c },
    { init_slideLayers_8c047276, 0x5d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049fc8[] = {
    { init_slideLayers_8c04726c, 0x5e },
    { init_slideLayers_8c047276, 0x5f },
    { init_slideLayers_8c04727a, 0x60 },
    { init_slideLayers_8c04726c, 0x61 },
    { init_slideLayers_8c04727a, 0x62 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c049ff8[] = {
    { init_slideLayers_8c04726c, 0x63 },
    { init_slideLayers_8c047276, 0x64 },
    { init_slideLayers_8c04727a, 0x65 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a018[] = {
    { init_slideLayers_8c046fd4, 0x66 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a028[] = {
    { init_slideLayers_8c046fd4, 0x67 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a038[] = {
    { init_slideLayers_8c046fe6, 0x68 },
    { init_slideLayers_8c046fdc, 0x69 },
    { init_slideLayers_8c047006, 0x6a },
    { init_slideLayers_8c047002, 0x6b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a060[] = {
    { init_slideLayers_8c046fdc, 0x6c },
    { init_slideLayers_8c047006, 0x6d },
    { init_slideLayers_8c047002, 0x6e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a080[] = {
    { init_slideLayers_8c047002, 0x6f },
    { init_slideLayers_8c047006, 0x70 },
    { init_slideLayers_8c046fee, 0x71 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a0a0[] = {
    { init_slideLayers_8c047006, 0x72 },
    { init_slideLayers_8c046fdc, 0x73 },
    { init_slideLayers_8c047006, 0x74 },
    { init_slideLayers_8c046ffc, 0x75 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a0c8[] = {
    { init_slideLayers_8c046ffc, 0x76 },
    { init_slideLayers_8c046fdc, 0x77 },
    { init_slideLayers_8c047002, 0x78 },
    { init_slideLayers_8c047006, 0x79 },
    { init_slideLayers_8c046fee, 0x7a },
    { init_slideLayers_8c047006, 0x7b },
    { init_slideLayers_8c046fdc, 0x7c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a108[] = {
    { init_slideLayers_8c04727e, 0x7d },
    { init_slideLayers_8c047282, 0x7e },
    { init_slideLayers_8c04727e, 0x7f },
    { init_slideLayers_8c047290, 0x80 },
    { init_slideLayers_8c04728c, 0x81 },
    { init_slideLayers_8c04727e, 0x82 },
    { init_slideLayers_8c04728c, 0x83 },
    { init_slideLayers_8c047298, 0x84 },
    { init_slideLayers_8c047286, 0x85 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a158[] = {
    { init_slideLayers_8c04727e, 0x86 },
    { init_slideLayers_8c047282, 0x87 },
    { init_slideLayers_8c047298, 0x88 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a178[] = {
    { init_slideLayers_8c04727e, 0x89 },
    { init_slideLayers_8c04728c, 0x8a },
    { init_slideLayers_8c047290, 0x8b },
    { init_slideLayers_8c047298, 0x8c },
    { init_slideLayers_8c04727e, 0x8d },
    { init_slideLayers_8c047298, 0x8e },
    { init_slideLayers_8c047286, 0x8f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a1b8[] = {
    { init_slideLayers_8c04727e, 0x90 },
    { init_slideLayers_8c047282, 0x91 },
    { init_slideLayers_8c047298, 0x92 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a1d8[] = {
    { init_slideLayers_8c047286, 0x93 },
    { init_slideLayers_8c04728c, 0x94 },
    { init_slideLayers_8c04727e, 0x95 },
    { init_slideLayers_8c047294, 0x96 },
    { init_slideLayers_8c04727e, 0x97 },
    { init_slideLayers_8c047294, 0x98 },
    { init_slideLayers_8c047282, 0x99 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a218[] = {
    { init_slideLayers_8c04727e, 0x9a },
    { init_slideLayers_8c047294, 0x9b },
    { init_slideLayers_8c04728c, 0x9c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a238[] = {
    { init_slideLayers_8c04727e, 0x9d },
    { init_slideLayers_8c047290, 0x9e },
    { init_slideLayers_8c047282, 0x9f },
    { init_slideLayers_8c047294, 0xa0 },
    { init_slideLayers_8c04728c, 0xa1 },
    { init_slideLayers_8c04727e, 0xa2 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a270[] = {
    { init_slideLayers_8c04727e, 0xa3 },
    { init_slideLayers_8c047290, 0xa4 },
    { init_slideLayers_8c047294, 0xa5 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a290[] = {
    { init_slideLayers_8c04729e, 0xa6 },
    { init_slideLayers_8c0472a2, 0xa7 },
    { init_slideLayers_8c0472ac, 0xa8 },
    { init_slideLayers_8c04729e, 0xa9 },
    { init_slideLayers_8c0472a6, 0xaa },
    { init_slideLayers_8c04729e, 0xab },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a2c8[] = {
    { init_slideLayers_8c04729e, 0xac },
    { init_slideLayers_8c0472ac, 0xad },
    { init_slideLayers_8c0472a2, 0xae },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a2e8[] = {
    { init_slideLayers_8c04729e, 0xaf },
    { init_slideLayers_8c0472a2, 0xb0 },
    { init_slideLayers_8c0472ac, 0xb1 },
    { init_slideLayers_8c04729e, 0xb2 },
    { init_slideLayers_8c0472a6, 0xb3 },
    { init_slideLayers_8c0472ac, 0xb4 },
    { init_slideLayers_8c04729e, 0xb5 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a328[] = {
    { init_slideLayers_8c04729e, 0xb6 },
    { init_slideLayers_8c0472a2, 0xb7 },
    { init_slideLayers_8c0472ac, 0xb8 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a348[] = {
    { init_slideLayers_8c04729e, 0xb9 },
    { init_slideLayers_8c0472ac, 0xba },
    { init_slideLayers_8c0472a6, 0xbb },
    { init_slideLayers_8c04729e, 0xbc },
    { init_slideLayers_8c0472ac, 0xbd },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a378[] = {
    { init_slideLayers_8c0472a6, 0xbe },
    { init_slideLayers_8c0472ac, 0xbf },
    { init_slideLayers_8c04729e, 0xc0 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a398[] = {
    { init_slideLayers_8c0472b0, 0xc1 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a3a8[] = {
    { init_slideLayers_8c0472b0, 0xc2 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a3b8[] = {
    { init_slideLayers_8c0472b0, 0xc3 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a3c8[] = {
    { init_slideLayers_8c0472b0, 0xc4 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a3d8[] = {
    { init_slideLayers_8c0472b0, 0xc5 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a3e8[] = {
    { init_slideLayers_8c0472b0, 0xc6 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a3f8[] = {
    { init_slideLayers_8c0472b4, 0xc7 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a408[] = {
    { init_slideLayers_8c0472b4, 0xc8 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a418[] = {
    { init_slideLayers_8c0472b4, 0xc9 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a428[] = {
    { init_slideLayers_8c0472b4, 0xca },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a438[] = {
    { init_slideLayers_8c0472b8, 0xcb },
    { init_slideLayers_8c0472c2, 0xcc },
    { init_slideLayers_8c0472c6, 0xcd },
    { init_slideLayers_8c0472b8, 0xce },
    { init_slideLayers_8c0472ca, 0xcf },
    { init_slideLayers_8c0472bc, 0xd0 },
    { init_slideLayers_8c0472ca, 0xd1 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a478[] = {
    { init_slideLayers_8c0472b8, 0xd2 },
    { init_slideLayers_8c0472c6, 0xd3 },
    { init_slideLayers_8c0472c2, 0xd4 },
    { init_slideLayers_8c0472c6, 0xd5 },
    { init_slideLayers_8c0472b8, 0xd6 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a4a8[] = {
    { init_slideLayers_8c0472d0, 0xd7 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a4b8[] = {
    { init_slideLayers_8c0472d0, 0xd8 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a4c8[] = {
    { init_slideLayers_8c0472b8, 0xd9 },
    { init_slideLayers_8c0472c2, 0xda },
    { init_slideLayers_8c0472c6, 0xdb },
    { init_slideLayers_8c0472b8, 0xdc },
    { init_slideLayers_8c0472c6, 0xdd },
    { init_slideLayers_8c0472b8, 0xde },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a500[] = {
    { init_slideLayers_8c0472b8, 0xdf },
    { init_slideLayers_8c0472c6, 0xe0 },
    { init_slideLayers_8c0472b8, 0xe1 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a520[] = {
    { init_slideLayers_8c0472d0, 0xe2 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a530[] = {
    { init_slideLayers_8c0472d0, 0xe3 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a540[] = {
    { init_slideLayers_8c047074, 0xe4 },
    { init_slideLayers_8c04709e, 0xe5 },
    { init_slideLayers_8c047074, 0xe6 },
    { init_slideLayers_8c047086, 0xe7 },
    { init_slideLayers_8c0470a2, 0xe8 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a570[] = {
    { init_slideLayers_8c04707e, 0xe9 },
    { init_slideLayers_8c047078, 0xea },
    { init_slideLayers_8c04709a, 0xeb },
    { init_slideLayers_8c047074, 0xec },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a598[] = {
    { init_slideLayers_8c047074, 0xed },
    { init_slideLayers_8c04709a, 0xee },
    { init_slideLayers_8c047074, 0xef },
    { init_slideLayers_8c04709a, 0xf0 },
    { init_slideLayers_8c0470a2, 0xf1 },
    { init_slideLayers_8c04709a, 0xf2 },
    { init_slideLayers_8c04709e, 0xf3 },
    { init_slideLayers_8c047086, 0xf4 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a5e0[] = {
    { init_slideLayers_8c047086, 0xf5 },
    { init_slideLayers_8c04734e, 0xf6 },
    { init_slideLayers_8c047086, 0xf7 },
    { init_slideLayers_8c04709a, 0xf8 },
    { init_slideLayers_8c04709e, 0xf9 },
    { init_slideLayers_8c047074, 0xfa },
    { init_slideLayers_8c047086, 0xfb },
    { init_slideLayers_8c04709e, 0xfc },
    { init_slideLayers_8c047074, 0xfd },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a630[] = {
    { init_slideLayers_8c047094, 0xfe },
    { init_slideLayers_8c047074, 0xff },
    { init_slideLayers_8c04709e, 0x100 },
    { init_slideLayers_8c04709a, 0x101 },
    { init_slideLayers_8c047074, 0x102 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a660[] = {
    { init_slideLayers_8c0472d4, 0x103 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a670[] = {
    { init_slideLayers_8c0472d4, 0x104 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a680[] = {
    { init_slideLayers_8c0472d4, 0x105 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a690[] = {
    { init_slideLayers_8c0472d4, 0x106 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a6a0[] = {
    { init_slideLayers_8c0472d4, 0x107 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a6b0[] = {
    { init_slideLayers_8c0472d4, 0x108 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a6c0[] = {
    { init_slideLayers_8c0472d4, 0x109 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a6d0[] = {
    { init_slideLayers_8c0472d4, 0x10a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a6e0[] = {
    { init_slideLayers_8c0472d4, 0x10b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a6f0[] = {
    { init_slideLayers_8c0472d4, 0x10c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a700[] = {
    { init_slideLayers_8c0472d4, 0x10d },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a710[] = {
    { init_slideLayers_8c0472d4, 0x10e },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a720[] = {
    { init_slideLayers_8c0472d8, 0x10f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a730[] = {
    { init_slideLayers_8c0472d8, 0x110 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a740[] = {
    { init_slideLayers_8c0472dc, 0x111 },
    { init_slideLayers_8c0472e4, 0x112 },
    { init_slideLayers_8c0472e0, 0x113 },
    { init_slideLayers_8c0472e8, 0x114 },
    { init_slideLayers_8c0472dc, 0x115 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a770[] = {
    { init_slideLayers_8c0472ee, 0x116 },
    { init_slideLayers_8c0472fc, 0x117 },
    { init_slideLayers_8c0472ee, 0x118 },
    { init_slideLayers_8c0472f2, 0x119 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a798[] = {
    { init_slideLayers_8c0472ee, 0x11a },
    { init_slideLayers_8c0472f8, 0x11b },
    { init_slideLayers_8c0472fc, 0x11c },
    { init_slideLayers_8c0472f2, 0x11d },
    { init_slideLayers_8c0472f8, 0x11e },
    { init_slideLayers_8c0472ee, 0x11f },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a7d0[] = {
    { init_slideLayers_8c04711c, 0x120 },
    { init_slideLayers_8c04711c, 0x121 },
    { init_slideLayers_8c047146, 0x122 },
    { init_slideLayers_8c04711c, 0x123 },
    { init_slideLayers_8c047142, 0x124 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a800[] = {
    { init_slideLayers_8c047118, 0x125 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a810[] = {
    { init_slideLayers_8c047118, 0x126 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a820[] = {
    { init_slideLayers_8c04711c, 0x127 },
    { init_slideLayers_8c04711c, 0x128 },
    { init_slideLayers_8c047142, 0x129 },
    { init_slideLayers_8c047146, 0x12a },
    { init_slideLayers_8c04711c, 0x12b },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a850[] = {
    { init_slideLayers_8c04721a, 0x12c },
    { init_slideLayers_8c047228, 0x12d },
    { init_slideLayers_8c04722c, 0x12e },
    { init_slideLayers_8c04721e, 0x12f },
    { init_slideLayers_8c04721a, 0x130 },
    { init_slideLayers_8c04722c, 0x131 },
    { init_slideLayers_8c047270, 0x132 },
    { init_slideLayers_8c04727a, 0x133 },
    { init_slideLayers_8c047270, 0x134 },
    { init_slideLayers_8c04727a, 0x135 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a8a8[] = {
    { init_slideLayers_8c04722c, 0x136 },
    { init_slideLayers_8c04721e, 0x137 },
    { init_slideLayers_8c04721a, 0x138 },
    { init_slideLayers_8c047270, 0x139 },
    { init_slideLayers_8c04727a, 0x13a },
    { init_slideLayers_8c047270, 0x13b },
    { init_slideLayers_8c04727a, 0x13c },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a8e8[] = {
    { init_slideLayers_8c0472b4, 0x13d },
    { init_slideLayers_8c0472b8, 0x13e },
    { init_slideLayers_8c0472c2, 0x13f },
    { init_slideLayers_8c0472c6, 0x140 },
    { init_slideLayers_8c0472b8, 0x141 },
    { init_slideLayers_8c0472ca, 0x142 },
    { init_slideLayers_8c0472bc, 0x143 },
    { init_slideLayers_8c0472ca, 0x144 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a930[] = {
    { init_slideLayers_8c0472b4, 0x145 },
    { init_slideLayers_8c0472b8, 0x146 },
    { init_slideLayers_8c0472c6, 0x147 },
    { init_slideLayers_8c0472c2, 0x148 },
    { init_slideLayers_8c0472c6, 0x149 },
    { init_slideLayers_8c0472b8, 0x14a },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide init_eventSlides_8c04a968[] = {
    { init_slideLayers_8c0472ee, 0x14b },
    { init_slideLayers_8c0472f8, 0x14c },
    { init_slideLayers_8c0472fc, 0x14d },
    { init_slideLayers_8c0472f2, 0x14e },
    { init_slideLayers_8c0472f8, 0x14f },
    { init_slideLayers_8c0472ee, 0x150 },
    { init_slideLayers_8c04711c, 0x151 },
    { init_slideLayers_8c04711c, 0x152 },
    { init_slideLayers_8c047146, 0x153 },
    { init_slideLayers_8c04711c, 0x154 },
    { init_slideLayers_8c047142, 0x155 },
    { (Uint16 *)-1, 0 },
};

STATIC EventSlide *init_omeEvents_8c04a9c8[] = {
    init_eventSlides_8c049c38, init_eventSlides_8c049c70, init_eventSlides_8c049c90, init_eventSlides_8c049cf0,
    init_eventSlides_8c049d10, init_eventSlides_8c049d48, init_eventSlides_8c049d68, init_eventSlides_8c049d90,
    init_eventSlides_8c049dd0, init_eventSlides_8c049df0, init_eventSlides_8c049e08, init_eventSlides_8c049e30,
    init_eventSlides_8c049e58, init_eventSlides_8c049e70, init_eventSlides_8c049eb0, init_eventSlides_8c049ed8,
    init_eventSlides_8c049f20, init_eventSlides_8c049f48, init_eventSlides_8c049f68, init_eventSlides_8c049fa8,
    init_eventSlides_8c049fc8, init_eventSlides_8c049ff8, init_eventSlides_8c04a018, init_eventSlides_8c04a028,
    init_eventSlides_8c04a038, init_eventSlides_8c04a060, init_eventSlides_8c04a080, init_eventSlides_8c04a0a0,
    init_eventSlides_8c04a0c8, init_eventSlides_8c04a108, init_eventSlides_8c04a158, init_eventSlides_8c04a178,
    init_eventSlides_8c04a1b8, init_eventSlides_8c04a1d8, init_eventSlides_8c04a218, init_eventSlides_8c04a238,
    init_eventSlides_8c04a270, init_eventSlides_8c04a290, init_eventSlides_8c04a2c8, init_eventSlides_8c04a2e8,
    init_eventSlides_8c04a328, init_eventSlides_8c04a348, init_eventSlides_8c04a378, init_eventSlides_8c04a398,
    init_eventSlides_8c04a3a8, init_eventSlides_8c04a3b8, init_eventSlides_8c04a3c8, init_eventSlides_8c04a3d8,
    init_eventSlides_8c04a3e8, init_eventSlides_8c04a3f8, init_eventSlides_8c04a408, init_eventSlides_8c04a418,
    init_eventSlides_8c04a428, init_eventSlides_8c04a438, init_eventSlides_8c04a478, init_eventSlides_8c04a4a8,
    init_eventSlides_8c04a4b8, init_eventSlides_8c04a4c8, init_eventSlides_8c04a500, init_eventSlides_8c04a520,
    init_eventSlides_8c04a530, init_eventSlides_8c04a540, init_eventSlides_8c04a570, init_eventSlides_8c04a598,
    init_eventSlides_8c04a5e0, init_eventSlides_8c04a630, init_eventSlides_8c04a660, init_eventSlides_8c04a670,
    init_eventSlides_8c04a680, init_eventSlides_8c04a690, init_eventSlides_8c04a6a0, init_eventSlides_8c04a6b0,
    init_eventSlides_8c04a6c0, init_eventSlides_8c04a6d0, init_eventSlides_8c04a6e0, init_eventSlides_8c04a6f0,
    init_eventSlides_8c04a700, init_eventSlides_8c04a710, init_eventSlides_8c04a720, init_eventSlides_8c04a730,
    init_eventSlides_8c04a740, init_eventSlides_8c04a770, init_eventSlides_8c04a798, init_eventSlides_8c04a7d0,
    init_eventSlides_8c04a800, init_eventSlides_8c04a810, init_eventSlides_8c04a820, init_eventSlides_8c04a850,
    init_eventSlides_8c04a8a8, init_eventSlides_8c04a8e8, init_eventSlides_8c04a930, init_eventSlides_8c04a968,
    NULL,
};

/* Full-screen (640x480) tilemap of 32x32 cells (20x15 grid); reused for
 * every message-box layer, with texlist/map patched in per layer before
 * each njDrawScroll. */
STATIC NJS_SCROLL init_msgScroll_8c04ab3c = {
    /* celps      */ 32,
    /* mapw, maph */ 20, 15,
    /* sw, sh     */ 0, 0,
    /* list, map  */ NULL, NULL,
    /* px, py     */ 0.0f, 0.0f,
    /* bx, by     */ 0.0f, 0.0f,
    /* pr         */ 0.0f,
    /* sflag      */ 0,
    /* sx, sy     */ 1.0f, 1.0f,
    /* spx, spy   */ 0.0f, 0.0f,
    /* mflag      */ 0,
    /* cx, cy     */ 0.0f, 0.0f,
    /* m          */ { 0.0f, 0.0f, 0.0f, 0.0f },
    /* colmode    */ 0x0210000A,
    /* clip       */ { { 0, 0 }, { 0, 0 } },
    /* attr       */ 0,
    /* sclc       */ ARGB(0xff, 0xff, 0xff, 0xff),
};


/* ====================
 * Forward Declarations
 * ====================
 */

/* Called by ObjectsStartAssetRequests_8c029ad4 before its own definition. */
void ObjectsFreeAssetRequests_8c029cfe(void);

/* Called by ObjectsStartMessageBox_8c02ad8c before its own definition. */
void ObjectsOpenTextbox_8c02ae3e(int x, int y, float priority, int width, int height, int x2, int y2, int enable_offset);

/* Called by messageBoxTask_8c02ab7a before their own definitions. */
void ObjectsFreeMessageAssets_8c02adee(void);
int ObjectsSwapMessageBoxFor_8c02aefc(char *string);
int ObjectsMenuTextboxText_8c02af1c(int limit);

/* Called by ObjectsFreeMessageAssets_8c02adee before its own definition. */
void ObjectsFreeTextboxes_8c02af32(void);

/* ====================
 * Functions
 * ====================
 */

/* Advances a pedestrian along its path (pPathNode_0x4c, flPathPos_0x54),
 * forward or backward per nReverse_0x40, moving into the next/previous segment
 * and wrapping at the path's ends. Returns whether the segment changed. */
STATIC Bool advancePedPathPos_8c0289ac(PedestrianState *ped)
{
    PedPathNode *node;
    PedPathNode *startNode;
    float pathPos;

    node = ped->pPathNode_0x4c;
    startNode = node;
    pathPos = ped->flPathPos_0x54;

    if (ped->nReverse_0x40 == 0) {
        while (node->flLength_0x00 <= pathPos) {
            pathPos -= node->flLength_0x00;
            node++;
            if (node->flLength_0x00 == 0.0f) {
                node = ped->pPathFirst_0x44;
            }
        }
    } else {
        for (; pathPos <= 0.0f; pathPos += node->flLength_0x00) {
            if (node == ped->pPathFirst_0x44) {
                node = ped->pPathLast_0x48;
            } else {
                node--;
            }
        }
    }

    if (startNode != node) {
        ped->pPathNode_0x4c = node;
        ped->nNodeFlags_0x60 = node->nFlags_0x14;
    }
    ped->flPathPos_0x54 = pathPos;
    ped->sprite_0x00.p.x = pathPos * node->flStepX_0x0c + node->flBaseX_0x04 + ped->flBaseX_0x20;
    ped->sprite_0x00.p.z = pathPos * node->flStepZ_0x10 + node->flBaseZ_0x08 + ped->flBaseZ_0x24;
    return startNode != node;
}

/* Mirror-view counterpart of drawPedestrians_8c028b74, registered as its
 * layer-1 draw callback by pedestriansTask_8c0293f6. Takes the facing bucket
 * from the path node's flag bits instead of the camera angle, and flips the
 * facing test. */
STATIC void drawPedestriansMirror_8c028a38(int arg0)
{
    PedGroupEntry *groups = (PedGroupEntry *)var_pedGroups_8c228230;
    int pageListIndex;
    Task *slot;
    PedestrianState *ped;
    int facing;
    int bucket;
    int spriteIndex;

    for (pageListIndex = 0; pageListIndex < var_pedGroupCount_8c228234; pageListIndex++) {
        if (groups[pageListIndex].active_0x00 == 0) {
            continue;
        }

        for (slot = groups[pageListIndex].list_0x08; slot->action != NULL; slot++) {
            if (slot->action == (TaskAction)-1) {
                continue;
            }

            ped = (PedestrianState *)slot->state;
            facing = ped->nReverse_0x40;

            if (ped->nKind_0x3c == 2) {
                switch (facing) {
                    case 0: spriteIndex = 0x20; break;
                    case 1: spriteIndex = 0x21; break;
                    case 2: spriteIndex = 0x22; break;
                    case 3: spriteIndex = 0x23; break;
                }
            } else {
                bucket = ped->nNodeFlags_0x60 & 0x30000000;
                if (facing == 1) {
                    switch (bucket) {
                        case 0: spriteIndex = 0; break;
                        case 0x10000000: spriteIndex = 8; break;
                        case 0x20000000: spriteIndex = 0x10; break;
                        case 0x30000000: spriteIndex = 0x18; break;
                    }
                } else {
                    switch (bucket) {
                        case 0: spriteIndex = 8; break;
                        case 0x10000000: spriteIndex = 0; break;
                        case 0x20000000: spriteIndex = 0x18; break;
                        case 0x30000000: spriteIndex = 0x10; break;
                    }
                }
                spriteIndex += (ped->nAnimPhase_0x5c >> 2) & 7;
            }

            njDrawSprite3D(&ped->sprite_0x00, spriteIndex, 0x30);
        }
    }
}

/* Draws every active pedestrian's sprite for the frame. A type 2 object
 * (checked at 0x3c) just maps its own facing state (0x40, 0-3) to one of
 * four sprite frames. Any other type is bucketed by the angle between the
 * camera and the direction its path segment runs (flStepX_0x0c/flStepZ_0x10
 * of pPathNode_0x4c); every pedestrian on a segment shares that angle, so
 * the bucket is cached per frame keyed by the node pointer.
 * Installed as a DrawCallback1; the callback arg is unused. */
STATIC void drawPedestrians_8c028b74(int arg0)
{
    PedGroupEntry *groups = (PedGroupEntry *)var_pedGroups_8c228230;
    DirCacheEntry *cache = (DirCacheEntry *)syMalloc(DIR_CACHE_CAPACITY * sizeof(DirCacheEntry));
    DirCacheEntry *cacheEnd = cache;
    DirCacheEntry *found;
    Angle selfAngle;
    Angle targetAngle;
    int diff;
    int pageListIndex;
    Task *slot;
    PedestrianState *ped;
    int facing;
    PedPathNode *key;
    int bucket;
    int spriteIndex;

    selfAngle = njArcTan2(-var_busState_8c1bb9d0.moveDeltaX_0x308, -var_busState_8c1bb9d0.moveDeltaZ_0x310);

    for (pageListIndex = 0; pageListIndex < var_pedGroupCount_8c228234; pageListIndex++) {
        if (groups[pageListIndex].active_0x00 == 0) {
            continue;
        }

        for (slot = groups[pageListIndex].list_0x08; slot->action != NULL; slot++) {
            if (slot->action == (TaskAction)-1) {
                continue;
            }

            ped = (PedestrianState *)slot->state;
            facing = ped->nReverse_0x40;

            if (ped->nKind_0x3c == 2) {
                switch (facing) {
                    case 0: spriteIndex = 0x20; break;
                    case 1: spriteIndex = 0x21; break;
                    case 2: spriteIndex = 0x22; break;
                    case 3: spriteIndex = 0x23; break;
                }
            } else {
                key = ped->pPathNode_0x4c;

                for (found = cache; found < cacheEnd && found->key != key; found++) {
                }

                if (cacheEnd <= found) {
                    if (cache + DIR_CACHE_CAPACITY <= cacheEnd) {
                        continue; /* cache full; skip drawing this object */
                    }
                    found->key = key;
                    targetAngle = njArcTan2(-key->flStepX_0x0c, -key->flStepZ_0x10);
                    diff = selfAngle - targetAngle;
                    if ((diff < -0x2000 || 0x1fff < diff) &&
                        diff < 0xe000 && -0xe001 < diff) {
                        if ((diff < 0x2000 || 0x5fff < diff) &&
                            (diff < -0xe000 || -0xa001 < diff)) {
                            if ((diff < -0x6000 || -0x2001 < diff) &&
                                (diff < 0xa000 || 0xdfff < diff)) {
                                found->bucket = 0x10000000;
                            } else {
                                found->bucket = 0x30000000;
                            }
                        } else {
                            found->bucket = 0x20000000;
                        }
                    } else {
                        found->bucket = 0;
                    }
                    cacheEnd++;
                }

                bucket = found->bucket;
                if (facing == 0) {
                    switch (bucket) {
                        case 0: spriteIndex = 0; break;
                        case 0x10000000: spriteIndex = 8; break;
                        case 0x20000000: spriteIndex = 0x10; break;
                        case 0x30000000: spriteIndex = 0x18; break;
                    }
                } else {
                    switch (bucket) {
                        case 0: spriteIndex = 8; break;
                        case 0x10000000: spriteIndex = 0; break;
                        case 0x20000000: spriteIndex = 0x18; break;
                        case 0x30000000: spriteIndex = 0x10; break;
                    }
                }
                spriteIndex += (ped->nAnimPhase_0x5c >> 2) & 7;
            }

            njDrawSprite3D(&ped->sprite_0x00, spriteIndex, 0x30);
        }
    }

    syFree(cache);
}

/* Relocation fixup for a freshly-loaded macHumM0 blob (which becomes
 * var_pedGroupLists_8c228240): a 0-terminated array of self-relative offsets,
 * each turned into an absolute pointer in place. */
void ObjectsRelocatePedGroupLists_8c028dd0(void *handle)
{
    int *entry = handle;
    while (*entry != 0) {
        *entry += (int)handle;
        entry++;
    }
}
/* Relocation fixup for a freshly-loaded PedGroupDef array (013ae8 hands it
 * the course's macHumG0 blob, which becomes var_pedGroupDefs_8c22823c): adds
 * the array's own base address to each entry's specs_0x08, converting it from
 * a self-relative offset to an absolute pointer. Terminated by an entry whose
 * id_0x00 is -1. */
void ObjectsRelocatePedGroupDefs_8c028de8(void *handle)
{
    int *entry = handle;
    while (entry[0] != -1) {
        entry[2] += (int)handle;
        entry += 3;
    }
}
STATIC void pedestrianTask_8c028e00(Task *task, PedestrianState *ped)
{
    int state;
    Bool shouldMove;
    Bool crossingStarted;
    int *entry;
    float pos[2];

    state = ped->nState_0x64;
    shouldMove = TRUE;
    crossingStarted = FALSE;

    if (state == 0 && (ped->nNodeFlags_0x60 & 0xfff) == 0) {
        /* Nothing ahead on this node: keep walking. */
    } else if (state == 0 || state == 1) {
        if (state == 0) {
            ped->nState_0x64 = 1;
            ped->nSignalIdA_0x68 = (int)(short)((ped->nNodeFlags_0x60 >> 16) & 0xfff);
            ped->nSignalIdB_0x6c = ped->nNodeFlags_0x60 & 0xfff;
            ped->nAnimPhase_0x5c = 0;
        }
        /* Held at the kerb; frame 1 on the signal object means "don't walk". */
        if (SignalGetFrame_8c028900(ped->nSignalIdA_0x68) != 1) {
            ped->nState_0x64 = 2;
        }
        shouldMove = FALSE;
    } else if (state == 4 ||
               (state == 2 && SignalGetFrame_8c028900(ped->nSignalIdA_0x68) == 1 &&
                SignalIsCrossingOccupied_8c02898e(ped->nSignalIdB_0x6c) == 0)) {
        if (state == 2) {
            ped->nState_0x64 = 4;
        }
        if ((ped->nNodeFlags_0x60 & 0xfff) == 0) {
            ped->nState_0x64 = 0;
        } else {
            for (entry = var_crosswalkTable_8c228248; entry < var_crosswalkTableEnd_8c228244; entry += 2) {
                if (ped->pPathNode_0x4c == (PedPathNode *)*entry) {
                    pos[0] = ped->sprite_0x00.p.x;
                    pos[1] = ped->sprite_0x00.p.z;
                    if (ped->nReverse_0x40 == 0) {
                        if (IntersectSegments_8c0206f0(var_stopLinePointA_8c228268, var_stopLinePointB_8c228270, pos,
                                (float *)(entry[1] + 4), &var_crossingIntersectPoint_8c1bc458) != 0) {
                            shouldMove = FALSE;
                        }
                    } else {
                        if (IntersectSegments_8c0206f0(var_stopLinePointA_8c228268, var_stopLinePointB_8c228270, pos,
                                (float *)(*entry + 4), &var_crossingIntersectPoint_8c1bc458) != 0) {
                            shouldMove = FALSE;
                        }
                    }
                    break;
                }
            }
            SignalMarkPedCrossing_8c02897a(ped->nSignalIdB_0x6c);
            CollisionQueueAdd_8c02e48e(ped);
            crossingStarted = TRUE;
        }
    } else if (state == 2) {
        shouldMove = FALSE;
    }

    if (shouldMove) {
        state = ped->nState_0x64;
        if (state == 0) {
            if (ped->nReverse_0x40 == 0) {
                ped->flPathPos_0x54 = ped->flPathPos_0x54 + ped->flSpeed_0x58;
            } else {
                ped->flPathPos_0x54 = ped->flPathPos_0x54 - ped->flSpeed_0x58;
            }
        } else if (state == 4) {
            if (SignalGetFrame_8c028900(ped->nSignalIdA_0x68) == 1) {
                if (ped->nReverse_0x40 == 0) {
                    ped->flPathPos_0x54 = ped->flPathPos_0x54 + ped->flSpeed_0x58 * 2.0f;
                } else {
                    ped->flPathPos_0x54 = ped->flPathPos_0x54 - ped->flSpeed_0x58 * 2.0f;
                }
            } else {
                if (ped->nReverse_0x40 == 0) {
                    ped->flPathPos_0x54 = ped->flPathPos_0x54 + ped->flSpeed_0x58 * 3.0f;
                } else {
                    ped->flPathPos_0x54 = ped->flPathPos_0x54 - ped->flSpeed_0x58 * 3.0f;
                }
                ped->nAnimPhase_0x5c = ped->nAnimPhase_0x5c + 1;
            }
        }
        ped->nAnimPhase_0x5c = ped->nAnimPhase_0x5c + 1;

        if ((advancePedPathPos_8c0289ac(ped) || (ped->nKind_0x3c == 1 && !crossingStarted)) &&
            (GroundProbeTrackPolygon_8c020b6c(ped->sprite_0x00.p.x - ped->flBaseX_0x20, ped->sprite_0x00.p.y,
                          ped->sprite_0x00.p.z - ped->flBaseZ_0x24, &ped->aGround_0x28),
             ped->aGround_0x28.count_0x0c != 0)) {
            GroundProbeInterpolateHeight_8c020f7e(&ped->aGround_0x28, (float *)ped);
        }
    }
}
/* Do-nothing TaskAction for a kind-2 (static sprite) ped-group entry:
 * unlike a walking pedestrian it needs no per-frame update once placed. */
STATIC void pedStaticObjectTask_8c02903e() {}

/* Per-frame driver for one pedestrian-group slot's spawn list, installed via
 * TaskPush_8c014ae8 by pedestriansTask_8c0293f6. Each call consumes one
 * PedGroupSpawnSpec entry from the group's spec list (task->spec_0x1c),
 * spawning either a walking pedestrian (pedestrianTask_8c028e00) or a static
 * sprite object (pedStaticObjectTask_8c02903e) into the group's own subtask
 * array (task->subTasks_0x18), then runs that subtask array for the frame.
 * If var_pedGroups_8c228230[task->pageListIndex_0x08] is no longer wanted this
 * frame, tears the whole group down instead. */
STATIC void pedGroupTask_8c029078(PedGroupTask *task)
{
    PedGroupEntry *group;
    PedGroupSpawnSpec *spec;
    PedPathInfo *path;
    Task *subTask;
    PedestrianState *state;
    float radius;
    float targetDist;
    PedPathNode *node;
    int pageListIndex;
    int pushFailed;

    pageListIndex = task->pageListIndex_0x08;
    group = &((PedGroupEntry *)var_pedGroups_8c228230)[pageListIndex];

    if (group->wanted_0x04 == 0) {
        group->active_0x00 = 0;
        TaskFreeGroup_8c014ab4(task->subTasks_0x18);
        syFree(task->subTasks_0x18);
        TaskFree_8c014b66((Task *)task);
        return;
    }

    spec = task->spec_0x1c;
    if (spec->nKindId_0x00 != 0xffff) {
        path = &((PedPathInfo *)var_pedPaths_8c228238)[pageListIndex];

        if (spec->flX_0x04 == 0.0f || spec->flX_0x04 <= path->flLength_0x08) {
            /* Faithful to the original: an unrecognised nKind falls into the
             * sprite setup below with subTask/state still uninitialised. Only a
             * failed TaskPush skips it. */
            pushFailed = 0;
            if (spec->nKind_0x02 == 0 || spec->nKind_0x02 == 1) {
                if (TaskPush_8c014ae8(task->subTasks_0x18, pedestrianTask_8c028e00,
                        &subTask, (void **)&state, sizeof(PedestrianState))) {
                    radius = task->radius_0x10;
                    state->flBaseX_0x20 = ((float)rand() / 32768.0f) * radius * 2.0f - radius;
                    state->flBaseZ_0x24 = ((float)rand() / 32768.0f) * radius * 2.0f - radius;
                    /* Only the seed's low 16 bits: 0.04 to 0.06 units per frame. */
                    state->flSpeed_0x58 =
                            ((float)(Uint16)AsqGetRandomA_8c012166() / 65536.0f) / 50.0f + 0.04f;
                    state->nAnimPhase_0x5c = AsqGetRandomA_8c012166();
                    state->nState_0x64 = 0;
                    state->pPathFirst_0x44 = path->pFirst_0x00;
                    state->pPathLast_0x48 = path->pLast_0x04;
                    state->flPathLength_0x50 = path->flLength_0x08;

                    if (spec->flX_0x04 == 0.0f) {
                        targetDist = ((float)rand() / 32768.0f) * state->flPathLength_0x50;
                    } else if (spec->nReverse_0x03 == 0) {
                        targetDist = spec->flX_0x04;
                    } else {
                        targetDist = state->flPathLength_0x50 - spec->flX_0x04;
                    }

                    node = state->pPathFirst_0x44;
                    while (node->flLength_0x00 <= targetDist) {
                        targetDist -= node->flLength_0x00;
                        node++;
                        if (node->flLength_0x00 == 0.0f) {
                            node = state->pPathFirst_0x44;
                        }
                    }

                    state->nNodeFlags_0x60 = node->nFlags_0x14;
                    while ((state->nNodeFlags_0x60 & 0xfff) != 0) {
                        if (spec->nReverse_0x03 == 0) {
                            if (node == state->pPathFirst_0x44) {
                                node = state->pPathLast_0x48;
                            } else {
                                node--;
                            }
                            targetDist = node->flLength_0x00;
                        } else {
                            targetDist = 0.0f;
                            if (node == state->pPathLast_0x48) {
                                node = state->pPathFirst_0x44;
                            } else {
                                node++;
                            }
                        }
                        state->nNodeFlags_0x60 = node->nFlags_0x14;
                    }

                    state->flPathPos_0x54 = targetDist;
                    state->pPathNode_0x4c = node;
                    state->nKindId_0x38 = spec->nKindId_0x00;
                    state->nKind_0x3c = spec->nKind_0x02;
                    state->nReverse_0x40 = spec->nReverse_0x03;

                    advancePedPathPos_8c0289ac(state);
                    GroundQueryFindPolygon_8c020914(state->sprite_0x00.p.x, state->sprite_0x00.p.y, state->sprite_0x00.p.z,
                            &state->aGround_0x28);
                    GroundProbeInterpolateHeight_8c020f7e(&state->aGround_0x28, (float *)state);
                } else {
                    pushFailed = 1;
                }
            } else if (spec->nKind_0x02 == 2) {
                if (TaskPush_8c014ae8(task->subTasks_0x18, pedStaticObjectTask_8c02903e,
                        &subTask, (void **)&state, sizeof(PedestrianState))) {
                    state->flBaseX_0x20 = 0.0f;
                    state->flBaseZ_0x24 = 0.0f;
                    state->sprite_0x00.p.x = spec->flX_0x04;
                    state->sprite_0x00.p.z = spec->flZ_0x08;

                    GroundQueryFindPolygon_8c020914(state->sprite_0x00.p.x, 0.0f, state->sprite_0x00.p.z, &state->aGround_0x28);
                    GroundProbeInterpolateHeight_8c020f7e(&state->aGround_0x28, (float *)state);

                    state->nKindId_0x38 = spec->nKindId_0x00;
                    state->nKind_0x3c = spec->nKind_0x02;
                    state->nReverse_0x40 = spec->nReverse_0x03;
                } else {
                    pushFailed = 1;
                }
            }

            if (!pushFailed) {
                state->sprite_0x00.sx = 0.015f;
                state->sprite_0x00.sy = 0.015f;
                state->sprite_0x00.ang = 0;

                if (var_pedestrianAssets_8c1bbfdc[state->nKindId_0x38].texlist_0x08 == (NJS_TEXLIST *)-1) {
                    TaskFree_8c014b66(subTask);
                } else {
                    state->sprite_0x00.tlist = var_pedestrianAssets_8c1bbfdc[state->nKindId_0x38].texlist_0x08;
                    state->sprite_0x00.tanim = init_pedestrianTexAnims_8c04623c;
                }
            }
        }

        task->spec_0x1c = spec + 1;
    }

    TaskExecGroup_8c014b42(task->subTasks_0x18);
}

/* Per-frame driver for every pedestrian group: syncs var_pedGroups_8c228230's
 * wanted/active state with the current preset (task->debounce_0x0c debounces a
 * single pending-change flag across frames where var_busState_8c1bb9d0.scenePresetIds_0x3bc
 * reads 0), rebuilds this frame's crosswalk intersection scratch, runs every
 * group's Task, then registers the draw callback(s) -- drawPedestriansMirror_8c028a38
 * only outside demo mode. */
STATIC void pedestriansTask_8c0293f6(PedestriansTask *task)
{
    PedGroupEntry *groups;
    PedGroupEntry *group;
    PedGroupDef *groupDef;
    PedPathInfo *paths;
    PedGroupTask *subTask;
    void *state;
    Task *subtasks;
    int *list;
    PedPathNode *node;
    int *cursor;
    int presetField;
    int pageListIndex;
    int i;
    Bool isDemo;
    int layer;
    DrawCallback1 fn;

    if (var_runState_8c2285c4.runPhase_0x00 == 0) {
        return;
    }

    var_activeGroundGrid_8c2264d4 = var_currentCourse_8c1bb868.atariHum_0x28;

    presetField = var_busState_8c1bb9d0.scenePresetIds_0x3bc & 0xff0000;
    if (task->debounce_0x0c == 0) {
        if (presetField == 0) {
            task->debounce_0x0c = 1;
        }
    } else if (presetField != 0) {
        var_activePedPreset_8c22822c = (int)(short)(presetField >> 16);
        task->debounce_0x0c = 0;
    }

    groups = (PedGroupEntry *)var_pedGroups_8c228230;

    if (task->lastPreset_0x08 != var_activePedPreset_8c22822c) {
        task->lastPreset_0x08 = var_activePedPreset_8c22822c;

        for (i = 0; i < var_pedGroupCount_8c228234; i++) {
            groups[i].wanted_0x04 = 0;
        }

        list = ((int **)var_pedGroupLists_8c228240)[var_activePedPreset_8c22822c];
        for (; *list != -1; list++) {
            pageListIndex = *list;
            group = &groups[pageListIndex];

            if (group->active_0x00 == 0) {
                /* One slot over PED_GROUP_SLOTS: TaskClear_8c014a9c terminates
                 * the array with a NULL action past the slots it frees. */
                subtasks = syMalloc((PED_GROUP_SLOTS + 1) * sizeof(Task));
                if (subtasks == NULL) {
                    break;
                }
                group->list_0x08 = subtasks;
                TaskClear_8c014a9c(subtasks, PED_GROUP_SLOTS);

                if (!TaskPush_8c014ae8(var_tasks_8c1ba808, pedGroupTask_8c029078,
                        (Task **)&subTask, &state, 0)) {
                    syFree(subtasks);
                    break;
                }

                for (groupDef = (PedGroupDef *)var_pedGroupDefs_8c22823c;
                        groupDef->id_0x00 != pageListIndex; groupDef++) {
                }

                subTask->subTasks_0x18 = subtasks;
                subTask->spec_0x1c = groupDef->specs_0x08;
                subTask->pageListIndex_0x08 = pageListIndex;
                subTask->radius_0x10 = groupDef->radius_0x04;
                group->active_0x00 = 1;
            }

            group->wanted_0x04 = 1;
        }
    }

    CollisionQueueReset_8c02e486();
    SignalClearPedCrossingFlags_8c02890c();

    njCalcPoint(&var_busState_8c1bb9d0.worldMatrix_0x084, &init_stopLineLocalA_8c04650c, &var_groundQueryPoint_8c1bc460);
    var_stopLinePointA_8c228268[0] = var_groundQueryPoint_8c1bc460.x;
    var_stopLinePointA_8c228268[1] = var_groundQueryPoint_8c1bc460.z;

    njCalcPoint(&var_busState_8c1bb9d0.worldMatrix_0x084, &init_stopLineLocalB_8c046518, &var_groundQueryPoint_8c1bc460);
    var_stopLinePointB_8c228270[0] = var_groundQueryPoint_8c1bc460.x;
    var_stopLinePointB_8c228270[1] = var_groundQueryPoint_8c1bc460.z;

    var_crosswalkTableEnd_8c228244 = var_crosswalkTable_8c228248;
    paths = (PedPathInfo *)var_pedPaths_8c228238;

    for (pageListIndex = 0; pageListIndex < var_pedGroupCount_8c228234; pageListIndex++) {
        if (groups[pageListIndex].active_0x00 != 0) {
            for (node = paths[pageListIndex].pFirst_0x00; node->flLength_0x00 != 0.0f; node++) {
                if ((node->nFlags_0x14 & 0xfff) != 0 &&
                        IntersectSegments_8c0206f0(var_stopLinePointA_8c228268, var_stopLinePointB_8c228270,
                                &node->flBaseX_0x04, &node[1].flBaseX_0x04,
                                &var_crossingIntersectPoint_8c1bc458)) {
                    cursor = var_crosswalkTableEnd_8c228244;
                    var_crosswalkTableEnd_8c228244 = cursor + 1;
                    *cursor = (int)node;

                    cursor = var_crosswalkTableEnd_8c228244;
                    var_crosswalkTableEnd_8c228244 = cursor + 1;
                    *cursor = (int)(node + 1);
                }
            }
        }
    }

    TaskExecGroup_8c014b42(var_tasks_8c1ba808);

    isDemo = var_playMode_8c1bb8d0 == PLAY_MODE_DEMO;
    if (isDemo) {
        fn = drawPedestrians_8c028b74;
    } else {
        RenderPushCall1_8c0223ea(0, drawPedestrians_8c028b74, 0);
        fn = drawPedestriansMirror_8c028a38;
    }
    layer = !isDemo;
    RenderPushCall1_8c0223ea(layer, fn, layer);

    if (var_runState_8c2285c4.stopPhase_0x20 == 2) {
        RenderPushCall1_8c0223ea(0, StopDrawWaitingPassengers_8c02d06c, 0);
        RenderPushCall1_8c0223ea(1, StopDrawWaitingPassengers_8c02d06c, 1);
    }
}

/* One-shot pedestrian-group setup for the current run, called from
 * GameEnterDrive_8c01306e. Copies the route's pedestrian tables (loaded by
 * loadRouteModels_8c014088 into var_currentCourse_8c1bb868's Hum fields) into
 * var_pedPaths_8c228238/var_pedGroupDefs_8c22823c/var_pedGroupLists_8c228240
 * and installs pedestriansTask_8c0293f6 to drive them every frame. No-op if
 * the route defines no pedestrian groups. */
void ObjectsInitPedestrianGroups_8c0296d6(void)
{
    PedGroupEntry *groups;
    int **lists;
    int i, j;
    PedestriansTask *task;
    void *state;

    var_pedPaths_8c228238 = var_currentCourse_8c1bb868.lineHum_0x2c;
    var_pedGroupLists_8c228240 = var_currentCourse_8c1bb868.macHumM0_0x34;
    var_pedGroupDefs_8c22823c = var_currentCourse_8c1bb868.macHumG0_0x30;

    lists = (int **)var_pedGroupLists_8c228240;
    var_pedGroupCount_8c228234 = -1;
    for (i = 0; lists[i] != NULL; i++) {
        for (j = 0; lists[i][j] != -1; j++) {
            if (var_pedGroupCount_8c228234 < lists[i][j]) {
                var_pedGroupCount_8c228234 = lists[i][j];
            }
        }
    }

    if (var_pedGroupCount_8c228234 < 0) {
        CollisionQueueReset_8c02e486();
        return;
    }

    var_pedGroupCount_8c228234++;
    groups = (PedGroupEntry *)syMalloc(var_pedGroupCount_8c228234 * sizeof(PedGroupEntry));
    var_pedGroups_8c228230 = groups;
    for (i = 0; i < var_pedGroupCount_8c228234; i++) {
        groups[i].active_0x00 = 0;
    }

    TaskPush_8c014ae8(var_tasks_8c1ba5e8, pedestriansTask_8c0293f6, (Task **)&task, &state, 0);
    task->lastPreset_0x08 = -1;
    task->debounce_0x0c = 1;
}

/* Teardown counterpart to ObjectsInitPedestrianGroups_8c0296d6: frees every
 * still-active group's subtask block (see pedestriansTask_8c0293f6, which
 * stashes it in PedGroupEntry.list_0x08) and the group array itself, then
 * resets to the not-yet-loaded
 * sentinel state. No-op if groups were never loaded this run. */
void ObjectsFreePedestrianGroups_8c0297da(void)
{
    PedGroupEntry *groups;
    int i;

    if (var_pedGroupCount_8c228234 < 0) {
        return;
    }

    groups = (PedGroupEntry *)var_pedGroups_8c228230;
    for (i = 0; i < var_pedGroupCount_8c228234; i++) {
        if (groups[i].active_0x00 != 0) {
            TaskFreeGroup_8c014ab4(groups[i].list_0x08);
            syFree(groups[i].list_0x08);
        }
    }

    syFree(var_pedGroups_8c228230);
    var_pedGroups_8c228230 = (void *)-1;
    var_pedGroupCount_8c228234 = -1;
}

/* Task private state for routeBlinkerTask_8c029904, set up by
 * ObjectsInitBlinkers_8c029920. */
typedef struct {
    TaskAction action;
    void *state;            /* NJS_MATRIX* on the callback */
    int count_0x08;          /* number of NJS_MATRIX entries in state */
    int blinkCounter_0x0c;   /* per-frame counter, round-tripped through Task's void* field_0x0c */
    int field_0x10;
    int field_0x14;
    void *queuedItem_0x18;
    int field_0x1c;
} RouteBlinkerTask;

STATIC void resolveObjectChildren_8c029868(NJS_OBJECT **nodes)
{
    NJS_OBJECT *child = nodes[0]->child;
    nodes[1] = child;
    child = child->sibling;
    nodes[2] = child;
    nodes[3] = child->sibling;
}

/* Draws all `count` blinker matrices built by ObjectsInitBlinkers_8c029920,
 * cycling var_routeBlinkerNodes_8c228278's child nodes' eval flags through a 3-phase blink. */
STATIC void drawBlinkers_8c029878(int taskArg, int matricesArg)
{
    RouteBlinkerTask *task = (RouteBlinkerTask *)taskArg;
    NJS_MATRIX *matrix = (NJS_MATRIX *)matricesArg;
    int count = task->count_0x08;
    int phase = (task->blinkCounter_0x0c >> 2) % 3;
    int i;

    switch (phase) {
    case 0:
        var_routeBlinkerNodes_8c228278[1]->evalflags = 0x3f;
        var_routeBlinkerNodes_8c228278[2]->evalflags = 0x3f;
        var_routeBlinkerNodes_8c228278[3]->evalflags = 0x37;
        break;
    case 1:
        var_routeBlinkerNodes_8c228278[1]->evalflags = 0x3f;
        var_routeBlinkerNodes_8c228278[2]->evalflags = 0x37;
        var_routeBlinkerNodes_8c228278[3]->evalflags = 0x3f;
        break;
    case 2:
        var_routeBlinkerNodes_8c228278[1]->evalflags = 0x37;
        var_routeBlinkerNodes_8c228278[2]->evalflags = 0x3f;
        var_routeBlinkerNodes_8c228278[3]->evalflags = 0x3f;
        break;
    default:
        break;
    }

    for (i = 0; i < count; i++) {
        njSetCamera(var_drawCamera_8c226558);
        njMultiMatrix(0, matrix);
        njCnkSimpleDrawObject((NJS_CNK_OBJECT *)var_routeBlinkerNodes_8c228278[0]);
        matrix++;
    }
}

/* Per-frame TaskAction for the blinker group: advances the blink counter and
 * queues this frame's draw through the fade command pipeline. */
STATIC void routeBlinkerTask_8c029904(RouteBlinkerTask *task, NJS_MATRIX *state)
{
    task->blinkCounter_0x0c++;
    RenderPushCall2_8c022420(0, drawBlinkers_8c029878, (int)task, (int)state);
}

/* One-shot setup for the current run's route blinkers, called from
 * GameEnterDrive_8c01306e. Resolves var_routeBlinkerNodes_8c228278 from the route model
 * table (mirroring ObjectsInitPedestrianGroups_8c0296d6's sibling setup calls)
 * and installs routeBlinkerTask_8c029904 to draw them every frame. No-op if
 * the route's table is empty.
 *
 * Each point's height uses its own ground-query fallback, which differs
 * from snapPointToGround_8c02840c's at the tail. */
void ObjectsInitBlinkers_8c029920(void)
{
    const RouteMarkerPoint *p, *points;
    int count;
    RouteBlinkerTask *task;
    NJS_MATRIX *matrices;

    var_routeBlinkerNodes_8c228278[0] = (NJS_OBJECT *)((int *)var_routeModels_8c1bc3ec)[9];
    resolveObjectChildren_8c029868(var_routeBlinkerNodes_8c228278);

    switch (var_route_8c18ad1c) {
    case ROUTE_WANGAN:
        points = init_wanganBlinkerPoints_8c0465d8;
        break;
    case ROUTE_SHINJUKU:
        points = init_shinjukuBlinkerPoints_8c046524;
        break;
    case ROUTE_OME:
        points = init_omeBlinkerPoints_8c0466a4;
        break;
    }

    count = 0;
    for (p = points; p->x_0x00 != 0.0f; p++) {
        count++;
    }

    if (count == 0) {
        return;
    }

    TaskPush_8c014ae8(var_tasks_8c1ba5e8, &routeBlinkerTask_8c029904, (Task **)&task,
                       (void **)&matrices, count * sizeof(NJS_MATRIX));
    task->count_0x08 = count;
    task->blinkCounter_0x0c = 0;

    for (p = points; p->x_0x00 != 0.0f; p++) {
        GroundQueryResult result;

        var_groundQueryPoint_8c1bc460.x = p->x_0x00;
        var_groundQueryPoint_8c1bc460.z = p->z_0x04;

        var_activeGroundGrid_8c2264d4 = var_currentCourse_8c1bb868.atariHum_0x28;
        GroundQueryFindPolygon_8c020914(var_groundQueryPoint_8c1bc460.x, var_groundQueryPoint_8c1bc460.y, var_groundQueryPoint_8c1bc460.z, &result);
        if (result.count_0x0c == 0) {
            var_activeGroundGrid_8c2264d4 = var_currentCourse_8c1bb868.atariBus_0x04;
            GroundQueryFindPolygon_8c020914(var_groundQueryPoint_8c1bc460.x, var_groundQueryPoint_8c1bc460.y, var_groundQueryPoint_8c1bc460.z, &result);
            if (result.count_0x0c != 0) {
                GroundProbeInterpolateHeight_8c020f7e(&result, (float *)&var_groundQueryPoint_8c1bc460);
            } else {
                var_groundQueryPoint_8c1bc460.y = var_busState_8c1bb9d0.posY_0x0f8;
            }
        }

        njUnitMatrix(matrices);
        njTranslate(matrices, var_groundQueryPoint_8c1bc460.x, var_groundQueryPoint_8c1bc460.y, var_groundQueryPoint_8c1bc460.z);
        njRotateY(matrices, (int)((float)p->angleDeg_0x08 * 65536.0f / 360.0f));
        matrices++;
    }
}
void ObjectsClearAssetRequestTable_8c029acc(void)
{
    var_assetRequestTable_8c228408 = (int *)-1;
}

void ObjectsStartAssetRequests_8c029ad4(int *table)
{
    AssetRequestSlot *slots;
    AssetRequestSlot *slot;
    int rowType;
    char *name;
    int rowIndex;
    int *data;

    if (table == NULL) {
        ObjectsFreeAssetRequests_8c029cfe();
        return;
    }
    if (var_assetRequestTable_8c228408 == table) {
        return;
    }

    ObjectsFreeAssetRequests_8c029cfe();
    slots = (AssetRequestSlot *)var_assetRequestSlots_8c228288;
    rowIndex = 0;
    var_assetRequestTable_8c228408 = table;
    for (; *table != -1; table = table + 2) {
        rowType = *table;
        slot = &slots[rowIndex];
        if (rowType == 0) {
            name = *(char **)(table[1] + 0xc);
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, name, &slot->dat_0x08, 0);
        }
        else if (rowType == 1) {
            data = (int *)table[1];
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, (void *)data[0], &slot->nj_0x04, 0);
            AsqRequestPvm_8c011ac0(var_commonDirCopy_8c18ad8c, (void *)data[1], &slot->pvm_0x00,
                data[2], 0);
            AsqRequestDat_8c011182(var_commonDirCopy_8c18ad8c, (void *)data[3], &slot->dat_0x08);
        }
        else if (rowType == 2) {
            data = (int *)table[1];
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, (void *)data[0], &slot->nj_0x04, 0);
            AsqRequestPvm_8c011ac0(var_commonDirCopy_8c18ad8c, (void *)data[1], &slot->pvm_0x00,
                data[2], 0);
            slot->sceneGate_0x14 = ((Sint8 *)data)[0xc];
            slot->taskFlag_0x15 = ((Sint8 *)data)[0xd];
            slot->fogEnable_0x16 = ((Uint8 *)data)[0xe];
            slot->control3DEnable_0x17 = ((Uint8 *)data)[0xf];
        }
        else if (rowType == 3) {
            data = (int *)table[1];
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, (void *)data[0], &slot->nj_0x04, 0);
            AsqRequestPvm_8c011ac0(var_commonDirCopy_8c18ad8c, (void *)data[1], &slot->pvm_0x00,
                data[2], 0);
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, (void *)data[3], &slot->dat_0x08, 0);
            slot->sceneGate_0x14 = ((Sint8 *)data)[0x10];
            slot->taskFlag_0x15 = ((Sint8 *)data)[0x11];
            slot->fogEnable_0x16 = ((Uint8 *)data)[0x12];
            slot->control3DEnable_0x17 = ((Uint8 *)data)[0x13];
        }
        else if (rowType == 4) {
            data = (int *)table[1];
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, (void *)data[0], &slot->nj_0x04, 0);
            AsqRequestPvm_8c011ac0(var_commonDirCopy_8c18ad8c, (void *)data[1], &slot->pvm_0x00,
                data[2], 0);
            slot->taskFlag_0x15 = (Sint8)data[3];
        }
        else if (rowType == 5) {
            data = (int *)table[1];
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, (void *)data[0], &slot->nj_0x04, 0);
            AsqRequestPvm_8c011ac0(var_commonDirCopy_8c18ad8c, (void *)data[1], &slot->pvm_0x00,
                data[2], 0);
            slot->pos_0x0c = *(ObjectAssetType5Extra *)((Uint8 *)data + 0xc);
        }
        else if (rowType == 6) {
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, "O_FUMI_00.njd", &var_fumiGateModel_8c22840c, 0);
            AsqRequestPvm_8c011ac0(var_commonDirCopy_8c18ad8c, "O_FUMI.pvm", &var_fumiTexlist_8c228410, 2, 0);
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, "O_FUMI_00.njm", &var_fumiCloseMotion_8c228414, 0);
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, "O_FUMI_01.njm", &var_fumiOpenMotion_8c228418, 0);
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, "O_FUMI_LAMP.njd", &var_fumiLampModel_8c22841c, 0);
            AsqRequestPvm_8c011ac0(var_commonDirCopy_8c18ad8c, "O_FUMI_LAMP.pvm", &var_fumiLampTexlist_8c228420, 0x10, 0);
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, "od_chu00.njd", &var_fumiTrainModel_8c228424, 0);
            AsqRequestPvm_8c011ac0(var_commonDirCopy_8c18ad8c, "od_chu00.pvm", &var_fumiTrainTexlist_8c228428, 0x10, 0);
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, "o01_tra0.njm", &var_fumiTrainMotionA_8c22842c, 0);
            AsqRequestNj_8c011492(var_commonDirCopy_8c18ad8c, "o01_tra1.njm", &var_fumiTrainMotionB_8c228430, 0);
        }
        rowIndex = rowIndex + 1;
    }
}
/* Frees every asset handle written into var_assetRequestSlots_8c228288 by a prior
 * ObjectsStartAssetRequests_8c029ad4(table), mirroring its per-row type
 * switch. No-op if no table is currently in flight. */
void ObjectsFreeAssetRequests_8c029cfe(void)
{
    AssetRequestSlot *slots;
    AssetRequestSlot *slot;
    int *entry;
    int rowIndex;
    int type;

    if (var_assetRequestTable_8c228408 == (int *)-1) {
        return;
    }

    slots = (AssetRequestSlot *)var_assetRequestSlots_8c228288;
    rowIndex = 0;
    for (entry = var_assetRequestTable_8c228408; *entry != -1; entry += 2) {
        type = *entry;
        slot = &slots[rowIndex];

        if (type == 0) {
            syFree(slot->dat_0x08);
        } else if (type == 1 || type == 3) {
            syFree(slot->nj_0x04);
            AsqReleaseAndFreeTexlist_8c011e3c(slot->pvm_0x00);
            syFree(slot->dat_0x08);
        } else if (type == 2 || type == 4 || type == 5) {
            syFree(slot->nj_0x04);
            AsqReleaseAndFreeTexlist_8c011e3c(slot->pvm_0x00);
        } else if (type == 6) {
            syFree(var_fumiGateModel_8c22840c);
            AsqReleaseAndFreeTexlist_8c011e3c(var_fumiTexlist_8c228410);
            syFree(var_fumiCloseMotion_8c228414);
            syFree(var_fumiOpenMotion_8c228418);
            syFree(var_fumiLampModel_8c22841c);
            AsqReleaseAndFreeTexlist_8c011e3c(var_fumiLampTexlist_8c228420);
            syFree(var_fumiTrainModel_8c228424);
            AsqReleaseAndFreeTexlist_8c011e3c(var_fumiTrainTexlist_8c228428);
            syFree(var_fumiTrainMotionA_8c22842c);
            syFree(var_fumiTrainMotionB_8c228430);
        }

        rowIndex = rowIndex + 1;
    }

    var_assetRequestTable_8c228408 = (int *)-1;
}
/* DrawCallback1 draw callback for the fly-by model spawned by flyByModelTask_8c029e68;
 * state is the spawned task's state array (texlist at 0x40, model at 0x44,
 * motion at 0x48, current frame at 0x58, per rowFlyByTask_8c029e94/flyByModelTask_8c029e68). */
STATIC void drawFlyByModel_8c029e46(int state)
{
    RowTaskState *st = (RowTaskState *)state;

    njSetTexture(st->texlist_0x40);
    njCnkSimpleDrawMotion(st->model_0x44, (NJS_MOTION *)st->dat_0x48, st->frame_0x58);
}
/* TaskAction spawned by rowFlyByTask_8c029e94 once its countdown lapses; animates a
 * randomly-chosen route model's fly-by until its frame count runs out. */
STATIC void flyByModelTask_8c029e68(Task *task, RowTaskState *state)
{
    state->frame_0x58 = state->frame_0x58 + 1.0f;
    if (state->frameLimit_0x5c <= state->frame_0x58) {
        TaskFree_8c014b66(task);
        return;
    }
    RenderPushCall1_8c0223ea(0, drawFlyByModel_8c029e46, (int)state);
}
/* TaskAction for a type-0 row, installed by ObjectsPushTasks_8c02a6ac. Counts
 * down task->field_0x08; once it lapses, spawns a fly-by task (flyByModelTask_8c029e68)
 * seeded from a random route model slot and re-arms the countdown. */
STATIC void rowFlyByTask_8c029e94(Task *task, RowTaskState *state)
{
    Task *newTask;
    RowTaskState *newState;
    int idx;

    if (--task->field_0x08 < 0) {
        if (TaskPush_8c014ae8(var_tasks_8c1bb448, &flyByModelTask_8c029e68, &newTask,
                (void **)&newState, sizeof(RowTaskState))) {
            do {
                idx = AsqGetRandomInRangeA_8c012178(0xd) * 2 + 1;
            } while (var_routeModelSlots_8c1bbddc[idx].texlist_0x08 == (NJS_TEXLIST *)-1);
            newState->texlist_0x40 = var_routeModelSlots_8c1bbddc[idx].texlist_0x08;
            newState->model_0x44 = var_routeModelSlots_8c1bbddc[idx].nj_0x0c;
            newState->dat_0x48 = state->dat_0x48;
            newState->frame_0x58 = 0.0f;
            newState->frameLimit_0x5c = state->frameLimit_0x5c;
        }
        task->field_0x08 = AsqGetRandomInRangeA_8c012178(0x30) + 0x10;
    }
}
/* DrawCallback1 draw callback for a type-1 row task; state is the spawned
 * task's state (see RowTaskState), with the DatBlob at RowTaskState::dat_0x48. */
STATIC void drawDatModel_8c029f2a(int state)
{
    DatBlob *dat = (DatBlob *)((RowTaskState *)state)->dat_0x48;
    njSetTexture(dat->texlist_0x18);
    njCnkSimpleDrawObject(dat->model_0x1c);
}
STATIC void initDatBlob_8c029f42(DatBlob *dat, NJS_TEXLIST *texlist, NJS_CNK_OBJECT *model)
{
    dat->step_0x0c = 0;
    dat->tick_0x10 = 0;
    dat->texlist_0x18 = texlist;
    dat->model_0x1c = model;
    dat->masks_0x14 = dat->masks_0x20;
}
/* Returns 0 if the model's child chain ran out before nNodes_0x00 children,
 * leaving the step unadvanced. */
STATIC int advanceDatBlob_8c029f54(DatBlob *dat)
{
    int stepIndex;
    int tick;

    if (dat->tick_0x10 == 0) {
        NJS_CNK_OBJECT *node;
        int nodeIndex;

        node = dat->model_0x1c->child;
        for (nodeIndex = 1; nodeIndex < dat->nNodes_0x00; nodeIndex++) {
            if ((dat->masks_0x14[dat->step_0x0c] & (1 << nodeIndex)) == 0) {
                node->evalflags = 0x3e;
            }
            else {
                node->evalflags = 0x36;
            }
            if (node->sibling == NULL) {
                if (nodeIndex != dat->nNodes_0x00 - 1) {
                    return 0;
                }
            }
            else {
                node = node->sibling;
            }
        }
        stepIndex = dat->step_0x0c;
        dat->step_0x0c = stepIndex + 1;
        if (dat->stepCount_0x08 <= stepIndex + 1) {
            dat->step_0x0c = 0;
        }
    }
    tick = dat->tick_0x10;
    dat->tick_0x10 = tick + 1;
    if (dat->framesPerStep_0x04 <= tick + 1) {
        dat->tick_0x10 = 0;
    }
    return 1;
}
/* TaskAction for a type-1 row, installed by ObjectsPushTasks_8c02a6ac. */
STATIC void rowDatTask_8c029fcc(Task *task, RowTaskState *state)
{
    advanceDatBlob_8c029f54((DatBlob *)state->dat_0x48);
    RenderPushCall1_8c0223ea(0, drawDatModel_8c029f2a, (int)state);
    RenderPushCall1_8c0223ea(1, drawDatModel_8c029f2a, (int)state);
}
/* DrawCallback1 draw callback for a type-2 row task; state is the spawned
 * task's state array. Draws the {texlist, model} pair at offset 0x40/0x44,
 * with fog and njControl3D left toggled off for the draw when the row's
 * flags at 0x78/0x79 are clear. */
STATIC void drawRowModel_8c02a048(int state)
{
    RowTaskState *st = (RowTaskState *)state;

    if (st->control3DEnable_0x79 == 0) {
        njControl3D(0);
    }
    if (st->fogEnable_0x78 == 0) {
        njFogDisable();
    }
    njSetTexture(st->texlist_0x40);
    njCnkSimpleDrawObject(st->model_0x44);
    njFogEnable();
    njControl3D(0x100);
}
/* TaskAction for a type-2 row, installed by ObjectsPushTasks_8c02a6ac;
 * task->field_0x08 is the target var_busState_8c1bb9d0.scenePresetIds_0x3bc marker, set from
 * row[0x14] there. */
STATIC void rowModelTask_8c02a08a(Task *task, RowTaskState *state)
{
    if (state->phase_0x54 == 0) {
        if (task->field_0x08 == (int)(var_busState_8c1bb9d0.scenePresetIds_0x3bc & 0xff000000)) {
            state->phase_0x54 = state->phase_0x54 + 1;
        }
    } else if (state->phase_0x54 == 1) {
        RenderPushCall1_8c0223ea(0, drawRowModel_8c02a048, (int)state);
        RenderPushCall1_8c0223ea(1, drawRowModel_8c02a048, (int)state);
    }
}
/* DrawCallback1 draw callback for a type-3 row task; state is the spawned
 * task's state array, laid out like drawFlyByModel_8c029e46's (texlist 0x40,
 * model 0x44, motion 0x48, current frame 0x58) with drawRowModel_8c02a048's
 * fog/njControl3D toggles (flags at 0x78/0x79). */
STATIC void drawRowMotionModel_8c02a0d6(int state)
{
    RowTaskState *st = (RowTaskState *)state;

    if (st->control3DEnable_0x79 == 0) {
        njControl3D(0);
    }
    if (st->fogEnable_0x78 == 0) {
        njFogDisable();
    }
    njSetTexture(st->texlist_0x40);
    njCnkSimpleDrawMotion(st->model_0x44, (NJS_MOTION *)st->dat_0x48, st->frame_0x58);
    njFogEnable();
    njControl3D(0x100);
}
/* TaskAction for a type-3 row, installed by ObjectsPushTasks_8c02a6ac.
 * task->field_0x08 is the target marker, per type-2's rowModelTask_8c02a08a;
 * state->frameLimit_0x5c is set like type-0's flyByModelTask_8c029e68's. */
STATIC void rowMotionModelTask_8c02a120(Task *task, RowTaskState *state)
{
    if (state->phase_0x54 == 0) {
        if (task->field_0x08 == (int)(var_busState_8c1bb9d0.scenePresetIds_0x3bc & 0xff000000)) {
            state->phase_0x54 = state->phase_0x54 + 1;
        }
    } else if (state->phase_0x54 == 1) {
        state->frame_0x58 = state->frame_0x58 + 1.0f;
        if (state->frame_0x58 < state->frameLimit_0x5c) {
            RenderPushCall1_8c0223ea(0, drawRowMotionModel_8c02a0d6, (int)state);
            return;
        }
        if (task->field_0x0c == 0) {
            TaskFree_8c014b66(task);
            return;
        }
        state->frame_0x58 = 0.0f;
    }
}
/* Draws a {texlist, model} pair embedded at offset 0x40 in state, fog
 * left enabled only when fogEnable is non-zero. */
STATIC void drawRowSimpleModel_8c02a1b2(int state, int fogEnable)
{
    RowTaskState *st = (RowTaskState *)state;

    njControl3D(0);
    if (fogEnable == 0) {
        njFogDisable();
    }
    njSetTexture(st->texlist_0x40);
    njCnkSimpleDrawObject(st->model_0x44);
    njFogEnable();
    njControl3D(0x100);
}
/* TaskAction for a type-4 row, installed by ObjectsPushTasks_8c02a6ac. */
STATIC void rowSimpleModelTask_8c02a1f0(Task *task, RowTaskState *state)
{
    RenderPushCall2_8c022420(0, drawRowSimpleModel_8c02a1b2, (int)state, task->field_0x08);
}
/* DrawCallback1 draw callback for a type-5 row task; state is the spawned
 * task's state array. Unlike drawRowModel_8c02a048, fades the whole draw via
 * a constant material color (alpha at 0x68, set by rowMaterialModelTask_8c02a27c) instead of
 * toggling fog/lighting per row flags. */
STATIC void drawRowMaterialModel_8c02a206(int state)
{
    RowTaskState *st = (RowTaskState *)state;

    njControl3D(0x920);
    njSetConstantAttr(0xffffffff, 0x800);
    njSetConstantMaterial(&st->material_0x68);
    njSetTexture(st->texlist_0x40);
    njCnkSimpleDrawObject(st->model_0x44);
    njControl3D(0x100);
}
/* TaskAction for a type-5 row, installed by ObjectsPushTasks_8c02a6ac. Fades
 * the row's material to invisible beyond a distance of 20 from the bus
 * (averaging var_busState_8c1bb9d0's two position pairs against the row's
 * own position, per ObjectAssetType5Extra), ramping from opaque at 10 down
 * to hidden at 20 within that band, gated by var_cameraMode_8c227d9c (some quality/LOD
 * level) being at least 2; then pushes two draw calls (opaque and
 * translucent draw layers) of the row's model via
 * drawRowMaterialModel_8c02a206. task is unused. */
STATIC void rowMaterialModelTask_8c02a27c(void *task, RowTaskState *state)
{
    ObjectAssetType5Extra *pos = &state->pos_0x4c;

    if (var_cameraMode_8c227d9c < 2) {
        state->material_0x68.a = 0.0f;
    } else {
        float dx = (var_busState_8c1bb9d0.posX_0x0f4 + var_busState_8c1bb9d0.posX_0x2fc) / 2.0f
                   - pos->posX_0x00;
        float dz = (var_busState_8c1bb9d0.posZ_0x0fc + var_busState_8c1bb9d0.posZ_0x304) / 2.0f
                   - pos->posZ_0x04;
        float dist = njSqrt(dx * dx + dz * dz);

        if (dist > 20.0f) {
            state->material_0x68.a = 0.0f;
        } else {
            dist = dist - 10.0f;
            if (dist < 0.0f) {
                dist = 0.0f;
            }
            state->material_0x68.a = dist * 0.05f - 0.5f;
        }
    }
    RenderPushCall1_8c0223ea(0, drawRowMaterialModel_8c02a206, (int)state);
    RenderPushCall1_8c0223ea(1, drawRowMaterialModel_8c02a206, (int)state);
}
/* Resolves 16 grandchildren of nodes[0] into nodes[1..16]: nodes[0]'s two
 * children (child, child->sibling) each contribute 8 nodes -- their own
 * child followed by 7 further siblings. Mirrors resolveObjectChildren_8c029868
 * one level deeper. */
STATIC void resolveObjectGrandchildren_8c02a322(NJS_OBJECT **nodes)
{
    NJS_OBJECT *node;
    int i;

    node = nodes[0]->child->child;
    nodes[1] = node;
    for (i = 2; i <= 8; i++) {
        node = node->sibling;
        nodes[i] = node;
    }

    node = nodes[0]->child->sibling->child;
    nodes[9] = node;
    for (i = 10; i <= 16; i++) {
        node = node->sibling;
        nodes[i] = node;
    }
}
/* Sets nodes[1..16]'s eval flags in a 3-phase pattern selected by
 * `selector`, one level deeper than drawBlinkers_8c029878's 3-phase cycle
 * over resolveObjectGrandchildren_8c02a322's output. Any other selector
 * value is a no-op. */
STATIC void setGrandchildEvalFlags_8c02a370(NJS_OBJECT **nodes, char selector)
{
    switch (selector) {
    case 0:
        nodes[1]->evalflags = 0x37;
        nodes[2]->evalflags = 0x37;
        nodes[3]->evalflags = 0x3f;
        nodes[4]->evalflags = 0x3f;
        nodes[5]->evalflags = 0x3f;
        nodes[6]->evalflags = 0x3f;
        nodes[7]->evalflags = 0x3f;
        nodes[8]->evalflags = 0x3f;
        nodes[9]->evalflags = 0x37;
        nodes[10]->evalflags = 0x37;
        nodes[11]->evalflags = 0x3f;
        nodes[12]->evalflags = 0x3f;
        nodes[13]->evalflags = 0x3f;
        nodes[14]->evalflags = 0x3f;
        nodes[15]->evalflags = 0x3f;
        break;
    case 1:
        nodes[1]->evalflags = 0x3f;
        nodes[2]->evalflags = 0x3f;
        nodes[3]->evalflags = 0x37;
        nodes[4]->evalflags = 0x37;
        nodes[5]->evalflags = 0x3f;
        nodes[6]->evalflags = 0x3f;
        nodes[7]->evalflags = 0x3f;
        nodes[8]->evalflags = 0x37;
        nodes[9]->evalflags = 0x3f;
        nodes[10]->evalflags = 0x3f;
        nodes[11]->evalflags = 0x37;
        nodes[12]->evalflags = 0x37;
        nodes[13]->evalflags = 0x3f;
        nodes[14]->evalflags = 0x3f;
        nodes[15]->evalflags = 0x3f;
        nodes[16]->evalflags = 0x37;
        return;
    case 2:
        nodes[1]->evalflags = 0x3f;
        nodes[2]->evalflags = 0x3f;
        nodes[3]->evalflags = 0x3f;
        nodes[4]->evalflags = 0x3f;
        nodes[5]->evalflags = 0x37;
        nodes[6]->evalflags = 0x37;
        nodes[7]->evalflags = 0x37;
        nodes[8]->evalflags = 0x3f;
        nodes[9]->evalflags = 0x3f;
        nodes[10]->evalflags = 0x3f;
        nodes[11]->evalflags = 0x3f;
        nodes[12]->evalflags = 0x3f;
        nodes[13]->evalflags = 0x37;
        nodes[14]->evalflags = 0x37;
        nodes[15]->evalflags = 0x37;
        break;
    default:
        return;
    }
    nodes[16]->evalflags = 0x3f;
}
/* DrawCallback1 draw callback for a type-6 (FUMI railway crossing) row task,
 * called by fumiCrossingTask_8c02a4f8. Draws the gate, the lamp housing, and --
 * while a passing train is still due (state->trainActive_0x64 > 0) -- the train
 * model stashed at state->model_0x44/state->dat_0x48 (same slots as
 * drawRowMotionModel_8c02a0d6's). */
STATIC void drawFumiCrossing_8c02a47c(int state)
{
    RowTaskState *st = (RowTaskState *)state;
    int phase = st->phase_0x54;

    njSetTexture((NJS_TEXLIST *)var_fumiTexlist_8c228410);
    if (phase == 0 || phase == 1) {
        njCnkSimpleDrawMotion((NJS_CNK_OBJECT *)var_fumiGateModel_8c22840c, (NJS_MOTION *)var_fumiCloseMotion_8c228414,
                              st->frame_0x58);
    } else if (phase == 2 || phase == 3 || phase == 4) {
        njCnkSimpleDrawMotion((NJS_CNK_OBJECT *)var_fumiGateModel_8c22840c, (NJS_MOTION *)var_fumiOpenMotion_8c228418,
                              st->frame_0x58);
    }

    njSetTexture((NJS_TEXLIST *)var_fumiLampTexlist_8c228420);
    njCnkSimpleDrawObject((NJS_CNK_OBJECT *)var_fumiLampModel_8c22841c);

    if (st->trainActive_0x64 > 0.0f) {
        njSetTexture((NJS_TEXLIST *)var_fumiTrainTexlist_8c228428);
        njCnkSimpleDrawMotion(st->model_0x44, (NJS_MOTION *)st->dat_0x48, st->trainFrame_0x60);
    }
}
/* TaskAction for a type-6 (FUMI railway crossing) row, installed by
 * ObjectsPushTasks_8c02a6ac. Cycles the crossing through closing, waiting out
 * a passing train, then opening; idles at phase 0 until
 * var_busState_8c1bb9d0.scenePresetIds_0x3bc's top byte (the set-piece trigger) goes nonzero.
 * From phase 1 onward, pushes a draw call of drawFumiCrossing_8c02a47c every
 * frame. task is unused. */
STATIC void fumiCrossingTask_8c02a4f8(void *task, RowTaskState *state)
{
    int phase = state->phase_0x54;

    if (phase != 0) {
        if (phase == 1) {
            state->frame_0x58 = state->frame_0x58 + 1.0f;
            if (state->frameLimit_0x5c <= state->frame_0x58) {
                state->phase_0x54 = state->phase_0x54 + 1;
                state->frame_0x58 = 0.0f;
                state->frameLimit_0x5c = (float)*(Uint32 *)((Uint8 *)var_fumiOpenMotion_8c228418 + 4) - 1.0f;
            }
        } else if (phase == 2) {
            state->trainFrame_0x60 = state->trainFrame_0x60 + 1.0f;
            if (state->trainActive_0x64 <= state->trainFrame_0x60) {
                state->phase_0x54 = state->phase_0x54 + 1;
                state->trainActive_0x64 = 0.0f;
            }
        } else if (phase == 3) {
            state->frame_0x58 = state->frame_0x58 + 1.0f;
            if (state->frameLimit_0x5c <= state->frame_0x58) {
                state->phase_0x54 = state->phase_0x54 + 1;
                state->frame_0x58 = state->frameLimit_0x5c;
            }
        }
        RenderPushCall1_8c0223ea(0, drawFumiCrossing_8c02a47c, (int)state);
        return;
    }

    if ((var_busState_8c1bb9d0.scenePresetIds_0x3bc & 0xff000000) != 0) {
        state->phase_0x54 = state->phase_0x54 + 1;
        var_trafficSignalFrames_8c227e24[0] = 0;
    }
}
/* Installed as a DrawCallback1 on both draw layers (arg0 = layer). Sets up
 * njCnk's simple light for the row draw calls that follow: direction from
 * var_simpleLightDir_8c2264d8 (layer 0) or var_mirrorSimpleLightDir_8c2264e4
 * (layer 1, the mirror side), and a constant intensity/ambient/color already
 * copied from the scene params by 0222dc_fadecmd. */
STATIC void setSimpleLightCallback_8c02a5d0(int arg0)
{
    float *dir = (arg0 == 0) ? var_simpleLightDir_8c2264d8 : var_mirrorSimpleLightDir_8c2264e4;

    njCnkSetSimpleLight(dir[0], dir[1], dir[2]);
    njCnkSetSimpleLightIntensity(var_simpleLightIntensity_8c2264f0[0], var_simpleLightIntensity_8c2264f0[1]);
    njCnkSetSimpleLightColor(var_simpleLightColor_8c2264f8[0], var_simpleLightColor_8c2264f8[1], var_simpleLightColor_8c2264f8[2]);
}
/* TaskAction spawned once after the table is fully processed, installed by
 * ObjectsPushTasks_8c02a6ac. Once the run is under way, pushes
 * setSimpleLightCallback_8c02a5d0 as a draw callback on both
 * draw layers, then runs the row tasks just spawned into var_tasks_8c1bb448 to
 * completion. */
STATIC void execRowTaskGroupTask_8c02a60e(void)
{
    if (var_runState_8c2285c4.runPhase_0x00 != 0) {
        RenderPushCall1_8c0223ea(0, setSimpleLightCallback_8c02a5d0, 0);
        RenderPushCall1_8c0223ea(1, setSimpleLightCallback_8c02a5d0, 1);
        TaskExecGroup_8c014b42(var_tasks_8c1bb448);
    }
}
/* For each {type, dataPtr} row in the in-flight table (var_assetRequestTable_8c228408, filled by
 * ObjectsStartAssetRequests_8c029ad4), spawns a per-type task seeded from the
 * row's loaded asset handles in var_assetRequestSlots_8c228288, then spawns one closing task once
 * the whole table has been processed. */
void ObjectsPushTasks_8c02a6ac(void)
{
    AssetRequestSlot *slots;
    int *entry;
    int rowIndex;
    Task *task;
    RowTaskState *state;

    if (var_assetRequestTable_8c228408 == (int *)-1) {
        return;
    }

    slots = (AssetRequestSlot *)var_assetRequestSlots_8c228288;

    rowIndex = 0;
    for (entry = var_assetRequestTable_8c228408; *entry != -1; entry += 2) {
        int type = entry[0];
        AssetRequestSlot *row = &slots[rowIndex];
        NJS_TEXLIST *pvm = row->pvm_0x00;
        NJS_CNK_OBJECT *nj = row->nj_0x04;
        void *dat = row->dat_0x08;

        if (type == 0) {
            TaskPush_8c014ae8(var_tasks_8c1bb448, &rowFlyByTask_8c029e94, &task, (void **)&state, sizeof(RowTaskState));
            state->dat_0x48 = dat;
            state->frameLimit_0x5c = (float)*(Uint32 *)((Uint8 *)dat + 4) - 1.0f;
            task->field_0x08 = 0;
        } else if (type == 1) {
            TaskPush_8c014ae8(var_tasks_8c1bb448, &rowDatTask_8c029fcc, &task, (void **)&state, sizeof(RowTaskState));
            initDatBlob_8c029f42((DatBlob *)dat, pvm, nj);
            state->dat_0x48 = dat;
        } else if (type == 2) {
            TaskPush_8c014ae8(var_tasks_8c1bb448, &rowModelTask_8c02a08a, &task, (void **)&state, sizeof(RowTaskState));
            state->phase_0x54 = 0;
            state->texlist_0x40 = pvm;
            state->model_0x44 = nj;
            task->field_0x08 = (int)row->sceneGate_0x14 << 0x18;
            task->field_0x0c = (void *)(int)row->taskFlag_0x15;
            state->fogEnable_0x78 = row->fogEnable_0x16;
            state->control3DEnable_0x79 = row->control3DEnable_0x17;
        } else if (type == 3) {
            TaskPush_8c014ae8(var_tasks_8c1bb448, &rowMotionModelTask_8c02a120, &task, (void **)&state, sizeof(RowTaskState));
            state->phase_0x54 = 0;
            state->texlist_0x40 = pvm;
            state->model_0x44 = nj;
            state->dat_0x48 = dat;
            state->frame_0x58 = 0.0f;
            state->frameLimit_0x5c = (float)*(Uint32 *)((Uint8 *)dat + 4) - 1.0f;
            task->field_0x08 = (int)row->sceneGate_0x14 << 0x18;
            task->field_0x0c = (void *)(int)row->taskFlag_0x15;
            state->fogEnable_0x78 = row->fogEnable_0x16;
            state->control3DEnable_0x79 = row->control3DEnable_0x17;
        } else if (type == 4) {
            TaskPush_8c014ae8(var_tasks_8c1bb448, &rowSimpleModelTask_8c02a1f0, &task, (void **)&state, sizeof(RowTaskState));
            state->texlist_0x40 = pvm;
            state->model_0x44 = nj;
            task->field_0x08 = (int)row->taskFlag_0x15;
        } else if (type == 5) {
            TaskPush_8c014ae8(var_tasks_8c1bb448, &rowMaterialModelTask_8c02a27c, &task, (void **)&state, sizeof(RowTaskState));
            state->texlist_0x40 = pvm;
            state->model_0x44 = nj;
            state->pos_0x4c = row->pos_0x0c;
            state->material_0x68.b = 0.0f;
            state->material_0x68.g = 0.0f;
            state->material_0x68.r = 0.0f;
        } else if (type == 6) {
            void *chosen;

            /* Bug-for-bug: passes state itself (its stale value from the
             * previous row), not &state, as the create_state out-param --
             * matches the archived asm exactly. */
            TaskPush_8c014ae8(var_tasks_8c1bb448, &fumiCrossingTask_8c02a4f8, &task, (void **)state, sizeof(RowTaskState));
            state->phase_0x54 = 0;
            state->frame_0x58 = 0.0f;
            state->frameLimit_0x5c = (float)*(Uint32 *)((Uint8 *)var_fumiCloseMotion_8c228414 + 4) - 1.0f;
            state->trainFrame_0x60 = 0.0f;
            chosen = (AsqGetRandomInRangeA_8c012178(2) == 0) ? var_fumiTrainMotionB_8c228430 : var_fumiTrainMotionA_8c22842c;
            state->dat_0x48 = chosen;
            state->trainActive_0x64 = (float)*(Uint32 *)((Uint8 *)chosen + 4) - 1.0f;
            var_fumiLampNodes_8c228434[0] = (NJS_OBJECT *)var_fumiLampModel_8c22841c;
            resolveObjectGrandchildren_8c02a322(var_fumiLampNodes_8c228434);
        }

        rowIndex++;
    }

    TaskPush_8c014ae8(var_tasks_8c1ba5e8, &execRowTaskGroupTask_8c02a60e, &task, (void **)&state, 0);
}
/* Relocation fixup for a freshly-loaded message text dat (s_text.dat /
 * w_text.dat / o_text.dat, see ObjectsRequestMessageAssets_8c02aa36): a
 * 0-terminated array of group
 * offsets, each group an array of (string offset, value) pairs terminated
 * by an entry whose string is empty. Converts every offset to an absolute
 * pointer, in place. */
STATIC void relocateMessageText_8c02a9fc(void *handle)
{
    int *group;
    int *entry;
    int offset;

    for (group = (int *)handle; *group != 0; group++) {
        entry = (int *)(*group + (int)handle);
        *group = (int)entry;
        for (;;) {
            offset = *entry;
            *entry = offset + (int)handle;
            if (*(char *)(offset + (int)handle) == '\0') {
                break;
            }
            entry += 2;
        }
    }
}

/* Drops the message-asset dedup list; the pvm/map handles themselves are owned
 * by the asset queues. */
void ObjectsClearMessageAssets_8c02aa28(void)
{
    var_messageAssetCount_8c228514 = 0;
    var_messageTextDat_8c228518 = (EventLine **) -1;
}
/* Picks the current route's message-text dat (s_text.dat / w_text.dat /
 * o_text.dat) and its per-event slide table, requests the dat, then walks
 * the selected event's slides (var_eventSlides_8c228480[var_selectedEventEntry_8c228478],
 * a 0-terminated {ushort *layers, unused} pair list, each layers array itself
 * 0xffff-terminated) requesting the pvm/map pair for every distinct id seen,
 * deduped into var_messageAssets_8c228484 (count in var_messageAssetCount_8c228514). Bug-for-bug: an
 * unrecognized route leaves var_eventSlides_8c228480 untouched (stale) and skips the
 * dat request entirely, but the dedup walk below still runs against
 * whatever var_eventSlides_8c228480 already held. */
void ObjectsRequestMessageAssets_8c02aa36(void)
{
    EventSlide *slide;
    unsigned short *ids;
    unsigned short id;
    int i;

    if (var_route_8c18ad1c == ROUTE_SHINJUKU) {
        var_eventSlides_8c228480 = init_shinjukuEvents_8c049a6c;
        AsqRequestDat_8c011182(var_commonDir_8c18ad6c, "s_text.dat", &var_messageTextDat_8c228518);
    } else if (var_route_8c18ad1c == ROUTE_WANGAN) {
        var_eventSlides_8c228480 = init_wanganEvents_8c04843c;
        AsqRequestDat_8c011182(var_commonDir_8c18ad6c, "w_text.dat", &var_messageTextDat_8c228518);
    } else if (var_route_8c18ad1c == ROUTE_OME) {
        var_eventSlides_8c228480 = init_omeEvents_8c04a9c8;
        AsqRequestDat_8c011182(var_commonDir_8c18ad6c, "o_text.dat", &var_messageTextDat_8c228518);
    }

    var_messageAssetCount_8c228514 = 0;

    for (slide = var_eventSlides_8c228480[var_selectedEventEntry_8c228478]; (int)slide->layers_0x00 != -1; slide++) {
        for (ids = slide->layers_0x00; *ids != 0xffff; ids++) {
            id = *ids;

            for (i = 0; i < var_messageAssetCount_8c228514 && var_messageAssets_8c228484[i].id_0x00 != (int)id; i++) {
            }

            if (i != var_messageAssetCount_8c228514) {
                continue;
            }

            var_messageAssets_8c228484[i].id_0x00 = id;
            AsqRequestPvm_8c011ac0(var_commonDir_8c18ad6c, init_objectAssetFiles_8c046758[id].pvm_0x04,
                                   &var_messageAssets_8c228484[i].pvm_0x04, 0xde, 0);
            AsqRequestDat_8c011182(var_commonDir_8c18ad6c, init_objectAssetFiles_8c046758[id].map_0x00,
                                   &var_messageAssets_8c228484[i].dat_0x08);
            var_messageAssetCount_8c228514++;
        }
    }
}
/* TaskAction pushed by ObjectsStartMessageBox_8c02ad8c to drive the event
 * message box: swaps in each line, waits (or fast-forwards on held A),
 * advances through the current slide's lines then to the next slide, and
 * finally requests the fade-out/cleanup once the last slide ends. */
STATIC void messageBoxTask_8c02ab7a(Task *task, MessageBoxState *state)
{
    MsgBoxStage stage;
    int i;
    unsigned short *ids;
    unsigned short id;
    char *nextString;

    stage = MSGBOX_STAGE_DRAW;

    switch (state->phase_0x00) {
    case 0:
        state->ids_0x14 = state->slide_0x10->layers_0x00;
        state->line_0x18 = var_messageTextDat_8c228518[state->slide_0x10->lineListIndex_0x04];
        stage = MSGBOX_STAGE_SWAP;
        break;

    case 1:
        stage = MSGBOX_STAGE_SWAP;
        break;

    case 2:
        stage = MSGBOX_STAGE_WAIT;
        break;

    case 3:
        /* Fast-forward while A stays held; releasing it drops back to the wait. */
        if ((var_peripheral_8c1ba358->on & PDD_DGT_TA) == 0) {
            state->phase_0x00 = 2;
        } else {
            state->pageIndex_0x08 += 2;
            if (state->pageIndex_0x08 >= state->pageCount_0x04) {
                state->phase_0x00 = 4;
            }
        }
        break;

    case 4:
        if ((var_peripheral_8c1ba358->press & PDD_DGT_TA) != 0) {
            SndStopAdx_8c010ca6(1);
            state->line_0x18++;
            nextString = state->line_0x18->text_0x00;
            if (*nextString == '\0') {
                state->slide_0x10++;
                if ((int)state->slide_0x10->layers_0x00 == -1) {
                    var_fadeRequest_8c226564 = FADE_REQUEST_IN;
                    state->phase_0x00 = 5;
                } else {
                    state->phase_0x00 = 0;
                }
            } else {
                state->phase_0x00 = 1;
            }
        }
        break;

    case 5:
        if (!var_isFading_8c226568) {
            ObjectsFreeMessageAssets_8c02adee();
            TaskFree_8c014b66(task);
            EventApplyFlags_8c02b292();
            RouteLoadStartRouteModelLoadPass_8c013d78();
            var_fadeRequest_8c226564 = FADE_REQUEST_OUT;
            var_arrivalOverlayGate_8c226560 = 1;
            var_messageBoxActive_8c22847c = 0;
            return;
        }
        break;
    }

    if (stage == MSGBOX_STAGE_SWAP) {
        state->pageCount_0x04 = ObjectsSwapMessageBoxFor_8c02aefc(state->line_0x18->text_0x00);
        SndPlayAdx_8c010cd6(2, state->line_0x18->voiceId_0x04);
        state->pageIndex_0x08 = 1;
        state->frameCounter_0x0c = 0;
        state->phase_0x00 = 2;
        stage = MSGBOX_STAGE_WAIT;
    }

    if (stage == MSGBOX_STAGE_WAIT) {
        if ((var_peripheral_8c1ba358->press & PDD_DGT_TA) != 0) {
            state->frameCounter_0x0c = 99;
            state->phase_0x00 = 3;
            SndStopAdx_8c010ca6(1);
        }
        state->frameCounter_0x0c++;
        if (state->frameCounter_0x0c >= 3) {
            state->pageIndex_0x08++;
            if (state->pageIndex_0x08 < state->pageCount_0x04) {
                state->frameCounter_0x0c = 0;
            } else {
                state->phase_0x00 = 4;
            }
        }
    }

    init_msgScroll_8c04ab3c.pr = -3.0f;
    ids = state->ids_0x14;
    for (; *ids != 0xffff; ids++) {
        id = *ids;
        for (i = 0; i < var_messageAssetCount_8c228514; i++) {
            if (var_messageAssets_8c228484[i].id_0x00 == id) {
                init_msgScroll_8c04ab3c.list = (NJS_TEXLIST *)var_messageAssets_8c228484[i].pvm_0x04;
                init_msgScroll_8c04ab3c.map = (Uint32 *)var_messageAssets_8c228484[i].dat_0x08;
                break;
            }
        }
        njDrawScroll(&init_msgScroll_8c04ab3c);
        init_msgScroll_8c04ab3c.pr += 0.1f;
    }
    ObjectsMenuTextboxText_8c02af1c(state->pageIndex_0x08);
    SndPollVoiceEnd_8c0106ac();
}
/* Starts the event message-box display: applies the message-text relocation
 * fixup, spawns the message task with the selected event's slide table,
 * opens the on-screen textbox (ObjectsOpenTextbox_8c02ae3e), and marks the event-message
 * flag so the pause menu stays suppressed (see pauseUpdate_8c0129cc) while it
 * runs. Also counts the shown event toward the run's completion bonus. */
void ObjectsStartMessageBox_8c02ad8c(void)
{
    Task *task;
    MessageBoxState *state;

    relocateMessageText_8c02a9fc(var_messageTextDat_8c228518);
    TaskPush_8c014ae8(var_tasks_8c1ba3c8, &messageBoxTask_8c02ab7a, &task, (void **)&state, 0x1c);
    state->slide_0x10 = var_eventSlides_8c228480[var_selectedEventEntry_8c228478];
    ObjectsOpenTextbox_8c02ae3e(0x20, 0x180, -2.0, 0x240, 0x40, 0, 0, -1);
    state->phase_0x00 = 0;
    var_messageBoxActive_8c22847c = 1;
    var_eventCount_8c1bb8e8++;
}
/* Releases everything ObjectsRequestMessageAssets_8c02aa36 requested: the
 * per-id pvm/dat pairs deduped into var_messageAssets_8c228484 (count var_messageAssetCount_8c228514), the
 * shared message-text dat in var_messageTextDat_8c228518, and the message-box textbox
 * resources (ObjectsFreeTextboxes_8c02af32). Called at the end of the message-box
 * slideshow (messageBoxTask_8c02ab7a) and again during full session teardown
 * (ReplayMenuFreeSessionAssets_8c016182). */
void ObjectsFreeMessageAssets_8c02adee(void)
{
    int i;

    for (i = 0; i < var_messageAssetCount_8c228514; i++) {
        syFree(var_messageAssets_8c228484[i].dat_0x08);
        AsqReleaseAndFreeTexlist_8c011e3c(var_messageAssets_8c228484[i].pvm_0x04);
    }
    var_messageAssetCount_8c228514 = 0;

    if (var_messageTextDat_8c228518 != (EventLine **) -1) {
        syFree(var_messageTextDat_8c228518);
        var_messageTextDat_8c228518 = (EventLine **) -1;
    }

    ObjectsFreeTextboxes_8c02af32();
}
/* (Re)opens the message textbox: tears down any existing pair via
 * ObjectsFreeTextboxes_8c02af32, re-inits the text module, and creates a fresh
 * double-buffered pair -- (&var_messageTextBoxA_8c1bc404)[0] and [1] -- with identical
 * geometry, toggled between by ObjectsSwapMessageBoxFor_8c02aefc. Geometry
 * args mirror TxtCreateTextBox_8c0152fc. The original returns
 * &var_menuTextboxCharLimit_8c225fb8 (just reset to 0 here), but no caller
 * uses the return value. */
void ObjectsOpenTextbox_8c02ae3e(int x, int y, float priority, int width, int height, int x2, int y2, int enable_offset)
{
    if (var_messageTextBoxA_8c1bc404 != (void *) -1) {
        ObjectsFreeTextboxes_8c02af32();
    }

    TxtInit_8c01524c();
    var_messageTextBoxA_8c1bc404 = TxtCreateTextBox_8c0152fc(x, y, priority, width, height, x2, y2, enable_offset);
    var_messageTextBoxB_8c1bc408 = TxtCreateTextBox_8c0152fc(x, y, priority, width, height, x2, y2, enable_offset);
    var_messageTextBoxIndex_8c1bc40c = 1;
    var_menuTextboxCharLimit_8c225fb8 = 0;
}
/* Flips the active buffer index and lays the given text out into the
 * newly-active box, returning TxtPrepareTextBoxLayout_8c01543a's result
 * (the caller uses it as a wait-frame count). */
int ObjectsSwapMessageBoxFor_8c02aefc(char *string)
{
    var_messageTextBoxIndex_8c1bc40c ^= 1;
    return TxtPrepareTextBoxLayout_8c01543a((TextBox *)(&var_messageTextBoxA_8c1bc404)[var_messageTextBoxIndex_8c1bc40c], string);
}
/* Draws the currently-revealed portion of the active message box's text,
 * called every frame from messageBoxTask_8c02ab7a's draw tail with the
 * running char-reveal counter as limit. Returns TxtDrawTextbox_8c0155e0's
 * result (some callers check it in an if()). */
int ObjectsMenuTextboxText_8c02af1c(int limit)
{
    return TxtDrawTextbox_8c0155e0((TextBox *)(&var_messageTextBoxA_8c1bc404)[var_messageTextBoxIndex_8c1bc40c], limit);
}
/* Destroys the double-buffered textbox pair and re-inits the text module.
 * Guarded by the var_messageTextBoxA_8c1bc404 sentinel so a second call (e.g. teardown
 * after an already-closed textbox) is a no-op. Note var_messageTextBoxB_8c1bc408 and
 * var_messageTextBoxIndex_8c1bc40c are left stale -- only var_messageTextBoxA_8c1bc404 is reset -- matching
 * the original. */
void ObjectsFreeTextboxes_8c02af32(void)
{
    if (var_messageTextBoxA_8c1bc404 != (void *) -1) {
        TxtDestroyTextBox_8c015410((TextBox *)var_messageTextBoxA_8c1bc404);
        TxtDestroyTextBox_8c015410((TextBox *)var_messageTextBoxB_8c1bc408);
        TxtDestroy_8c01529c();
        var_messageTextBoxA_8c1bc404 = (void *) -1;
    }
}
