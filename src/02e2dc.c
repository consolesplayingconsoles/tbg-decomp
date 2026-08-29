#include <shinobi.h>
#include "014a9c_tasks.h"
#include "02081c.h"
#include "02e2dc.h"
#include "sectionB.h"
#include "serial_debug.h" /* STATIC */

/* ====================
 * Initialized Globals
 * ====================
 */

/* Per-traffic-variant oriented bounding box, local space: 8 corners (top
 * face, then bottom face, each going around). Only init_8c04c820 (the
 * player bus's own box) and the init_8c04c940 pointer table below are
 * referenced outside this file; the rest are reached only through that
 * table. Literals carry full float32 precision (not clean decimals like
 * -0.835/3.05) because the original constants themselves aren't exact --
 * e.g. init_8c04c520.z bakes 3.0500001907348633, one ULP off "3.05" -- so a
 * rounder-looking literal would compile to a different bit pattern. */
STATIC NJS_POINT3 init_8c04c520[8] = {
    {-0.8349999785423279f, 1.5f, 3.0500001907348633f},
    {-0.8349999785423279f, 1.5f, -0.6499999761581421f},
    {0.8349999785423279f, 1.5f, -0.6499999761581421f},
    {0.8349999785423279f, 1.5f, 3.0500001907348633f},
    {-0.8349999785423279f, 0.0f, 3.0500001907348633f},
    {-0.8349999785423279f, 0.0f, -0.6499999761581421f},
    {0.8349999785423279f, 0.0f, -0.6499999761581421f},
    {0.8349999785423279f, 0.0f, 3.0500001907348633f},
};

STATIC NJS_POINT3 init_8c04c580[8] = {
    {-0.8650000095367432f, 1.5f, 3.0399999618530273f},
    {-0.8650000095367432f, 1.5f, -0.7599999904632568f},
    {0.8650000095367432f, 1.5f, -0.7599999904632568f},
    {0.8650000095367432f, 1.5f, 3.0399999618530273f},
    {-0.8650000095367432f, 0.0f, 3.0399999618530273f},
    {-0.8650000095367432f, 0.0f, -0.7599999904632568f},
    {0.8650000095367432f, 0.0f, -0.7599999904632568f},
    {0.8650000095367432f, 0.0f, 3.0399999618530273f},
};

STATIC NJS_POINT3 init_8c04c5e0[8] = {
    {-0.8999999761581421f, 1.5f, 3.799999952316284f},
    {-0.8999999761581421f, 1.5f, -0.8999999761581421f},
    {0.8999999761581421f, 1.5f, -0.8999999761581421f},
    {0.8999999761581421f, 1.5f, 3.799999952316284f},
    {-0.8999999761581421f, 0.0f, 3.799999952316284f},
    {-0.8999999761581421f, 0.0f, -0.8999999761581421f},
    {0.8999999761581421f, 0.0f, -0.8999999761581421f},
    {0.8999999761581421f, 0.0f, 3.799999952316284f},
};

STATIC NJS_POINT3 init_8c04c640[8] = {
    {-0.8500000238418579f, 1.5f, 3.9000000953674316f},
    {-0.8500000238418579f, 1.5f, -0.699999988079071f},
    {0.8500000238418579f, 1.5f, -0.699999988079071f},
    {0.8500000238418579f, 1.5f, 3.9000000953674316f},
    {-0.8500000238418579f, 0.0f, 3.9000000953674316f},
    {-0.8500000238418579f, 0.0f, -0.699999988079071f},
    {0.8500000238418579f, 0.0f, -0.699999988079071f},
    {0.8500000238418579f, 0.0f, 3.9000000953674316f},
};

STATIC NJS_POINT3 init_8c04c6a0[8] = {
    {-1.0149999856948853f, 1.5f, 8.899999618530273f},
    {-1.0149999856948853f, 1.5f, -1.5f},
    {1.0149999856948853f, 1.5f, -1.5f},
    {1.0149999856948853f, 1.5f, 8.899999618530273f},
    {-1.0149999856948853f, 0.0f, 8.899999618530273f},
    {-1.0149999856948853f, 0.0f, -1.5f},
    {1.0149999856948853f, 0.0f, -1.5f},
    {1.0149999856948853f, 0.0f, 8.899999618530273f},
};

STATIC NJS_POINT3 init_8c04c700[8] = {
    {-0.8999999761581421f, 1.5f, 4.0f},
    {-0.8999999761581421f, 1.5f, -1.0f},
    {0.8999999761581421f, 1.5f, -1.0f},
    {0.8999999761581421f, 1.5f, 4.0f},
    {-0.8999999761581421f, 0.0f, 4.0f},
    {-0.8999999761581421f, 0.0f, -1.0f},
    {0.8999999761581421f, 0.0f, -1.0f},
    {0.8999999761581421f, 0.0f, 4.0f},
};

STATIC NJS_POINT3 init_8c04c760[8] = {
    {-1.034999966621399f, 1.5f, 5.300000190734863f},
    {-1.034999966621399f, 1.5f, -2.0f},
    {1.034999966621399f, 1.5f, -2.0f},
    {1.034999966621399f, 1.5f, 5.300000190734863f},
    {-1.034999966621399f, 0.0f, 5.300000190734863f},
    {-1.034999966621399f, 0.0f, -2.0f},
    {1.034999966621399f, 0.0f, -2.0f},
    {1.034999966621399f, 0.0f, 5.300000190734863f},
};

STATIC NJS_POINT3 init_8c04c7c0[8] = {
    {-0.7649999856948853f, 1.5f, 3.5f},
    {-0.7649999856948853f, 1.5f, -0.8999999761581421f},
    {0.7649999856948853f, 1.5f, -0.8999999761581421f},
    {0.7649999856948853f, 1.5f, 3.5f},
    {-0.7649999856948853f, 0.0f, 3.5f},
    {-0.7649999856948853f, 0.0f, -0.8999999761581421f},
    {0.7649999856948853f, 0.0f, -0.8999999761581421f},
    {0.7649999856948853f, 0.0f, 3.5f},
};

/* The player bus's own box -- fed to njCalcPoints against the bus's world
 * matrix at the top of FUN_8c02e2dc. */
STATIC NJS_POINT3 init_8c04c820[8] = {
    {-1.1649999618530273f, 1.5f, 8.0f},
    {-1.1649999618530273f, 1.5f, -2.5999999046325684f},
    {1.1649999618530273f, 1.5f, -2.5999999046325684f},
    {1.1649999618530273f, 1.5f, 8.0f},
    {-1.1649999618530273f, 0.0f, 8.0f},
    {-1.1649999618530273f, 0.0f, -2.5999999046325684f},
    {1.1649999618530273f, 0.0f, -2.5999999046325684f},
    {1.1649999618530273f, 0.0f, 8.0f},
};

STATIC NJS_POINT3 init_8c04c880[8] = {
    {-0.8450000286102295f, 1.5f, 3.9600000381469727f},
    {-0.8450000286102295f, 1.5f, -0.7400000095367432f},
    {0.8450000286102295f, 1.5f, -0.7400000095367432f},
    {0.8450000286102295f, 1.5f, 3.9600000381469727f},
    {-0.8450000286102295f, 0.0f, 3.9600000381469727f},
    {-0.8450000286102295f, 0.0f, -0.7400000095367432f},
    {0.8450000286102295f, 0.0f, -0.7400000095367432f},
    {0.8450000286102295f, 0.0f, 3.9600000381469727f},
};

STATIC NJS_POINT3 init_8c04c8e0[8] = {
    {-0.8399999737739563f, 1.5f, 3.8999998569488525f},
    {-0.8399999737739563f, 1.5f, -1.0f},
    {0.8399999737739563f, 1.5f, -1.0f},
    {0.8399999737739563f, 1.5f, 3.8999998569488525f},
    {-0.8399999737739563f, 0.0f, 3.8999998569488525f},
    {-0.8399999737739563f, 0.0f, -1.0f},
    {0.8399999737739563f, 0.0f, -1.0f},
    {0.8399999737739563f, 0.0f, 3.8999998569488525f},
};

/* Table of 16 pointers, each to a box above -- a per-variant oriented
 * bounding box in local space, indexed by a traffic entry's variant index
 * (entry+0x2e0). Declared in 02e2dc.h; also used by 02e400_collision. */
NJS_POINT3 *init_8c04c940[16] = {
    init_8c04c520, init_8c04c520, init_8c04c580, init_8c04c5e0,
    init_8c04c5e0, init_8c04c5e0, init_8c04c640, init_8c04c6a0,
    init_8c04c6a0, init_8c04c700, init_8c04c760, init_8c04c760,
    init_8c04c7c0, init_8c04c820, init_8c04c880, init_8c04c8e0,
};

/* ====================
 * Functions
 * ====================
 */

/* Finds the vehicle/pedestrian the player's bus is currently bumping into
 * (called by handleBump_8c02b6d4, 02b464). Scans var_tasks_8c1bac28 from
 * the start, skipping the -1 sentinel action, and for each candidate whose
 * TrafficEntry.field_0x490 (distance to the bus, refreshed per-frame by
 * TrafficDriveVehicle_8c025b98/TrafficDriveDecoration_8c02656a) is under
 * 12 world units, transforms that variant's local-space box to world space
 * and box-tests it against the bus's own box (set up once, up front).
 * Returns the first hit's state, or NULL if none. */
BusState *FUN_8c02e2dc(void)
{
    Task *cursor;
    Uint8 *state;

    njCalcPoints(&var_busWorldMatrix_8c1bba54, init_8c04c820,
                 var_collideSelfBox_8c228978.v, 8);

    var_collideScanCursor_8c228974 = var_tasks_8c1bac28;

    while (var_collideScanCursor_8c228974->action != NULL) {
        cursor = var_collideScanCursor_8c228974;

        if (cursor->action != (TaskAction)-1) {
            state = (Uint8 *)cursor->state;

            if (12.0f > *(float *)(state + 0x490)) {
                njCalcPoints((NJS_MATRIX *)(state + 0x84),
                             init_8c04c940[*(Sint32 *)(state + 0x2e0)],
                             var_collideCandidateBox_8c2289d8.v, 8);

                if (FUN_8c020842(var_collideSelfBox_8c228978.v, var_collideCandidateBox_8c2289d8.v)) {
                    return (BusState *)state;
                }
            }
        }

        var_collideScanCursor_8c228974 =
            (Task *)((Uint8 *)var_collideScanCursor_8c228974 + 0x20);
    }

    return NULL;
}

/* Never called -- confirmed dead code kept for object parity with the
 * original binary (see docs/next_units.md's dead-functions section: real,
 * separate, compiled code, zero references anywhere in src/). A near-copy
 * of FUN_8c02e2dc's scan loop, but it does not (re)initialize
 * var_collideScanCursor_8c228974 or recompute the bus's own box -- it
 * resumes the scan from wherever the cursor is already sitting, advancing
 * past the entry there first. */
STATIC BusState *unused_8c02e35a(void)
{
    Task *cursor;
    Uint8 *state;

    var_collideScanCursor_8c228974 =
        (Task *)((Uint8 *)var_collideScanCursor_8c228974 + 0x20);

    while (var_collideScanCursor_8c228974->action != NULL) {
        cursor = var_collideScanCursor_8c228974;

        if (cursor->action != (TaskAction)-1) {
            state = (Uint8 *)cursor->state;

            if (12.0f > *(float *)(state + 0x490)) {
                njCalcPoints((NJS_MATRIX *)(state + 0x84),
                             init_8c04c940[*(Sint32 *)(state + 0x2e0)],
                             var_collideCandidateBox_8c2289d8.v, 8);

                if (FUN_8c020842(var_collideSelfBox_8c228978.v, var_collideCandidateBox_8c2289d8.v)) {
                    return (BusState *)state;
                }
            }
        }

        var_collideScanCursor_8c228974 =
            (Task *)((Uint8 *)var_collideScanCursor_8c228974 + 0x20);
    }

    return NULL;
}
