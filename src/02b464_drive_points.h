/* Driver points: the running score a drive is graded against, and what
 * happens when it runs out or the route finishes. */
#ifndef _02B464_DRIVE_POINTS_H
#define _02B464_DRIVE_POINTS_H

/* Starts a drive: installs the master per-frame drive task
 * (taskCallback_8c02c072) and resets this unit's whole scratch scoring/
 * state region for a fresh run. */
void DrivePointsReset_8c02c46a(void);

/* Reports whether the run has reached the last stop it owes: a required
 * var_nextStopSegment_8c228710, picked by practice-lesson id when
 * practicing and by route otherwise. */
int DrivePointsRunComplete_8c02c586(void);

/* var_fadeCompleteCallback_8c22656c for a finished drive: retries the route
 * segment if the run still owes a stop, otherwise heads for the results or
 * lesson-retry screen. Installed by BusStopUpdateArrival_8c02ce48 (02c884)
 * and by this unit's own drive-end paths. */
void DrivePointsOnFadeDriveEnd_8c02c784(void);

#endif // _02B464_DRIVE_POINTS_H
