/* Bus stops: which segments have one, the passengers waiting at them, and the
 * arrival/departure state machine. */
#ifndef _02C884_BUS_STOP_H
#define _02C884_BUS_STOP_H

#include "013ae8_route_load.h" /* CourseSegment */

/* A stop's spawn strip: origin (x_0x04, z_0x08) and direction (dx_0x0c,
 * dz_0x10) that waiting passengers are laid out along. */
typedef struct {
    void *ukn_0x00;
    float x_0x04;
    float z_0x08;
    float dx_0x0c;
    float dz_0x10;
} StopAreaRecord;

/* Frees the bus-stop task group and marks it unallocated. */
void BusStopFreeTaskGroup_8c02ca96(void);

/* Per-run setup, called once the course has loaded: decides which segments
 * get a stop, then primes the first one and its waiting passengers. */
void BusStopSetup_8c02caba(void);

/* Locks in the current stop's heading, then finds the next segment with a
 * stop and primes its position and heading. */
void BusStopUpdateStopHeadings_8c02ccc6(void);

CourseSegment *BusStopGetSegment_8c02cd6a(int segmentIndex);

/* Returns the stop-area record for segmentIndex's segment
 * (var_stopAreaTable_8c1bb870, selected by the segment's stopAreaId_0x02). */
StopAreaRecord *BusStopGetStopArea_8c02cd7a(int segmentIndex);

/* Per-frame arrival state machine, driven by 02b464_drive_points. Phases run
 * 0 cruising -> 2 approach -> 3 stopped -> 4 finishing -> 1 post-departure ->
 * 0; this function never enters 3 or 4 on its own, 02b464 does. See the
 * function for what each phase watches. */
void BusStopUpdateArrival_8c02ce48(void);

#endif // _02C884_BUS_STOP_H
