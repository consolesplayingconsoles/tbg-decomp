/* 8c020528 */
#ifndef _020528_DRIVE_CUE_INIT_H
#define _020528_DRIVE_CUE_INIT_H

/* Starts the ambient drive cues for a new run: seeds
 * var_driveCueState_8c2264b8 (first idle chime 150-450 frames out) and
 * pushes DriveCueTask_8c020214. Not run in demo playback. */
void DriveCueInit_8c020528();

#endif // _020528_DRIVE_CUE_INIT_H
