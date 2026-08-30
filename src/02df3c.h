/* 8c02df3c */
#ifndef _02DF3C_H
#define _02DF3C_H

#include "014a9c_tasks.h"       /* Task */
#include "026710_traffic.h"     /* TrafficEntry */

/* Seeds entry->lookaheadPoints_0x49c from scratch (entry->lookaheadCacheLen_0x4ec/0x4f4/
 * 0x4fc/0x4f8 all start at 0), walking the path from its own current
 * position out to 25.0 units. Called once, at the tail of
 * initEntryState_8c026748 (026710_traffic.h), after the entry's path
 * cursor (pathRecord_0x2b8/pathDistance_0x2bc) has been resolved. */
void TrafficLookaheadInit_8c02df3c(TrafficEntry *entry);

/* Tops entry->lookaheadPoints_0x49c back up to 20.0 units ahead (it was
 * left short by however far the entry moved since the last call -- see
 * lookaheadCacheLen_0x4ec's odometer comment) if needed, then scans the cache for a
 * point matching either the player bus's current/recent position or
 * another live traffic entry's -- returns a BusState/TrafficEntry-
 * compatible pointer (may be var_8c1bbd9c, the player sentinel) or NULL.
 * Called by TrafficDriveVehicle_8c025b98 (025b98) with a lookahead of
 * entry->speed_0x27c*36.0 + entry->lookaheadMargin_0x41c, +5.0 once entry->obstacleLimitActive_0x424
 * (already braking for an obstacle) is set, to avoid the candidate
 * flapping in and out of range every frame. `self` (the entry's own task)
 * is used only to skip the entry itself when scanning other traffic. */
void *TrafficLookaheadScan_8c02dfca(Task *self, TrafficEntry *entry, float lookahead);

#endif // _02DF3C_H
