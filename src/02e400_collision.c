/* @unit Collide */

#include <shinobi.h>
#include "014a9c_tasks.h"
#include "026710_traffic.h" /* TrafficEntry */
#include "02e2dc.h"
#include "02e400_collision.h"
#include "sectionB.h"

/* ====================
 * Functions
 * ====================
 */

/* Finds the first other live traffic entry whose oriented bounding box overlaps
 * `entry`'s, and returns that entry (the colliding task's state), or NULL.
 * `self` is the caller's own task, skipped by the scan along with any slot whose
 * action is the -1 sentinel; the scan ends at the first empty (action == NULL)
 * slot.
 *
 * Original bug, preserved: self's box comes from `&init_8c04c940[idx]` -- the
 * address of the table slot -- while a candidate's comes from
 * `init_8c04c940[idx]`, the box it points at. Self is therefore built from the
 * pointer table's own bytes reinterpreted as floats. */
TrafficEntry *CollideFindTaskHit_8c02e400(Task *self, TrafficEntry *entry)
{
    TrafficEntry *candidateEntry;

    njCalcPoints(&entry->worldMatrix_0x84,
                 (NJS_POINT3 *)&init_8c04c940[entry->variantIndex_0x2e0],
                 var_collideSelfBox_8c228978.v, 8);

    var_collideScanCursor_8c228974 = var_tasks_8c1bac28;
    for (;;) {
        if (var_collideScanCursor_8c228974->action == NULL) {
            return NULL;
        }

        if (var_collideScanCursor_8c228974 != self
            && var_collideScanCursor_8c228974->action != (TaskAction)-1) {
            candidateEntry = (TrafficEntry *)var_collideScanCursor_8c228974->state;
            njCalcPoints(&candidateEntry->worldMatrix_0x84,
                         init_8c04c940[candidateEntry->variantIndex_0x2e0],
                         var_collideCandidateBox_8c2289d8.v, 8);
            if (njCollisionCheckBB(&var_collideSelfBox_8c228978,
                                   &var_collideCandidateBox_8c2289d8)) {
                return candidateEntry;
            }
        }

        var_collideScanCursor_8c228974++;
    }
}

void CollideQueueReset_8c02e486(void)
{
    var_collideQueueCount_8c228b38 = 0;
}

void CollideQueueAdd_8c02e48e(void *obj)
{
    if (var_collideQueueCount_8c228b38 < 0x40) { // 64-slot queue; obj silently dropped once full
        var_collideQueueCount_8c228b38 = var_collideQueueCount_8c228b38 + 1;
        var_collideQueue_8c228a38[var_collideQueueCount_8c228b38 - 1] = obj;
    }
}

/* Returns the first queued entry whose bounding sphere overlaps self's box,
 * or NULL if none of the queued entries do. */
void *CollideQueueTest_8c02e4ac(void)
{
    Sint32 i;

    for (i = 0; i < var_collideQueueCount_8c228b38; i++) {
        if (njCollisionCheckBS(&var_collideSelfBox_8c228978, var_collideQueue_8c228a38[i])) {
            return var_collideQueue_8c228a38[i];
        }
    }

    return NULL;
}
