/* 8c023310 */
#ifndef _023310_BUS_INIT_H
#define _023310_BUS_INIT_H

#include "023938_bus_drive.h" /* LineBusSegment, LineBusNode */

/* =======================
 * Non-initialized Globals
 * =======================
 */

/* The active course's route line: its segments (each a LinePoint list) and
 * the node table that links them. Copied from
 * var_currentCourse_8c1bb868.lineBus_0x08/lineNodes_0x0c by BusInitStart_8c023610,
 * alongside var_activeGroundGrid_8c2264d4/var_activeAttrGrid_8c228b3c. */
extern LineBusSegment *var_lineSegments_8c227d84;
extern LineBusNode *var_lineNodes_8c227d88;

/* =========
 * Functions
 * =========
 */

/* Places the player's bus at the current run's starting stop and arms its
 * per-frame task (BusTask_8c022bdc). Called once at the start of a run. */
void BusInitStart_8c023610(void);

#endif // _023310_BUS_INIT_H
