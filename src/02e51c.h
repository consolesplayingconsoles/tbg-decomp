/* @unit: not stamped -- this TU groups two unrelated jobs: four float-heavy
 * grid/geometry lookups (FUN_8c02e51c/eab4/e69c/ec50) and one integer
 * task-scan (FUN_8c02f08a). */
#ifndef _02E51C_H
#define _02E51C_H

#include "014a9c_tasks.h"

/* =======================
 * Non-initialized Globals
 * =======================
 */

/* Selects the attribute grid for searching. */
extern void *var_activeAttrGrid_8c228b3c;

/* Looks up the road junction under world point (x, z) -- y is unused -- in
 * the attribute grid selected by var_activeAttrGrid_8c228b3c, writing the match into *out
 * (out->count, at offset 0x08, nonzero on a hit; gates a track-vs-full-search
 * branch on the next call, same idea as GroundQueryResult's count_0x0c) --
 * same (x, y, z, out) call shape as GroundQueryFindPolygon_8c020914
 * (020914_ground_query.h)'s ground-polygon query, but a different, 3-field
 * result layout: {int *slot; int *vertexIds; int count;}, no leading attr
 * field. On a hit, returns &polys[*slot].attr_0x08 instead; NULL on a miss.
 * Consumed by initEntryState_8c026748 (026710_traffic.h) for night junction-
 * light colors. */
void *FUN_8c02e51c(float x, float y, float z, void *out);

/* Height-filtered counterpart of FUN_8c02e51c, same (x, y, z, out) shape and
 * result layout; y drives the height filter here. Stored in
 * BusState.junctionQueryFnCpu_0x2cc (023310_bus_init) for the Wangan-route/segment-10
 * special case, paired with FUN_8c02ec50. Also stored as a traffic entry's
 * junction-query callback by spawnEntry_8c0272b8 (026710_traffic.h) for the
 * elevated-road case (entry type code bit 0x4000 set). */
void *FUN_8c02eab4(float x, float y, float z, void *out);

/* Same (x, y, z, out) shape and result layout as FUN_8c02e51c; stored in
 * BusState.junctionQueryFnRoute_0x2d0 (023310_bus_init) as the normal-case lookup, paired
 * with the AtHeight variant FUN_8c02ec50 for the Wangan-route/segment-10
 * special case. */
void *FUN_8c02e69c(float x, float y, float z, void *out);

/* AtHeight counterpart of FUN_8c02e69c, same (x, y, z, out) shape and
 * result layout; y drives the height filter here. */
void *FUN_8c02ec50(float x, float y, float z, void *out);

/* Scans var_tasks_8c1bac28 for a traffic entry (task state, other than
 * self's) whose signalId_0x410 equals value; falls back to the player bus's
 * own BusState.fallbackTaskMatchId_0x3a0 when no other task matches. Returns nonzero on
 * either match. Consumed by TrafficDriveVehicle_8c025b98 (025b98_traffic_drive). */
int FUN_8c02f08a(Task *self, int value);

#endif // _02E51C_H
