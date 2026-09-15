#ifndef _02412C_BUS_LINE_H
#define _02412C_BUS_LINE_H

/* Called by BusTask_8c022bdc (022bdc), once per frame while driving under
 * mapped-route steering. Advances the bus along its route line, writes the
 * resulting lane target waypoint, offsets the bus's actual position from it,
 * and recomputes the heading angle. Always returns 1; the caller ignores it. */
int BusLineAdvance_8c02412c(void);

#endif // _02412C_BUS_LINE_H
