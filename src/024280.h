#ifndef _024280_H
#define _024280_H

/* Called by BusTask_8c022bdc (022bdc) once per frame while driving: drives
 * the throttle/brake needle-ramp state machine (FUN_8c024320,
 * applyBraking_8c024530, applyBrakingSfx_8c024606), the rear/side-mirror
 * buttons, and -- in direct steering mode -- the steering-wheel
 * force-feedback ramp. See 024280.c for the full breakdown. */
void FUN_8c0246b2(void);

/* Called by BusTask_8c022bdc (022bdc) with no arguments when
 * BusState.mirror_0x268 is set; role unclear (mirror view update?). */
void FUN_8c024280(void);

#endif // _024280_H
