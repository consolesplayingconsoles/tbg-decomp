/* 8c02e51c: undecompiled */
#ifndef _02E51C_H
#define _02E51C_H

/* Looks up the road junction under world point (x, y, z), writing the match
 * into *out (out->field_0x04 nonzero on a hit) -- same (x, y, z, out) shape
 * as GroundQueryFindPolygon_8c020914 (020914_ground_query.h)'s ground-polygon query. Consumed by
 * initEntryState_8c026748 (026710_traffic.h) for night junction-light
 * colors. */
void *FUN_8c02e51c(float x, float y, float z, void *out);

#endif // _02E51C_H
