#ifndef _020214_DRIVE_CUE_TASK_H
#define _020214_DRIVE_CUE_TASK_H

#include "014a9c_tasks.h"

/* Per-frame audio cue task for a drive, installed by DriveCueInit_8c020528
 * on var_driveCueState_8c2264b8 (reached directly, not through the state
 * param) and freed once the drive reaches its ending phase. Runs three
 * independent cues:
 *
 * - an ambient chime plus pad rumble, replayed on a randomized timer while
 *   the bus is above a low speed threshold
 *   (idleChimeState_0x00/idleChimeTimer_0x04), plus a one-shot variant armed
 *   by TrafficDriveVehicle_8c025b98;
 * - the driver's own stop announcement (stopAnnounceState_0x08): a
 *   route-specific chime, then the spoken stop name 60 frames later via
 *   SndProc_8c010cd6, requested by nearStopLatch_0x0c;
 * - a chime at a fixed list of per-route segments, third-person camera only
 *   (nearStopChimeLatch_0x14).
 *
 * It is also what advances rumble playback, while the VIBRATION setting is
 * on. */
void DriveCueTask_8c020214(Task *task, void *state);

#endif // _020214_DRIVE_CUE_TASK_H
