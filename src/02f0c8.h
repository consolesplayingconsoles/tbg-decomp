#ifndef _02F0C8_H
#define _02F0C8_H

#include "014a9c_tasks.h"
#include "026710_traffic.h" /* TrafficEntry, PathRecord */

/* Continues the sample-point scan started by TrafficPathScanBuild_8c02f0c8: walks the
 * remaining (x, z) samples in var_8c228b48 (cursor var_8c228b9c, bound
 * var_8c228ba0), returning the state of the first live, non-excluded
 * (var_8c228b98) task whose entry sits within 2.5 units of a sample.
 * Takes no arguments -- all state is the shared globals TrafficPathScanBuild_8c02f0c8 set
 * up. */
void *TrafficPathScanNext_8c02f212(void);

/* Walks `startProgress` units along the path records starting at
 * firstRecord (advancing through entry->resolvedArgs_0x304 on each
 * record run's end), then samples (x, z) positions every 5.0 units across
 * the next `window` units into the shared scratch buffer var_8c228b48
 * (cursor/bound var_8c228b9c/var_8c228ba0, also read by TrafficPathScanNext_8c02f212).
 * Returns the first sample's occupant -- the player's bus sentinel
 * (var_8c1bbd9c) if a sample lands near either of its two tracked points,
 * else the state of the first live task (other than self) within 2.5 units
 * -- or NULL if no sample was collected or none matched. Nonzero means
 * "reject": the caller frees the task and aborts the spawn. */
void *TrafficPathScanBuild_8c02f0c8(Task *self, TrafficEntry *entry, PathRecord *firstRecord,
                    Sint32 argIndex, float startProgress, float window);

/* Caches (var_8c228b44) the id-group of var_8c228b40 containing the first
 * live task's entry->signalWaitFrameId_0x450 marker, then reports whether typeCode is a
 * member of that same group. */
Sint32 TrafficPathScanTypeInGroup_8c02f28a(Sint32 typeCode);

#endif // _02F0C8_H
