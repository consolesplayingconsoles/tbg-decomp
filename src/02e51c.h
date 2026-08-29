/* 8c02e51c: undecompiled */
#ifndef _02E51C_H
#define _02E51C_H

/* Looks up the road junction under world point (x, y, z), writing the match
 * into *out (out->field_0x04 nonzero on a hit) -- same (x, y, z, out) shape
 * as GroundQueryFindPolygon_8c020914 (020914_ground_query.h)'s ground-polygon query. Consumed by
 * initEntryState_8c026748 (026710_traffic.h) for night junction-light
 * colors. */
void *FUN_8c02e51c(float x, float y, float z, void *out);

/* Same (x, y, z, out) shape; stored in BusState.field_0x2d0 (023310_bus_init)
 * as the normal-case lookup, paired with the AtHeight variant FUN_8c02ec50
 * for the Wangan-route/segment-10 special case. */
void *FUN_8c02e69c(float x, float y, float z, void *out);

/* AtHeight counterpart of FUN_8c02e69c, same (x, y, z, out) shape. */
void *FUN_8c02ec50(float x, float y, float z, void *out);

#endif // _02E51C_H
