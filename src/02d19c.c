#include <shinobi.h>
#include <sg_sd.h>
#include "serial_debug.h"
#include "sectionB.h"
#include "014a9c_tasks.h"
#include "013ae8_route_load.h"
#include "014f54_text.h"
#include "022464_fade.h"
#include "0222dc_fadecmd.h"
#include "0100bc_sound.h"
#include "024b4c_bus_render.h"
#include "028258_objects.h"
#include "02c884_bus_stop.h"
#include "02d06c.h"
#include "02d19c.h"

/* ====================
 * Forward Declarations
 * ====================
 */

STATIC void FUN_8c02d19c(int arg0);
STATIC void FUN_8c02d1f4(int arg0);
STATIC void FUN_8c02d5d8(void);

/* ====================
 * Initialized Globals
 * ====================
 */

InitEntry_8c04c3e4 init_8c04c3e4[31] = {
    { 0x3F570A3D, -0.33000001311302185f },
    { 0x3F5C28F6, 3.950000047683716f },
    { 0x3F570A3D, 4.78000020980835f },
    { 0x3F5C28F6, 5.599999904632568f },
    { 0x3F000000, 6.400000095367432f },
    { 0x3F570A3D, 6.400000095367432f },
    { 0x3F5C28F6, 7.25f },
    { 0x3F0A3D71, 7.25f },
    { 0xBF570A3D, -0.33000001311302185f },
    { 0xBF5C28F6, 0.550000011920929f },
    { 0xBF570A3D, 1.5f },
    { 0xBF5C28F6, 2.380000114440918f },
    { 0xBF570A3D, 3.200000047683716f },
    { 0xBF5C28F6, 3.950000047683716f },
    { 0xBF570A3D, 4.78000020980835f },
    { 0xBF5C28F6, 5.599999904632568f },
    { 0xBF000000, 6.400000095367432f },
    { 0xBF570A3D, 6.400000095367432f },
    { 0xBF5C28F6, 7.25f },
    { 0xBF0A3D71, 7.25f },
    { 0x3EB33333, 0.14499999582767487f },
    { 0x3E570A3D, 0.6399999856948853f },
    { 0x3E75C28F, 1.600000023841858f },
    { 0x3E6147AE, 3.4000000953674316f },
    { 0x3E8F5C29, 4.25f },
    { 0xBCA3D70A, 4.599999904632568f },
    { 0xBE4CCCCD, 0.27000001072883606f },
    { 0xBE800000, 0.800000011920929f },
    { 0xBE947AE1, 1.899999976158142f },
    { 0xBE4CCCCD, 2.8399999141693115f },
    { 0xBE83126F, 4.0f },
};

/* ====================
 * Functions
 * ====================
 */

/* Installed as a FadeCallback1 (via literal-pool pointer, both by
 * FUN_8c02d5ca and by the per-passenger task actions below); draws one
 * waiting-passenger/scripted-stop rider's sprite. arg0 is really a
 * StopScheduleState* threaded through the int parameter (the same idiom as
 * FUN_8c02d0fc/d146 in 02d06c). A stop index (the byte at *ref_0x00) out of
 * range, or whose asset slot has no texlist loaded yet, is skipped. */
STATIC void FUN_8c02d19c(int arg0)
{
    StopScheduleState *state = (StopScheduleState *)arg0;
    Uint8 stopIndex = *(Uint8 *)state->ref_0x00;

    if (stopIndex >= 0x41) {
        return;
    }
    if (var_pedestrianAssets_8c1bbfdc[stopIndex].texlist_0x08 == (NJS_TEXLIST *)-1) {
        return;
    }

    var_8c2288d8.tlist = var_pedestrianAssets_8c1bbfdc[stopIndex].texlist_0x08;
    var_8c2288d8.p = *(NJS_POINT3 *)&state->field_0x08;
    njDrawSprite3D(&var_8c2288d8, state->field_0x14, state->field_0x28 == 0 ? 0x32 : 0x30);
}

/* Installed as a FadeCallback1 (via literal-pool pointer in FUN_8c02d644);
 * ignores its arg. Draws the bus interior model under the layer-0 (bus)
 * simple light, sibling of FUN_8c02d0fc (02d06c) which does the same for
 * the bus-stop anchor points. */
STATIC void FUN_8c02d1f4(int arg0)
{
    njCnkSetSimpleLight(var_busSimpleLightDir_8c227db8[0], var_busSimpleLightDir_8c227db8[1], var_busSimpleLightDir_8c227db8[2]);
    njSetTexture(var_interiorTexlist_8c1bc438);
    njCnkSimpleDrawObject(var_interiorNj_8c1bc43c);
}

/* Task action for a scripted-stop slot whose segment differs from the bus's
 * current one -- spawned immediately by FUN_8c02d968 (no shuffle, no
 * countdown). Just registers the draw callback every frame; ignores task. */
void FUN_8c02d5ca(Task *task, void *state)
{
    FadeCmdPushCall1_8c0223ea(2, FUN_8c02d19c, (int)state);
}

/* var_8c2285c4[7], the per-frame step FUN_8c02d644's case 5 adds to
 * var_8c2285c4[6] while counting it up to var_8c2285c4[5]: 1/5th of the
 * remaining distance (idx5 - idx6) once that's at least 50, else a flat
 * step of 10 to close the last stretch. */
STATIC void FUN_8c02d5d8(void)
{
    int remaining = var_8c2285c4[5] - var_8c2285c4[6];

    if (remaining >= 0x32) {
        var_8c2285c4[7] = remaining / 5;
    } else {
        var_8c2285c4[7] = 0xa;
    }
}

/* Task action for an already-picked waiting passenger (state->field_0x04 ==
 * 1 always, from 02d968); a small state machine walking it through the
 * boarding animation: countdown-then-position at each of the three door
 * anchor points (var_8c228928/91c... no -- 8c228928, then 8c228910,
 * 8c22891c in turn), claiming an empty scripted-stop slot once done so it
 * continues as an ordinary scripted rider, then just keeps re-registering
 * the draw callback in its terminal state (0). var_8c22895c gates whether
 * each phase's positioning work runs this frame (set up per-frame
 * elsewhere in the bus-stop subsystem; not owned by this unit). */
void FUN_8c02d21c(Task *task, void *state_)
{
    StopScheduleState *state = (StopScheduleState *)state_;
    int mode = state->field_0x04;

    switch (mode) {
    case 1:
        if (var_8c22895c != 0) {
            Sint32 n = (Sint32)state->field_0x20 - 1;
            state->field_0x20 = n;
            if (n < 0) {
                *(float *)&state->field_0x08 = var_8c228928.x + *(float *)&state->field_0x18;
                *(float *)&state->field_0x0c = var_8c228928.y;
                *(float *)&state->field_0x10 = var_8c228928.z + *(float *)&state->field_0x1c;
                state->field_0x14 = 0x10;
                state->field_0x04 = 2;
                if (var_route_8c18ad1c != ROUTE_OME) {
                    sdMidiPlay(var_midiHandles_8c0fcd28[state->field_0x2c], 1, state->field_0x30, 0);
                }
            }
        }
        goto done;

    case 2:
        if (var_8c22895c != 0) {
            *(NJS_POINT3 *)&state->field_0x08 = var_8c228910;
            state->field_0x14 = (var_route_8c18ad1c == ROUTE_OME) ? 0x18 : 8;
            state->field_0x04 = 3;
            if (var_route_8c18ad1c != ROUTE_OME) {
                sdMidiPlay(var_midiHandles_8c0fcd28[5], 1, AsqGetRandomInRangeA_8c012178(3) + 0x1f, 0);
            }
        }
        goto registerLayer1;

    case 3:
        if (var_8c22895c != 0) {
            *(float *)&state->field_0x08 = var_8c22891c.x + *(float *)&state->field_0x18;
            *(float *)&state->field_0x0c = var_8c22891c.y;
            *(float *)&state->field_0x10 = var_8c22891c.z + *(float *)&state->field_0x1c;
            state->field_0x04 = 4;
            if (var_route_8c18ad1c != ROUTE_OME) {
                sdMidiPlay(var_midiHandles_8c0fcd28[state->field_0x2c], 1, state->field_0x34, 0);
            }
        }
        goto registerLayer2;

    case 4:
        if (var_8c22895c != 0) {
            int slot = AsqGetRandomInRangeA_8c012178(31);

            while (var_8c228718[slot] != -1) {
                slot++;
                if (slot >= 31) {
                    slot = 0;
                }
            }
            var_8c228718[slot] = (int)state->ref_0x00;
            *(float *)&state->field_0x08 = *(float *)&init_8c04c3e4[slot].field_0x00;
            *(float *)&state->field_0x10 = init_8c04c3e4[slot].field_0x04;
            if (slot < 0x14) {
                state->field_0x14 = 0x20;
                *(float *)&state->field_0x0c -= 0.18000000715255737f;
            } else if (slot < 0x1a) {
                state->field_0x14 = 0x22;
            } else {
                state->field_0x14 = 0x23;
            }
            state->field_0x04 = 5;
        }
        goto registerLayer2;

    case 5:
        if (!(1.0f > var_8c228960[0])) {
            state->field_0x04 = 0;
            state->field_0x28 = 1;
        }
        goto registerLayer2;

    case 0:
        FadeCmdPushCall1_8c0223ea(2, FUN_8c02d19c, (int)state);
        return;

    default:
        goto done;
    }

registerLayer1:
    FadeCmdPushCall1_8c0223ea(1, FUN_8c02d19c, (int)state);
    goto done;

registerLayer2:
    FadeCmdPushCall1_8c0223ea(2, FUN_8c02d19c, (int)state);

done:
    var_8c228958 = 1;
}

/* Task action for a scripted-stop slot that matched the bus's current
 * segment (spawned after the Fisher-Yates shuffle by FUN_8c02d968).
 * Sibling of FUN_8c02d21c: same shape (countdown, then walk three door
 * anchor points), but the anchor points, sound-gating route and terminal
 * behavior all differ -- this one frees itself once var_8c22895c fires in
 * its terminal state, rather than looping forever. */
void FUN_8c02d46c(Task *task, void *state_)
{
    StopScheduleState *state = (StopScheduleState *)state_;
    int mode = state->field_0x04;

    switch (mode) {
    case 0:
        if (var_8c22895c != 0) {
            Sint32 n = (Sint32)state->field_0x20 - 1;
            state->field_0x20 = n;
            if (n < 0) {
                state->field_0x04 = 6;
            }
        }
        goto registerLayer2;

    case 6:
        if (!(1.0f > var_8c228960[0])) {
            state->field_0x28 = 0;
        }
        if (var_8c22895c != 0) {
            var_8c228718[state->field_0x24] = -1;
            *(float *)&state->field_0x08 = var_8c228934.x + *(float *)&state->field_0x18;
            *(float *)&state->field_0x10 = var_8c228934.z + *(float *)&state->field_0x1c;
            state->field_0x14 = (var_route_8c18ad1c == ROUTE_OME) ? 0x20 : 0x10;
            state->field_0x04 = 7;
            if (var_route_8c18ad1c == ROUTE_OME) {
                sdMidiPlay(var_midiHandles_8c0fcd28[state->field_0x2c], 1, state->field_0x30, 0);
            }
        }
        goto registerLayer2;

    case 7:
        if (var_8c22895c != 0) {
            *(NJS_POINT3 *)&state->field_0x08 = var_8c228940;
            state->field_0x04 = 8;
            if (var_route_8c18ad1c == ROUTE_OME) {
                sdMidiPlay(var_midiHandles_8c0fcd28[5], 1, AsqGetRandomInRangeB_8c0121be(3) + 0x1f, 0);
            }
        }
        goto registerLayer2;

    case 8:
        if (var_8c22895c != 0) {
            *(float *)&state->field_0x08 = var_8c22894c.x + *(float *)&state->field_0x18;
            *(float *)&state->field_0x0c = var_8c22894c.y;
            *(float *)&state->field_0x10 = var_8c22894c.z + *(float *)&state->field_0x1c;
            state->field_0x14 = 0x18;
            state->field_0x04 = 9;
            if (var_route_8c18ad1c == ROUTE_OME) {
                sdMidiPlay(var_midiHandles_8c0fcd28[state->field_0x2c], 1, state->field_0x34, 0);
            }
        }
        goto registerLayer2;

    case 9:
        if (var_8c22895c != 0) {
            TaskFree_8c014b66(task);
            return;
        }
        goto registerLayer1;

    default:
        goto done;
    }

registerLayer1:
    FadeCmdPushCall1_8c0223ea(1, FUN_8c02d19c, (int)state);
    goto done;

registerLayer2:
    FadeCmdPushCall1_8c0223ea(2, FUN_8c02d19c, (int)state);

done:
    var_8c228958 = 1;
}

/* Per-frame driver for the whole bus-stop passenger subsystem: pumps
 * var_stopTaskGroup_8c2288f8's tasks, fades var_8c228960[0] (the bus
 * interior's own light level, separate from the anchor-point one in
 * 02d06c) in and out around the fade-arrival overlay, and eventually tears
 * the whole subsystem down and frees itself. Spawned once by FUN_8c02d968
 * with a private 2-int state (field_0x00 the phase, field_0x04 a sub-phase
 * used only by phase 1); NOT the 0x38-byte StopScheduleState the other
 * task actions in this unit use. Every call re-registers this frame's
 * interior-draw (FUN_8c02d1f4) and light (FUN_8c02d0fc) FadeCallback1s
 * regardless of phase. */
void FUN_8c02d644(Task *task, void *state_)
{
    Uint32 *state = (Uint32 *)state_;
    int phase = state[0];

    FadeCmdPushCall1_8c0223ea(2, FUN_8c02d1f4, 0);
    FadeCmdPushCall1_8c0223ea(1, FUN_8c02d0fc, 0);
    FadeCmdPushCall1_8c0223ea(2, FUN_8c02d0fc, 0);

    switch (phase) {
    case 0:
        if (var_isFading_8c226568 == 0) {
            state[0] = 1;
        }
        var_8c2285c4[6]++;
        goto tail_d7b4;

    case 1: {
        /* The original re-enters this sub-state-machine in place (a plain
         * branch back, not a fresh call) when var_playMode_8c1bb8d0 ==
         * PLAY_MODE_DEMO and the task group did something this frame --
         * it does not re-run the three FadeCmdPushCall1 registrations
         * above or re-read phase. */
        for (;;) {
            switch (state[1]) {
            case 0: {
                float v = var_8c228960[0] - 0.06666667014360428f;
                var_8c228960[0] = v;
                if (!(v > 0.0f)) {
                    var_8c228960[0] = 0.0f;
                    var_8c22895c = 1;
                    state[1] = 1;
                } else {
                    var_8c22895c = 0;
                }
                break;
            }
            case 1: {
                float v = var_8c228960[0] + 0.06666667014360428f;
                var_8c228960[0] = v;
                if (!(1.0f > v)) {
                    var_8c228960[0] = 1.0f;
                    state[1] = 0;
                }
                var_8c22895c = 0;
                break;
            }
            default:
                break;
            }

            var_8c228958 = 0;
            TaskExecGroup_8c014b42((Task *)var_stopTaskGroup_8c2288f8);
            if (var_8c228958 != 0) {
                var_8c2285c4[6]++;
                if (var_playMode_8c1bb8d0 == PLAY_MODE_DEMO) {
                    continue;
                }
                goto tail_d8cc;
            }

            if (var_cutsceneActive_8c1bb900 == 0 || var_playMode_8c1bb8d0 != PLAY_MODE_NORMAL) {
                var_8c2285c4[0] = 2;
                state[0] = 5;
                FUN_8c02d5d8();
            } else {
                state[0] = 2;
                var_fadeRequest_8c226564 = FADE_REQUEST_IN;
            }
            var_8c2285c4[6]++;
            goto tail_d8cc;
        }
    }

    case 2:
        if (var_isFading_8c226568 == 0) {
            state[0] = 3;
            var_fadeRequest_8c226564 = FADE_REQUEST_OUT;
            var_fadeArrivalGate_8c226560 = 0;
            ObjectsStartMessageBox_8c02ad8c();
        }
        goto tail_d7b4;

    case 3:
        if (var_messageBoxActive_8c22847c == 0) {
            state[0] = 4;
        }
        goto tail_d8cc;

    case 4:
        if (RouteLoadIsPvmReady_8c01432a() != 0 && var_isFading_8c226568 == 0) {
            var_8c2285c4[0] = 2;
            state[0] = 5;
            FUN_8c02d5d8();
        }
        goto tail_d7b4;

    case 5:
        if (var_8c2285c4[6] >= var_8c2285c4[5]) {
            goto teardown;
        }

        var_8c2285c4[6] += var_8c2285c4[7];
        if (var_8c2285c4[6] >= var_8c2285c4[5]) {
            var_8c2285c4[6] = var_8c2285c4[5];
        }
        if (var_playMode_8c1bb8d0 == PLAY_MODE_NORMAL) {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 6, 0);
        }
        goto tail_d7b4;

    default:
        goto tail_d8cc;
    }

tail_d7b4:
    TaskExecGroup_8c014b42((Task *)var_stopTaskGroup_8c2288f8);
    goto tail_d8cc;

teardown:
    BusStopFreeTaskGroup_8c02ca96();
    njReleaseTexture(var_interiorTexlist_8c1bc438);
    if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE) {
        var_8c227d9c = 2;
        var_8c227da4 = 0;
        FUN_8c024f32();
    } else {
        if (var_playMode_8c1bb8d0 != PLAY_MODE_DEMO) {
            FUN_8c024b86();
        }
    }

    var_busState_8c1bb9d0.bus_state_0x2b4 = 1;
    var_busState_8c1bb9d0.field_0x25c =
        (var_playMode_8c1bb8d0 != PLAY_MODE_PRACTICE && var_route_8c18ad1c == ROUTE_OME &&
         var_currentSegment_8c228708 == 0) ? 1 : 2;

    var_fadeArrivalVariant_8c22655c = 2;

    if (var_playMode_8c1bb8d0 != PLAY_MODE_PRACTICE) {
        int soundOffset;

        switch (var_route_8c18ad1c) {
        case ROUTE_SHINJUKU:
            soundOffset = 2;
            break;
        case ROUTE_WANGAN:
            soundOffset = 8;
            break;
        case ROUTE_OME:
            soundOffset = 5;
            break;
        default:
            /* unreachable: var_route_8c18ad1c only ever holds the three
             * ROUTE_* values above -- the original leaves the register
             * that becomes this arg uninitialized in this case. */
            soundOffset = 0;
            break;
        }
        SndProc_8c010cd6(0, var_timeOfDay_8c18ad20 + soundOffset);
    }

    TaskFree_8c014b66(task);
    return;

tail_d8cc:
    FadeCmdPushCall1_8c0223ea(1, FUN_8c02d146, 0);
    FadeCmdPushCall1_8c0223ea(2, FUN_8c02d146, 0);
}

/* Task action spawned instead of the normal per-passenger tasks when
 * var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE and the course-restart flag
 * var_8c226410 bit 3 is clear -- resets the fade-arrival bookkeeping once
 * (guarded by var_isFading_8c226568) and frees itself. */
void FUN_8c02d8f0(Task *task, void *state)
{
    if (var_isFading_8c226568 == 0) {
        var_8c1bbc84 = 1;
        var_fadeArrivalVariant_8c22655c = 0;
        var_8c227d9c = 2;
        var_8c227da4 = 0;
        FUN_8c024f32();
        TaskFree_8c014b66(task);
    }
}
