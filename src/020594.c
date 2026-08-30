/* @unit VehicleModel */
#include <shinobi.h>
#include "includes.h" /* TWO_PI, STATIC */
#include "serial_debug.h"

#include "sectionB.h"
#include "020594.h"

/* ====================
 * Forward Declarations
 * ====================
 */

STATIC Float unused_8c020676(Float step, NJS_POINT3 *from, NJS_POINT3 *to,
                              NJS_POINT3 *out);

/* ====================
 * Functions
 * ====================
 */

void VehicleModelPlace_8c020594(NJS_MATRIX *matrix, BusState *bus)
{
    Float *m = *matrix;
    Float dy = bus->posHistory_0x100[0].y - bus->posY_0x0f8;
    Float dist = njSqrt(dy * dy + bus->field_0x23c * bus->field_0x23c);
    Float cosT = bus->field_0x23c / dist;
    Float sinT = dy / dist;
    Float angle;

    bus->pitchCos_0x270 = cosT;
    bus->pitchSin_0x26c = sinT;

    m[0] = bus->headingDirZ_0x278;
    m[1] = 0.0f;
    m[2] = -bus->headingDirX_0x274;
    m[3] = 0.0f;

    m[4] = -(bus->headingDirX_0x274 * sinT);
    m[5] = cosT;
    m[6] = -(bus->headingDirZ_0x278 * sinT);
    m[7] = 0.0f;

    m[8] = bus->headingDirX_0x274 * cosT;
    m[9] = sinT;
    m[10] = bus->headingDirZ_0x278 * cosT;
    m[11] = 0.0f;

    angle = atan2f(bus->posHistory_0x100[3].y - bus->posHistory_0x100[2].y,
                    bus->field_0x244);
    njRotateZ(matrix, (Sint32)(angle * 65536.0f / TWO_PI));

    m[12] = bus->posX_0x0f4;
    m[13] = bus->posY_0x0f8;
    m[14] = bus->posZ_0x0fc;
    m[15] = 1.0f;
}

/* Never called -- dead code kept for object parity with the original
 * binary. Scales (to - from) so its length equals `step`, via
 * njDistanceP2P for the true distance between the two points. */
STATIC Float unused_8c020676(Float step, NJS_POINT3 *from, NJS_POINT3 *to,
                              NJS_POINT3 *out)
{
    Float dist = njDistanceP2P(from, to);
    Float scale = dist / step;

    out->x = (to->x - from->x) / scale;
    out->y = (to->y - from->y) / scale;
    out->z = (to->z - from->z) / scale;

    return dist;
}
