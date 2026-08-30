/* @unit BusLine */
#include <shinobi.h>
#include <math.h> /* acosf */

#include "sectionB.h"
#include "023938_bus_drive.h" /* LinePoint, LineBusSegment, LineBusNode */
#include "02412c.h"

/* ====================
 * Functions
 * ====================
 */

/* See 02412c.h. */
int BusLineAdvance_8c02412c(void)
{
    LinePoint *point = (LinePoint *)var_busState_8c1bb9d0.currentLinePointPtr_0x2b8;
    float remaining = var_busState_8c1bb9d0.lineSegmentRemaining_0x2bc;
    float dx, dz, dist, dxN, dzN;
    float angle, angleUnits;
    int angleUnitsInt;

    for (;;) {
        if (point->len_0x00 > remaining) {
            break;
        }

        remaining -= point->len_0x00;
        point++;

        if (point->len_0x00 == 0.0f) {
            /* Segment exhausted -- move to the next one via the node table. */
            LineBusNode *nodes = var_8c227d88;
            LineBusSegment *segs = var_8c227d84;
            LineBusNode *node = &nodes[var_busState_8c1bb9d0.currentLineNodeIdx_0x33c];
            int nextIdx;

            if (node->altNext_0x04[1] != 0xffff && var_busState_8c1bb9d0.mirrorButtonState_0x25c == 1) {
                nextIdx = node->altNext_0x04[1];
            } else if (node->altNext_0x04[2] != 0xffff && var_8c1bbc2c == 2) {
                nextIdx = node->altNext_0x04[2];
            } else {
                nextIdx = node->altNext_0x04[0];
            }

            var_busState_8c1bb9d0.currentLineNodeIdx_0x33c = (Uint16)nextIdx;
            var_busState_8c1bb9d0.lineSegmentProgress_0x2c0 = remaining;
            point = segs[nextIdx].points_0x00;
        }
    }

    var_busState_8c1bb9d0.currentLinePointPtr_0x2b8 = (int)point;
    var_busState_8c1bb9d0.lineSegmentRemaining_0x2bc = remaining;

    var_busState_8c1bb9d0.laneTargetX_0x0ec = point->x_0x04 + remaining * point->dx_0x0c;
    var_busState_8c1bb9d0.laneTargetZ_0x0f0 = point->z_0x08 + remaining * point->dz_0x10;

    dx = var_busState_8c1bb9d0.posX_0x0f4 - var_busState_8c1bb9d0.laneTargetX_0x0ec;
    dz = var_busState_8c1bb9d0.posZ_0x0fc - var_busState_8c1bb9d0.laneTargetZ_0x0f0;
    dist = njSqrt(dx * dx + dz * dz);
    dxN = dx / dist;
    dzN = dz / dist;

    var_busState_8c1bb9d0.posX_0x0f4 = var_busState_8c1bb9d0.laneTargetX_0x0ec + dxN * var_busState_8c1bb9d0.laneOffset_0x2c4;
    var_busState_8c1bb9d0.posZ_0x0fc = var_busState_8c1bb9d0.laneTargetZ_0x0f0 + dzN * var_busState_8c1bb9d0.laneOffset_0x2c4;

    /* acosf(dzN) as a 0..65536 angle unit, signed by dxN's sign (a signed
     * atan2-style angle built from acos, matching the asm's FTRC + NEG). */
    angle = acosf(dzN);
    angleUnits = angle * 65536.0f / 6.283184051513672f;
    angleUnitsInt = (int)angleUnits;
    if (dxN <= 0.0f) {
        angleUnitsInt = -angleUnitsInt;
    }

    var_busState_8c1bb9d0.targetHeadingAngle_0x254 = angleUnitsInt;

    return 1;
}
