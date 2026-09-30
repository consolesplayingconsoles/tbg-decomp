/* 8c022bdc */
#ifndef _022BDC_BUS_H
#define _022BDC_BUS_H

#include "014a9c_tasks.h" /* Task */

/* Per-frame player-bus dispatcher, spawned as a task's action by
 * BusInitStart_8c023610 (023310_bus_init). */
void BusTask_8c022bdc(Task *task, void *state);

#endif // _022BDC_BUS_H
