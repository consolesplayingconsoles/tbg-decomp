#ifndef _02E400_COLLISION_H
#define _02E400_COLLISION_H

#include "014a9c_tasks.h"
#include "026710_traffic.h" /* TrafficEntry */

/* Reports which other traffic entry `entry` is currently overlapping, or
 * NULL. Leaves entry's own world-space box in var_collisionSelfBox_8c228978,
 * which CollisionQueueTest_8c02e4ac below then tests against. */
TrafficEntry *CollisionFindTaskHit_8c02e400(
    /* The caller's own task, excluded from the scan */
    Task *self,
    /* The entry to test, and the source of the box */
    TrafficEntry *entry
);

/* Empties the queue. */
void CollisionQueueReset_8c02e486(void);

/* Appends obj to the queue; silently dropped once the 64 slots are full. */
void CollisionQueueAdd_8c02e48e(void *obj);

/* Reports which queued object's bounding sphere overlaps
 * var_collisionSelfBox_8c228978, or NULL.
 *
 * Does not fill that box -- it tests against whatever
 * CollisionFindTaskHit_8c02e400 or BusCollisionFindHit_8c02e2dc left there,
 * so what this answers depends on who ran last. */
void *CollisionQueueTest_8c02e4ac(void);

#endif // _02E400_COLLISION_H
