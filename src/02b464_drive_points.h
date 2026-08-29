#ifndef _02B464_H
#define _02B464_H

/* Starts a drive: installs the master per-frame drive task
 * (taskCallback_8c02c072) and resets this unit's whole scratch scoring/
 * state region for a fresh run. */
void DrivePointsReset_8c02c46a(void);

/* Called from BusStopUpdateArrival_8c02ce48 (02c884) and from this unit's
 * own drive-end logic; picks a required next-stop-segment threshold (by
 * practice-lesson id when practicing, else by route) and reports whether
 * var_nextStopSegment_8c228710 has reached it yet. */
int FUN_8c02c586(void);

/* Installed as var_fadeCompleteCallback_8c22656c by
 * BusStopUpdateArrival_8c02ce48 when a stop completes, and by this unit's
 * own drive-end logic (taskCallback_8c02c072/FUN_8c02c624). */
void FUN_8c02c784(void);

#endif // _02B464_H
