#ifndef _02412C_H
#define _02412C_H

/* Called by BusTask_8c022bdc (022bdc) with no arguments, once per frame while
 * driving under mapped-route steering. Advances the bus along its route line
 * by the distance accumulated in lineSegmentRemaining_0x2bc/lineSegmentProgress_0x2c0 (switching segments
 * via the var_8c227d88 node table as needed), writes the resulting waypoint
 * to laneTargetX_0x0ec/laneTargetZ_0x0f0, offsets the bus's actual position from it by
 * the lane offset laneOffset_0x2c4, and recomputes the heading angle targetHeadingAngle_0x254.
 * Always returns 1; the caller ignores it. */
int BusLineAdvance_8c02412c(void);

#endif // _02412C_H
