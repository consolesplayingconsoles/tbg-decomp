/* 8c022464 */
#ifndef _022464_FADE_H
#define _022464_FADE_H

#include <njdef.h>
#include "014a9c_tasks.h"

/* =================
 * Type Declarations
 * =================
 */

/* var_fadePhase_8c227d7c: the fade state machine's current phase. */
typedef enum {
    FADE_PHASE_IDLE = 0,
    FADE_PHASE_OUT  = 1,
    FADE_PHASE_IN   = 2,
    FADE_PHASE_HELD = 3
} FadePhase;

/* var_fadeRequest_8c226564: a transition requested of FadeUpdate_8c022560 /
 * FadeUpdatePlain_8c022910, consumed once phase is FADE_PHASE_IDLE. */
typedef enum {
    FADE_REQUEST_NONE = 0,
    FADE_REQUEST_OUT  = 1,
    FADE_REQUEST_IN   = 2
} FadeRequest;

/* BusState.mirror_0x268: which mirror view is up. */
typedef enum {
    FADE_MIRROR_NONE  = 0,
    FADE_MIRROR_LEFT  = 1,
    FADE_MIRROR_RIGHT = 2,
    /* Not a wing mirror: the door-side view a run starts in
     * (busInitPlaceBus_8c023310) -- BusRenderUpdateMirrorCamera_8c025604 puts
     * the camera close alongside the front door instead of back down the
     * flank. FadeUpdate_8c022560 draws it like the wing mirrors but with no
     * label sprite. */
    FADE_MIRROR_DOOR  = 3
} FadeMirrorSelect;

/* Task private state for FadeInTask_8c022a54/FadePushIn_8c022a9c. Its fields
 * match Task (see 014a9c_tasks.h) up through the one this task uses -- the
 * real slot is a full Task (var_tasks_8c1ba3c8 is an array of those), and
 * the rest is unused by this callback. */
typedef struct {
    TaskAction action;
    void *state;
    Uint32 frames_0x08;
} FadeInTask;

/* FadeOutTask.phase_0x0c: RAMP while ramping var_fadeProgress_8c227d80 up to
 * full black, HOLD while holding solid black for the two ticks before the
 * task frees itself. */
typedef enum {
    FADE_OUT_PHASE_RAMP = 0,
    FADE_OUT_PHASE_HOLD = 1
} FadeOutPhase;

/* Task private state for FadeOutTask_8c022ad0/FadePushOut_8c022b60. Same
 * layout as FadeInTask above, but field_0x0c is reused as phase_0x0c
 * instead of a pointer. */
typedef struct {
    TaskAction action;
    void *state;
    Uint32 frames_0x08;
    FadeOutPhase phase_0x0c;
} FadeOutTask;

/* Opaque per-entry callbacks for FadeDrawCommand types 5/6 (see below),
 * queued by the push helpers in 0222dc_fadecmd.c. They pass whatever
 * function pointer their caller supplies, so there is no fixed SDK
 * signature to name these after. */
typedef void (*FadeCallback1)(int);
typedef void (*FadeCallback2)(int, int);

/* FadeDrawCommand.type: which arm of its union holds the remaining 12 bytes.
 * Anything outside these 7 values is a no-op entry. */
typedef enum {
    FADE_CMD_0_DRAW_OBJECT            = 0,
    FADE_CMD_1_CNK_DRAW_OBJECT        = 1,
    FADE_CMD_2_CNK_EASY_DRAW_OBJECT   = 2,
    FADE_CMD_3_CNK_SIMPLE_DRAW_OBJECT = 3,
    FADE_CMD_4_CNK_MOD_DRAW_OBJECT    = 4,
    FADE_CMD_5_CALL1                  = 5,
    FADE_CMD_6_CALL2                  = 6
} FadeDrawCommandType;

/* One 16-byte entry in var_fadeDrawCommands_8c22657c's per-layer draw-command
 * queue. */
typedef struct {
    FadeDrawCommandType type;
    union {
        /* 0-4: an nj*DrawObject variant, type 4 skipping njSetTexture */
        struct {
            NJS_MATRIX *matrix;
            NJS_TEXLIST *texlist;
            union {
                NJS_OBJECT *object;        /* type 0 */
                NJS_CNK_OBJECT *cnkObject; /* types 1-4 */
            } obj;
        } draw;
        /* 5 */
        struct {
            FadeCallback1 fn;
            int arg0;
        } call1;
        /* 6 */
        struct {
            FadeCallback2 fn;
            int arg0;
            int arg1;
        } call2;
    } u;
} FadeDrawCommand;

/* =========
 * Functions
 * =========
 */

void FadeUpdate_8c022560(void);
void FadeStartRunTransition_8c0228a2(void);
void FadeUpdatePlain_8c022910(void);
void FadePushIn_8c022a9c(int frames);
void FadePushOut_8c022b60(int frames);

#endif // _022464_FADE_H
