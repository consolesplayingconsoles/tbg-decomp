#ifndef _02786C_VEHICLE_PARTS_H
#define _02786C_VEHICLE_PARTS_H

#include "026710_traffic.h" /* TrafficEntry */

/* Called by spawnEntry_8c0272b8 (026710_traffic.h) with the newly
 * allocated entry and its masked type code, right before the entry's
 * script is first run. Also called by 023310_bus_init.c with the player's
 * BusState*, which shares TrafficEntry's 0x00-0x60 prefix. */
void VehPartsBind_8c02786c(TrafficEntry *entry, Uint32 typeCode);

#endif // _02786C_VEHICLE_PARTS_H
