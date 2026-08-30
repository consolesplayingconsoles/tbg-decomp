#ifndef _023938_BUS_DRIVE_H
#define _023938_BUS_DRIVE_H

/* One point on a route line segment (var_8c227d84 entries point into an
 * array of these): remaining distance to the NEXT point, that point's
 * world x/z, and the unit direction (dx,dz) of the segment leading to it. */
typedef struct {
    float len_0x00;
    float x_0x04;
    float z_0x08;
    float dx_0x0c;
    float dz_0x10;
} LinePoint;

/* var_8c227d84 entry: a route line's point list plus its total length. */
typedef struct {
    LinePoint *points_0x00;
    float length_0x04;
} LineBusSegment;

/* var_8c227d88 entry, 0xc bytes/6 ushorts; altNext_0x04[1] is the next
 * segment when field_0x25c == 1, altNext_0x04[2] is the next segment when
 * var_8c1bbc2c == 2, altNext_0x04[0] is the default (used by 02412c);
 * fwdNext_0x00/backNext_0x02 are read by FUN_8c023e7e. */
typedef struct {
    Uint16 fwdNext_0x00;
    Uint16 backNext_0x02;
    Uint16 altNext_0x04[3];
    Uint16 fallbackNext_0x0a;
} LineBusNode;

/* Resets the bus's drivetrain to idle after a collision knockback or a
 * braking-sound update; sets bus_state_0x2b4 == 2. */
void BusDriveStop_8c023bce(void);

/* Computes 10 corner/lookahead ground-sample points around the bus and
 * queries each through the ground-query callback in field_0x2c8, filling
 * groundSamples_0x190; also reseeds posHistory_0x100[0]/[1] and the heading
 * unit vector field_0x274/0x278. Called by busInitPlaceBus_8c023310/
 * BusInitStart_8c023610 (023310_bus_init) and BusTask_8c022bdc (022bdc). */
void FUN_8c023938(void);

/* Walks the current route line segment forward by the bus's per-frame move
 * distance to find the next lane-crossing point, confirming it against the
 * crossing segment via IntersectSegments_8c0206f0. Called once per frame by
 * BusTask_8c022bdc (022bdc). */
void FUN_8c023e7e(void);

/* Rebuilds the bus's steering-correction direction from the corner
 * ground-probe results FUN_8c023938 fills into groundSamples_0x190, then
 * averages two pairs of those probes' interpolated heights into
 * posY_0x0f8/posHistory_0x100[0].y. Called by BusInitStart_8c023610 (023310_bus_init)
 * and BusTask_8c022bdc (022bdc) once per frame while driving. */
void FUN_8c023cba(void);

#endif // _023938_BUS_DRIVE_H
