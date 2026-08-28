/* 8c022464 */
#ifndef _022464_FADE_H
#define _022464_FADE_H

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

/* var_mirrorSelect_8c1bbc38: which wing mirror the bus-stop-arrival overlay draws. */
typedef enum {
    FADE_MIRROR_NONE  = 0,
    FADE_MIRROR_LEFT  = 1,
    FADE_MIRROR_RIGHT = 2
} FadeMirrorSelect;

/* Task private state for FadeInTask_8c022a54/FadePushIn_8c022a9c. Leading
 * fields match Task (see 014a9c_tasks.h) up through the one this task
 * actually uses; the real slot is still a full Task (var_tasks_8c1ba3c8 is
 * an array of those), the rest is just untouched by this callback. */
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
 * deal as FadeInTask above, but field_0x0c is reused as phase_0x0c
 * instead of a pointer. */
typedef struct {
    TaskAction action;
    void *state;
    Uint32 frames_0x08;
    FadeOutPhase phase_0x0c;
} FadeOutTask;

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
