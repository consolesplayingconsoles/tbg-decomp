/* @unit PracticeMenu */
#include <shinobi.h>

#include "01e27c_practice_menu.h"
#include "011120_asset_queues.h"
#include "012f44_game.h"
#include "013ae8_route_load.h"
#include "015ab8_title.h"
#include "01614c_debug_menu.h"
#include "016c58_prompt.h"
#include "016d2c_course_menu.h"
#include "01f3c0.h"
#include "028258.h"
#include "0100bc_sound.h"
#include "sectionB.h"
#include "serial_debug.h"
#include "strings_ja_jp.sjis.h"

/* ====================
 * Type Declarations
 * ====================
 */

enum STATE {
    STATE_INIT,                /* 0 */
    STATE_DESCRIPTION_FADE_IN, /* 1 */
    STATE_DESCRIPTION_VIEW,    /* 2 */
    STATE_PROMPT,              /* 3 */
    STATE_CONFIRM_FADE_OUT,    /* 4 */
    STATE_LOADING_FADE_IN,     /* 5 */
    STATE_LOADING_HOLD,        /* 6 */
    STATE_LOADING_START,       /* 7 */
    STATE_CANCEL_FADE_OUT      /* 8 */
};

/* Separate state space for showLesson_8c01e63c, sharing state_0x18
 * with the enum above (each task action owns it while installed). */
enum SHOW_LESSON_STATE {
    SHOW_LESSON_STATE_INIT,            /* 0 */
    SHOW_LESSON_STATE_FADE_IN,         /* 1 */
    SHOW_LESSON_STATE_VIEW,            /* 2 */
    SHOW_LESSON_STATE_VIEW_END,        /* 3: field_0x40 scrolled to the last page */
    SHOW_LESSON_STATE_FADE_OUT_BACK,   /* 4: fades out, then -> initDescriptionReveal_8c01e576 */
    SHOW_LESSON_STATE_FADE_OUT_START   /* 5: fades out, then -> PracticeMenuLessonStart_8c01f114 */
};

/* Separate state space for FUN_8c01ebf2, sharing state_0x18 with the enums
 * above (each task action owns it while installed). */
enum LESSON_STATE {
    LESSON_STATE_INIT,             /* 0 */
    LESSON_STATE_DIALOG_FADE_IN,   /* 1 */
    LESSON_STATE_DIALOG_QUEUE,     /* 2 */
    LESSON_STATE_MENU,             /* 3 */
    LESSON_STATE_MENU_LOCKED,      /* 4: selected_0x38 scrolled past 10, only scroll-up allowed */
    LESSON_STATE_QUIT_PROMPT,      /* 5 */
    LESSON_STATE_CANCEL_FADE_OUT,  /* 6: fades out, then -> practiceCancelReturn_8c01e920 */
    LESSON_STATE_LOADING_FADE_OUT  /* 7: fades out, then commits the run's result and returns */
};

/* ====================
 * Initialized Globals
 * ====================
 */

/* Message-box text (Shift-JIS, private to this unit). Sized to include the
 * archived asm's trailing zero padding. */
STATIC char const_8c03896c[24] = MSG_CONFIRM_START_PRACTICE;
STATIC char const_8c038984[4] = "";
STATIC char const_8c038988[44] = MSG_CONFIRM_QUIT_PRACTICE;

/* Cumulative per-course description-page offsets: course N's pages run
 * [init_8c0451b4[N], init_8c0451b4[N+1]). */
STATIC char init_8c0451b4[] = {
    0, 2, 3, 4, 6, 9, 12, 15, 17, 19, 24, 25
};
/* Per-course loading-screen background id, indexed by var_8c22640c. */
STATIC Uint32 init_8c0451c0[] = { 0, 0, 0, 0, 0, 0, 0, 6, 0xf, 0xf, 0xf };
STATIC char init_8c0451ec[] = { 0x10, 0x0a, 0x14, 0x11, 0x12, 0x14, 0x15, 0x13, 0x13, 0x1b, 0x09, 0x00 };
STATIC char init_8c0451f8[] = { 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0xc0, 0x40, 0x00, 0x00, 0x30, 0x41, 0x00, 0x00, 0xe0, 0x40 };
STATIC char init_8c045208[] = { 0x20, 0x21, 0x23, 0x24, 0x25, 0x26, 0x28, 0x29, 0x29, 0x2b, 0x2c, 0x2d, 0x2d, 0x2e, 0x2e, 0x2f, 0x32, 0x33, 0x34, 0x35, 0x31, 0x31, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3b, 0x3d, 0x31, 0x27, 0x3e, 0x3f, 0x00, 0x00 };

/* ====================
 * Forward Declarations
 * ====================
 */

STATIC void practiceCancelReturn_8c01e920(Task *task);

/* ====================
 * Functions
 * ====================
 */

STATIC void FUN_8c01e27c(Task *task)
{
    switch (var_menuState_8c1bc7a8.state_0x18) {
        case STATE_INIT:
            if (RouteLoadIsPvmReady_8c01432a()) break;

            AsqFreeQueues_8c011f7e();
            var_menuState_8c1bc7a8.state_0x18 = STATE_DESCRIPTION_FADE_IN;
            var_menuState_8c1bc7a8.field_0x54 = init_8c0451b4[var_8c22640c];
            var_menuState_8c1bc7a8.field_0x5c = var_menuState_8c1bc7a8.field_0x54;
            var_menuState_8c1bc7a8.field_0x58 = init_8c0451b4[var_8c22640c + 1] - 1;
            SndProc_8c010cd6(0, 0xd);
            push_fadein_8c022a9c(10);
            break;

        case STATE_DESCRIPTION_FADE_IN:
            if (var_isFading_8c226568 == 0)
                var_menuState_8c1bc7a8.state_0x18 = STATE_DESCRIPTION_VIEW;

            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                var_menuState_8c1bc7a8.field_0x5c,
                0.0f, 0.0f, -4.5f
            );
            break;

        case STATE_DESCRIPTION_VIEW:
            if (var_peripherals_8c1ba35c[0].press & 0x4) {
                if (var_menuState_8c1bc7a8.field_0x5c < var_menuState_8c1bc7a8.field_0x58) {
                    var_menuState_8c1bc7a8.field_0x5c++;
                } else {
                    var_menuState_8c1bc7a8.state_0x18 = STATE_PROMPT;
                    var_menuState_8c1bc7a8.selected_0x38 = 0;
                    swapMessageBoxFor_8c02aefc(const_8c03896c);
                }

                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            }

            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                var_menuState_8c1bc7a8.field_0x5c,
                0.0f, 0.0f, -4.5f
            );
            break;

        case STATE_PROMPT: {
            int result = PromptHandleBinary_8c016caa(&var_menuState_8c1bc7a8.selected_0x38);

            if (result == 1) {
                var_menuState_8c1bc7a8.state_0x18 = STATE_CONFIRM_FADE_OUT;
                SndStartAdxFadeOut_8c010bae(0);
                SndStartAdxFadeOut_8c010bae(1);
                push_fadeout_8c022b60(10);
            } else if (result == 2) {
                var_menuState_8c1bc7a8.state_0x18 = STATE_CANCEL_FADE_OUT;
                SndStartAdxFadeOut_8c010bae(0);
                SndStartAdxFadeOut_8c010bae(1);
                push_fadeout_8c022b60(10);
            }

            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                var_menuState_8c1bc7a8.selected_0x38 + 2,
                224.0f, 300.0f, -4.0f
            );

            if (menuTextboxText_8c02af1c(0xff))
                TxtDrawSprite_8c014f54(
                    &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                    1,
                    0.0f, 0.0f, -5.0f
                );

            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                0,
                0.0f, 0.0f, -8.0f
            );
            break;
        }

        case STATE_CONFIRM_FADE_OUT:
            if (var_isFading_8c226568 == 0) {
                var_menuState_8c1bc7a8.state_0x18 = STATE_LOADING_FADE_IN;
                var_menuState_8c1bc7a8.logo_timer_0x68 = 0;
                njSetBackColor(0, 0, 0);
                push_fadein_8c022a9c(0x14);
                break;
            }

            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                var_menuState_8c1bc7a8.selected_0x38 + 2,
                224.0f, 300.0f, -4.0f
            );

            if (menuTextboxText_8c02af1c(0xff))
                TxtDrawSprite_8c014f54(
                    &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                    1,
                    0.0f, 0.0f, -5.0f
                );

            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                0,
                0.0f, 0.0f, -8.0f
            );
            break;

        case STATE_LOADING_FADE_IN:
            if (var_isFading_8c226568 == 0)
                var_menuState_8c1bc7a8.state_0x18 = STATE_LOADING_HOLD;

            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                var_8c22640c + 0x19,
                0.0f, 0.0f, -4.5f
            );
            break;

        case STATE_LOADING_HOLD:
            if (++var_menuState_8c1bc7a8.logo_timer_0x68 > 10) {
                var_menuState_8c1bc7a8.state_0x18 = STATE_LOADING_START;
                push_fadeout_8c022b60(0x14);
            }

            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                var_8c22640c + 0x19,
                0.0f, 0.0f, -4.5f
            );
            break;

        case STATE_LOADING_START:
            if (var_isFading_8c226568 == 0) {
                if (init_8c03bd80 != 0) break;

                FUN_8c016182();
                var_8c1bb8f0 = 0;
                var_8c1bb8ec = 0x1d;
                var_8c1bb8f4 = 0;
                var_8c226410 = init_8c0451c0[var_8c22640c];
                GamePushLoadingTask_8c013310(var_8c22640c + 0x1b);
                break;
            }

            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                var_8c22640c + 0x19,
                0.0f, 0.0f, -4.5f
            );
            break;

        case STATE_CANCEL_FADE_OUT:
            if (var_isFading_8c226568 == 0) {
                if (init_8c03bd80 != 0) break;

                practiceCancelReturn_8c01e920(task);
                break;
            }

            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                var_menuState_8c1bc7a8.selected_0x38 + 2,
                224.0f, 300.0f, -4.0f
            );

            if (menuTextboxText_8c02af1c(0xff))
                TxtDrawSprite_8c014f54(
                    &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                    1,
                    0.0f, 0.0f, -4.5f
                );
            break;
    }
}

/* One-time setup for the description-text reveal (FUN_8c01e27c): installs it
 * as the task action and kicks off the course's sys resource group load. */
STATIC void initDescriptionReveal_8c01e576(Task *task)
{
    TaskSetAction_8c014b3e(task, FUN_8c01e27c);
    var_menuState_8c1bc7a8.state_0x18 = 0;

    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();
    CourseMenuRequestSysResgrp_8c018568(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, &init_8c044284);
    RouteLoadSetPvmReady_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteLoadResetPvmReady_8c014322);
}

/* Post-cancel re-entry: shows the same lesson's practice-guide text again,
 * scrollable (field_0x40, window of 4 rows out of init_8c0451ec[var_8c22640c]),
 * then either backs out to the full guide reveal (initDescriptionReveal_8c01e576)
 * or restarts the lesson directly, skipping the confirm prompt. */
STATIC void showLesson_8c01e63c(Task *task)
{
    switch (var_menuState_8c1bc7a8.state_0x18) {
        case SHOW_LESSON_STATE_INIT:
            if (RouteLoadIsPvmReady_8c01432a()) return;

            AsqFreeQueues_8c011f7e();
            var_menuState_8c1bc7a8.state_0x18 = SHOW_LESSON_STATE_FADE_IN;
            SndProc_8c010cd6(0, 0xd);
            push_fadein_8c022a9c(10);
            return;

        case SHOW_LESSON_STATE_FADE_IN:
            if (var_isFading_8c226568 == 0) {
                var_menuState_8c1bc7a8.state_0x18 = SHOW_LESSON_STATE_VIEW;
            }
            break;

        case SHOW_LESSON_STATE_VIEW:
            if (var_peripherals_8c1ba35c[0].press & 0x4) {
                var_menuState_8c1bc7a8.state_0x18 = SHOW_LESSON_STATE_FADE_OUT_BACK;
                SndStartAdxFadeOut_8c010bae(0);
                SndStartAdxFadeOut_8c010bae(1);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                push_fadeout_8c022b60(10);
            } else if (var_peripherals_8c1ba35c[0].press & 0x2) {
                var_menuState_8c1bc7a8.state_0x18 = SHOW_LESSON_STATE_FADE_OUT_START;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
                push_fadeout_8c022b60(10);
            } else if (!(var_peripherals_8c1ba35c[0].press & 0x10)) {
                if (var_peripherals_8c1ba35c[0].press & 0x20) {
                    if (var_menuState_8c1bc7a8.field_0x40 + 3 < (signed char)init_8c0451ec[var_8c22640c])
                        var_menuState_8c1bc7a8.field_0x40++;
                    else
                        var_menuState_8c1bc7a8.state_0x18 = SHOW_LESSON_STATE_VIEW_END;

                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                }
            } else if (var_menuState_8c1bc7a8.field_0x40 >= 1) {
                var_menuState_8c1bc7a8.field_0x40--;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            }
            break;

        case SHOW_LESSON_STATE_VIEW_END:
            if (var_peripherals_8c1ba35c[0].press & 0x4) {
                var_menuState_8c1bc7a8.state_0x18 = SHOW_LESSON_STATE_FADE_OUT_START;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
                push_fadeout_8c022b60(10);
            } else if (var_peripherals_8c1ba35c[0].press & 0x10) {
                var_menuState_8c1bc7a8.state_0x18 = SHOW_LESSON_STATE_VIEW;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            }
            break;

        case SHOW_LESSON_STATE_FADE_OUT_BACK:
            if (var_isFading_8c226568 == 0) {
                if (init_8c03bd80 != 0) return;
                initDescriptionReveal_8c01e576(task);
                return;
            }
            break;

        case SHOW_LESSON_STATE_FADE_OUT_START:
            if (var_isFading_8c226568 == 0) {
                var_menuState_8c1bc7a8.selected_0x38 = var_8c22640c;
                PracticeMenuLessonStart_8c01f114(task);
                return;
            }
            break;
    }

    if (var_gameMode_8c1bb8fc == 0)
        CourseMenuDrawDateAndExp_8c016ee6();

    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_8c22640c + 0x1c,
        0.0f, 0.0f, -4.0f
    );

    njUserClipping(2, init_8c0451f8);
    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_8c22640c + 0x27,
        65.0f, 195.0f - (float)var_menuState_8c1bc7a8.field_0x40 * 24.0f, -4.0f
    );
    njUserClipping(0, init_8c0451f8);

    if (var_menuState_8c1bc7a8.field_0x40 > 0)
        TxtDrawSprite_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
            0x18,
            0.0f, 0.0f, -4.5f
        );

    if (var_menuState_8c1bc7a8.field_0x40 + 3 != (signed char)init_8c0451ec[var_8c22640c])
        TxtDrawSprite_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
            0x19,
            0.0f, 0.0f, -4.5f
        );

    if (var_menuState_8c1bc7a8.state_0x18 == SHOW_LESSON_STATE_VIEW_END)
        TxtDrawSprite_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
            0x1b,
            0.0f, 0.0f, -4.5f
        );

    if (menuTextboxText_8c02af1c(0xff))
        TxtDrawSprite_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupA_0x00,
            1,
            0.0f, 0.0f, -5.0f
        );

    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        0x32,
        0.0f, 0.0f, -6.0f
    );

    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_gameMode_8c1bb8fc == 0 ? 0x17 : 0x41,
        0.0f, 0.0f, -7.0f
    );

    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00,
        0,
        0.0f, 0.0f, -8.0f
    );
}

/* User confirmed cancel: rewinds to showLesson_8c01e63c and either goes
 * straight back to it if the sys resource group is still around,
 * or frees the queues and fades in to reload it. */
STATIC void practiceCancelReturn_8c01e920(Task *task)
{
    TaskSetAction_8c014b3e(task, showLesson_8c01e63c);
    var_menuState_8c1bc7a8.field_0x40 = 0;
    swapMessageBoxFor_8c02aefc(const_8c038984);

    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();

    if (CourseMenuRequestSysResgrp_8c018568(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, &init_8c044274)) {
        RouteLoadSetPvmReady_8c014330();
        AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteLoadResetPvmReady_8c014322);
        var_menuState_8c1bc7a8.state_0x18 = STATE_INIT;
        return;
    }

    AsqFreeQueues_8c011f7e();
    var_menuState_8c1bc7a8.state_0x18 = STATE_DESCRIPTION_FADE_IN;
    push_fadein_8c022a9c(10);
    // Unreachable: compiled as a tail call, never returns here.
    // coverage:ignore-next-line
}

/* Builds the post-lesson dialog sequence queue (var_8c226414, -1 terminated).
 * IDs below are dialog_sequences_8c044c08 indices, same numbering as
 * 016d2c_course_menu's (currently unexported) lesson-mode SEQ_LESSON_* range
 * 18-31: 0x16 = TIPS, 0x17 = WARNING, 0x18 = CHOOSE, 0x19 = SCORE_RECORD,
 * 0x1a = FINAL_DAY, 0x1b = PERFECT, 0x1c = GOOD, 0x1d = PASS,
 * 0x1e = FAIL_MINOR, 0x1f = FAIL_MAJOR. */
STATIC void buildDialogQueue_8c01e992(void)
{
    int i = 0;

    /* var_8c1bb8b8/bc track whether this state0 slot was already queued this
     * lesson attempt; only the "already queued, still pending" combo skips
     * state0 and falls through to the pass/fail states below. Every other
     * outcome queues 1-3 entries and returns without reaching them. */
    if (var_gameMode_8c1bb8fc == 1 || var_8c1bb8b8 == 0 || var_8c1bb8bc == 0) {
        if (var_gameMode_8c1bb8fc != 1 && var_8c1bb8b8 == 0 && var_8c1bb8bc != 0) {
            if (var_progress_8c1ba1cc.days_0x00 < 0x1e) {
                var_8c226414[i++] = 0x16;
                if (var_gameMode_8c1bb8fc == 0) var_8c226414[i++] = 0x17;
            } else {
                var_8c226414[i++] = 0x1a;
            }

            var_8c226414[i++] = 0x18;
            var_8c1bb8b8 = 1;
        } else {
            var_8c226414[i++] = 0x18;
        }

        var_8c226414[i] = -1;
        return;
    }

    if (var_8c1bb8f4 == 0) {
        var_8c226414[i++] = 0x1b;
    } else {
        var_8c226414[i++] = (unsigned char)init_8c045208[var_8c1bb8ec];

        if (var_8c1bb8dc == 0)
            var_8c226414[i++] = (var_8c1bb8f4 == 1) ? 0x1e : 0x1f;
        else
            var_8c226414[i++] = (var_8c1bb8f4 == 1) ? 0x1c : 0x1d;
    }

    if (var_award_8c1bb8f8 != 0) var_8c226414[i++] = 0x19;

    var_8c226414[i++] = 0x18;
    var_8c226414[i] = -1;
}

/* Draws value's decimal digits right-to-left as sprite widgets 0xc-0x15
 * (digit 0-9), starting at x=362 and stepping left by 18 per digit. */
STATIC void drawDigits_8c01ead8(int value, float y)
{
    float x = 362.0f;

    do {
        TxtDrawSprite_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
            value % 10 + 0xc,
            x, y + 6.0f, -4.3f
        );

        value /= 10;
        x -= 18.0f;
    } while (value != 0);
}

/* Eases field_0x44 (scroll position) toward selected_0x38, clamped so it
 * never scrolls more than 6 rows ahead of the selection. */
STATIC void scrollTowardSelection_8c01ebc8(void)
{
    if (var_menuState_8c1bc7a8.field_0x44 < var_menuState_8c1bc7a8.selected_0x38 - 4) {
        var_menuState_8c1bc7a8.field_0x44 = var_menuState_8c1bc7a8.selected_0x38 - 4;
        if (var_menuState_8c1bc7a8.field_0x44 > 6)
            var_menuState_8c1bc7a8.field_0x44 = 6;
    } else if (var_menuState_8c1bc7a8.selected_0x38 < var_menuState_8c1bc7a8.field_0x44) {
        var_menuState_8c1bc7a8.field_0x44 = var_menuState_8c1bc7a8.selected_0x38;
    }
}

/* Main in-lesson gameplay task action, not yet decompiled; only ever
 * referenced by address (as a TaskSetAction_8c014b3e/TaskPush_8c014ae8
 * target), never exported by the archived asm. */
STATIC void FUN_8c01ebf2(Task *task, void *state)
{
    int row, i, spriteId, modeIcon;
    float y;

    switch (var_menuState_8c1bc7a8.state_0x18) {
        case LESSON_STATE_INIT:
            if (RouteLoadIsPvmReady_8c01432a()) return;

            AsqFreeQueues_8c011f7e();
            var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_DIALOG_FADE_IN;
            FUN_8c010d8a();
            SndProc_8c010cd6(0, 0xd);
            push_fadein_8c022a9c(10);
            return;

        case LESSON_STATE_DIALOG_FADE_IN:
            if (var_isFading_8c226568 == 0) {
                CourseMenuPushDialogTask_8c0170c6(var_8c226414[0], 0);
                var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_DIALOG_QUEUE;
            }
            break;

        case LESSON_STATE_DIALOG_QUEUE:
            if (var_dialogSequenceIsActive_8c225fb4 == 0) {
                task->field_0x08++;
                if (var_8c226414[task->field_0x08] == -1) {
                    var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_MENU;
                    swapMessageBoxFor_8c02aefc(const_8c038984);
                } else {
                    CourseMenuPushDialogTask_8c0170c6(var_8c226414[task->field_0x08], 0);
                }
            }
            break;

        case LESSON_STATE_MENU:
            if ((var_peripherals_8c1ba35c[0].press & 4) != 0) {
                var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_CANCEL_FADE_OUT;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                push_fadeout_8c022b60(10);
            }
            else if ((var_peripherals_8c1ba35c[0].press & 2) != 0) {
                var_menuState_8c1bc7a8.field_0x1c = var_menuState_8c1bc7a8.state_0x18;
                var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_QUIT_PROMPT;
                var_menuState_8c1bc7a8.field_0x3c = 0;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
                var_menuTextboxCharLimit_8c225fb8 = swapMessageBoxFor_8c02aefc(const_8c038988);
            }
            else if ((var_peripherals_8c1ba35c[0].press & 0x10) != 0) {
                if (var_menuState_8c1bc7a8.selected_0x38 != 0) {
                    var_menuState_8c1bc7a8.selected_0x38--;
                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                    swapMessageBoxFor_8c02aefc(const_8c038984);
                }
            }
            else if ((var_peripherals_8c1ba35c[0].press & 0x20) != 0) {
                var_menuState_8c1bc7a8.selected_0x38++;
                swapMessageBoxFor_8c02aefc(const_8c038984);
                if (var_menuState_8c1bc7a8.selected_0x38 > 10) {
                    var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_MENU_LOCKED;
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            }
            scrollTowardSelection_8c01ebc8();
            break;

        case LESSON_STATE_MENU_LOCKED:
            if ((var_peripherals_8c1ba35c[0].press & 4) != 0) {
                var_menuState_8c1bc7a8.field_0x1c = var_menuState_8c1bc7a8.state_0x18;
                var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_QUIT_PROMPT;
                var_menuState_8c1bc7a8.field_0x3c = 0;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                var_menuTextboxCharLimit_8c225fb8 = swapMessageBoxFor_8c02aefc(const_8c038988);
            }
            else if ((var_peripherals_8c1ba35c[0].press & 0x10) != 0) {
                var_menuState_8c1bc7a8.selected_0x38--;
                var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_MENU;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            }
            break;

        case LESSON_STATE_QUIT_PROMPT: {
            int result = PromptHandleBinary_8c016caa(&var_menuState_8c1bc7a8.field_0x3c);
            if (result == 1) {
                var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_LOADING_FADE_OUT;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                SndStartAdxFadeOut_8c010bae(0);
                SndStartAdxFadeOut_8c010bae(1);
                push_fadeout_8c022b60(10);
            }
            else if (result == 2) {
                var_menuState_8c1bc7a8.state_0x18 = var_menuState_8c1bc7a8.field_0x1c;
                swapMessageBoxFor_8c02aefc(const_8c038984);
                var_menuTextboxCharLimit_8c225fb8 = 0;
            }

            /* Yes/No prompt indicator sprite, shared by every state that
             * reaches here without an early return (states 5-7). */
            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8,
                var_menuState_8c1bc7a8.field_0x3c + 2,
                228.0f, 320.0f, -5.0f
            );
            break;
        }

        case LESSON_STATE_CANCEL_FADE_OUT:
            if (var_isFading_8c226568 == 0) {
                var_8c22640c = var_menuState_8c1bc7a8.selected_0x38;
                var_8c1bb8bc = 0;
                practiceCancelReturn_8c01e920(task);
                return;
            }
            break;

        case LESSON_STATE_LOADING_FADE_OUT:
            if (var_isFading_8c226568 == 0) {
                if (init_8c03bd80 != 0) return;

                if (var_gameMode_8c1bb8fc == 0) {
                    if (var_8c22642c == 0) {
                        var_8c1bb8b8 = 0;
                    } else {
                        var_progress_8c1ba1cc.days_0x00++;
                        var_progress_8c1ba1cc.exp_0x90 += 0x32;
                        if (var_progress_8c1ba1cc.exp_0x90 > 99999) {
                            var_progress_8c1ba1cc.exp_0x90 = 99999;
                        }
                        var_8c1bb8b8 = 1;
                    }
                    var_8c1bb8bc = 1;
                }

                var_menuState_8c1bc7a8.field_0x3c = 0;
                var_menuState_8c1bc7a8.field_0x40 = 0;
                /* Offset 0x24, not field_0x44 -- Ghidra conflated the two
                 * like the earlier x/y mixups; this union slot is otherwise
                 * unused by this screen. */
                var_menuState_8c1bc7a8.pos.title.flagY_0x24 = 0.0f;
                FUN_8c016182();

                if (var_progress_8c1ba1cc.days_0x00 > 0x1e && var_gameMode_8c1bb8fc != 1) {
                    CourseMenuBuildCourseUnlockList_8c0172dc();
                    CourseMenuApplyUnlocks_8c0173e6();
                    FUN_8c01f954();
                    return;
                }

                CourseMenuReturn_8c017ef2();
                return;
            }

            /* Still fading out: shares LESSON_STATE_QUIT_PROMPT's Yes/No
             * prompt indicator draw (every non-early-return path through
             * states 5/7 falls into it). */
            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8,
                var_menuState_8c1bc7a8.field_0x3c + 2,
                228.0f, 320.0f, -5.0f
            );
            break;
    }

    /* Shared drawing tail, reached by every state that doesn't return/goto
     * out early above. */
    if (var_gameMode_8c1bb8fc == 0) {
        CourseMenuDrawDateAndExp_8c016ee6();
    }

    y = 108.0f;
    row = var_menuState_8c1bc7a8.field_0x44;
    for (i = 0; i < 5; i++) {
        if (row == var_menuState_8c1bc7a8.selected_0x38 && var_menuState_8c1bc7a8.state_0x18 != 4) {
            spriteId = row + 0x34;
        } else {
            spriteId = row + 1;
        }
        TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, spriteId, 48.0f, y, -4.5f);
        drawDigits_8c01ead8(var_progress_8c1ba1cc.field_0x98[row], y);
        row++;
        y += 33.0f;
    }

    if (var_menuState_8c1bc7a8.selected_0x38 > 10) {
        TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0x1b, 0.0f, 0.0f, -2.0f);
    }
    if (var_menuState_8c1bc7a8.field_0x44 > 0) {
        TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0x1a, 0.0f, 0.0f, -2.0f);
    }
    if (var_menuState_8c1bc7a8.field_0x44 < 6) {
        TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0x19, 0.0f, 0.0f, -2.0f);
    }

    if (menuTextboxText_8c02af1c(var_menuTextboxCharLimit_8c225fb8)) {
        TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8, 1, 0.0f, 0.0f, -5.0f);
    }

    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_menuState_8c1bc7a8.instructorSprite_0x60 + 0x32, 0.0f, 0.0f, -6.0f);

    if (var_gameMode_8c1bb8fc == 0) {
        modeIcon = 0;
    } else {
        modeIcon = 0x40;
    }
    TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, modeIcon, 0.0f, 0.0f, -3.0f);

    TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8, 0, 0.0f, 0.0f, -8.0f);
}

/* Course-button onSelect (init_courseMenuButtons_8c04442c): starts a fresh
 * lesson attempt from the course menu. */
void PracticeMenuLessonStart_8c01f114(Task *task)
{
    var_playMode_8c1bb8d0 = PLAY_MODE_PRACTICE;
    TaskSetAction_8c014b3e(task, FUN_8c01ebf2);
    var_menuState_8c1bc7a8.field_0x44 = 0;
    scrollTowardSelection_8c01ebc8();
    var_8c22642c = 0;
    *(Sint8 *)&var_award_8c1bb8f8 = 0;
    buildDialogQueue_8c01e992();
    task->field_0x08 = 0;
    var_menuState_8c1bc7a8.instructorSprite_0x60 =
        init_dialogSequences_8c044c08[var_8c226414[0]]->instructorSpriteNo_0x04;
    njSetBackColor(0, 0, 0);
    njGarbageTexture(var_tex_8c157af8, 0xc00);

    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();

    if (CourseMenuRequestSysResgrp_8c018568(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, &init_8c044274)) {
        RouteLoadSetPvmReady_8c014330();
        AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteLoadResetPvmReady_8c014322);
        var_menuState_8c1bc7a8.state_0x18 = STATE_INIT;
        return;
    }

    AsqFreeQueues_8c011f7e();
    var_menuState_8c1bc7a8.state_0x18 = STATE_DESCRIPTION_FADE_IN;
    push_fadein_8c022a9c(10);
}

/* Called from the pause menu's practice-mode retire confirm: keeps the
 * best-score record if this attempt improved it, then restarts the lesson
 * task fresh (as opposed to PracticeMenuLessonStart_8c01f114's course-menu
 * entry, this always jumps straight back into the drive). */
void PracticeMenuLessonRetry_8c01f21c(void)
{
    Task *created_task;
    void *created_state;

    var_menuState_8c1bc7a8.field_0x44 = 0;
    scrollTowardSelection_8c01ebc8();
    *(Sint8 *)&var_award_8c1bb8f8 = 0;

    if (var_8c1bb8bc == 0) {
        var_8c226414[0] = 0x18;
        var_8c226414[1] = -1;
    } else {
        if (var_8c1bb8dc != 0 && var_progress_8c1ba1cc.field_0x98[var_8c22640c] < var_8c2285c4[3]) {
            var_progress_8c1ba1cc.field_0x98[var_8c22640c] = var_8c2285c4[3];
            *(Sint8 *)&var_award_8c1bb8f8 = 1;
        }

        var_8c22642c++;
        buildDialogQueue_8c01e992();
    }

    var_playMode_8c1bb8d0 = PLAY_MODE_PRACTICE;
    InputPushTask_8c0128cc(0);
    TaskPush_8c014ae8(var_tasks_8c1ba3c8, GameTask_8c012f44, &created_task, &created_state, 0);
    TaskPush_8c014ae8(var_tasks_8c1ba3c8, FUN_8c01ebf2, &created_task, &created_state, 0);

    var_menuState_8c1bc7a8.state_0x18 = STATE_INIT;
    var_menuState_8c1bc7a8.instructorSprite_0x60 =
        init_dialogSequences_8c044c08[var_8c226414[0]]->instructorSpriteNo_0x04;
    created_task->field_0x08 = 0;

    njGarbageTexture(var_tex_8c157af8, 0xc00);
    FUN_8c02ae3e(0x20, 0x180, -2.0f, 0x240, 0x40, 0, 0, -1);
    swapMessageBoxFor_8c02aefc(const_8c038984);

    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();
    var_currentSysResGroupInfo_8c225fb0 = (void *) -1;
    CourseMenuRequestSysResgrp_8c018568(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, &init_8c044274);
    CourseMenuRequestCommonResources_8c01852c();
    RouteLoadSetPvmReady_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteLoadResetPvmReady_8c014322);
}
