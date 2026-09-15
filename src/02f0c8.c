/* @unit TrafficPathScan */

#include <shinobi.h>
#include "014a9c_tasks.h"
#include "026710_traffic.h"
#include "02f0c8.h"
#include "sectionB.h"

/* =======================
 * Non-initialized Globals
 * =======================
 */

/** The route's signal ids grouped by intersection */
Sint32 *var_signalGroups_8c228b40;

/** The group whose intersection a vehicle occupies */
Sint32 *var_occupiedGroup_8c228b44;

/* Sample-point scratch buffer for up to 10 (x, z) pairs. */
float var_samples_8c228b48[20];

/* Task-slot pointer to exclude from scan. Setter still unclear. */
void *var_excludedTask_8c228b98;

float *var_sampleCursor_8c228b9c;
float *var_sampleEnd_8c228ba0;

/* ====================
 * Functions
 * ====================
 */

void *TrafficPathScanBuild_8c02f0c8(
    Task *self,
    TrafficEntry *entry,
    PathRecord *firstRecord,
    Sint32 argIndex,
    float startProgress,
    float window
)
{
    PathRecord *seg = firstRecord;
    float remaining = startProgress;
    float traveled;
    float *cursor;
    float *p;
    float x, z;
    Task *t;
    TrafficEntry *candidateEntry;

    while (seg->length_0x00 <= remaining) {
        remaining -= seg->length_0x00;
        seg++;
        if (seg->length_0x00 == 0.0f) {
            argIndex++;
            seg = entry->resolvedArgs_0x304[argIndex];
        }
    }

    cursor = var_samples_8c228b48;
    for (traveled = startProgress; traveled < startProgress + window; traveled += 5.0f) {
        while (seg != (PathRecord *)-1 && seg->length_0x00 <= remaining) {
            remaining -= seg->length_0x00;
            seg++;
            if (seg->length_0x00 == 0.0f) {
                argIndex++;
                seg = entry->resolvedArgs_0x304[argIndex];
            }
        }
        if (seg == (PathRecord *)-1) {
            break;
        }

        cursor[0] = remaining * seg->dirX_0x0c + seg->x_0x04;
        cursor[1] = remaining * seg->dirZ_0x10 + seg->z_0x08;
        cursor += 2;
        remaining += 5.0f;
    }

    if (cursor != var_samples_8c228b48) {
        var_sampleEnd_8c228ba0 = cursor;
        for (p = var_samples_8c228b48; p < cursor; p += 2) {
            x = p[0];
            z = p[1];

            if ((fabsf(var_busState_8c1bb9d0.posX_0x0f4 - x) < 2.5f &&
                 fabsf(var_8c1bbacc - z) < 2.5f) ||
                (fabsf(var_busState_8c1bb9d0.posHistory_0x100[0].x - x) < 2.5f &&
                 fabsf(var_8c1bbad8 - z) < 2.5f)) {
                var_sampleCursor_8c228b9c = p + 2;
                return var_8c1bbd9c;
            }

            for (t = var_tasks_8c1bac28; t->action != NULL; t++) {
                if (t->action != (TaskAction)-1 && t != self) {
                    candidateEntry = t->state;
                    if (fabsf(candidateEntry->posX_0xf4 - x) < 2.5f &&
                        fabsf(candidateEntry->posZ_0xfc - z) < 2.5f) {
                        var_sampleCursor_8c228b9c = p + 2;
                        return candidateEntry;
                    }
                }
            }
        }
    }

    return NULL;
}

void *TrafficPathScanNext_8c02f212(void)
{
    float *p;
    Task *t;
    TrafficEntry *candidateEntry;
    int advance;

    p = var_sampleCursor_8c228b9c;
    for (;;) {
        if (p >= var_sampleEnd_8c228ba0) {
            return NULL;
        }

        for (t = var_tasks_8c1bac28; t->action != NULL; t++) {
            advance = 0;

            // Note: var_excludedTask_8c228b98 is never written
            if (t->action != (TaskAction) -1 && (void *) t != var_excludedTask_8c228b98) {
                candidateEntry = t->state;
                advance = 1;
                if (fabsf(candidateEntry->posX_0xf4 - p[0]) < 2.5f) {
                    advance = 2;
                    if (fabsf(candidateEntry->posZ_0xfc - p[1]) < 2.5f) {
                        var_sampleCursor_8c228b9c = p + 2;
                        return candidateEntry;
                    }
                }
            }

            p += advance;
        }
    }
}

Sint32 TrafficPathScanJunctionOccupied_8c02f28a(Sint32 signalId)
{
    Sint32 *group;
    Sint32 *scan;
    Task *t;

    group = var_occupiedGroup_8c228b44;
    if (var_occupiedGroup_8c228b44 == (Sint32 *)-1) {
        for (t = var_tasks_8c1bac28; ; t++) {
            if (t->action == NULL) {
                return 0;
            }
            if (t->action != (TaskAction)-1 &&
                ((TrafficEntry *)t->state)->atGroundJunction_0x50c != 0) {
                break;
            }
        }

        scan = var_signalGroups_8c228b40;
        group = var_signalGroups_8c228b40;
        while (((TrafficEntry *)t->state)->signalWaitFrameId_0x450 != (Uint32)*scan) {
            if (*scan == -1) {
                scan++;
                group = scan;
            } else {
                scan++;
            }
        }
        var_occupiedGroup_8c228b44 = group;
    }

    while (*group != -1) {
        if (signalId == *group) {
            return 1;
        }
        group++;
    }
    return 0;
}
