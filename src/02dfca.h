/* 8c02dfca: undecompiled */
#ifndef _02DFCA_H
#define _02DFCA_H

#include "014a9c_tasks.h"       /* Task */

/* Scans ahead of entry for another vehicle/the player bus within a lookahead
 * distance, similarly to FUN_8c02f0c8/FUN_8c02f212 (02f0c8) -- returns a
 * BusState/TrafficEntry-compatible pointer (may be var_8c1bbd9c, the player
 * sentinel) or NULL. Called by TrafficDriveVehicle_8c025b98 (025b98) with a
 * lookahead of entry->field_0x41c scaled by whether entry+0x424 (braking-
 * for-obstacle flag) is already set: 5.0 units longer once already braking,
 * to avoid the candidate flapping in and out of range every frame. */
void *FUN_8c02dfca(Task *self, void *entry, float lookahead);

#endif // _02DFCA_H
