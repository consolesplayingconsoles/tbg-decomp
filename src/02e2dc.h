/* 8c02e2dc */
#ifndef _02E2DC_H
#define _02E2DC_H

#include <shinobi.h>
#include "026710_traffic.h" /* TrafficEntry */
#include "sectionB.h"

/* Table of 16 pointers, each to a 0x60-byte block of 8 NJS_POINT3 -- a
 * per-variant oriented bounding box in local space, indexed by a traffic
 * entry's variant index (entry+0x2e0). */
extern NJS_POINT3 *init_8c04c940[16];

/* Finds the vehicle/pedestrian the player's bus is currently bumping into
 * (called by handleBump_8c02b6d4, 02b464); returns its TrafficEntry
 * (field_0x490 is past BusState's 0x3cc end, so a hit is never the player's
 * own bus), or NULL if none. */
TrafficEntry *BusCollideFindHit_8c02e2dc(void);

#endif // _02E2DC_H
