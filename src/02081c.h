/* 8c02081c: undecompiled */
#ifndef _02081C_H
#define _02081C_H

/* Distance between world point a (a full x/y/z point, only x/z used) and
 * 2D point b (an x/z pair) via njSqrt(dx*dx+dz*dz). Called by
 * BusTask_8c022bdc (022bdc) with &BusState.posX_0x0f4 and &BusState.field_0xec. */
float FUN_8c02081c(void *a, void *b);

#endif // _02081C_H
