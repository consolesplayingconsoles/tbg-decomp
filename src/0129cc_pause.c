/* @unit Pause */
#include <shinobi.h>
#include "0129cc_pause.h"
#include "0100bc_sound.h"
#include "011120_asset_queues.h"
#include "012f44_game.h"
#include "014f54_text.h"
#include "sectionB.h"
#include "01614c_debug_menu.h"
#include "015ab8_title.h"
#include "016d2c_course_menu.h"
#include "01e27c_practice_menu.h"
#include "022464_fade.h"
#include "0222dc_fadecmd.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

/* marks from var_markTexlist_8c1bc418 */
#define MARK_BASE          0x74
#define MARK_CONTINUE      0x75
#define MARK_RETIRE        0x7a
#define MARK_CONFIRM_YES   0x76
#define MARK_CONFIRM_NO    0x77

#define MARK_Z_ARROW       -1.09f
#define MARK_Z_BASE        -1.1f

/* the stick deflection that counts as a dpad press */
#define STICK_THRESHOLD    0x40

#define RETIRE_PHASE_IDLE      0
#define RETIRE_PHASE_CONFIRM   1
#define RETIRE_PHASE_FADING    2

#define CONFIRM_YES        0
#define CONFIRM_NO         1

#ifdef SERIAL_DEBUG
char *DEBUG_retirePhaseNames[] = {
    "IDLE",
    "CONFIRM",
    "FADING",
};
#endif

#define CHANGE_RETIRE_PHASE(x)                                                 \
    var_retirePhase_8c18ad08 = x;                                              \
    LOG_DEBUG(("[PAUSE] Retire phase changed: %s\n", DEBUG_retirePhaseNames[x]))

/* PauseDemoEndTask_8c012d5a: demo ending marks and phases (see phase_0x08) */
#define MARK_DEMO          0x7b
#define MARK_DEMO_BLINK    0x7c

#define DEMO_END_PLAYING   0
#define DEMO_END_SKIPPED   1
#define DEMO_END_TIMED_OUT 2

#define DEMO_END_TIMEOUT   0x708 /* 1800 frames, 30s */

/* =========
 * Functions
 * =========
 */

/*
 * Pause menu (Start during a drive): CONTINUE / RETIRE, RETIRE guarded by a
 * YES/NO confirm. Returns nonzero while closed -- the caller takes that as
 * leave for the rest of the frame's tasks to run.
 *
 * njDrawPolygon draws the dimmed backdrop quad behind the marks.
 *
 * var_onRetire_8c18ad10 is the cursor: 0 CONTINUE, 1 RETIRE. The confirm
 * always opens on NO.
 */
STATIC int update_8c0129cc(void)
{
    PDS_PERIPHERAL *p;
    int i;

    /* Start opens the menu -- so does losing the controller. A message box on
       screen blocks it. */
    if (!var_pauseActive_8c1bb8cc) {
        if (((var_peripheral_8c1ba358->press & PDD_DGT_ST) != 0 || var_activeCtrlType_8c157a70 == -1)
            && var_messageBoxActive_8c22847c == 0) {
            var_pauseActive_8c1bb8cc = TRUE;
            var_pauseSettle_8c18ad04 = 0;
            var_onRetire_8c18ad10 = 0;
            SndControlAdxtWithOutVol_8c0107d2(1);
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            if (var_vibport_8c1ba354 != -1) {
                pdVibMxStop(var_vibport_8c1ba354);
            }
            LOG_DEBUG(("[PAUSE] update_8c0129cc: menu opened\n"));
        }
        return 1;
    }

    /* Dead: nothing in the image ever stores a nonzero here, and opening the
       menu clears it. Kept because it is in the original. */
    if (var_pauseSettle_8c18ad04 != 0) {
        if (var_pauseSettle_8c18ad04 == 1) {
            var_pauseSettle_8c18ad04 = 2;
        }
        return 0;
    }

    FadeUpdate_8c022560();
    p = var_peripheral_8c1ba358;

    if ((p->press & PDD_DGT_ST) != 0) {
        /* Start again: close, no sound. */
        var_pauseActive_8c1bb8cc = 0;
        SndControlAdxtWithOutVol_8c0107d2(0);
        LOG_DEBUG(("[PAUSE] update_8c0129cc: menu closed (Start)\n"));
    } else if (var_onRetire_8c18ad10 == 0) {
        /* Cursor on CONTINUE: A resumes, Down moves to RETIRE. */
        if ((p->press & PDD_DGT_TA) != 0) {
            var_pauseActive_8c1bb8cc = 0;
            SndControlAdxtWithOutVol_8c0107d2(0);
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            LOG_DEBUG(("[PAUSE] update_8c0129cc: resumed (A)\n"));
        } else if ((p->press & PDD_DGT_KD) != 0 || p->y1 > STICK_THRESHOLD) {
            var_onRetire_8c18ad10 = 1;
            var_retirePhase_8c18ad08 = RETIRE_PHASE_IDLE;
            var_confirmChoice_8c18ad0c = CONFIRM_NO;
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            LOG_DEBUG(("[PAUSE] update_8c0129cc: cursor -> RETIRE\n"));
        }
        TxtDrawSprite_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_CONTINUE, 0.0f, 0.0f, MARK_Z_ARROW);
    } else {
        switch (var_retirePhase_8c18ad08) {
        case RETIRE_PHASE_IDLE:
            /* Cursor on RETIRE: A opens the confirm, Up back to CONTINUE. */
            if ((p->press & PDD_DGT_TA) != 0) {
                CHANGE_RETIRE_PHASE(RETIRE_PHASE_CONFIRM);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            } else if ((p->press & PDD_DGT_KU) != 0 || p->y1 < -STICK_THRESHOLD) {
                var_onRetire_8c18ad10 = 0;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                LOG_DEBUG(("[PAUSE] update_8c0129cc: cursor -> CONTINUE\n"));
            }
            TxtDrawSprite_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_RETIRE, 0.0f, 0.0f, MARK_Z_ARROW);
            break;

        case RETIRE_PHASE_CONFIRM:
            /* Confirm: B cancels; Left=YES / Right=NO; A acts on the choice. */
            if ((p->press & PDD_DGT_TB) != 0) {
                CHANGE_RETIRE_PHASE(RETIRE_PHASE_IDLE);
                TxtDrawSprite_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_RETIRE, 0.0f, 0.0f, MARK_Z_ARROW);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
                break;
            }
            if (var_confirmChoice_8c18ad0c == CONFIRM_YES) {
                /* A commits -- start the fade, no mark this frame. */
                if ((p->press & PDD_DGT_TA) != 0) {
                    CHANGE_RETIRE_PHASE(RETIRE_PHASE_FADING);
                    FadePushOut_8c022b60(10);
                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                    LOG_DEBUG(("[PAUSE] update_8c0129cc: retire committed, fading out\n"));
                    break;
                }
                if ((p->press & PDD_DGT_KR) != 0 || p->x1 > STICK_THRESHOLD) {
                    var_confirmChoice_8c18ad0c = CONFIRM_NO;
                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                    LOG_DEBUG(("[PAUSE] update_8c0129cc: confirm choice -> NO\n"));
                }
                TxtDrawSprite_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_CONFIRM_YES, 0.0f, 0.0f, MARK_Z_ARROW);
                break;
            }
            /* A cancels back to RETIRE. */
            if ((p->press & PDD_DGT_TA) != 0) {
                CHANGE_RETIRE_PHASE(RETIRE_PHASE_IDLE);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            } else if ((p->press & PDD_DGT_KL) != 0 || p->x1 < -STICK_THRESHOLD) {
                var_confirmChoice_8c18ad0c = CONFIRM_YES;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                LOG_DEBUG(("[PAUSE] update_8c0129cc: confirm choice -> YES\n"));
            }
            TxtDrawSprite_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_CONFIRM_NO, 0.0f, 0.0f, MARK_Z_ARROW);
            break;

        case RETIRE_PHASE_FADING:
            /* Retire committed: hold the YES mark through the fade, then leave the drive. */
            if (var_isFading_8c226568 != 0) {
                TxtDrawSprite_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_CONFIRM_YES, 0.0f, 0.0f, MARK_Z_ARROW);
                njDrawPolygon(init_pauseDimQuad_8c03bf4c, 4, 1);
                return 0;
            }
            var_runSucceeded_8c1bb8dc = 0;
            var_runReportPending_8c1bb8b8 = 0;
            var_runWasPractice_8c1bb8bc = 0;
            DebugMenuFreeSessionAssets_8c016182();
            if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE) {
                var_menuState_8c1bc7a8.selected_0x38 = var_practiceLesson_8c22640c;
                PracticeMenuLessonRetry_8c01f21c();
                LOG_INFO(("[PAUSE] update_8c0129cc: retire complete, leaving drive (practice)\n"));
                return 0;
            }
            for (i = 0; i < 5; i++) {
                var_progress_8c1ba1cc.eventProgressFlags_0x04[i] = var_eventFlagsSnapshot_8c1ba2b8[i];
                var_progress_8c1ba1cc.profileProgressFlags_0x18[i] = var_profileFlagsSnapshot_8c1ba2cc[i];
            }
            CourseMenuReturn_8c017ef2();
            LOG_INFO(("[PAUSE] update_8c0129cc: retire complete, leaving drive (course menu)\n"));
            return 0;
        }
    }

    TxtDrawSprite_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_BASE, 0.0f, 0.0f, MARK_Z_BASE);
    njDrawPolygon(init_pauseDimQuad_8c03bf4c, 4, 1);
    return 0;
}

/*
 * TaskPush_8c014ae8 action for a normal drive: soft-reset check first, then
 * the pause menu, and the rest of the frame's tasks only while it is closed.
 */
void PauseTask_8c012cbc()
{
    LOG_TRACE(("[PAUSE] PauseTask_8c012cbc\n"));

    if (var_resetRequested_8c157a78 != 0 && var_vmBusy_8c157a7c == 0
        && var_queuesAreInitialized_8c157a60 == 0) {
        DebugMenuFreeSessionAssets_8c016182();
        TitlePushTitle_8c015fd6(1);
        LOG_DEBUG(("[PAUSE] PauseTask_8c012cbc: reset requested, returning to title\n"));
        return;
    }

    if (update_8c0129cc() != 0) {
        FadeCmdResetQueues_8c02239c();
        TaskExecGroup_8c014b42(var_tasks_8c1ba5e8);
        FadeUpdate_8c022560();
    }
}

/*
 * TaskPush_8c014ae8 action for VMU replay playback (PLAY_MODE_DEMO with
 * var_isAttractDemo_8c1bb8d4 == 0). No CONTINUE/RETIRE menu -- Start only
 * freezes the replay.
 */
void PauseToggleTask_8c012d06()
{
    LOG_TRACE(("[PAUSE] PauseToggleTask_8c012d06\n"));

    if (var_resetRequested_8c157a78 != 0 && var_vmBusy_8c157a7c == 0
        && var_queuesAreInitialized_8c157a60 == 0) {
        DebugMenuFreeSessionAssets_8c016182();
        TitlePushTitle_8c015fd6(1);
        LOG_DEBUG(("[PAUSE] PauseToggleTask_8c012d06: reset requested, returning to title\n"));
        return;
    }

    if ((var_peripheral_8c1ba358->press & PDD_DGT_ST) != 0) {
        var_pauseActive_8c1bb8cc ^= 1;
    }

    if (var_pauseActive_8c1bb8cc == 0) {
        FadeCmdResetQueues_8c02239c();
        TaskExecGroup_8c014b42(var_tasks_8c1ba5e8);
    }

    FadeUpdatePlain_8c022910();
}

/*
 * TaskPush_8c014ae8 action for the attract loop (PLAY_MODE_DEMO with
 * var_isAttractDemo_8c1bb8d4 != 0): plays out until Start or the timeout, then
 * fades back to the title -- skipping the FortyFive logo when Start ended it,
 * replaying it on a timeout.
 */
void PauseDemoEndTask_8c012d5a(PauseDemoEndTaskData *task)
{
    LOG_TRACE(("[PAUSE] PauseDemoEndTask_8c012d5a\n"));

    if (var_resetRequested_8c157a78 != 0 && var_vmBusy_8c157a7c == 0
        && var_queuesAreInitialized_8c157a60 == 0) {
        DebugMenuFreeSessionAssets_8c016182();
        init_8c03bd80 = 1;
        init_8c03bd84 = 0;
        LOG_DEBUG(("[PAUSE] PauseDemoEndTask_8c012d5a: reset requested\n"));
        return;
    }

    SndUpdateAdxVolFade_8c010a40();

    switch (task->phase_0x08) {
    case DEMO_END_PLAYING:
        if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_ST) != 0) {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            task->phase_0x08 = DEMO_END_SKIPPED;
            LOG_DEBUG(("[PAUSE] PauseDemoEndTask_8c012d5a: demo skipped (Start), fading out\n"));
        } else {
            task->counter_0x0c++;
            if (task->counter_0x0c <= DEMO_END_TIMEOUT) {
                break;
            }
            task->phase_0x08 = DEMO_END_TIMED_OUT;
            LOG_DEBUG(("[PAUSE] PauseDemoEndTask_8c012d5a: demo timed out, fading out\n"));
        }
        SndStartAdxFadeOut_8c010bae(0);
        SndStartAdxFadeOut_8c010bae(1);
        FadePushOut_8c022b60(0x1e);
        break;

    case DEMO_END_SKIPPED:
    case DEMO_END_TIMED_OUT:
        if (var_isFading_8c226568 != 0) {
            task->counter_0x0c++;
            break;
        }
        if (init_8c03bd80 != 0) {
            return;
        }
        DebugMenuFreeSessionAssets_8c016182();
        TitlePushTitle_8c015fd6(task->phase_0x08 == DEMO_END_SKIPPED ? 1 : 0);
        LOG_INFO(("[PAUSE] PauseDemoEndTask_8c012d5a: demo ended, returning to title\n"));
        return;
    }

    FadeCmdResetQueues_8c02239c();
    TaskExecGroup_8c014b42(var_tasks_8c1ba5e8);
    FadeUpdatePlain_8c022910();
    TxtDrawSprite_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_DEMO, 0.0f, 0.0f, MARK_Z_BASE);
    if ((task->counter_0x0c & 0x18) != 0) {
        TxtDrawSprite_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_DEMO_BLINK, 0.0f, 0.0f, MARK_Z_BASE);
    }
}
