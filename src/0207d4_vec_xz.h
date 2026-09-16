#ifndef _0207D4_VEC_XZ_H
#define _0207D4_VEC_XZ_H

#include <shinobi.h>

/* A packed x/z pair -- the 2D points the game keeps next to its full
 * NJS_POINT3s (BusState.laneTargetX_0x0ec, TrafficEntry.pathPointX_0x0ec). */
typedef struct {
    float x;
    float z;
} PointXZ;

/* Dot of (a - origin) and (b - origin), x/z only: positive while b is on a's
 * side of origin. */
float VecXZDot_8c0207d4(NJS_POINT3 *origin, NJS_POINT3 *a, PointXZ *b);

/* Cross (z component) of (a - origin) and (b - origin), x/z only: the sign
 * is which side of the origin->a line b falls on. */
float VecXZCross_8c0207fa(NJS_POINT3 *origin, PointXZ *a, PointXZ *b);

#endif // _0207D4_VEC_XZ_H
