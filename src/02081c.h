/* 8c02081c */
#ifndef _02081C_H
#define _02081C_H

#include <shinobi.h>

/* Distance between world point a (a full x/y/z point, only x/z used) and
 * 2D point b (an x/z pair) via njSqrt(dx*dx+dz*dz). */
float GeomDistanceXZ_8c02081c(void *a, void *b);

/* Separating-axis overlap test between two convex quads (4 NJS_POINT3s
 * each; only x/z are used, except a coarse pre-gate on the y of point
 * index 1). */
Bool GeomQuadOverlap_8c020842(NJS_POINT3 *a, NJS_POINT3 *b);

#endif // _02081C_H
