/* @unit Ending */
#include <shinobi.h>

#include "01f3c0_ending.h"
#include "0100bc_sound.h"
#include "011120_asset_queues.h"
#include "012504_input.h"
#include "012f44_game.h"
#include "013ae8_route_load.h"
#include "014a9c_tasks.h"
#include "014f54_text.h"
#include "015ab8_title.h"
#include "01614c_debug_menu.h"
#include "016d2c_course_menu.h"
#include "01d7fc_results.h"
#include "022464_fade.h"
#include "028258_objects.h"
#include "sectionB.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "strings.h"

/* ====================
 * Type Declarations
 * ====================
 */

/* Ending-sequence phase, held in MenuState.state_0x18 (reused generically by
 * every screen that drives this struct's state machine). */
enum ENDING_TASK_STATE {
    ENDING_TASK_STATE_WAIT_PVM = 0,             /* waits for the route PVM to be ready, then fades in */
    ENDING_TASK_STATE_FADE_IN = 1,              /* waits for the fade-in, then pushes the pass/fail instructor dialog */
    ENDING_TASK_STATE_INSTRUCTOR_DIALOG = 2,    /* waits for the dialog, then fades out to the credits */
    ENDING_TASK_STATE_FADE_OUT_TO_CREDITS = 3,  /* waits for the fade, then opens the credits textboxes */
    ENDING_TASK_STATE_CREDITS_INTRO = 4,        /* waits for the credits fade-in before starting the scroll timer */
    ENDING_TASK_STATE_CREDITS_SCROLL = 5,       /* scrolls until scrollCreditsText_8c01f50e runs out of credit lines */
    ENDING_TASK_STATE_CREDITS_HOLD = 6,         /* holds on the last frame, then fades out */
    ENDING_TASK_STATE_EXIT = 7,                 /* waits for the fade, then returns to title or the failed-run results */
};

/* ====================
 * Initialized Globals
 * ====================
 */

STATIC const char const_endingPartsName_8c039f4c[20] = "ending_parts.dat";
STATIC const char const_endingDatName_8c039f60[12] = "ending.dat";
STATIC const char const_endingPvmName_8c039f6c[12] = "ending.pvm";

/* Voice ids for CourseMenuPushDialogTask_8c0170c6's dialog_0x04->text_0x00
 * SndProc_8c010cd6(2, id) walk (016d2c_course_menu.c:368); tiers named after
 * the matching entries of init_instructorDialogs_8c044c08 (index 1-4), which
 * selectEndingDialog_8c01f3c0 picks with the same var_dialogQueue_8c225fbc[0]
 * value. */
STATIC int init_endingVoicesPerfect_8c04522c[] = {
    0x51a, 0x51b, 0x51c, 0x51d, 0x51e, 0,
};

STATIC int init_endingVoicesHigh_8c045244[] = {
    0x51f, 0x520, 0x521, 0x522, 0x523, 0,
};

STATIC int init_endingVoicesNormal_8c04525c[] = {
    0x524, 0x525, 0x526, 0x527, 0x528, 0x529, 0,
};

STATIC int init_endingVoicesFailure_8c045278[] = {
    0x52a, 0x52b, 0x52c, 0x52d, 0,
};

/* init_8c04528c and init_8c045290 are laid out back to back in the original
 * binary (1 + 37 pointers with no gap), so scrollCreditsText_8c01f50e reads
 * across the pair as one contiguous 38-entry table via startTimer_0x64. */
STATIC const char *const init_8c04528c[] = {
    MSG_ENDING_CREDITS_01,
};

/* Terminated by an empty string (originally a pointer to a 4-byte zeroed
 * buffer), not a null pointer. */
STATIC const char *const init_8c045290[] = {
    MSG_ENDING_CREDITS_02, MSG_ENDING_CREDITS_03, MSG_ENDING_CREDITS_04, MSG_ENDING_CREDITS_05,
    MSG_ENDING_CREDITS_06, MSG_ENDING_CREDITS_07, MSG_ENDING_CREDITS_08, MSG_ENDING_CREDITS_09,
    MSG_ENDING_CREDITS_10, MSG_ENDING_CREDITS_11, MSG_ENDING_CREDITS_12, MSG_ENDING_CREDITS_13,
    MSG_ENDING_CREDITS_14, MSG_ENDING_CREDITS_15, MSG_ENDING_CREDITS_16, MSG_ENDING_CREDITS_17,
    MSG_ENDING_CREDITS_18, MSG_ENDING_CREDITS_19, MSG_ENDING_CREDITS_20, MSG_ENDING_CREDITS_21,
    MSG_ENDING_CREDITS_22, MSG_ENDING_CREDITS_23, MSG_ENDING_CREDITS_24, MSG_ENDING_CREDITS_25,
    MSG_ENDING_CREDITS_26, MSG_ENDING_CREDITS_27, MSG_ENDING_CREDITS_28, MSG_ENDING_CREDITS_29,
    MSG_ENDING_CREDITS_30, MSG_ENDING_CREDITS_31, MSG_ENDING_CREDITS_32, MSG_ENDING_CREDITS_33,
    MSG_ENDING_CREDITS_34, MSG_ENDING_CREDITS_35, MSG_ENDING_CREDITS_36, MSG_ENDING_CREDITS_37,
    "",
};

STATIC ResourceGroupInfo init_endingResourceGroup_8c045324 = {
    (char *)const_endingPartsName_8c039f4c,
    (char *)const_endingDatName_8c039f60,
    (char *)const_endingPvmName_8c039f6c,
    4,
};

/* ====================
 * Forward Declarations
 * ====================
 */

STATIC void selectEndingDialog_8c01f3c0(void);
STATIC void updateEndingOverlay_8c01f42c(void);
STATIC int scrollCreditsText_8c01f50e(void);
STATIC void creditsTask_8c01f658(void);

/* ====================
 * Functions
 * ====================
 */

/* Tallies each of the 9 courses' badge tier and picks the ending's dialog
 * tier accordingly. */
STATIC void selectEndingDialog_8c01f3c0(void)
{
    int i;
    int perfectCount;
    int attemptedCount;
    Uint8 state;

    perfectCount = 0;
    attemptedCount = 0;
    for (i = 0; i < 9; i++) {
        state = var_progress_8c1ba1cc.courses_0x44[i].storyAward_0x03;
        if (state == AWARD_TIER_GOLD) {
            perfectCount++;
            attemptedCount++;
        } else if (state == AWARD_TIER_SILVER || state == AWARD_TIER_BRONZE) {
            attemptedCount++;
        }
    }

    if (perfectCount >= 9) {
        var_dialogQueue_8c225fbc[0] = INSTR_SUCCESS_PERFECT;
        var_endingVoiceList_8c226430 = init_endingVoicesPerfect_8c04522c;
    } else if (attemptedCount >= 9) {
        var_dialogQueue_8c225fbc[0] = INSTR_SUCCESS_HIGH;
        var_endingVoiceList_8c226430 = init_endingVoicesHigh_8c045244;
    } else if (attemptedCount >= 5) {
        var_dialogQueue_8c225fbc[0] = INSTR_SUCCESS_NORMAL;
        var_endingVoiceList_8c226430 = init_endingVoicesNormal_8c04525c;
    } else {
        var_dialogQueue_8c225fbc[0] = INSTR_FAILURE_FINAL;
        var_endingVoiceList_8c226430 = init_endingVoicesFailure_8c045278;
    }
}

/* Per-frame easing for the credits header sprite and its draw. Reuses
 * MenuState.pos.title (busX/flagY) as the header's x/y and
 * cursorVelocity_0x30 as its x/y velocity: subState_0x1c drives a simple
 * grow-then-shrink vertical bounce (0 = growing, 1 = shrinking), while the
 * x position bounces between 0 and 90 by negating its velocity at the
 * limits. Also redraws the instructor portrait and resets the background
 * color every frame. */
STATIC void updateEndingOverlay_8c01f42c(void)
{
    const float step = 0.1f;

    if (var_menuState_8c1bc7a8.subState_0x1c == 0) {
        var_menuState_8c1bc7a8.cursorVelocity_0x30.y += step;
        var_menuState_8c1bc7a8.pos.title.flagY_0x24 += var_menuState_8c1bc7a8.cursorVelocity_0x30.y;
        if (var_menuState_8c1bc7a8.pos.title.flagY_0x24 > 300.0f) {
            var_menuState_8c1bc7a8.subState_0x1c = 1;
        }
    } else if (var_menuState_8c1bc7a8.subState_0x1c == 1) {
        var_menuState_8c1bc7a8.pos.title.flagY_0x24 -= var_menuState_8c1bc7a8.cursorVelocity_0x30.y;
        var_menuState_8c1bc7a8.cursorVelocity_0x30.y -= step;
        if (0.0f > var_menuState_8c1bc7a8.cursorVelocity_0x30.y) {
            var_menuState_8c1bc7a8.subState_0x1c = 0;
        }
    }

    var_menuState_8c1bc7a8.pos.title.busX_0x20 += var_menuState_8c1bc7a8.cursorVelocity_0x30.x;
    if (var_menuState_8c1bc7a8.pos.title.busX_0x20 < 0.0f
        || var_menuState_8c1bc7a8.pos.title.busX_0x20 > 90.0f) {
        var_menuState_8c1bc7a8.cursorVelocity_0x30.x = -var_menuState_8c1bc7a8.cursorVelocity_0x30.x;
    }

    TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 4,
        var_menuState_8c1bc7a8.pos.title.busX_0x20, var_menuState_8c1bc7a8.pos.title.flagY_0x24, -8.5f);

    if (ObjectsMenuTextboxText_8c02af1c(var_menuTextboxCharLimit_8c225fb8) != 0) {
        TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 1, 0.0f, 0.0f, -7.0f);
    }

    TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_menuState_8c1bc7a8.instructorSprite_0x60, 0.0f, 0.0f, -8.0f);

    njSetBackColor(0x5CA3D9, 0x5CA3D9, 0x5CA3D9);
}

/* Scrolls the double-buffered credit textboxes (var_messageTextBoxA/B) up by
 * 2px/frame. field_0x5c (0/1) tracks which of the pair is "current"; once it
 * scrolls fully off-screen (y < -480) it wraps back to the bottom (y += 960)
 * and loads the next credit line (startTimer_0x64 indexes across the combined
 * init_8c04528c/init_8c045290 table). When TxtPrepareTextBoxLayout_8c01543a
 * fails (the "" terminator), subState_0x1c is latched to signal "no more
 * credits" to creditsTask_8c01f658. field_0x54/field_0x58 are per-box
 * frame counters, clamped to 0xff, fed into TxtDrawTextbox_8c0155e0. */
STATIC int scrollCreditsText_8c01f50e(void)
{
    TextBox *box;

    if (var_menuState_8c1bc7a8.subState_0x1c == 0) {
        int idx = var_menuState_8c1bc7a8.field_0x5c;
        int other = idx ^ 1;

        ((TextBox *) (&var_messageTextBoxA_8c1bc404)[idx])->y_0x04 -= 2;
        ((TextBox *) (&var_messageTextBoxA_8c1bc404)[other])->y_0x04 -= 2;

        box = (TextBox *) (&var_messageTextBoxA_8c1bc404)[idx];
        if ((float) box->y_0x04 < -480.0f) {
            box->y_0x04 += 960;

            if (TxtPrepareTextBoxLayout_8c01543a(box, (&init_8c04528c[0])[var_menuState_8c1bc7a8.startTimer_0x64]) != 0) {
                if (idx != 0) {
                    var_menuState_8c1bc7a8.field_0x58 = 0;
                } else {
                    var_menuState_8c1bc7a8.field_0x54 = 0;
                }
                var_menuState_8c1bc7a8.field_0x5c = idx ^ 1;
                var_menuState_8c1bc7a8.startTimer_0x64++;
            } else {
                var_menuState_8c1bc7a8.subState_0x1c = 1;
            }
        }
    }

    if (++var_menuState_8c1bc7a8.field_0x54 > 0xFF) {
        var_menuState_8c1bc7a8.field_0x54 = 0xFF;
    }
    if (++var_menuState_8c1bc7a8.field_0x58 > 0xFF) {
        var_menuState_8c1bc7a8.field_0x58 = 0xFF;
    }

    TxtDrawTextbox_8c0155e0((TextBox *) var_messageTextBoxA_8c1bc404, var_menuState_8c1bc7a8.field_0x54);
    return TxtDrawTextbox_8c0155e0((TextBox *) var_messageTextBoxB_8c1bc408, var_menuState_8c1bc7a8.field_0x58);
}

/* Per-frame ending state machine, pushed as a Task by EndingStart_8c01f954.
 * Drives: waiting for route assets, fading in, the pass/fail instructor
 * dialog, fading out, opening the credits textboxes, scrolling the
 * credits, holding on the last line, then fading out to either the title
 * screen or the failed-run results (depending on var_selectedVm_8c1ba34c). */
STATIC void creditsTask_8c01f658(void)
{
    switch (var_menuState_8c1bc7a8.state_0x18) {
    case ENDING_TASK_STATE_WAIT_PVM:
        if (RouteLoadIsPvmReady_8c01432a() != 0) {
            return;
        }
        AsqFreeQueues_8c011f7e();
        var_menuState_8c1bc7a8.state_0x18 = ENDING_TASK_STATE_FADE_IN;
        var_menuState_8c1bc7a8.subState_0x1c = 0;
        var_menuState_8c1bc7a8.pos.title.busX_0x20 = 45.0f;
        var_menuState_8c1bc7a8.pos.title.flagY_0x24 = 100.0f;
        var_menuState_8c1bc7a8.cursorVelocity_0x30.x = 1.0f;
        var_menuState_8c1bc7a8.cursorVelocity_0x30.y = 0.0f;
        SndProc_8c010cd6(0, 1);
        FadePushIn_8c022a9c(10);
        return;

    case ENDING_TASK_STATE_FADE_IN:
        if (var_isFading_8c226568 == 0) {
            CourseMenuPushDialogTask_8c0170c6(var_dialogQueue_8c225fbc[0], var_endingVoiceList_8c226430);
            var_menuState_8c1bc7a8.state_0x18 = ENDING_TASK_STATE_INSTRUCTOR_DIALOG;
        }
        updateEndingOverlay_8c01f42c();
        return;

    case ENDING_TASK_STATE_INSTRUCTOR_DIALOG:
        if (var_instructorDialogActive_8c225fb4 == 0) {
            var_menuState_8c1bc7a8.state_0x18 = ENDING_TASK_STATE_FADE_OUT_TO_CREDITS;
            SndStartAdxFadeOut_8c010bae(0);
            SndStartAdxFadeOut_8c010bae(1);
            FadePushOut_8c022b60(30);
        }
        updateEndingOverlay_8c01f42c();
        return;

    case ENDING_TASK_STATE_FADE_OUT_TO_CREDITS:
        if (var_isFading_8c226568 != 0) {
            updateEndingOverlay_8c01f42c();
            return;
        }
        if (init_8c03bd80 != 0) {
            return;
        }

        var_menuState_8c1bc7a8.state_0x18 = ENDING_TASK_STATE_CREDITS_INTRO;
        var_menuState_8c1bc7a8.logo_timer_0x68 = 0;
        ObjectsFreeTextboxes_8c02af32();
        TxtInit_8c01524c();
        var_messageTextBoxA_8c1bc404 = TxtCreateTextBox_8c0152fc(0, 480, -5.0f, 640, 480, 0, 0, -1);
        var_messageTextBoxB_8c1bc408 = TxtCreateTextBox_8c0152fc(0, 960, -5.0f, 640, 480, 0, 0, -1);
        TxtPrepareTextBoxLayout_8c01543a((TextBox *) var_messageTextBoxA_8c1bc404, init_8c04528c[0]);
        TxtPrepareTextBoxLayout_8c01543a((TextBox *) var_messageTextBoxB_8c1bc408, init_8c045290[0]);
        var_menuState_8c1bc7a8.subState_0x1c = 0;
        var_menuState_8c1bc7a8.field_0x5c = 0;
        var_menuState_8c1bc7a8.startTimer_0x64 = 2;
        var_menuState_8c1bc7a8.field_0x54 = 0;
        var_menuState_8c1bc7a8.field_0x58 = 0;
        var_menuState_8c1bc7a8.logo_timer_0x68 = 3240;
        SndProc_8c010cd6(0, 12);
        FadePushIn_8c022a9c(30);
        return;

    case ENDING_TASK_STATE_CREDITS_INTRO:
        if (var_isFading_8c226568 == 0) {
            var_menuState_8c1bc7a8.state_0x18 = ENDING_TASK_STATE_CREDITS_SCROLL;
        }
        scrollCreditsText_8c01f50e();
        var_menuState_8c1bc7a8.logo_timer_0x68--;
        return;

    case ENDING_TASK_STATE_CREDITS_SCROLL:
        var_menuState_8c1bc7a8.logo_timer_0x68--;
        if (var_menuState_8c1bc7a8.logo_timer_0x68 < 0) {
            SndProc_8c010cd6(0, 11);
            var_menuState_8c1bc7a8.logo_timer_0x68 = 2700;
        }
        if (var_menuState_8c1bc7a8.subState_0x1c != 0) {
            var_menuState_8c1bc7a8.state_0x18 = ENDING_TASK_STATE_CREDITS_HOLD;
            var_menuState_8c1bc7a8.logo_timer_0x68 = 0;
        }
        scrollCreditsText_8c01f50e();
        return;

    case ENDING_TASK_STATE_CREDITS_HOLD:
        var_menuState_8c1bc7a8.logo_timer_0x68++;
        if (var_menuState_8c1bc7a8.logo_timer_0x68 > 150) {
            var_menuState_8c1bc7a8.state_0x18 = ENDING_TASK_STATE_EXIT;
            SndStartAdxFadeOut_8c010bae(0);
            SndStartAdxFadeOut_8c010bae(1);
            FadePushOut_8c022b60(60);
        }
        scrollCreditsText_8c01f50e();
        return;

    case ENDING_TASK_STATE_EXIT:
        if (var_isFading_8c226568 != 0) {
            scrollCreditsText_8c01f50e();
            return;
        }
        if (init_8c03bd80 != 0) {
            return;
        }
        DebugMenuFreeSessionAssets_8c016182();
        if (var_selectedVm_8c1ba34c == -1) {
            TitlePushTitle_8c015fd6(0);
        } else {
            ResultShowFailedRun_8c01e24e();
        }
        return;
    }
}

/* Entry point, called once the player finishes their final course (see the
 * var_progress_8c1ba1cc.days_0x00 > 30 checks at both call sites). Picks the
 * ending dialog tier, seeds the instructor sprite from it, kicks the
 * "did you pass?" input handling, and pushes GameTask_8c012f44 (the
 * underlying gameplay task, kept running for the driving-scene backdrop)
 * and creditsTask_8c01f658 (the ending state machine) as Tasks. */
void EndingStart_8c01f954(void)
{
    Task *gameTask;
    void *gameTaskState;
    Task *creditsTaskHandle;
    void *creditsTaskState;

    selectEndingDialog_8c01f3c0();

    var_menuState_8c1bc7a8.instructorSprite_0x60 =
        init_instructorDialogs_8c044c08[var_dialogQueue_8c225fbc[0]]->spriteNo_0x04;

    InputPushTask_8c0128cc(0);

    TaskPush_8c014ae8(var_tasks_8c1ba3c8, GameTask_8c012f44, &gameTask, &gameTaskState, 0);
    TaskPush_8c014ae8(var_tasks_8c1ba3c8, (void *) creditsTask_8c01f658, &creditsTaskHandle, &creditsTaskState, 0);

    var_menuState_8c1bc7a8.state_0x18 = 0;
    njGarbageTexture(var_tex_8c157af8, 3072);

    ObjectsOpenTextbox_8c02ae3e(32, 384, -2.0f, 576, 64, 0, 0, -1);
    ObjectsSwapMessageBoxFor_8c02aefc("");

    var_menuTextboxCharLimit_8c225fb8 = 0;
    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();

    CourseMenuRequestSysResgrp_8c018568(&var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        &init_endingResourceGroup_8c045324);
    CourseMenuRequestCommonResources_8c01852c();

    RouteLoadSetPvmReady_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteLoadResetPvmReady_8c014322);
}
