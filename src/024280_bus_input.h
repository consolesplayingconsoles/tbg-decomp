#ifndef _024280_BUS_INPUT_H
#define _024280_BUS_INPUT_H

/* =======================
 * Non-initialized Globals
 * =======================
 */

/* Peak brake-pedal travel of the current press, scaled to 0..255 and never
 * walked back down; applyBrakingSfx_8c024606 picks the release note from
 * it. */
extern int var_brakePressPeak_8c227d8c;

/* =========
 * Functions
 * =========
 */

/* Drives the engine's off/starting/running state machine
 * (applyThrottle_8c024320, applyBraking_8c024530, applyBrakingSfx_8c024606),
 * the rear/side-mirror buttons, and -- in direct steering mode -- the
 * steering-wheel force-feedback ramp. */
void BusInputUpdate_8c0246b2(void);

/* Holds traffic drawn in the mirror to 20 km/h below the bus's own speed.
 * Only called while the mirror view is up. */
void BusInputCapMirrorTraffic_8c024280(void);

#endif // _024280_BUS_INPUT_H
