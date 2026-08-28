/* 8c020b6c: undecompiled */
#ifndef _020B6C_H
#define _020B6C_H

/* Interpolates point's Y from the ground polygon match written by
 * FUN_8c020914 (020914.h) into queryResult; a no-match (count == 0)
 * leaves point->y untouched. */
void FUN_8c020f7e(void *queryResult, float *point);

/* Ground polygon lookup by (x, y, z) -- like FUN_8c020914 (020914.h) but
 * y is a dead argument, never read. Writes the match into queryResult. */
void FUN_8c020b6c(float x, float y, float z, void *queryResult);

#endif // _020B6C_H
