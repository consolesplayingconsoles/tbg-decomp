#include "shinobi.h"

#ifndef _02D19C_H_
#define _02D19C_H_

#include "014a9c_tasks.h"

/* TaskPush_8c014ae8 state, exactly the 0x38 bytes all three spawn paths
 * (BusRiderBoardTask_8c02d21c, BusRiderSeatedTask_8c02d5ca, BusRiderAlightTask_8c02d46c) ask for. The three callbacks
 * disagree on what several of the fields mean (e.g. field_0x08/0x0c/0x10 is
 * an NJS_POINT3 for BusRiderBoardTask_8c02d21c/BusRiderAlightTask_8c02d46c but an {int, float, float}
 * triple for BusRiderSeatedTask_8c02d5ca), so this is kept as raw words rather than one
 * semantically-typed struct. */
typedef struct {
    void *ref_0x00;
    Uint32 field_0x04;
    Uint32 field_0x08;
    Uint32 field_0x0c;
    Uint32 field_0x10;
    Uint32 field_0x14;
    Uint32 field_0x18;
    Uint32 field_0x1c;
    Uint32 field_0x20;
    Uint32 field_0x24;
    Uint32 field_0x28;
    Uint32 field_0x2c;
    Uint32 field_0x30;
    Uint32 field_0x34;
} StopScheduleState;

/* Per-scripted-stop schedule table, 31 {int, float} entries indexed by the
 * var_8c228718 slot index. field_0x00 stores a float's raw bits (read back
 * as float by 02d19c's spawn callbacks; 02d968 only ever moves it as a
 * Uint32-sized word). */
typedef struct {
    int field_0x00;
    float field_0x04;
} InitEntry_8c04c3e4;
extern InitEntry_8c04c3e4 init_8c04c3e4[31];

/* Task action for a scripted-stop slot whose segment differs from the bus's
 * current one -- spawned immediately (no shuffle) by StopSpawnInit_8c02d968. */
void BusRiderSeatedTask_8c02d5ca(Task *task, void *state);

/* Task action for an already-picked waiting passenger (state->field_0x04 == 1
 * always, from 02d968); handles positioning, the stop-bell sound and
 * despawn. */
void BusRiderBoardTask_8c02d21c(Task *task, void *state);

/* Task action for a scripted-stop slot that matches the bus's current
 * segment (spawned after the Fisher-Yates shuffle). */
void BusRiderAlightTask_8c02d46c(Task *task, void *state);

/* Per-frame countdown/animation task action shared by every waiting-
 * passenger spawn path once positioned; drives the wave/board animation and
 * frees itself when done. */
void BusRiderStopSceneTask_8c02d644(Task *task, void *state);

/* Task action spawned instead of the normal per-passenger tasks when
 * var_playMode_8c1bb8d0 == 1 (VM/replay mode) and course-restart flag
 * var_8c226410 bit 3 is clear -- just resets state on first run. */
void BusRiderSkipStopTask_8c02d8f0(Task *task, void *state);

#endif /* _02D19C_H_ */
