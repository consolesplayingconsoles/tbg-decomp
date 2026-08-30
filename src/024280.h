#ifndef _024280_H
#define _024280_H

/* Drives the throttle/brake needle-ramp state machine
 * (applyThrottle_8c024320, applyBraking_8c024530, applyBrakingSfx_8c024606),
 * the rear/side-mirror buttons, and -- in direct steering mode -- the
 * steering-wheel force-feedback ramp. */
void BusInputUpdate_8c0246b2(void);

/* Role unclear (mirror view update?). */
void BusInputMirrorLookahead_8c024280(void);

#endif // _024280_H
