/* 8c020594 */
#ifndef _020594_VEHICLE_MODEL_H
#define _020594_VEHICLE_MODEL_H

#include <shinobi.h>

#include "sectionB.h" /* BusState */

/* Builds a vehicle's world matrix from its ground probes: pitch from the
 * forward probe station's height minus the body's over width_0x23c (their
 * 4.9 spacing), heading from headingDirX_0x274/Z_0x278, then roll from the
 * lateral probe pair posHistory_0x100[2]/[3] over height_0x244 (their 2.5
 * spacing) -- njRotateZ, the matrix's local z being the heading by then.
 * Also caches the pitch in pitchSin_0x26c/pitchCos_0x270 for the camera.
 * `bus` may be a TrafficEntry* cast to BusState*: the two share this
 * rendering-field prefix. */
void VehicleModelPlace_8c020594(NJS_MATRIX *matrix, BusState *bus);

#endif // _020594_VEHICLE_MODEL_H
