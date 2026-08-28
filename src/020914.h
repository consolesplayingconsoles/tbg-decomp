/* 8c020914: undecompiled */
#ifndef _020914_H
#define _020914_H

/* Looks up the ground polygon under world point (x, y, z) in the grid
 * currently selected by var_activeGroundGrid_8c2264d4, writing the match (or a zeroed
 * count on miss) to *out. See FUN_8c020f7e (020b6c.h) for *out's layout. */
void FUN_8c020914(float x, float y, float z, void *out);

#endif // _020914_H
