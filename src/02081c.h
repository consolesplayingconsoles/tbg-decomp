/* 8c02081c */
#ifndef _02081C_H
#define _02081C_H

#include <shinobi.h>

/* Distance between world point a (a full x/y/z point, only x/z used) and
 * 2D point b (an x/z pair) via njSqrt(dx*dx+dz*dz). Called by
 * BusTask_8c022bdc (022bdc) with &BusState.posX_0x0f4 and &BusState.field_0xec. */
float FUN_8c02081c(void *a, void *b);

/* Separating-axis overlap test between two convex quads (4 NJS_POINT3s
 * each; only x/z are used, except a coarse pre-gate on the y of point
 * index 1). Not yet called by any decompiled unit -- xref only from the
 * still-asm 02e2dc. */
Bool FUN_8c020842(NJS_POINT3 *a, NJS_POINT3 *b);

#endif // _02081C_H
