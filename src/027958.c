/* @unit BusDraw */

#include <shinobi.h>

#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "sectionB.h"
#include "026710_traffic.h"     /* TrafficEntry */
#include "020914_ground_query.h" /* GroundQueryResult */
#include "020b6c_ground_probe.h" /* GroundProbeTrackPolygonAtHeight_8c021290, GroundProbeInterpolateHeight_8c020f7e */
#include "020594.h"              /* VehicleModelPlace_8c020594 */
#include "0222dc_fadecmd.h"      /* FadeCmdPushCall2_8c022420 */
#include "028258_objects.h"     /* TrafficSignal */
#include "027958.h"

/* ====================
 * Forward Declarations
 * ====================
 */

STATIC void busDrawSimpleCb_8c027a88(int entityArg, int lod);
STATIC void busDrawSimpleCb_8c027bac(int entityArg, int lod);

/* ====================
 * Functions
 * ====================
 */

/* Sets the visibility flag (0x37 = shown, 0x3f = hidden -- NJD_EVAL_HIDE-style
 * codes) on the bus's five blinker-arrow light models from blinker_0x080's
 * bit mask (bits 0-4 => models at blinkerLightA_0x02c/030/034/038/03c), then, based
 * on field_0x000 (a turn-maneuver phase), copies distance_traveled_0x070 into
 * a sixth light model (field_0x028's target, offset +0x14) and toggles up to
 * one more model (field_0x040/044/048) for the phases that use one. */
void BusDrawUpdateModels_8c027958(BusState *bus)
{
    bus->field_0x018->ang[0] = bus->acc_0x078;
    bus->field_0x018->ang[2] = bus->ang_0x07c;
    bus->frontWheelA_0x01c->ang[0] = bus->distance_traveled_0x070;
    bus->frontWheelA_0x01c->ang[1] = bus->ang_0x074;
    bus->frontWheelB_0x020->ang[0] = bus->distance_traveled_0x070;
    bus->frontWheelB_0x020->ang[1] = bus->ang_0x074;
    bus->rearWheel_0x024->ang[0] = bus->distance_traveled_0x070;

    bus->blinkerLightA_0x02c->evalflags = (bus->blinker_0x080 & 0x1) ? 0x37 : 0x3f;
    bus->blinkerLightB_0x030->evalflags = (bus->blinker_0x080 & 0x2) ? 0x37 : 0x3f;
    bus->blinkerLightC_0x034->evalflags = (bus->blinker_0x080 & 0x4) ? 0x37 : 0x3f;
    bus->blinkerLightD_0x038->evalflags = (bus->blinker_0x080 & 0x8) ? 0x37 : 0x3f;
    bus->blinkerLightE_0x03c->evalflags = (bus->blinker_0x080 & 0x10) ? 0x37 : 0x3f;

    switch (bus->field_0x000) {
    case 0x0:
    case 0x2:
    case 0x4:
    case 0x6:
    case 0x8:
    case 0x12:
        break;

    case 0x14:
    case 0x16:
        bus->field_0x028->ang[0] = bus->distance_traveled_0x070;
        if ((bus->blinker_0x080 & 0x40) == 0) {
            bus->field_0x044->evalflags = 0x3f;
        } else {
            bus->field_0x044->evalflags = 0x37;
        }
        break;

    case 0xe:
    case 0x10:
        bus->field_0x028->ang[0] = bus->distance_traveled_0x070;
        break;

    case 0x1c:
    case 0x1e:
        if ((bus->blinker_0x080 & 0x20) == 0) {
            bus->field_0x040->evalflags = 0x3f;
        } else {
            bus->field_0x040->evalflags = 0x37;
        }
        break;

    case 0x1a:
        if ((bus->blinker_0x080 & 0x40) != 0) {
            bus->field_0x048->evalflags = 0x37;
        } else {
            bus->field_0x048->evalflags = 0x3f;
        }
        break;

    default:
        break;
    }
}

/* FadeCmdPushCall2_8c022420 callback registered by BusDrawPlaceEntity_8c027c3c's near test
 * (lod = 1 once the entity is past 55m). lod == 0 draws the near/detailed
 * variant -- simple-light model_0x0c plus bodyModel_0x14 (an interior/window
 * layer bracketed by njControl3D 0x2500/0x100) and also updates the entity's
 * blinker lights via BusDrawUpdateModels_8c027958 -- lod != 0 draws model_0x10 only, using
 * simple light at night (var_timeOfDay_8c18ad20 == 2) or easy light by day. */
STATIC void busDrawSimpleCb_8c027a88(int entityArg, int lod)
{
    TrafficEntry *entity = (TrafficEntry *)entityArg;

    njMultiMatrix(0, &entity->worldMatrix_0x84);

    if (lod == 0) {
        njCnkSetSimpleLightIntensity(entity->simpleLightIntensity0_0x0c4, entity->simpleLightIntensity1_0x0c8);
        njCnkSetSimpleLightColor(entity->simpleLightColorR_0x0cc, entity->simpleLightColorG_0x0d0, entity->simpleLightColorB_0x0d4);
        BusDrawUpdateModels_8c027958((BusState *)entity);
        njSetTexture(entity->texlistLarge_0x04);
        njCnkSimpleDrawObject((NJS_CNK_OBJECT *)entity->modelLarge_0x0c);
        njControl3D(0x2500);
        njCnkModDrawObject((NJS_CNK_OBJECT *)entity->bodyModel_0x14);
        njControl3D(0x100);
    } else if (var_timeOfDay_8c18ad20 == 2) {
        njCnkSetSimpleLightIntensity(entity->simpleLightIntensity0_0x0c4, entity->simpleLightIntensity1_0x0c8);
        njCnkSetSimpleLightColor(entity->simpleLightColorR_0x0cc, entity->simpleLightColorG_0x0d0, entity->simpleLightColorB_0x0d4);
        njSetTexture(entity->texlistSmall_0x08);
        njCnkSimpleDrawObject((NJS_CNK_OBJECT *)entity->modelSmall_0x10);
    } else {
        njCnkSetEasyLightIntensity(entity->easyLightIntensity0_0x0d8, entity->easyLightIntensity1_0x0dc);
        njCnkSetEasyLightColor(entity->easyLightColorR_0x0e0, entity->easyLightColorG_0x0e4, entity->easyLightColorB_0x0e8);
        njSetTexture(entity->texlistSmall_0x08);
        njCnkEasyDrawObject((NJS_CNK_OBJECT *)entity->modelSmall_0x10);
    }
}

/* FadeCmdPushCall2_8c022420 callback registered by BusDrawPlaceEntity_8c027c3c's far
 * (rear-view mirror) test (lod = 1 once the entity is past 28m). Same shape
 * as busDrawSimpleCb_8c027a88's two draw variants, but with no near/detailed
 * variant and no night/day split for the far draw -- lod == 0 always uses
 * simple light plus BusDrawUpdateModels_8c027958, lod != 0 always uses easy light. */
STATIC void busDrawSimpleCb_8c027bac(int entityArg, int lod)
{
    TrafficEntry *entity = (TrafficEntry *)entityArg;

    njMultiMatrix(0, &entity->worldMatrix_0x84);

    if (lod == 0) {
        njCnkSetSimpleLightIntensity(entity->simpleLightIntensity0_0x0c4, entity->simpleLightIntensity1_0x0c8);
        njCnkSetSimpleLightColor(entity->simpleLightColorR_0x0cc, entity->simpleLightColorG_0x0d0, entity->simpleLightColorB_0x0d4);
        BusDrawUpdateModels_8c027958((BusState *)entity);
        njSetTexture(entity->texlistLarge_0x04);
        njCnkSimpleDrawObject((NJS_CNK_OBJECT *)entity->modelLarge_0x0c);
    } else {
        njCnkSetEasyLightIntensity(entity->easyLightIntensity0_0x0d8, entity->easyLightIntensity1_0x0dc);
        njCnkSetEasyLightColor(entity->easyLightColorR_0x0e0, entity->easyLightColorG_0x0e4, entity->easyLightColorB_0x0e8);
        njSetTexture(entity->texlistSmall_0x08);
        njCnkEasyDrawObject((NJS_CNK_OBJECT *)entity->modelSmall_0x10);
    }
}

/* See 027958.h for the full description. */
void BusDrawPlaceEntity_8c027c3c(TrafficEntry *entity, float heading)
{
    int registered = 0;
    float dx, dz, dist;

    /* Near test: is the entity within 200m and roughly ahead of the
     * player's last move-delta (dot product against the cos(~80deg)
     * threshold 0.174)? */
    dx = var_busState_8c1bb9d0.posX_0x2fc - entity->posX_0xf4;
    dz = var_busState_8c1bb9d0.posZ_0x304 - entity->posZ_0xfc;
    dist = njSqrt(dx * dx + dz * dz);
    if (dist < 200.0f &&
        0.174f <= (var_busState_8c1bb9d0.moveDeltaX_0x308 * dx + dz * var_busState_8c1bb9d0.moveDeltaZ_0x310) /
                  (var_busState_8c1bb9d0.moveDeltaMagnitude_0x314 * dist)) {
        FadeCmdPushCall2_8c022420(0, busDrawSimpleCb_8c027a88, (int)entity, 55.0f <= dist);
        registered = 1;
    }

    /* Far (rear-view mirror) test: within 50m and inside a tight
     * (~15-degree) cone around mirrorWorldOffsetX_0x318../0x330. */
    dx = entity->posX_0xf4 - var_busState_8c1bb9d0.mirrorWorldOffsetX_0x318;
    dz = entity->posZ_0xfc - var_busState_8c1bb9d0.mirrorWorldOffsetZ_0x320;
    dist = njSqrt(dx * dx + dz * dz);
    if (dist < 50.0f &&
        0.966f < (var_busState_8c1bb9d0.mirrorDirX_0x324 * dx + dz * var_busState_8c1bb9d0.mirrorDirZ_0x32c) /
                 (var_busState_8c1bb9d0.mirrorDist_0x330 * dist)) {
        int farLod;

        if (28.0f <= dist) {
            farLod = 1;
        } else {
            farLod = 0;
            if (15.0f < dist) {
                entity->mirrorVisible_0x268 = 1;
            }
        }
        FadeCmdPushCall2_8c022420(1, busDrawSimpleCb_8c027bac, (int)entity, farLod);
        registered = 1;
    }

    /* An entity whose ground-probe callback is the plain polygon tracker
     * (as opposed to some other, presumably cheaper probe) always
     * re-aligns below and starts each frame un-aligned. */
    if (entity->probeFn_0x2c8 == GroundProbeTrackPolygonAtHeight_8c021290) {
        registered = 1;
        entity->groundAligned_0x494 = 0;
    }

    if (registered || entity->field_0x490 < 85.333336f) {
        int prev, delta, headingDelta;

        entity->distanceTraveled_0x070 -= (int)(entity->speed_0x27c * 65536.0f);

        headingDelta = entity->headingAlt_0x254 - entity->heading_0x250;
        entity->headingDelta_0x258 = headingDelta;
        entity->ang_0x074 = headingDelta;

        prev = entity->ang_0x07c;
        delta = (int)-((float)headingDelta * entity->speed_0x27c) - prev;
        if (delta < 0) {
            if (delta < -0xb6) {
                delta = -0xb6;
            }
            delta += prev;
            if (delta < -0x2d8) {
                delta = -0x2d8;
            }
        } else {
            if (delta > 0xb6) {
                delta = 0xb6;
            }
            delta += prev;
            if (delta > 0x2d8) {
                delta = 0x2d8;
            }
        }
        entity->ang_0x07c = delta;

        prev = entity->acc_0x078;
        delta = (int)(heading * 45000.0f) - prev;
        if (delta < 0) {
            if (delta < -0x89) {
                delta = -0x89;
            }
            delta += prev;
            if (delta < -0x222) {
                delta = -0x222;
            }
        } else {
            if (delta > 0x89) {
                delta = 0x89;
            }
            delta += prev;
            if (delta > 0x222) {
                delta = 0x222;
            }
        }
        entity->acc_0x078 = delta;

        if (entity->groundAligned_0x494 == 0 || entity->speed_0x27c != 0.0f) {
            GroundProbeFn probe = entity->probeFn_0x2c8;
            GroundQueryResult *probeA = &entity->groundProbe_0x190[0];
            GroundQueryResult *probeB = &entity->groundProbe_0x190[1];
            GroundQueryResult *probeC = &entity->groundProbe_0x190[2];

            if (entity->driveState_0x2b4 == 1) {
                var_activeGroundGrid_8c2264d4 = var_groundGridFallback_8c1bb86c;
            }

            probe(entity->probeSideAX_0x118, entity->probeSideAY_0x11c, entity->probeSideAZ_0x120, probeA);
            probe(entity->probeSideBX_0x124, entity->probeSideBY_0x128, entity->probeSideBZ_0x12c, probeB);
            probe(entity->frontPointX_0x100, entity->frontPointY_0x104, entity->frontPointZ_0x108, probeC);

            GroundProbeInterpolateHeight_8c020f7e(probeA, &entity->probeSideAX_0x118);
            GroundProbeInterpolateHeight_8c020f7e(probeB, &entity->probeSideBX_0x124);
            entity->posY_0xf8 = (entity->probeSideAY_0x11c + entity->probeSideBY_0x128) / 2.0f;
            GroundProbeInterpolateHeight_8c020f7e(probeC, &entity->frontPointX_0x100);

            if (entity->driveState_0x2b4 == 1) {
                var_activeGroundGrid_8c2264d4 = var_8c1bb880;
            }

            VehicleModelPlace_8c020594(&entity->worldMatrix_0x84, (BusState *)entity);
            entity->groundAligned_0x494 = 1;
            return;
        }
    }

    entity->groundAligned_0x494 = 0;
}

/* FadeCmdPushCall2_8c022420 callback for a type-1 TrafficSignal: draws
 * model_0xb8/tlist_0xb4 at the given matrix (multiplied onto identity). */
void BusDrawSignal_8c0281ac(int objArg, int matrixArg)
{
    TrafficSignal *obj = (TrafficSignal *)objArg;
    NJS_MATRIX *matrix = (NJS_MATRIX *)matrixArg;

    switch (obj->frame_0x0c) {
    case 1:
        obj->frames_0x10[0]->evalflags = 0x37;
        obj->frames_0x10[1]->evalflags = 0x3f;
        obj->frames_0x10[2]->evalflags = 0x3f;
        break;
    case 2:
        obj->frames_0x10[0]->evalflags = 0x3f;
        obj->frames_0x10[1]->evalflags = 0x37;
        obj->frames_0x10[2]->evalflags = 0x3f;
        break;
    case 0:
        obj->frames_0x10[0]->evalflags = 0x3f;
        obj->frames_0x10[1]->evalflags = 0x3f;
        obj->frames_0x10[2]->evalflags = 0x37;
        break;
    default:
        break;
    }

    njMultiMatrix(0, matrix);
    njSetTexture(obj->tlist_0xb4);
    njCnkSimpleDrawObject((NJS_CNK_OBJECT *)obj->model_0xb8);
}

/* Called each frame; forces bits 0x08/0x10 on in blinker_0x080 (see
 * BusDrawUpdateModels_8c027958), then runs lightCoeffRow_0x0c4[0..4] -- the bus's copy of the
 * directional-light coefficient row -- through a 20-frame crossfade state
 * machine between the cached "off" row (var_8c1bbda8/var_8c1bbdc4) and "on"
 * row (var_8c1bbdb0/var_8c1bbdd0), gated by lightFadeGate_0x2dc (set elsewhere). */
void BusDrawFadeLights_8c028022(BusState *bus)
{
    bus->blinker_0x080 |= 0x18;

    switch (bus->lightFadeState_0x2d8) {
    case 0: /* idle: fades in once lightFadeGate_0x2dc goes nonzero */
        if (bus->lightFadeGate_0x2dc != 0) {
            bus->lightFadeState_0x2d8 = 1;
        }
        break;

    case 1: /* fading in */
        bus->lightCoeffRow_0x0c4[0] += var_8c1bbda0[0];
        if (bus->lightCoeffRow_0x0c4[0] >= var_8c1bbdb0[0]) {
            bus->lightCoeffRow_0x0c4[0] = var_8c1bbdb0[0];
            bus->lightCoeffRow_0x0c4[1] = var_8c1bbdb4;
            bus->lightCoeffRow_0x0c4[2] = var_8c1bbdd0[0];
            bus->lightCoeffRow_0x0c4[3] = var_8c1bbdd0[1];
            bus->lightCoeffRow_0x0c4[4] = var_8c1bbdd0[2];
            bus->lightFadeState_0x2d8 = 2;
        } else {
            bus->lightCoeffRow_0x0c4[1] += var_8c1bbda0[1];
            bus->lightCoeffRow_0x0c4[2] += var_8c1bbdb8[0];
            bus->lightCoeffRow_0x0c4[3] += var_8c1bbdb8[1];
            bus->lightCoeffRow_0x0c4[4] += var_8c1bbdb8[2];
        }
        break;

    case 2: /* holding at "on": fades out once lightFadeGate_0x2dc goes zero */
        if (bus->lightFadeGate_0x2dc == 0) {
            bus->lightFadeState_0x2d8 = 3;
        }
        break;

    case 3: /* fading out */
        bus->lightCoeffRow_0x0c4[0] -= var_8c1bbda0[0];
        if (bus->lightCoeffRow_0x0c4[0] <= var_8c1bbda8[0]) {
            bus->lightCoeffRow_0x0c4[0] = var_8c1bbda8[0];
            bus->lightCoeffRow_0x0c4[1] = var_8c1bbdac;
            bus->lightCoeffRow_0x0c4[2] = var_8c1bbdc4[0];
            bus->lightCoeffRow_0x0c4[3] = var_8c1bbdc4[1];
            bus->lightCoeffRow_0x0c4[4] = var_8c1bbdc4[2];
            bus->lightFadeState_0x2d8 = 0;
        } else {
            bus->lightCoeffRow_0x0c4[1] -= var_8c1bbda0[1];
            bus->lightCoeffRow_0x0c4[2] -= var_8c1bbdb8[0];
            bus->lightCoeffRow_0x0c4[3] -= var_8c1bbdb8[1];
            bus->lightCoeffRow_0x0c4[4] -= var_8c1bbdb8[2];
        }
        break;
    }
}

/* FadeCmdPushCall2_8c022420 callback for a type 2/3/4 TrafficSignal
 * attachment: toggles NJD_EVAL_HIDE (bit 3) on frames_0x10[0] from
 * obj->drawA_0xc8, then draws like BusDrawSignal_8c0281ac. */
void BusDrawSignalAttachment_8c028206(int objArg, int matrixArg)
{
    TrafficSignal *obj = (TrafficSignal *)objArg;
    NJS_MATRIX *matrix = (NJS_MATRIX *)matrixArg;

    if (obj->drawA_0xc8 == 0) {
        obj->frames_0x10[0]->evalflags |= 8;
    } else {
        obj->frames_0x10[0]->evalflags &= ~8;
    }

    njMultiMatrix(0, matrix);
    njSetTexture(obj->tlist_0xb4);
    njCnkSimpleDrawObject((NJS_CNK_OBJECT *)obj->model_0xb8);
}
