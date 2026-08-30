#ifndef _02E400_COLLISION_H
#define _02E400_COLLISION_H

#include "014a9c_tasks.h"
#include "026710_traffic.h" /* TrafficEntry */

/* Scans var_tasks_8c1bac28 for a task (other than self) whose traffic entry
 * collides with self's oriented bounding box, using the entry's variant
 * index and world matrix. Returns the colliding task's state pointer, or
 * NULL if the scan runs off the end. */
TrafficEntry *CollideFindTaskHit_8c02e400(Task *self, TrafficEntry *entry);

/* Clears the queue's write cursor (var_collideQueueCount_8c228b38). */
void CollideQueueReset_8c02e486(void);

/* Appends obj to a fixed 64-slot queue (var_collideQueue_8c228a38/var_collideQueueCount_8c228b38), silently
 * dropped once full. */
void CollideQueueAdd_8c02e48e(void *obj);

/* Returns the first queued entry (0..var_collideQueueCount_8c228b38) whose
 * bounding sphere overlaps var_collideSelfBox_8c228978, or NULL if none do. */
void *CollideQueueTest_8c02e4ac(void);

#endif // _02E400_COLLISION_H
