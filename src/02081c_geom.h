/* 8c02081c */
#ifndef _02081C_GEOM_H
#define _02081C_GEOM_H

#include <shinobi.h>

/* Distance between world point a (x/y/z, only x/z read) and 2D point b (an
 * x/z pair). */
float GeomDistanceXZ_8c02081c(void *a, void *b);

/* TRUE if either of two convex quads (4 NJS_POINT3s each, x/z only) holds a
 * vertex of the other -- a cross-shaped overlap with no vertex contained
 * reads as a miss. A one-sided height cull on vertex 1 runs first. */
Bool GeomQuadOverlap_8c020842(NJS_POINT3 *a, NJS_POINT3 *b);

#endif // _02081C_GEOM_H
