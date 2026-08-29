/* 8c020594 */
#ifndef _020594_H
#define _020594_H

#include <shinobi.h>

#include "sectionB.h" /* BusState */

/* Orients/positions a vehicle model: builds a basis from the bus's recent
 * ground-height history and width (banking the model to the local slope),
 * yaws it toward the height delta between two more history points via
 * atan2f+njRotateZ, then sets the translation to the bus's current
 * position. Called every frame the bus/traffic entity's ground probes are
 * refreshed. `bus` may be a BusState* or a TrafficEntry* cast to BusState*
 * -- the two share this rendering-field prefix. */
void VehicleModelPlace_8c020594(NJS_MATRIX *matrix, BusState *bus);

#endif // _020594_H
