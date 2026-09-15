#include "shinobi.h"

#ifndef _02D19C_PASSENGER_H_
#define _02D19C_PASSENGER_H_

#include "014a9c_tasks.h"

/* TaskPush_8c014ae8 state, exactly the 0x38 bytes all three spawn paths
 * (PassengerBoardTask_8c02d21c, PassengerSeatedTask_8c02d5ca, PassengerExitTask_8c02d46c) ask for. The three
 * callbacks agree on every field's type. */
typedef struct {
    void *entry_0x00;
    Uint32 state_0x04;
    NJS_POINT3 pos_0x08;
    Uint32 spriteNo_0x14;
    float jitterX_0x18;
    float jitterZ_0x1c;
    Sint32 delay_0x20;
    Sint32 slotIndex_0x24;
    Uint32 isSeated_0x28;
    Uint32 voice_0x2c;
    Uint32 soundIdA_0x30;
    Uint32 soundIdB_0x34;
} StopScheduleState;

/* Per-scripted-stop schedule table, 31 (x, z) seat positions indexed by the
 * var_stopSchedule_8c228718 slot index. */
typedef struct {
    float x;
    float z;
} SeatPos;
extern SeatPos init_seatPositions_8c04c3e4[31];

/* TaskPush_8c014ae8 state for PassengerStopSceneTask_8c02d644, exactly the
 * 8 bytes StopSpawnInit_8c02d968 asks for -- a private 2-int layout distinct
 * from StopScheduleState above. */
typedef struct {
    int phase_0x00;
    int subPhase_0x04;
} PassengerStopSceneState;

/* Task action for a scripted-stop slot whose segment differs from the bus's
 * current one -- spawned immediately (no shuffle) by StopSpawnInit_8c02d968. */
void PassengerSeatedTask_8c02d5ca(Task *task, void *state);

/* Task action for an already-picked waiting passenger (state->state_0x04 == 1
 * always, from 02d968); handles positioning, the stop-bell sound and
 * despawn. */
void PassengerBoardTask_8c02d21c(Task *task, StopScheduleState *state);

/* Task action for a scripted-stop slot that matches the bus's current
 * segment (spawned after the Fisher-Yates shuffle). */
void PassengerExitTask_8c02d46c(Task *task, StopScheduleState *state);

/* Per-frame countdown/animation task action shared by every waiting-
 * passenger spawn path once positioned; drives the wave/board animation and
 * frees itself when done. */
void PassengerStopSceneTask_8c02d644(Task *task, PassengerStopSceneState *state);

/* Task action spawned instead of the normal per-passenger tasks when
 * var_playMode_8c1bb8d0 == 1 (VM/replay mode) and course-restart flag
 * var_8c226410 bit 3 is clear -- resets state on first run. */
void PassengerSkipStopTask_8c02d8f0(Task *task, void *state);

#endif /* _02D19C_PASSENGER_H_ */
