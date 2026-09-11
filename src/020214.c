/* @unit DriveCue */
#include <shinobi.h>

#include "sectionB.h"
#include "014a9c_tasks.h"
#include "013ae8_route_load.h" /* enum ROUTE */
#include "011120_asset_queues.h" /* AsqGetRandomB_8c0121a8, AsqGetRandomInRangeB_8c0121be */
#include "010e90.h" /* VibStart_8c010f7a, VibStop_8c010fae */
#include "0100bc_sound.h" /* SndProc_8c010cd6, var_midiHandles_8c0fcd28 */
#include "020214.h"

/* ====================
 * Functions
 * ====================
 */

/* See 020214.h. */
void DriveCueTask_8c020214(Task *task, void *state)
{
    /* Mirrors R4, which the asm's stopAnnounceState_0x08 dispatch and the final
     * "near stop marker" check both read: seeded from nearStopLatch_0x0c right
     * before the switch below (a delay-slot load, so it runs no matter which
     * case is taken), then updated only by case 0/2's own sdMidiPlay calls or
     * by the final block's own route/segment match. */
    int nearFlag;

    (void)state;

    if (var_8c2285c4[0] >= 3) {
        TaskFree_8c014b66(task);
        return;
    }

    switch (var_8c2264b8.idleChimeState_0x00) {
    case 0:
        /* firstChimeArmed_0x18 is reset to 0 unconditionally below every call and
         * never written anywhere else, so it reads 0 here on every real
         * invocation -- the AsqGetRandomInRangeB "first chime" branch below
         * is dead in practice, kept for a test to drive directly. */
        if (var_8c2264b8.firstChimeArmed_0x18 != 0) {
            sdMidiPlay(var_midiHandles_8c0fcd28[3], 1, AsqGetRandomInRangeB_8c0121be(6) + 9, 0);
            var_8c2264b8.idleChimeTimer_0x04 = (AsqGetRandomInRangeB_8c0121be(3) + 2) * 30;
            var_8c2264b8.idleChimeState_0x00 = 1;
        } else if (var_8c1bbc4c > 0.09259258955717087f) {
            if (--var_8c2264bc < 0) {
                int r = AsqGetRandomB_8c0121a8();
                if ((r & 1) == 0) {
                    sdMidiPlay(var_midiHandles_8c0fcd28[3], 1, 0x3c, 0);
                    VibStart_8c010f7a(2);
                } else {
                    sdMidiPlay(var_midiHandles_8c0fcd28[3], 1, 0x3b, 0);
                    VibStart_8c010f7a(1);
                }
                var_8c2264b8.idleChimeTimer_0x04 = 60;
                var_8c2264b8.idleChimeState_0x00 = 1;
            }
        }
        break;

    case 1:
        if (--var_8c2264b8.idleChimeTimer_0x04 < 0) {
            var_8c2264b8.nearStopChimeLatch_0x14 = 0;
            var_8c2264b8.idleChimeTimer_0x04 = AsqGetRandomInRangeB_8c0121be(300) + 150;
            var_8c2264b8.idleChimeState_0x00 = 0;
        }
        break;

    default:
        break;
    }

    var_8c2264b8.firstChimeArmed_0x18 = 0;

    /* SH4 quirk: the asm's dispatch on stopAnnounceState_0x08 loads nearStopLatch_0x0c
     * into R4 in the delay slot of its first branch test, so it runs regardless
     * of which case is taken -- nearFlag always becomes nearStopLatch_0x0c's
     * value here, before the switch even runs. */
    nearFlag = var_8c2264b8.nearStopLatch_0x0c;

    switch (var_8c2264b8.stopAnnounceState_0x08) {
    case 0:
        if (nearFlag == 0) {
            break;
        }

        if (var_playMode_8c1bb8d0 != PLAY_MODE_PRACTICE || (var_8c226410 & 1) != 0) {
            int soundId;

            switch (var_route_8c18ad1c) {
            case ROUTE_SHINJUKU:
                soundId = 0x22;
                break;
            case ROUTE_WANGAN:
            case ROUTE_OME:
            default:
                soundId = 0x23;
                break;
            }

            sdMidiPlay(var_midiHandles_8c0fcd28[2], 1, soundId, 0);
            nearFlag = (int)var_midiHandles_8c0fcd28[2];
            var_8c2264b8.stopAnnounceState_0x08 = 1;
            var_8c2264b8.stopAnnounceTimer_0x10 = 0;
        }
        break;

    case 1: {
        int local0, local1;

        var_8c2264b8.stopAnnounceTimer_0x10++;
        if (var_8c2264b8.stopAnnounceTimer_0x10 <= 60) {
            break;
        }

        local0 = 0;
        if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE) {
            if (var_8c22640c == 8) {
                local0 = 6;
            } else if (var_8c22640c == 9) {
                local0 = 10;
            }
        }

        switch (var_route_8c18ad1c) {
        case ROUTE_WANGAN:
            local1 = var_prevStopSegment_8c22870c + 0x3f;
            break;
        case ROUTE_SHINJUKU:
        default:
            local1 = var_prevStopSegment_8c22870c + 0x28;
            break;
        case ROUTE_OME:
            local1 = var_prevStopSegment_8c22870c + 0x11;
            break;
        }

        SndProc_8c010cd6(1, local0 + local1);

        if (var_prevStopSegment_8c22870c == var_nextStopSegment_8c228710) {
            var_8c2264b8.stopAnnounceState_0x08 = 2;
            if (var_stopPhase_8c2285e4 != 0) {
                var_8c2264b8.stopAnnounceTimer_0x10 = AsqGetRandomInRangeB_8c0121be(60) + 120;
            } else {
                var_8c2264b8.stopAnnounceTimer_0x10 = 30;
            }
        } else {
            var_8c2264b8.stopAnnounceState_0x08 = 3;
        }
        break;
    }

    case 2:
        if (--var_8c2264b8.stopAnnounceTimer_0x10 >= 0) {
            break;
        }

        sdMidiPlay(var_midiHandles_8c0fcd28[4], 1, 0x1a, 0);
        nearFlag = (int)var_midiHandles_8c0fcd28[4];
        var_8c2264b8.stopAnnounceState_0x08 = 3;
        break;

    case 3:
        /* Restarts the whole jingle sequence once the A-press latch
         * (nearStopLatch_0x0c) has been consumed/reset elsewhere (02c884) and not
         * yet re-armed by 022bdc. */
        if (nearFlag == 0) {
            var_8c2264b8.stopAnnounceState_0x08 = 0;
        }
        break;

    default:
        break;
    }

    /* Unconditional reset (a delay slot that runs either way): discards
     * whatever the switch above left in nearFlag. The camera's mirror-view
     * level below gates whether the check runs at all; a route/segment
     * match is the only way nearFlag becomes 1. */
    nearFlag = 0;
    if (var_cameraMode_8c227d9c >= 2) {
        int prevSeg = var_prevStopSegment_8c22870c;

        switch (var_route_8c18ad1c) {
        case ROUTE_SHINJUKU:
            if (prevSeg == 4 || prevSeg == 5 || prevSeg == 10 ||
                prevSeg == 21 || prevSeg == 22 || prevSeg == 23) {
                nearFlag = 1;
            }
            break;
        case ROUTE_WANGAN:
            if (prevSeg == 10 || prevSeg == 11 || prevSeg == 16) {
                nearFlag = 1;
            }
            break;
        case ROUTE_OME:
        default:
            break;
        }
    }

    if (nearFlag != 0) {
        if (var_8c2264b8.nearStopChimeLatch_0x14 == 0) {
            int r = AsqGetRandomB_8c0121a8();
            int soundId = (r & 1) == 0 ? 0x12 : 0x11;

            sdMidiPlay(var_midiHandles_8c0fcd28[3], 1, soundId, 0);
            var_8c2264b8.nearStopChimeLatch_0x14 = 1;
        }
    } else {
        var_8c2264b8.nearStopChimeLatch_0x14 = 0;
    }

    if (var_vibport_8c1ba354 != (Uint32)-1 && var_8c1ba293 == 0) {
        VibStop_8c010fae(var_vibport_8c1ba354);
    }
}
