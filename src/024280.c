/* @unit BusInput */
#include <shinobi.h>
#include <math.h>                 /* asinf */
#include "includes.h" /* TWO_PI, STATIC */

#include "serial_debug.h"
#include "sectionB.h"
#include "014a9c_tasks.h"         /* Task */
#include "026710_traffic.h"       /* TrafficEntry */
#include "0100bc_sound.h"         /* var_midiHandles_8c0fcd28 */
#include "024280.h"

/* =====================
 * Type Declarations
 * =====================
 */

/* Per-gear table indexed by BusState.gear_0x2f4, shared by the throttle
 * (applyThrottle_8c024320) and brake (FUN_8c024530/FUN_8c024606) handlers.
 * field_0x08 is field_0x04 * 6000.0f -- the top speed for the gear, in the
 * units field_0x04 converts speed into before feeding asinf. field_0x00's
 * role is not yet confirmed from the functions decompiled so far. */
typedef struct {
    int field_0x00;
    float field_0x04;
    float field_0x08;
} GearTableEntry;

/* =====================
 * Initialized Globals
 * =====================
 */

STATIC GearTableEntry init_8c045638[5] = {
    { 546, 2.1604937501251698e-05f, 0.09259258955717087f },
    { 136, 4.3209875002503395e-05f, 0.18518517911434174f },
    { 68, 6.481481250375509e-05f, 0.32407405972480774f },
    { 45, 8.641975000500679e-05f, 0.46296295523643494f },
    { 34, 0.00010802468750625849f, 0.6481481194496155f },
};

/* =====================
 * Functions
 * =====================
 */

/* Called by BusTask_8c022bdc when BusState.mirror_0x268 is set: gives every
 * traffic entry roughly ahead of the bus (within ~90 degrees of its own
 * heading) that has field_0x268 set a lookahead distance derived from the
 * bus's own speed, for use by the mirror view. */
void BusInputMirrorLookahead_8c024280(void)
{
    Task *task;
    float lookahead;

    lookahead = var_busState_8c1bb9d0.speed_0x27c - 0.18518517911434174f;
    if (lookahead < 0.0f) {
        lookahead = 0.0f;
    }

    for (task = var_tasks_8c1bac28; task->action != NULL; task++) {
        TrafficEntry *entry;
        int diff;

        if (task->action == (TaskAction)-1) {
            continue;
        }

        entry = (TrafficEntry *)task->state;
        if (entry->field_0x268 == 0) {
            continue;
        }

        diff = var_busState_8c1bb9d0.ang_0x250 - entry->heading_0x250;
        if (diff < 0) {
            diff = -diff;
        }
        if (diff < 0x4000) {
            entry->field_0x418 = lookahead;
        }
    }
}

/* Debug gear override: in direct (non-mapped) steering mode, controller 0's
 * D-pad up/down forces the bus into gear 0 or reverse (5). */
STATIC void debugGearOverride_8c0242ce(void)
{
    if (var_inputMapSel_8c1bb8c8 != 0) {
        return;
    }

    if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_KU) != 0) {
        var_busState_8c1bb9d0.gear_0x2f4 = 0;
    } else if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_KD) != 0) {
        var_busState_8c1bb9d0.gear_0x2f4 = 5;
    }
}

/* Applies braking (the .l trigger) each frame: decelerates speed_0x27c by an
 * amount that grows with both current speed and how far the trigger moved
 * since var_8c1ba29d's saved deadzone, clamping speed to 0; downshifts one
 * gear if the new speed drops under the next-lower gear's top speed; then
 * derives the needle target_0x2e8/field_0x2e4 pair from the current gear's
 * table entry via asinf; and folds the brake amount into var_8c2285c4[36]'s
 * running average (a smoothed brake-intensity value). */
STATIC void applyBraking_8c024530(void)
{
    Uint8 prevDeadzone;
    float delta;
    float brakeAmount;
    int gear;
    float ratio;
    int angle;
    float *smoothedBrake;

    prevDeadzone = var_8c1ba29d;
    delta = (float)((int)var_8c1ba376 - (int)prevDeadzone);

    brakeAmount = 0.002f + (var_busState_8c1bb9d0.speed_0x27c / 48.0f) *
        (delta / (255.0f - prevDeadzone)) * (delta / (255.0f - prevDeadzone));

    var_busState_8c1bb9d0.speed_0x27c -= brakeAmount;
    if (var_busState_8c1bb9d0.speed_0x27c < 0.0f) {
        var_busState_8c1bb9d0.speed_0x27c = 0.0f;
    }

    gear = var_busState_8c1bb9d0.gear_0x2f4;
    if (gear != 0 && init_8c045638[gear - 1].field_0x08 > var_busState_8c1bb9d0.speed_0x27c) {
        gear -= 1;
        var_busState_8c1bb9d0.gear_0x2f4 = gear;
    }

    ratio = var_busState_8c1bb9d0.speed_0x27c / init_8c045638[gear].field_0x04;
    var_busState_8c1bb9d0.target_0x2e8 = ratio;

    angle = (int)((asinf(ratio / 6000.0f) * 65536.0f) / TWO_PI);
    var_busState_8c1bb9d0.field_0x2e4 = angle;

    smoothedBrake = (float *)&var_8c2285c4[36];
    *smoothedBrake += brakeAmount;
    *smoothedBrake /= 2.0f;
}

/* Throttle handler: while the .r trigger clears its saved deadzone
 * (var_8c1ba29c) by at least var_8c1bbcb4's minimum scaled step, ramps
 * BusState.field_0x2e4 (an engine-RPM needle) toward that step at a
 * per-gear rate (init_8c045638[gear].field_0x00), feeds it through njSin to
 * derive target_0x2e8/speed_0x27c, and upshifts (playing a shift-cue MIDI
 * note) once speed clears the next gear's top speed -- or, already at the
 * top gear, just clamps speed to its max. Otherwise coasts: decays
 * speed_0x27c by a fixed amount (clamped to 0), downshifting (same cue)
 * if speed drops under the current gear's own top speed, then -- for
 * every coast and every upshift, but not a plain top-gear clamp or an
 * accelerate that didn't yet clear the next gear -- recomputes
 * target_0x2e8/field_0x2e4 directly from the (possibly new) gear via
 * asinf, the same needle formula as applyBraking_8c024530. */
STATIC void applyThrottle_8c024320(void)
{
    Uint16 trigger;
    Uint8 deadzone;
    float delta;
    int step;
    int gear;

    trigger = var_8c1ba374;
    deadzone = var_8c1ba29c;
    delta = (float)((int)trigger - (int)deadzone);
    step = (int)((delta / (255.0f - deadzone)) * 16384.0f);

    if (trigger > deadzone && step >= var_8c1bbcb4) {
        gear = var_busState_8c1bb9d0.gear_0x2f4;

        var_busState_8c1bb9d0.field_0x2e4 += init_8c045638[gear].field_0x00;
        if (var_busState_8c1bb9d0.field_0x2e4 > step) {
            var_busState_8c1bb9d0.field_0x2e4 = step;
        }

        var_busState_8c1bb9d0.target_0x2e8 =
            njSin(var_busState_8c1bb9d0.field_0x2e4) * 6000.0f;
        var_busState_8c1bb9d0.speed_0x27c =
            var_busState_8c1bb9d0.target_0x2e8 * init_8c045638[gear].field_0x04;

        if (gear >= 4) {
            if (var_busState_8c1bb9d0.speed_0x27c > 0.6481481194496155f) {
                var_busState_8c1bb9d0.speed_0x27c = 0.6481481194496155f;
            }
            return;
        }

        if (var_busState_8c1bb9d0.speed_0x27c <= init_8c045638[gear].field_0x08) {
            return;
        }

        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, (var_8c227d9c >= 2) ? 0x25 : 0x26, 0);

        if (gear == 0) {
            var_8c22864c = 1;
        }
        var_busState_8c1bb9d0.gear_0x2f4 = gear + 1;
    } else {
        var_busState_8c1bb9d0.speed_0x27c -= 0.00025f;
        if (var_busState_8c1bb9d0.speed_0x27c < 0.0f) {
            var_busState_8c1bb9d0.speed_0x27c = 0.0f;
        }

        gear = var_busState_8c1bb9d0.gear_0x2f4;
        if (gear != 0 && init_8c045638[gear - 1].field_0x08 > var_busState_8c1bb9d0.speed_0x27c) {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, (var_8c227d9c >= 2) ? 0x25 : 0x26, 0);
            gear -= 1;
            var_busState_8c1bb9d0.gear_0x2f4 = gear;
        }
    }

    gear = var_busState_8c1bb9d0.gear_0x2f4;
    var_busState_8c1bb9d0.target_0x2e8 =
        var_busState_8c1bb9d0.speed_0x27c / init_8c045638[gear].field_0x04;
    var_busState_8c1bb9d0.field_0x2e4 =
        (int)((asinf(var_busState_8c1bb9d0.target_0x2e8 / 6000.0f) * 65536.0f) / TWO_PI);
}

/* Plays the brake pedal's SFX and tracks its press intensity in
 * var_8c227d8c: while the .l trigger keeps moving further from
 * var_8c1ba29d's saved deadzone, ratchets var_8c227d8c up to (never down
 * from) the scaled travel distance; once it stops advancing (released back
 * toward or past the deadzone), and var_8c227d8c is nonzero, plays one of
 * two midi notes depending on whether that peak was past the halfway point
 * (0x80) -- the low note (0x27) if it was, the high note (0x28) if not --
 * then resets var_8c227d8c to 0. */
STATIC void applyBrakingSfx_8c024606(void)
{
    Uint16 trigger;
    Uint8 prevDeadzone;

    trigger = var_8c1ba376;
    prevDeadzone = var_8c1ba29d;

    if (trigger > prevDeadzone) {
        int delta = trigger - prevDeadzone;
        int target = (delta * 255) / (255 - prevDeadzone);

        if (var_8c227d8c < target) {
            var_8c227d8c = target;
        }
    } else if (var_8c227d8c != 0) {
        int note = (var_8c227d8c >= 0x80) ? 0x27 : 0x28;
        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, note, 0);
        var_8c227d8c = 0;
    }
}

/* Per-frame driving dispatcher, called by BusTask_8c022bdc while driving.
 *
 * First plays the brake SFX (applyBrakingSfx_8c024606), then drives
 * BusState.field_0x2e0 (the needle-ramp mode) through three states:
 *
 *   0 (relax): braking sets blinker_0x080's bit 0; otherwise, once the
 *     throttle clears half its deadzone, resets field_0x2ec and switches to
 *     mode 1, kicking off a vibration cue (VibStart_8c010f7a(0)).
 *   1 (settle): braking still sets blinker_0x080's bit 0; field_0x2ec counts
 *     frames, and once it exceeds 30, either aborts back to mode 0 (stopping
 *     any vibration) if the pedals are pressed, or advances to mode 2 (ramp)
 *     if they're not.
 *   2 (ramp): in reverse (gear_0x2f4 == 5), brake/throttle/neither directly
 *     drive speed_0x27c toward 0 / a throttle-scaled negative target / 0
 *     respectively; otherwise braking calls applyBraking_8c024530 (setting
 *     blinker_0x080's bit 0) and not braking calls applyThrottle_8c024320 (which also
 *     decays applyBraking_8c024530's smoothed brake-average slot,
 *     var_8c2285c4[36], to 0.0 since nothing is braking), either way
 *     tracking a forward-gear idle-frame counter in var_8c2285c4[32] (reset
 *     at rest, else incremented -- a separate, undocumented int slot, not
 *     related to [36]). Mode 2 then always checks speed_0x27c == 0.0: if so, calls
 *     debugGearOverride_8c0242ce and, once field_0x2ec (idle-at-rest frames)
 *     reaches 30, drops back to mode 1 (again kicking VibStart_8c010f7a(0));
 *     otherwise resets field_0x2ec to 0.
 *
 * After the mode dispatch, handles the two rear/side-mirror-view buttons
 * (PDS_PERIPHERAL.press bits 0x400 and 0x2) via BusState.field_0x25c (a
 * small per-button press/hold/release state) toggling mirror_0x268 between
 * its 0/1/2 modes, gated by a var_8c2285c4[27] check against a sentinel
 * (0x10000000) or against var_8c228634[0]. In mapped-route steering mode
 * (var_inputMapSel_8c1bb8c8 != 0) this is everything -- field_0x338 is
 * force-set to 2 first, bailing out entirely if field_0x334 is set; the
 * first button's held (case 1) state clears field_0x338 back to 0 instead
 * of touching mirror_0x268.
 *
 * In direct steering mode (var_inputMapSel_8c1bb8c8 == 0), the same two
 * buttons instead only clear mirror_0x268/field_0x25c on release (never
 * setting field_0x338), and execution always continues into a steering-
 * wheel force-feedback ramp: PDS_PERIPHERAL.x1 (the analog steering axis,
 * dead-zoned by 8 either side) is converted to a target angle (BAM units,
 * via a 60-degree max deflection) for BusState.ang_0x258, then eased toward
 * it one of four ways depending on the current angle's sign and which side
 * of the target it sits on: moving further from center steps by a
 * per-frame float ~80.89 (from a positive angle) or a plain int 80 (from a
 * non-positive one) -- an asymmetry in the original code; moving back
 * toward or past center steps by a flat int 182 (target keeps the same
 * sign as the current angle) or 364 (target is zero or the opposite sign,
 * for a faster return to center), clamped on overshoot either way. */
void BusInputUpdate_8c0246b2(void)
{
    const PDS_PERIPHERAL *pad = &var_peripherals_8c1ba35c[0];
    Uint16 brakeTrigger, throttleTrigger;
    Uint8 brakeDeadzone, throttleDeadzone;
    int mode;
    Uint32 press;

    applyBrakingSfx_8c024606();

    brakeTrigger = pad->l;
    brakeDeadzone = var_8c1ba29d;
    throttleTrigger = pad->r;
    throttleDeadzone = var_8c1ba29c;

    mode = var_busState_8c1bb9d0.field_0x2e0;
    switch (mode) {
    case 0: /* relax */
        if (brakeTrigger > brakeDeadzone) {
            var_busState_8c1bb9d0.blinker_0x080 |= 1;
        } else if (throttleTrigger > (Uint16)(throttleDeadzone / 2)) {
            var_busState_8c1bb9d0.field_0x2ec = 0;
            var_busState_8c1bb9d0.field_0x2e0 = 1;
            VibStart_8c010f7a(0);
        }
        debugGearOverride_8c0242ce();
        break;

    case 1: { /* settle */
        int counter;

        if (brakeTrigger > brakeDeadzone) {
            var_busState_8c1bb9d0.blinker_0x080 |= 1;
        }

        counter = var_busState_8c1bb9d0.field_0x2ec;
        var_busState_8c1bb9d0.field_0x2ec = counter + 1;

        if (counter > 30) {
            /* Advance to the ramp only if the throttle is STILL held (the
             * press that got here in the first place) and the brake isn't;
             * anything else -- throttle let go, or the brake now pressed --
             * aborts back to relax. */
            if (throttleTrigger > throttleDeadzone && !(brakeTrigger > brakeDeadzone)) {
                var_busState_8c1bb9d0.field_0x2e0 = 2;
                var_busState_8c1bb9d0.field_0x2ec = 0;
                var_busState_8c1bb9d0.field_0x2f0 = 0;
            } else {
                var_busState_8c1bb9d0.field_0x2ec = 0;
                var_busState_8c1bb9d0.field_0x2e0 = 0;
                if (var_vibport_8c1ba354 != (Uint32)-1) {
                    pdVibMxStop(var_vibport_8c1ba354);
                }
            }
        }
        debugGearOverride_8c0242ce();
        break;
    }

    case 2: /* ramp */
        if (var_busState_8c1bb9d0.gear_0x2f4 == 5) {
            /* Reverse: drive speed_0x27c directly instead of going through
             * applyThrottle_8c024320/applyBraking_8c024530's forward-gear tables. */
            if (brakeTrigger > brakeDeadzone) {
                float ratio = (float)(brakeTrigger - brakeDeadzone) /
                    (255.0f - brakeDeadzone);
                var_busState_8c1bb9d0.speed_0x27c += ratio * 0.009999999776482582f;
                if (var_busState_8c1bb9d0.speed_0x27c > 0.0f) {
                    var_busState_8c1bb9d0.speed_0x27c = 0.0f;
                }
                var_busState_8c1bb9d0.blinker_0x080 |= 1;
            } else if (throttleTrigger > throttleDeadzone) {
                float ratio = (float)(throttleTrigger - throttleDeadzone) /
                    (255.0f - throttleDeadzone);
                float target = -(ratio * 0.18518517911434174f);

                if (var_busState_8c1bb9d0.speed_0x27c > target) {
                    var_busState_8c1bb9d0.speed_0x27c += -0.0015432097716256976f;
                    if (var_busState_8c1bb9d0.speed_0x27c < target) {
                        var_busState_8c1bb9d0.speed_0x27c = target;
                    }
                } else {
                    var_busState_8c1bb9d0.speed_0x27c -= -0.0015432097716256976f;
                    if (var_busState_8c1bb9d0.speed_0x27c > target) {
                        var_busState_8c1bb9d0.speed_0x27c = target;
                    }
                }
            } else {
                var_busState_8c1bb9d0.speed_0x27c += 0.0010000000474974513f;
                if (var_busState_8c1bb9d0.speed_0x27c > 0.0f) {
                    var_busState_8c1bb9d0.speed_0x27c = 0.0f;
                }
            }

            var_busState_8c1bb9d0.target_0x2e8 =
                -(var_busState_8c1bb9d0.speed_0x27c * 16384.0f);
        } else {
            /* Forward gears: var_8c2285c4[32] (0x228644, no export of its
             * own) is a plain idle-at-rest frame counter, unrelated to
             * applyBraking_8c024530's smoothed-average slot at [36]. */
            if (var_busState_8c1bb9d0.speed_0x27c == 0.0f) {
                var_8c2285c4[32] = 0;
            } else {
                var_8c2285c4[32] += 1;
            }

            if (brakeTrigger > brakeDeadzone) {
                applyBraking_8c024530();
                var_busState_8c1bb9d0.blinker_0x080 |= 1;
            } else {
                applyThrottle_8c024320();
                /* Not braking: also decays applyBraking_8c024530's smoothed
                 * brake-average slot straight to 0. */
                *(float *)&var_8c2285c4[36] = 0.0f;
            }
        }

        if (var_busState_8c1bb9d0.speed_0x27c == 0.0f) {
            debugGearOverride_8c0242ce();
            if (var_busState_8c1bb9d0.field_0x2ec >= 30) {
                var_busState_8c1bb9d0.field_0x2e0 = 1;
                var_busState_8c1bb9d0.field_0x2ec = 0;
                VibStart_8c010f7a(0);
            } else {
                var_busState_8c1bb9d0.field_0x2ec += 1;
            }
        } else {
            var_busState_8c1bb9d0.field_0x2ec = 0;
        }
        break;

    default:
        break;
    }

    press = pad->press;

    if (var_inputMapSel_8c1bb8c8 != 0) {
        var_busState_8c1bb9d0.field_0x338 = 2;
        if (var_busState_8c1bb9d0.field_0x334 != 0) {
            return;
        }

        if ((press & 0x400) != 0) {
            switch (var_busState_8c1bb9d0.field_0x25c) {
            case 0:
                var_busState_8c1bb9d0.field_0x25c = 1;
                if (var_8c2285c4[27] != 0x10000000) {
                    var_busState_8c1bb9d0.mirror_0x268 = 1;
                }
                break;
            case 1:
                var_busState_8c1bb9d0.field_0x338 = 0;
                break;
            case 2:
                var_busState_8c1bb9d0.field_0x25c = 0;
                var_busState_8c1bb9d0.mirror_0x268 = 0;
                break;
            default:
                break;
            }
        } else if ((press & 2) != 0) {
            switch (var_busState_8c1bb9d0.field_0x25c) {
            case 0:
                var_busState_8c1bb9d0.field_0x25c = 2;
                if ((var_8c2285c4[27] & ~1) != (var_8c228634[0] << 4)) {
                    var_busState_8c1bb9d0.mirror_0x268 = 2;
                }
                break;
            case 1:
                var_busState_8c1bb9d0.field_0x25c = 0;
                var_busState_8c1bb9d0.mirror_0x268 = 0;
                break;
            case 2:
                var_busState_8c1bb9d0.field_0x338 = 1;
                break;
            default:
                break;
            }
        }
        return;
    }

    /* Direct steering mode: same two mirror buttons, but case 1 (held) just
     * clears back to 0 on either button, and field_0x338 is never touched
     * here -- then the steering force-feedback ramp always runs below. */
    if ((press & 0x400) != 0) {
        switch (var_busState_8c1bb9d0.field_0x25c) {
        case 0:
            var_busState_8c1bb9d0.field_0x25c = 1;
            if (var_8c2285c4[27] != 0x10000000) {
                var_busState_8c1bb9d0.mirror_0x268 = 1;
            }
            break;
        case 1:
        case 2:
            var_busState_8c1bb9d0.field_0x25c = 0;
            var_busState_8c1bb9d0.mirror_0x268 = 0;
            break;
        default:
            break;
        }
    } else if ((press & 2) != 0) {
        switch (var_busState_8c1bb9d0.field_0x25c) {
        case 0:
            var_busState_8c1bb9d0.field_0x25c = 2;
            if ((var_8c2285c4[27] & ~1) != (var_8c228634[0] << 4)) {
                var_busState_8c1bb9d0.mirror_0x268 = 2;
            }
            break;
        case 1:
        case 2:
            var_busState_8c1bb9d0.field_0x25c = 0;
            var_busState_8c1bb9d0.mirror_0x268 = 0;
            break;
        default:
            break;
        }
    }

    {
        Sint16 x1 = pad->x1;
        int dev;
        int targetAngle;
        int cur;

        if (-x1 > 0) {
            /* steering left */
            dev = -x1 - 8;
            if (dev > 0) {
                if (dev >= 127) {
                    targetAngle = 0x2aaa;
                } else {
                    targetAngle = (int)((float)(dev * 60 / 127) * 65536.0f / 360.0f);
                }
            } else {
                targetAngle = 0;
            }
        } else {
            /* steering right (or centered) */
            dev = -x1 + 8;
            if (dev < 0) {
                if (dev > -127) {
                    targetAngle = (int)((float)(dev * 60 / 127) * 65536.0f / 360.0f);
                } else {
                    targetAngle = -0x2aaa;
                }
            } else {
                targetAngle = 0;
            }
        }

        cur = var_busState_8c1bb9d0.ang_0x258;
        if (cur > 0) {
            if (cur >= targetAngle) {
                cur += (targetAngle * cur > 0) ? -182 : -364;
                var_busState_8c1bb9d0.ang_0x258 = cur;
                if (cur < targetAngle) {
                    var_busState_8c1bb9d0.ang_0x258 = targetAngle;
                }
            } else {
                cur = (int)((float)cur + 80.888885f);
                var_busState_8c1bb9d0.ang_0x258 = cur;
                if (cur > targetAngle) {
                    var_busState_8c1bb9d0.ang_0x258 = targetAngle;
                }
            }
        } else {
            if (cur <= targetAngle) {
                cur += (targetAngle * cur > 0) ? 182 : 364;
                var_busState_8c1bb9d0.ang_0x258 = cur;
                if (cur > targetAngle) {
                    var_busState_8c1bb9d0.ang_0x258 = targetAngle;
                }
            } else {
                cur -= 80;
                var_busState_8c1bb9d0.ang_0x258 = cur;
                if (cur < targetAngle) {
                    var_busState_8c1bb9d0.ang_0x258 = targetAngle;
                }
            }
        }
    }
}
