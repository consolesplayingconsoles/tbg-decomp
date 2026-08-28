#ifndef _02E400_H
#define _02E400_H

/* Clears the queue's write cursor (var_8c228b38). */
void FUN_8c02e486(void);

/* Appends obj to a fixed 64-slot queue (var_8c228a38/var_8c228b38), silently
 * dropped once full. */
void FUN_8c02e48e(void *obj);

#endif // _02E400_H
