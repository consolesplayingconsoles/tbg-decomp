/* @unit TrafficLookahead */

#include <shinobi.h>

#include "014a9c_tasks.h"       /* Task, TaskAction */
#include "026710_traffic.h"     /* TrafficEntry, PathRecord */
#include "02df3c.h"
#include "sectionB.h"           /* var_busState_8c1bb9d0, var_8c1bbd9c, var_tasks_8c1bac28 */

/* ====================
 * Functions
 * ====================
 */

/* Seeds entry->lookaheadPoints_0x49c from scratch (entry->lookaheadCacheLen_0x4ec/0x4f4/
 * 0x4fc/0x4f8 all start at 0 at this point), walking the path in 5-unit
 * steps out to 25.0 units and writing each step's world (x,z) into the
 * cache, then terminates it with a 9999.0 sentinel. -1 in the path cursor
 * (entry->resolvedArgs_0x304 ran out) ends the walk early, same as
 * TrafficLookaheadScan_8c02dfca's own refill loop below. */
void TrafficLookaheadInit_8c02df3c(TrafficEntry *entry)
{
    float *out = (float *)entry->lookaheadPoints_0x49c;
    PathRecord *cursor = entry->lookaheadCursor_0x4f4;
    float dist = entry->lookaheadCursorDist_0x4fc;

    for (;;) {
        if (entry->lookaheadCacheLen_0x4ec >= 25.0f) {
            *out = 9999.0f;
            entry->lookaheadCursor_0x4f4 = cursor;
            entry->lookaheadCursorDist_0x4fc = dist;
            return;
        }

        for (;;) {
            if (cursor == (PathRecord *)-1) {
                entry->lookaheadCacheLen_0x4ec += 5.0f;
                *out = 9999.0f;
                entry->lookaheadCursor_0x4f4 = cursor;
                entry->lookaheadCursorDist_0x4fc = dist;
                return;
            }
            if (dist < cursor->length_0x00) {
                break;
            }
            dist -= cursor->length_0x00;
            cursor++;
            if (cursor->length_0x00 == 0.0f) {
                entry->lookaheadCursorBlock_0x4f8++;
                cursor = entry->resolvedArgs_0x304[entry->lookaheadCursorBlock_0x4f8];
            }
        }

        out[0] = dist * cursor->dirX_0x0c + cursor->x_0x04;
        out[1] = dist * cursor->dirZ_0x10 + cursor->z_0x08;
        out += 2;
        entry->lookaheadCacheLen_0x4ec += 5.0f;
        dist += 5.0f;
    }
}

/* Tops entry->lookaheadPoints_0x49c back up to 20.0 units ahead if it's
 * fallen short (same path-walk as TrafficLookaheadInit_8c02df3c, but first
 * shifting the still-valid tail of the old cache down to slot 0), then
 * scans the (possibly just-refreshed) cache for a candidate within
 * `lookahead` (+5.0) units: the player bus's own recent-history/current
 * position, or another live traffic entry's front-reference/current
 * position. Returns var_8c1bbd9c (the player sentinel), the matched
 * entry's state, or NULL. `self` is the caller's own task, excluded from
 * the traffic-entry scan. */
void *TrafficLookaheadScan_8c02dfca(Task *self, TrafficEntry *entry, float lookahead)
{
    float cacheDist = entry->lookaheadCacheLen_0x4ec;
    float *out = (float *)entry->lookaheadPoints_0x49c;
    float remain;

    if (cacheDist < 20.0f) {
        float *src = (float *)entry->lookaheadPoints_0x49c + 2;
        PathRecord *cursor;
        float dist;

        for (remain = cacheDist; remain > 0.0f; remain -= 5.0f) {
            out[0] = src[0];
            out[1] = src[1];
            src += 2;
            out += 2;
        }

        cursor = entry->lookaheadCursor_0x4f4;
        dist = entry->lookaheadCursorDist_0x4fc;

        for (; cacheDist < 20.0f; cacheDist += 5.0f) {
            while (cursor != (PathRecord *)-1 && dist >= cursor->length_0x00) {
                dist -= cursor->length_0x00;
                cursor++;
                if (cursor->length_0x00 == 0.0f) {
                    entry->lookaheadCursorBlock_0x4f8++;
                    cursor = entry->resolvedArgs_0x304[entry->lookaheadCursorBlock_0x4f8];
                }
            }
            if (cursor == (PathRecord *)-1) {
                cacheDist += 5.0f;
                break;
            }

            out[0] = dist * cursor->dirX_0x0c + cursor->x_0x04;
            out[1] = dist * cursor->dirZ_0x10 + cursor->z_0x08;
            out += 2;
            dist += 5.0f;
        }

        if (out == (float *)entry->lookaheadPoints_0x49c) {
            return NULL;
        }
        *out = 9999.0f;
        entry->lookaheadCursor_0x4f4 = cursor;
        entry->lookaheadCursorDist_0x4fc = dist;
        entry->lookaheadCacheLen_0x4ec = cacheDist;
    }

    out = (float *)entry->lookaheadPoints_0x49c;
    for (remain = lookahead + 5.0f; ; remain -= 5.0f) {
        float px, pz;

        if (remain <= 0.0f) {
            px = out[0];
            pz = out[1];
            if (px != 9999.0f &&
                ((fabsf(var_busState_8c1bb9d0.posHistory_0x100[10].x - px) < 2.0f &&
                  fabsf(var_busState_8c1bb9d0.posHistory_0x100[10].z - pz) < 2.0f) ||
                 (fabsf(var_busState_8c1bb9d0.posHistory_0x100[11].x - px) < 2.0f &&
                  fabsf(var_busState_8c1bb9d0.posHistory_0x100[11].z - pz) < 2.0f) ||
                 (fabsf(var_busState_8c1bb9d0.posHistory_0x100[8].x - px) < 2.0f &&
                  fabsf(var_busState_8c1bb9d0.posHistory_0x100[8].z - pz) < 2.0f) ||
                 (fabsf(var_busState_8c1bb9d0.posHistory_0x100[9].x - px) < 2.0f &&
                  fabsf(var_busState_8c1bb9d0.posHistory_0x100[9].z - pz) < 2.0f) ||
                 (fabsf(var_busState_8c1bb9d0.posX_0x0f4 - px) < 2.5f &&
                  fabsf(var_busState_8c1bb9d0.posZ_0x0fc - pz) < 2.5f) ||
                 (fabsf(var_busState_8c1bb9d0.posHistory_0x100[0].x - px) < 2.5f &&
                  fabsf(var_busState_8c1bb9d0.posHistory_0x100[0].z - pz) < 2.5f))) {
                return var_8c1bbd9c;
            }
            return NULL;
        }

        px = out[0];
        pz = out[1];
        out += 2;
        if (px == 9999.0f) {
            return NULL;
        }

        if (fabsf(var_busState_8c1bb9d0.posHistory_0x100[10].x - px) < 2.0f &&
            fabsf(var_busState_8c1bb9d0.posHistory_0x100[10].z - pz) < 2.0f) {
            return var_8c1bbd9c;
        }
        if (fabsf(var_busState_8c1bb9d0.posHistory_0x100[11].x - px) < 2.0f &&
            fabsf(var_busState_8c1bb9d0.posHistory_0x100[11].z - pz) < 2.0f) {
            return var_8c1bbd9c;
        }
        if (fabsf(var_busState_8c1bb9d0.posHistory_0x100[8].x - px) < 2.0f &&
            fabsf(var_busState_8c1bb9d0.posHistory_0x100[8].z - pz) < 2.0f) {
            return var_8c1bbd9c;
        }
        if (fabsf(var_busState_8c1bb9d0.posHistory_0x100[9].x - px) < 2.0f &&
            fabsf(var_busState_8c1bb9d0.posHistory_0x100[9].z - pz) < 2.0f) {
            return var_8c1bbd9c;
        }
        if (fabsf(var_busState_8c1bb9d0.posX_0x0f4 - px) < 2.5f &&
            fabsf(var_busState_8c1bb9d0.posZ_0x0fc - pz) < 2.5f) {
            break;
        }
        if (fabsf(var_busState_8c1bb9d0.posHistory_0x100[0].x - px) < 2.5f &&
            fabsf(var_busState_8c1bb9d0.posHistory_0x100[0].z - pz) < 2.5f) {
            return var_8c1bbd9c;
        }

        var_collisionScanCursor_8c228974 = var_tasks_8c1bac28;
        for (;;) {
            if (var_collisionScanCursor_8c228974->action == NULL) {
                break;
            }
            if (var_collisionScanCursor_8c228974 != self
                && var_collisionScanCursor_8c228974->action != (TaskAction)-1) {
                TrafficEntry *candidate = (TrafficEntry *)var_collisionScanCursor_8c228974->state;
                if ((fabsf(candidate->rearPointX_0x10c - px) < 2.5f &&
                     fabsf(candidate->rearPointZ_0x114 - pz) < 2.5f) ||
                    (fabsf(candidate->posX_0xf4 - px) < 2.5f &&
                     fabsf(candidate->posZ_0xfc - pz) < 2.5f)) {
                    return candidate;
                }
            }
            var_collisionScanCursor_8c228974++;
        }
    }

    return var_8c1bbd9c;
}
