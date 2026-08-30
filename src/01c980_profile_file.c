/* @unit ProfileFile */
#include <shinobi.h>
#include "01c980_profile_file.h"
#include "015ab8_title.h"
#include "sectionB.h"
#include "02af78_event.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "016d2c_course_menu.h"
#include "011120_asset_queues.h"
#include "013ae8_route_load.h"
#include "022464_fade.h"
#include "0100bc_sound.h"
#include "012f44_game.h"
#include "016c58_prompt.h"
#include "01614c_debug_menu.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#define PROFILE_COUNT 55

#define ROW_LENGTH 10
#define COL_WIDTH  0x2d
#define GRID_X     0x60
#define ROW_HEIGHT 0x30
#define GRID_Y     0x80

#define CHECKLIST_Y         267.0f
#define CHECKLIST_COL_STEP  38.0f
#define CHECKLIST_ROW_STEP  34.0f
#define CHECKLIST_WRAP_X    515.0f

/* =================
 * Type Declarations
 * =================
 */

enum STATE {
    STATE_INIT,               /* 0 */
    STATE_GRID_FADE_IN,       /* 1 */
    STATE_GRID_IDLE,          /* 2 */
    STATE_GRID_ANIMATING,     /* 3 */
    STATE_CONFIRM_FADE_OUT,   /* 4 */
    STATE_PAGE_LOAD,          /* 5 */
    STATE_PAGE_FADE_IN,       /* 6 */
    STATE_PAGE_VIEW,          /* 7 */
    STATE_PAGE_EXIT_FADE_OUT, /* 8 */
    STATE_PAGE_ADVANCE_DELAY, /* 9 */
    STATE_GRID_FADE_OUT,      /* 10 */
    STATE_EXIT_TO_COURSE_MENU /* 11 */
};

/* selected_0x38: the episode-checklist page's 5-option prompt (drawn as
 * sprites 5-8 plus the back arrow, sprite 11). PAGE_OPTION_JUMP_PREV/NEXT
 * jump to the previous/next unlocked slot; PREV/NEXT just step one slot. */
enum PAGE_OPTION {
    PAGE_OPTION_JUMP_PREV, /* 0 */
    PAGE_OPTION_PREV,      /* 1 */
    PAGE_OPTION_NEXT,      /* 2 */
    PAGE_OPTION_JUMP_NEXT, /* 3 */
    PAGE_OPTION_EXIT       /* 4 */
};

/* ====================
 * Initialized Globals
 * ====================
 */

/* One entry per PROFILE FILE grid slot; each list is a 0xff-terminated
 * set of progress-flag ids (see 02af78_event.c's ACTION_SET_PROGRESS) --
 * any set flag unlocks that slot. Declared in original physical/address
 * order so layout matches; init_profileUnlockFlags_8c044ffc below indexes
 * them by grid slot. */
STATIC Uint8 init_8c044ea0[] = {
    0x32, 0x33, 0x7c, 0x34, 0x7d, 0x35, 0x7e, 0x3b,
    0xff,
};
STATIC Uint8 init_8c044ea9[] = {
    0x32, 0x33, 0x34, 0x35, 0x3b, 0xff,
};
STATIC Uint8 init_8c044eaf[] = {
    0x36, 0x37, 0x38, 0x3a, 0x3b, 0x8e, 0x8f, 0xff,
};
STATIC Uint8 init_8c044eb7[] = {
    0x39, 0x36, 0x37, 0x38, 0xff,
};
STATIC Uint8 init_8c044ebc[] = {
    0x3c, 0x3d, 0x3e, 0xff,
};
STATIC Uint8 init_8c044ec0[] = {
    0x3c, 0x3d, 0x3e, 0xff,
};
STATIC Uint8 init_8c044ec4[] = {
    0x3c, 0x3d, 0x3e, 0xff,
};
STATIC Uint8 init_8c044ec8[] = {
    0x3f, 0x71, 0x5b, 0x82, 0x83, 0x5c, 0x45, 0x84,
    0x80, 0x91, 0x81, 0x28, 0x8d, 0x10, 0x11, 0x5e,
    0x27, 0xff,
};
STATIC Uint8 init_8c044eda[] = {
    0x40, 0x41, 0x73, 0x85, 0x12, 0x42, 0x4d, 0x74,
    0x13, 0x1e, 0x1c, 0x29, 0x43, 0x4e, 0x44, 0x45,
    0x86, 0x72, 0x14, 0x20, 0x21, 0x60, 0x5c, 0x4f,
    0x2a, 0x87, 0x15, 0x1f, 0x10, 0x11, 0x61, 0x5e,
    0x92, 0x8f, 0x2b, 0x27, 0xff,
};
STATIC Uint8 init_8c044eff[] = {
    0x40, 0x41, 0x73, 0x85, 0x12, 0x42, 0x4d, 0x74,
    0x13, 0x1e, 0x1c, 0x29, 0x43, 0x4e, 0x44, 0x45,
    0x86, 0x72, 0x14, 0x20, 0x21, 0x60, 0x5c, 0x4f,
    0x2a, 0x87, 0x15, 0x1f, 0x10, 0x11, 0x61, 0x5e,
    0x92, 0x8f, 0x2b, 0x27, 0xff,
};
STATIC Uint8 init_8c044f24[] = {
    0x46, 0x47, 0x48, 0xff,
};
STATIC Uint8 init_8c044f28[] = {
    0x46, 0x47, 0x48, 0xff,
};
STATIC Uint8 init_8c044f2c[] = {
    0x46, 0x47, 0x48, 0xff,
};
STATIC Uint8 init_8c044f30[] = {
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0xff,
};
STATIC Uint8 init_8c044f37[] = {
    0x50, 0x53, 0x55, 0xff,
};
STATIC Uint8 init_8c044f3b[] = {
    0x51, 0x5b, 0x56, 0x54, 0x55, 0xff,
};
STATIC Uint8 init_8c044f41[] = {
    0x52, 0x55, 0xff,
};
STATIC Uint8 init_8c044f44[] = {
    0x58, 0x57, 0x5a, 0xff,
};
STATIC Uint8 init_8c044f48[] = {
    0x58, 0x57, 0x5a, 0xff,
};
STATIC Uint8 init_8c044f4c[] = {
    0x49, 0x4a, 0x4b, 0x4c, 0xff,
};
STATIC Uint8 init_8c044f51[] = {
    0x00, 0x01, 0x02, 0x03, 0xff,
};
STATIC Uint8 init_8c044f56[] = {
    0x00, 0x01, 0x02, 0x03, 0xff,
};
STATIC Uint8 init_8c044f5b[] = {
    0x04, 0x05, 0x06, 0xff,
};
STATIC Uint8 init_8c044f5f[] = {
    0x04, 0x05, 0x06, 0xff,
};
STATIC Uint8 init_8c044f63[] = {
    0x07, 0x08, 0x0d, 0x09, 0x0a, 0x0f, 0x24, 0xff,
};
STATIC Uint8 init_8c044f6b[] = {
    0x07, 0x08, 0x0d, 0x09, 0x0a, 0x0f, 0x24, 0xff,
};
STATIC Uint8 init_8c044f73[] = {
    0x0b, 0x0c, 0x0d, 0x0e, 0x24, 0xff,
};
STATIC Uint8 init_8c044f79[] = {
    0x0b, 0x0c, 0x0d, 0xff,
};
STATIC Uint8 init_8c044f7d[] = {
    0x16, 0x17, 0x18, 0x19, 0xff,
};
STATIC Uint8 init_8c044f82[] = {
    0x16, 0x17, 0x18, 0x19, 0xff,
};
STATIC Uint8 init_8c044f87[] = {
    0x1c, 0x25, 0x1d, 0x26, 0x27, 0xff,
};
STATIC Uint8 init_8c044f8d[] = {
    0x1c, 0x25, 0x1d, 0x26, 0xff,
};
STATIC Uint8 init_8c044f92[] = {
    0x1a, 0x22, 0x1b, 0xff,
};
STATIC Uint8 init_8c044f96[] = {
    0x1a, 0x22, 0x1b, 0xff,
};
STATIC Uint8 init_8c044f9a[] = {
    0x1a, 0x22, 0x1b, 0xff,
};
STATIC Uint8 init_8c044f9e[] = {
    0x2c, 0x22, 0xff,
};
STATIC Uint8 init_8c044fa1[] = {
    0x2c, 0x22, 0xff,
};
STATIC Uint8 init_8c044fa4[] = {
    0x64, 0x65, 0x8e, 0x66, 0x68, 0x8f, 0xff,
};
STATIC Uint8 init_8c044fab[] = {
    0x64, 0x65, 0x66, 0x68, 0xff,
};
STATIC Uint8 init_8c044fb0[] = {
    0x64, 0x65, 0x66, 0x68, 0xff,
};
STATIC Uint8 init_8c044fb5[] = {
    0x6e, 0x6f, 0x67, 0x70, 0xff,
};
STATIC Uint8 init_8c044fba[] = {
    0x6e, 0x7f, 0x83, 0x8c, 0x6f, 0x80, 0x81, 0x8d,
    0x67, 0x70, 0xff,
};
STATIC Uint8 init_8c044fc5[] = {
    0x69, 0x6a, 0x6b, 0xff,
};
STATIC Uint8 init_8c044fc9[] = {
    0x69, 0x6a, 0x6b, 0xff,
};
STATIC Uint8 init_8c044fcd[] = {
    0x6c, 0x6d, 0xff,
};
STATIC Uint8 init_8c044fd0[] = {
    0x6c, 0x6d, 0xff,
};
STATIC Uint8 init_8c044fd3[] = {
    0x6c, 0x6d, 0xff,
};
STATIC Uint8 init_8c044fd6[] = {
    0x6c, 0x6d, 0xff,
};
STATIC Uint8 init_8c044fd9[] = {
    0x75, 0x76, 0x77, 0x78, 0xff,
};
STATIC Uint8 init_8c044fde[] = {
    0x75, 0x76, 0x77, 0x78, 0xff,
};
STATIC Uint8 init_8c044fe3[] = {
    0x75, 0x76, 0x77, 0x78, 0xff,
};
STATIC Uint8 init_8c044fe8[] = {
    0x75, 0x76, 0x77, 0x78, 0xff,
};
STATIC Uint8 init_8c044fed[] = {
    0x79, 0x7a, 0x7b, 0xff,
};
STATIC Uint8 init_8c044ff1[] = {
    0x79, 0x7a, 0x7b, 0xff,
};
STATIC Uint8 init_8c044ff5[] = {
    0x88, 0x89, 0x8a, 0x8b, 0xff,
};

Uint8 *init_profileUnlockFlags_8c044ffc[PROFILE_COUNT] = {
    init_8c044ea0,
    init_8c044ea9,
    init_8c044eaf,
    init_8c044eb7,
    init_8c044ebc,
    init_8c044ec0,
    init_8c044ec4,
    init_8c044f24,
    init_8c044f28,
    init_8c044f2c,
    init_8c044f4c,
    init_8c044f30,
    init_8c044f37,
    init_8c044f3b,
    init_8c044f41,
    init_8c044f44,
    init_8c044f48,
    init_8c044f51,
    init_8c044f56,
    init_8c044f5b,
    init_8c044f5f,
    init_8c044f63,
    init_8c044f6b,
    init_8c044f73,
    init_8c044f79,
    init_8c044f7d,
    init_8c044f82,
    init_8c044f92,
    init_8c044f96,
    init_8c044f9a,
    init_8c044f87,
    init_8c044f8d,
    init_8c044f9e,
    init_8c044fa1,
    init_8c044fa4,
    init_8c044fab,
    init_8c044fb0,
    init_8c044fc5,
    init_8c044fc9,
    init_8c044fcd,
    init_8c044fd0,
    init_8c044fd3,
    init_8c044fd6,
    init_8c044fb5,
    init_8c044fba,
    init_8c044fd9,
    init_8c044fde,
    init_8c044fe3,
    init_8c044fe8,
    init_8c044fed,
    init_8c044ff1,
    init_8c044ff5,
    init_8c044eda,
    init_8c044eff,
    init_8c044ec8,
};

/* Overview page (record 0) + one page per selected grid row (records 1-6). */
ResourceGroupInfo init_8c0450d8[7] = {
    {"prof01_parts.dat", "prof01.dat", "prof01.pvm", 2},
    {"prof02_parts.dat", "prof02.dat", "prof02.pvm", 4},
    {"prof03_parts.dat", "prof03.dat", "prof03.pvm", 4},
    {"prof04_parts.dat", "prof04.dat", "prof04.pvm", 4},
    {"prof05_parts.dat", "prof05.dat", "prof05.pvm", 4},
    {"prof06_parts.dat", "prof06.dat", "prof06.pvm", 4},
    {"prof07_parts.dat", "prof07.dat", "prof07.pvm", 4},
};

/* Indexed by selected grid row (0-5). */
ResourceGroupInfo *init_8c045148[6] = {
    &init_8c0450d8[1],
    &init_8c0450d8[2],
    &init_8c0450d8[3],
    &init_8c0450d8[4],
    &init_8c0450d8[5],
    &init_8c0450d8[6],
};

/* ====================
 * Functions
 * ====================
 */

/* Refreshes the 55-slot unlock grid: a slot unlocks once any progress flag
 * in its init_profileUnlockFlags_8c044ffc list is set. Also tallies the unlocked count into
 * var_profileUnlockedCount_8c2263a4 (saved to the VMU as var_progress_8c1ba1cc.profileUnlockedCount_0x8c). */
void ProfileFileUpdateUnlocks_8c01c980(void)
{
    int i;

    var_profileUnlockedCount_8c2263a4 = 0;
    for (i = 0; i < PROFILE_COUNT; i++) {
        Uint8 *flags;

        for (flags = init_profileUnlockFlags_8c044ffc[i]; *flags != 0xff; flags++) {
            if (!EventHasProgressFlagAlt_8c02aff0(*flags))
                continue;

            var_profileUnlocked_8c2263b4[i] = 1;
            var_profileUnlockedCount_8c2263a4++;

            // Break once we see a progress flag set for this character
            break;
        }

        // If we reached the end of the list, no progress flag was set
        if (*flags == 0xff) {
            var_profileUnlocked_8c2263b4[i] = 0;
        }
    }
}

/* Draws the 55-slot unlock grid: a lock icon (sprite 2) over every slot
 * still marked locked in var_profileUnlocked_8c2263b4, laid out 10 columns
 * by 6 rows (row = slot/10, col = slot%10). Also draws the row-selection
 * highlight (sprite 3) at the menu's cursor position and a background
 * overlay (sprite 1). */
STATIC void drawUnlockGrid_8c01c9f2(void)
{
    int i;

    for (i = 0; i < PROFILE_COUNT; i++) {
        if (!var_profileUnlocked_8c2263b4[i]) {
            int row = i / ROW_LENGTH;
            int col = i % ROW_LENGTH;

            // Cover locked slot
            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                2,
                col * COL_WIDTH + GRID_X,
                row * ROW_HEIGHT + GRID_Y,
                -3.0f
            );
        }
    }

    // Draw cursor
    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        3,
        var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x,
        var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.y,
        -2.0f
    );

    // Draw background
    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        1,
        0.0f,
        0.0f,
        -4.0f
    );
}

/* Draws the selected slot's episode checklist: walks the same flag list as
 * ProfileFileUpdateUnlocks_8c01c980 for the currently selected grid slot
 * (var_menuState_8c1bc7a8's col/row scratch fields), putting a checkmark
 * (sprite 9) per flag currently set, wrapping to a new line every 11
 * columns. */
STATIC void drawEpisodeChecklist_8c01cac8(void)
{
    Uint8 *flags;
    float x = GRID_X;
    float y = CHECKLIST_Y;
    Bool anyChecked = FALSE;

    flags = init_profileUnlockFlags_8c044ffc[
        var_menuState_8c1bc7a8.field_0x40 * ROW_LENGTH
        + var_menuState_8c1bc7a8.field_0x3c
    ];

    while (*flags != 0xff) {
        if (EventHasProgressFlagAlt_8c02aff0(*flags)) {
            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                9, x, y, -2.0f
            );
            anyChecked = TRUE;
        }

        x += CHECKLIST_COL_STEP;
        flags++;
        if (x > CHECKLIST_WRAP_X) {
            x = GRID_X;
            y += CHECKLIST_ROW_STEP;
        }
    }

    if (anyChecked) {
        TxtDrawSprite_8c014f54(
            &var_resourceGroup_8c2263a8,
            var_menuState_8c1bc7a8.field_0x3c,
            0.0f, 0.0f, -3.0f
        );
    }

    if (var_menuState_8c1bc7a8.selected_0x38 < PAGE_OPTION_EXIT) {
        TxtDrawSprite_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
            var_menuState_8c1bc7a8.selected_0x38 + 5,
            0.0f, 0.0f, -3.0f
        );

        TxtDrawSprite_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
            10,
            0.0f, 0.0f, -3.0f
        );
    } else {
        TxtDrawSprite_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
            11,
            0.0f, 0.0f, -3.0f
        );
    }

    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        4,
        0.0f, 0.0f, -4.0f
    );
}

/* Drives the selected page's resource-group swap: if the selected row's
 * resgrp (init_8c045148[field_0x40]) is already the active sys resgrp
 * (previous request has completed), the load is done -- play the confirm
 * jingle and move to state 6. Otherwise kick off a fresh async load of that
 * row's resgrp and move to state 5 (loading), to be polled again next
 * frame. */
STATIC void updatePageLoad_8c01cbec(void)
{
    ResourceGroupInfo *pageResgrpInfo = init_8c045148[var_menuState_8c1bc7a8.field_0x40];

    if (var_currentSysResGroupInfo_8c225fb0 == pageResgrpInfo) {
        switch (var_menuState_8c1bc7a8.selected_0x38) {
            case PAGE_OPTION_JUMP_PREV:
            case PAGE_OPTION_JUMP_NEXT:
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 8, 0);
                break;

            case PAGE_OPTION_PREV:
            case PAGE_OPTION_NEXT:
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 7, 0);
                break;
        }

        var_menuState_8c1bc7a8.state_0x18 = STATE_PAGE_FADE_IN;
        FadePushIn_8c022a9c(10);
        return;
    }

    CourseMenuFreeResourceGroup_8c0185c4(&var_resourceGroup_8c2263a8);
    njGarbageTexture(var_tex_8c157af8, 0xc00);
    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();
    CourseMenuRequestSysResgrp_8c018568(&var_resourceGroup_8c2263a8, pageResgrpInfo);
    RouteLoadSetPvmReady_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteLoadResetPvmReady_8c014322);
    var_menuState_8c1bc7a8.state_0x18 = STATE_PAGE_LOAD;
}

/* Per-frame PROFILE FILE task: drives the 55-slot grid cursor, the drill-in
 * to a selected slot's episode-checklist page, and the fade transitions
 * between the two. */
STATIC void menuTask_8c01ccec(Task *task, void *state)
{
    int press = var_peripherals_8c1ba35c[0].press;

    switch (var_menuState_8c1bc7a8.state_0x18) {
        case STATE_INIT:
            if (RouteLoadIsPvmReady_8c01432a())
                return;
            AsqFreeQueues_8c011f7e();
            var_menuState_8c1bc7a8.state_0x18 = STATE_GRID_FADE_IN;
            FadePushIn_8c022a9c(10);
            return;

        case STATE_GRID_FADE_IN:
            if (!var_isFading_8c226568)
                var_menuState_8c1bc7a8.state_0x18 = STATE_GRID_IDLE;
            drawUnlockGrid_8c01c9f2();
            break;

        case STATE_GRID_IDLE: {
            Bool moved = FALSE;

            /* Grid is 10 cols x 5 rows, plus a 5-wide row 6 (55 slots total). */
            if (press & PDD_DGT_KU) {
                if (--var_menuState_8c1bc7a8.field_0x40 < 0) {
                    var_menuState_8c1bc7a8.field_0x40 =
                        (var_menuState_8c1bc7a8.field_0x3c < 5) ? 5 : 4;
                }
                moved = TRUE;
            } else if (press & PDD_DGT_KD) {
                ++var_menuState_8c1bc7a8.field_0x40;
                if ((var_menuState_8c1bc7a8.field_0x3c < 5 &&
                     var_menuState_8c1bc7a8.field_0x40 > 5) ||
                    (var_menuState_8c1bc7a8.field_0x3c > 4 &&
                     var_menuState_8c1bc7a8.field_0x40 > 4)) {
                    var_menuState_8c1bc7a8.field_0x40 = 0;
                }
                moved = TRUE;
            } else if (press & PDD_DGT_KL) {
                if (--var_menuState_8c1bc7a8.field_0x3c < 0) {
                    var_menuState_8c1bc7a8.field_0x3c =
                        (var_menuState_8c1bc7a8.field_0x40 < 5) ? 9 : 4;
                }
                moved = TRUE;
            } else if (press & PDD_DGT_KR) {
                ++var_menuState_8c1bc7a8.field_0x3c;
                if ((var_menuState_8c1bc7a8.field_0x40 < 5 &&
                     var_menuState_8c1bc7a8.field_0x3c > 9) ||
                    (var_menuState_8c1bc7a8.field_0x40 > 4 &&
                     var_menuState_8c1bc7a8.field_0x3c > 4)) {
                    var_menuState_8c1bc7a8.field_0x3c = 0;
                }
                moved = TRUE;
            }

            if (moved) {
                var_menuState_8c1bc7a8.state_0x18 =
                    STATE_GRID_ANIMATING;
                var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.x =
                    var_menuState_8c1bc7a8.field_0x3c * 45.0f + 96.0f;
                var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.y =
                    var_menuState_8c1bc7a8.field_0x40 * 48.0f + 128.0f;
                var_menuState_8c1bc7a8.cursorVelocity_0x30.x =
                    (var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.x
                        - var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x) / 6.0f;
                var_menuState_8c1bc7a8.cursorVelocity_0x30.y =
                    (var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.y
                        - var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.y) / 6.0f;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            } else if (press & PDD_DGT_TA) {
                var_menuState_8c1bc7a8.state_0x18 =
                    STATE_CONFIRM_FADE_OUT;
                var_menuState_8c1bc7a8.selected_0x38 = PAGE_OPTION_NEXT;
                FadePushOut_8c022b60(10);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 7, 0);
            } else if (press & PDD_DGT_TB) {
                var_menuState_8c1bc7a8.state_0x18 =
                    STATE_EXIT_TO_COURSE_MENU;
                FadePushOut_8c022b60(10);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
            }
            drawUnlockGrid_8c01c9f2();
            break;
        }

        case STATE_GRID_ANIMATING:
            if (CourseMenuInterpolateCursor_8c016d2c())
                var_menuState_8c1bc7a8.state_0x18 = STATE_GRID_IDLE;
            drawUnlockGrid_8c01c9f2();
            break;

        case STATE_CONFIRM_FADE_OUT:
            if (var_isFading_8c226568) {
                drawUnlockGrid_8c01c9f2();
                break;
            }
            updatePageLoad_8c01cbec();
            return;

        case STATE_PAGE_LOAD:
            if (RouteLoadIsPvmReady_8c01432a())
                return;
            AsqFreeQueues_8c011f7e();
            switch (var_menuState_8c1bc7a8.selected_0x38) {
                case PAGE_OPTION_JUMP_PREV:
                case PAGE_OPTION_JUMP_NEXT:
                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 8, 0);
                    break;

                case PAGE_OPTION_PREV:
                case PAGE_OPTION_NEXT:
                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 7, 0);
                    break;
            }
            var_menuState_8c1bc7a8.state_0x18 = STATE_PAGE_FADE_IN;
            FadePushIn_8c022a9c(10);
            return;

        case STATE_PAGE_FADE_IN:
            if (!var_isFading_8c226568)
                var_menuState_8c1bc7a8.state_0x18 = STATE_PAGE_VIEW;
            drawEpisodeChecklist_8c01cac8();
            break;

        case STATE_PAGE_VIEW:
            PromptHandleMultiple_8c016c58(&var_menuState_8c1bc7a8.selected_0x38, 5);

            if (press & PDD_DGT_TA) {
                if (var_menuState_8c1bc7a8.selected_0x38 < PAGE_OPTION_EXIT) {
                    var_menuState_8c1bc7a8.state_0x18 =
                        STATE_PAGE_EXIT_FADE_OUT;
                } else {
                    var_menuState_8c1bc7a8.state_0x18 =
                        STATE_GRID_FADE_OUT;
                }
                FadePushOut_8c022b60(10);
            } else if (press & PDD_DGT_TB) {
                var_menuState_8c1bc7a8.state_0x18 =
                    STATE_GRID_FADE_OUT;
                FadePushOut_8c022b60(10);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
            }
            drawEpisodeChecklist_8c01cac8();
            break;

        case STATE_PAGE_EXIT_FADE_OUT:
            if (var_isFading_8c226568) {
                drawEpisodeChecklist_8c01cac8();
                break;
            }
            var_menuState_8c1bc7a8.state_0x18 =
                STATE_PAGE_ADVANCE_DELAY;
            var_menuState_8c1bc7a8.logo_timer_0x68 = 0;
            return;

        case STATE_PAGE_ADVANCE_DELAY:
            if (++var_menuState_8c1bc7a8.logo_timer_0x68 < 4)
                return;

            switch (var_menuState_8c1bc7a8.selected_0x38) {
                case PAGE_OPTION_JUMP_PREV:
                    /* Search backward for the previous unlocked slot. */
                    if (var_profileUnlockedCount_8c2263a4 != 0) {
                        int slot = var_menuState_8c1bc7a8.field_0x40 * 10
                            + var_menuState_8c1bc7a8.field_0x3c;
                        do {
                            if (--slot < 0) slot = 0x36;
                        } while (!var_profileUnlocked_8c2263b4[slot]);
                        var_menuState_8c1bc7a8.field_0x3c = slot % 10;
                        var_menuState_8c1bc7a8.field_0x40 = slot / 10;
                    }
                    break;

                case PAGE_OPTION_PREV:
                    if (--var_menuState_8c1bc7a8.field_0x3c < 0) {
                        var_menuState_8c1bc7a8.field_0x3c = 9;
                        if (--var_menuState_8c1bc7a8.field_0x40 < 0) {
                            var_menuState_8c1bc7a8.field_0x3c = 4;
                            var_menuState_8c1bc7a8.field_0x40 = 5;
                        }
                    }
                    break;

                case PAGE_OPTION_NEXT:
                    if (var_menuState_8c1bc7a8.field_0x40 < 5) {
                        if (++var_menuState_8c1bc7a8.field_0x3c > 9) {
                            var_menuState_8c1bc7a8.field_0x3c = 0;
                            ++var_menuState_8c1bc7a8.field_0x40;
                        }
                    } else {
                        if (++var_menuState_8c1bc7a8.field_0x3c > 4) {
                            var_menuState_8c1bc7a8.field_0x3c = 0;
                            var_menuState_8c1bc7a8.field_0x40 = 0;
                        }
                    }
                    break;

                case PAGE_OPTION_JUMP_NEXT: {
                    int slot;

                    if (var_profileUnlockedCount_8c2263a4 == 0)
                        break;

                    /* Search forward for the next unlocked slot. */
                    slot = var_menuState_8c1bc7a8.field_0x40 * 10
                        + var_menuState_8c1bc7a8.field_0x3c;
                    do {
                        if (++slot > 0x36) slot = 0;
                    } while (!var_profileUnlocked_8c2263b4[slot]);
                    var_menuState_8c1bc7a8.field_0x3c = slot % 10;
                    var_menuState_8c1bc7a8.field_0x40 = slot / 10;
                    break;
                }
            }

            updatePageLoad_8c01cbec();
            return;

        case STATE_GRID_FADE_OUT:
            if (var_isFading_8c226568) {
                drawEpisodeChecklist_8c01cac8();
                break;
            }
            var_menuState_8c1bc7a8.state_0x18 = STATE_GRID_FADE_IN;
            var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x =
                var_menuState_8c1bc7a8.field_0x3c * 45.0f + 96.0f;
            var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.y =
                var_menuState_8c1bc7a8.field_0x40 * 48.0f + 128.0f;
            FadePushIn_8c022a9c(10);
            return;

        case STATE_EXIT_TO_COURSE_MENU:
            if (var_isFading_8c226568)
                break;
            /* Reset the grid cursor for the next visit. */
            var_menuState_8c1bc7a8.field_0x3c = 0;
            var_menuState_8c1bc7a8.field_0x40 = 1;
            var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x = 0.0f;
            DebugMenuFreeSessionAssets_8c016182();
            CourseMenuReturn_8c017ef2();
            return;

        default:
            break;
    }

    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        0, 0.0f, 0.0f, -5.0f
    );
    njSetBackColor(0xff000000, 0xff000000, 0xff000000);
}

/* Installs menuTask_8c01ccec on the task, resets the grid cursor
 * to slot (0,0), refreshes the unlock flags, and kicks off the async load
 * of the overview page's resource group. */
void ProfileFilePushTask_8c01d1c4(Task *task)
{
    TaskSetAction_8c014b3e(task, menuTask_8c01ccec);

    var_menuState_8c1bc7a8.state_0x18 = STATE_INIT;
    var_menuState_8c1bc7a8.field_0x3c = 0;
    var_menuState_8c1bc7a8.field_0x40 = 0;
    var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x = GRID_X;
    var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.y = GRID_Y;

    ProfileFileUpdateUnlocks_8c01c980();

    CourseMenuFreeResourceGroup_8c0185c4(&var_menuState_8c1bc7a8.resourceGroupA_0x00);
    njGarbageTexture(var_tex_8c157af8, 0xc00);
    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();
    CourseMenuRequestSysResgrp_8c018568(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, &init_8c0450d8[0]);
    RouteLoadSetPvmReady_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteLoadResetPvmReady_8c014322);
}
