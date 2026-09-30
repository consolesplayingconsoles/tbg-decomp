#ifndef _020214_DRIVE_CUE_H
#define _020214_DRIVE_CUE_H

/* Ambient drive-cue state (see driveCueTask_8c020214). Three fields are
 * written from outside this unit: BusTask_8c022bdc (022bdc) sets
 * nearStopLatch_0x0c on the first A press of a drive and
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

extern DriveCueState var_driveCueState_8c2264b8;

/* Starts the ambient drive cues for a new run: seeds
 * var_driveCueState_8c2264b8 (first idle chime 150-450 frames out) and
 * pushes driveCueTask_8c020214. Not run in demo playback. */
void DriveCueInit_8c020528();

#endif // _020214_DRIVE_CUE_H
