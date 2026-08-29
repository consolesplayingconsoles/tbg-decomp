#ifndef _023938_BUS_DRIVE_H
#define _023938_BUS_DRIVE_H

/* Resets the bus's drivetrain to idle after a collision knockback or a
 * braking-sound update; sets bus_state_0x2b4 == 2. */
void BusDriveStop_8c023bce(void);

/* Computes 10 corner/lookahead ground-sample points around the bus and
 * queries each through the ground-query callback in field_0x2c8, filling
 * groundSamples_0x190; also reseeds posHistory_0x100[0]/[1] and the heading
 * unit vector field_0x274/0x278. Called by busInitPlaceBus_8c023310/
 * FUN_8c023610 (023310_bus_init) and BusTask_8c022bdc (022bdc). */
void FUN_8c023938(void);

/* Walks the current route line segment forward by the bus's per-frame move
 * distance to find the next lane-crossing point, confirming it against the
 * crossing segment via IntersectSegments_8c0206f0. Called once per frame by
 * BusTask_8c022bdc (022bdc). */
void FUN_8c023e7e(void);

/* Rebuilds the bus's steering-correction direction from the corner
 * ground-probe results FUN_8c023938 fills into groundSamples_0x190, then
 * averages two pairs of those probes' interpolated heights into
 * posY_0x0f8/posHistory_0x100[0].y. Called by FUN_8c023610 (023310_bus_init)
 * and BusTask_8c022bdc (022bdc) once per frame while driving. */
void FUN_8c023cba(void);

#endif // _023938_BUS_DRIVE_H
