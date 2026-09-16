#ifndef _020214_H
#define _020214_H

#include "014a9c_tasks.h"

/* Ambient driving cue task (installed by DriveCueInit_8c020528 with
 * var_driveCueState_8c2264b8 as its state, ignored via the state param --
 * DriveCueTask_8c020214 addresses that struct directly). Frees itself via
 * TaskFree_8c014b66 once the drive has reached its ending phase
 * (var_8c2285c4[0] >= 3). While active: plays a periodic idle
 * chime/vibration once the bus is moving fast enough
 * (var_busState_8c1bb9d0.speed_0x27c) and a countdown (var_8c2264bc)
 * expires, then -- gated by the A-press latch nearStopLatch_0x0c -- runs a
 * stop-approach jingle sequence once per drive (stopAnnounceState_0x08's
 * state machine, restarted if the latch gets cleared again by 02c884),
 * then a "near stop marker" chime gated by var_cameraMode_8c227d9c's
 * mirror-view level and a route/segment match, latched by
 * nearStopChimeLatch_0x14, and finally stops any active controller
 * vibration when the VIBRATION setting is off. */
void DriveCueTask_8c020214(Task *task, void *state);

#endif // _020214_H
