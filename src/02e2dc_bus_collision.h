/* 8c02e2dc */
#ifndef _02E2DC_BUS_COLLISION_H
#define _02E2DC_BUS_COLLISION_H

#include <shinobi.h>
#include "026710_traffic.h" /* TrafficEntry */
#include "sectionB.h"

/* Table of 16 pointers, each to a 0x60-byte block of 8 NJS_POINT3 -- a
 * per-variant oriented bounding box in local space, indexed by a traffic
 * entry's variant index (entry+0x2e0). */
extern NJS_POINT3 *init_variantBoxes_8c04c940[16];

/* Reports which vehicle or pedestrian the player's bus is currently bumping
 * into, or NULL. Only candidates within 12 world units (field_0x490) are
 * box-tested; that field is past BusState's 0x3cc end, so a hit is never the
 * bus itself. Leaves the bus's own world-space box in
 * var_collisionSelfBox_8c228978. */
TrafficEntry *BusCollisionFindHit_8c02e2dc(void);

#endif // _02E2DC_BUS_COLLISION_H
