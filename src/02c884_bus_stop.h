/* @unit BusStop -- bus stops: which segments have one, the passengers waiting
 * at them, and the arrival/departure state machine. */
#ifndef _02C884_H
#define _02C884_H

#include "013ae8_route_load.h" /* CourseSegment */

/* Stop-area record: same layout as pickWaitingPassengers_8c02c8ae's
 * var_8c22890c -- origin (x_0x04, z_0x08) and direction (dx_0x0c, dz_0x10)
 * of a stop's spawn strip; field_0x00 unknown. */
typedef struct {
    void *ukn_0x00;
    float x_0x04;
    float z_0x08;
    float dx_0x0c;
    float dz_0x10;
} StopAreaRecord;

void BusStopFreeTaskGroup_8c02ca96(void);
void BusStopSetup_8c02caba(void);
void BusStopUpdateStopHeadings_8c02ccc6(void);

/* Returns the segment record for segmentIndex (see
 * pickWaitingPassengers_8c02c8ae, 02c884). */
CourseSegment *BusStopGetSegment_8c02cd6a(int segmentIndex);

/* Returns the stop-area record for segmentIndex's segment (var_stopAreaTable_8c1bb870
 * entry selected by the segment record's stopAreaId_0x02). */
StopAreaRecord *BusStopGetStopArea_8c02cd7a(int segmentIndex);

/* Per-frame bus-stop arrival state machine; see 02c884_bus_stop.c for
 * details. Installed as a raw callback in a dispatch table in the driving
 * task (02b464); takes no meaningful argument. */
void BusStopUpdateArrival_8c02ce48(void);

#endif // _02C884_H
