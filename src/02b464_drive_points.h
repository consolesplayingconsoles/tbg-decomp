#ifndef _02B464_H
#define _02B464_H

/* Adjusts the run's driver points by delta, clamped to [0, var_8c2285d4];
 * unless msgSet is -1, also queues a driver-comment banner for it (see
 * definition for the message-queue mechanics). */
void DrivePointsAdjust_8c02b464(int msgSet, int delta);

/* Arms one offense type's cooldown, resetting all 5 to ready first (see
 * definition for per-type details). */
void DrivePointsArmCooldowns_8c02b578(int type);

/* Reacts to the player bus bumping a pedestrian/vehicle: knockback physics,
 * vibration, and a driver-points penalty (see definition for details). */
void DrivePointsHandleBump_8c02b6d4(void);

/* Grades a driver-points penalty from var_8c228660's warning bits (see
 * definition for details). */
void DrivePointsHandleFlags_8c02b7ea(void);

/* Grades a driver-points penalty when var_8c228680 signals a specific
 * offense code (0x30000); see definition. */
void FUN_8c02b864(void);

/* Grades a driver-points penalty when var_8c228680 is nonzero and
 * var_8c22868c != 2; see definition for the offense-code split. */
void FUN_8c02b886(void);

/* Grades a driver-points penalty from var_8c228680/var_8c22868c and, on a
 * traffic-signal state change (var_busState_8c1bb9d0's 0x34c low bits),
 * grades a further one via FUN_8c028900; see definition. */
void FUN_8c02b8b8(void);

/* Grades driver-points penalties from var_8c228680/var_8c22868c and from a
 * held-signal timeout tracked via var_8c2285fc[3]/[4]; see definition. */
void FUN_8c02b986(void);

/* Grades driver-points penalties for a stale traffic signal, speeding, and
 * a lane-change/turn-signal mismatch; see definition. */
void FUN_8c02bb1c(void);

/* Per-frame grading tick, called once per drive frame from
 * taskCallback_8c02c072 (see definition for the full list of what it
 * grades); also advances the run's pass/fail progress counter. */
void FUN_8c02bcd8(void);

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
