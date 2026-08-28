/* @unit FileMenu */
#include <shinobi.h>
#include "018644_file_menu.h"
#include "011120_asset_queues.h"
#include "014a9c_tasks.h"
#include "014b8c_backup.h"
#include "0193c8_vm_menu.h"
#include "019e98_main_menu.h"
#include "0100bc_sound.h"
#include "014f54_text.h"
#include "028258.h"
#include "01bb48_vm_game.h"
#include "01b19c_system_menu.h"
#include "022464_fade.h"
#include "sectionB.h"
#include "serial_debug.h"
#include "strings.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#ifdef SERIAL_DEBUG
char *DEBUG_fileMenuStateNames[] = {
    "LOADING",
    "LOAD_ERROR",
    "ERROR_FADE_OUT",
    "READY",
    "CONFIRM",
    "CONFIRM_FADE_OUT",
    "MOUNTING",
    "CANCEL_FADE_OUT",
    "UNMOUNTING",
};
#endif

#define CHANGE_STATE(x) var_menuState_8c1bc7a8.state_0x18 = x; LOG_DEBUG(("[FILE_MENU] State changed: %s\n", DEBUG_fileMenuStateNames[x]))

/* =================
 * Type Declarations
 * =================
 */

enum FILE_MENU_STATE {
    FILE_MENU_STATE_LOADING = 0,
    FILE_MENU_STATE_LOAD_ERROR = 1,
    FILE_MENU_STATE_ERROR_FADE_OUT = 2,
    FILE_MENU_STATE_READY = 3,
    FILE_MENU_STATE_CONFIRM = 4,
    FILE_MENU_STATE_CONFIRM_FADE_OUT = 5,
    FILE_MENU_STATE_MOUNTING = 6,
    FILE_MENU_STATE_CANCEL_FADE_OUT = 7,
    FILE_MENU_STATE_UNMOUNTING = 8,
};

/* ====================
 * Functions
 * ====================
 */

/*
 * Multi-frame task that loads a null-terminated list of save files (task->0x18)
 * from the selected VMU. State 0 requests the next missing file via BupLoad and
 * yields; state 1 waits for the drive, analyzes the loaded image, and appends its
 * header to the growing buffer. var_8c226010 reports the outcome: 1 = all loaded,
 * 2 = error.
 */
STATIC void loadFileTask_8c018644(Task *task)
{
    char **names;

    LOG_TRACE(("[FILE_MENU] loadFileTask_8c018644\n"));

    if (task->field_0x08 == 0) {
        for (names = (char **)task->queuedItem_0x18; **names != '\0'; names++) {
            int err = buIsExistFile(var_selectedVm_8c1ba34c, *names);
            if (err == 0) {
                LOG_DEBUG(("[FILE_MENU] loadFileTask_8c018644: requesting load for \"%s\"\n", *names));
                BupLoad_8c014bc6(var_selectedVm_8c1ba34c, *names, var_8c225fe0);
                var_8c225fe4[var_8c22600c] = (int)task->field_0x0c;
                var_8c22600c++;
                task->queuedItem_0x18 = names + 1;
                task->field_0x08 = 1;
                task->field_0x0c = (void *)((int)task->field_0x0c + 1);
                return;
            }
            if (err != -0xfb) {
                LOG_WARN(("[FILE_MENU] loadFileTask_8c018644: enumeration failed for \"%s\" (err=%d)\n", *names, err));
                TaskFree_8c014b66(task);
                var_8c226010 = 2;
                return;
            }
            task->field_0x0c = (void *)((int)task->field_0x0c + 1);
        }
        LOG_DEBUG(("[FILE_MENU] loadFileTask_8c018644: all files loaded (%d)\n", var_8c22600c));
        TaskFree_8c014b66(task);
        var_8c226010 = 1;
    } else if (task->field_0x08 == 1 && buStat(var_selectedVm_8c1ba34c) == 0) {
        if (buGetLastError(var_selectedVm_8c1ba34c) != 0) {
            LOG_WARN(("[FILE_MENU] loadFileTask_8c018644: load failed\n"));
            TaskFree_8c014b66(task);
            var_8c226010 = 2;
            return;
        }
        var_backupFileImageBuf_8c1ba348 = syMalloc(0xe8);
        buAnalyzeBackupFileImage(&var_8c1ba2e4, var_8c225fe0);
        njMemCopy(var_backupFileImageBuf_8c1ba348, var_8c1ba33c, 0xe8);
        njMemCopy(var_8c225fe0, var_backupFileImageBuf_8c1ba348, 0xe8);
        syFree(var_backupFileImageBuf_8c1ba348);
        var_backupFileImageBuf_8c1ba348 = (void *)-1;
        var_8c225fe0 = (char *)var_8c225fe0 + 0x600;
        task->field_0x08 = 0;
    }
}

/*
 * Kicks off a VMU load: queues FileMenuTask over the full save-file list and
 * allocates the 0x3c00 staging buffer the task streams images into.
 */
STATIC void startVmLoad_8c018784(void)
{
    Task *task;
    void *state;

    LOG_DEBUG(("[FILE_MENU] startVmLoad_8c018784: starting VMU load\n"));

    TaskPush_8c014ae8(var_tasks_8c1ba3c8, (void *)loadFileTask_8c018644, &task, &state, 0);
    var_vmBusy_8c157a7c = 1;
    task->field_0x08 = 0;
    task->field_0x0c = 0;
    task->queuedItem_0x18 = init_saveNames_8c044d50;
    var_8c1ba2e0 = syMalloc(0x3c00);
    var_8c225fe0 = var_8c1ba2e0;
    var_8c226010 = 0;
}

/* Releases the VM-load staging buffers; -1 marks a slot as already freed. */
void FileMenuFreeBuffers_8c0187d0(void)
{
    LOG_DEBUG(("[FILE_MENU] FileMenuFreeBuffers_8c0187d0: freeing buffers\n"));

    if (var_8c1ba2e0 != (void *)-1) {
        syFree(var_8c1ba2e0);
        var_8c1ba2e0 = (void *)-1;
    }
    if (var_vmuIconFileBuf_8c1ba344 != (void *)-1) {
        syFree(var_vmuIconFileBuf_8c1ba344);
        var_vmuIconFileBuf_8c1ba344 = (void *)-1;
    }
}

/*
 * Sanity-checks a loaded save image: day 1..30, the 9 course records (0x44,
 * stride 8) with three <=1 flags and two <=3 ranks each, and EXP capped at
 * 99999. Returns 1 if plausible, 0 otherwise.
 */
int FileMenuIsSaveValid_8c018804(int *save)
{
    unsigned char *rec;

    if (0 < save[0] && save[0] < 0x1f) {
        for (rec = (unsigned char *)(save + 0x11); rec < (unsigned char *)(save + 0x23); rec += 8) {
            if (rec[0] > 1) return 0;
            if (rec[1] > 1) return 0;
            if (rec[2] > 1) return 0;
            if (rec[3] > 3) return 0;
            if (rec[4] > 3) return 0;
        }
        if (save[0x24] <= 99999) {
            return 1;
        }
    }
    return 0;
}

/*
 * Restores the progress control-option defaults: 0xc4 (gated <2 in route load),
 * 0xc5 (input-map selection), 0xc6, and the first two 0xc7 bytes.
 */
void FileMenuResetControlDefaults_8c018862(void)
{
    LOG_DEBUG(("[FILE_MENU] FileMenuResetControlDefaults_8c018862\n"));

    var_progress_8c1ba1cc.field_0xc4 = 1;
    var_progress_8c1ba1cc.field_0xc5 = 0;
    var_progress_8c1ba1cc.field_0xc6 = 2;
    var_progress_8c1ba1cc.field_0xc7[0] = 0;
    var_progress_8c1ba1cc.field_0xc7[1] = 0;
}

/*
 * Restores the progress view/input defaults: the 0xcc-0xcf asset-selection
 * flags (read in 011120) and the 0xd0/0xd1 input deadzone thresholds (0x10).
 */
void FileMenuResetViewDefaults_8c0188bc(void)
{
    LOG_DEBUG(("[FILE_MENU] FileMenuResetViewDefaults_8c0188bc\n"));

    var_progress_8c1ba1cc.field_0xc7[5] = 0;
    var_progress_8c1ba1cc.field_0xc7[6] = 0;
    var_progress_8c1ba1cc.field_0xc7[7] = 0;
    var_progress_8c1ba1cc.field_0xc7[8] = 0;
    var_progress_8c1ba1cc.field_0xd0 = 0x10;
    var_progress_8c1ba1cc.field_0xd1 = 0x10;
}

/*
 * Restores the sound defaults: caches the console's sound mode (clamped to
 * non-negative) and resets the 0xd4-0xd6 progress audio bytes to 9/5/9.
 */
void FileMenuResetSoundDefaults_8c0188dc(void)
{
    LOG_DEBUG(("[FILE_MENU] FileMenuResetSoundDefaults_8c0188dc\n"));

    var_soundMode_8c226070 = SndGetSoundMode_8c010924();
    if (var_soundMode_8c226070 < 0) {
        var_soundMode_8c226070 = 0;
    }
    var_progress_8c1ba1cc.field_0xd4 = 9;
    var_progress_8c1ba1cc.field_0xd5 = 5;
    var_progress_8c1ba1cc.field_0xd6 = 9;
}

/*
 * Resets player progress to the new-game base state: day 1, cleared unlock
 * bitsets and per-course flags, courses 0 and 6 unlocked, EXP 0.
 */
void FileMenuResetProgress_8c01890a(void)
{
    int i;

    LOG_DEBUG(("[FILE_MENU] FileMenuResetProgress_8c01890a\n"));

    var_progress_8c1ba1cc.days_0x00 = 1;
    for (i = 0; i < 5; i++) {
        var_progress_8c1ba1cc.field_0x04[i] = 0;
    }
    for (i = 0; i < 9; i++) {
        var_progress_8c1ba1cc.courses_0x44[i].unlocked_0x00 = 0;
        var_progress_8c1ba1cc.courses_0x44[i].field_0x02 = 0;
        var_progress_8c1ba1cc.courses_0x44[i].storySpriteNo_0x03 = 0;
    }
    var_progress_8c1ba1cc.courses_0x44[0].unlocked_0x00 = 1;
    var_progress_8c1ba1cc.courses_0x44[6].unlocked_0x00 = 1;
    var_progress_8c1ba1cc.exp_0x90 = 0;
    var_8c1bb8b8 = 1;
    var_8c1bb8bc = 0;
}

/*
 * Full new-game reset: base progress (FileMenuResetProgress) plus the second
 * unlock bitset, letters, per-course new/free-run-sprite flags (courses 0 and
 * 6 marked new), and the 0x8c-0xc0 scratch fields.
 */
void FileMenuResetNewGame_8c01895e(void)
{
    int i;

    LOG_DEBUG(("[FILE_MENU] FileMenuResetNewGame_8c01895e\n"));

    FileMenuResetProgress_8c01890a();
    for (i = 0; i < 5; i++) {
        var_progress_8c1ba1cc.field_0x18[i] = 0;
    }
    for (i = 0; i < 6; i++) {
        var_progress_8c1ba1cc.letters_0x2c[i] = 0;
    }
    for (i = 0; i < 9; i++) {
        var_progress_8c1ba1cc.courses_0x44[i].new_0x01 = 0;
        var_progress_8c1ba1cc.courses_0x44[i].freeRunSpriteNo_0x04 = 0;
    }
    var_progress_8c1ba1cc.courses_0x44[0].new_0x01 = 1;
    var_progress_8c1ba1cc.courses_0x44[6].new_0x01 = 1;
    var_progress_8c1ba1cc.field_0x8c = 0;
    var_progress_8c1ba1cc.field_0x94 = 0;
    for (i = 0; i < 11; i++) {
        var_progress_8c1ba1cc.field_0x98[i] = 0;
    }
}

/* Restores all option defaults: control, view, and sound. */
void FileMenuResetOptionDefaults_8c0189d2(void)
{
    LOG_DEBUG(("[FILE_MENU] FileMenuResetOptionDefaults_8c0189d2\n"));

    FileMenuResetControlDefaults_8c018862();
    FileMenuResetViewDefaults_8c0188bc();
    FileMenuResetSoundDefaults_8c0188dc();
}

/* Pushes the saved 0xd4-0xd6 audio settings to the sound engine. */
void FileMenuApplySoundSettings_8c0189fc(void)
{
    LOG_DEBUG(("[FILE_MENU] FileMenuApplySoundSettings_8c0189fc\n"));

    SndSetAdxVol_8c010972(var_progress_8c1ba1cc.field_0xd4, 0);
    SndSetMidiVolAndInitStruct_8c0109f4(var_progress_8c1ba1cc.field_0xd5);
    SndSetAdxVol_8c010972(var_progress_8c1ba1cc.field_0xd6, 1);
}

/*
 * Builds the FILE SELECT card list: prepends a NEW FILE card (0xa) when the
 * selected VMU can take one (status 4, or status 6 with room), appends the
 * var_8c22600c loaded saves, and pads the remaining 12 slots with 0xb (empty).
 * var_8c226014 ends as the total visible card count.
 */
STATIC void buildFileList_8c018a22(void)
{
    int dst;
    int i;
    int *p;

    var_8c226014 = 0;
    if (var_vmuStatus_8c226048[var_selectedVm_8c1ba34c] == VMU_STATUS_SAVING_POSSIBLE ||
        (var_vmuStatus_8c226048[var_selectedVm_8c1ba34c] == VMU_STATUS_SAVE_EXISTS_NO_SPACE &&
         var_8c22600c < 10)) {
        var_8c226018[0] = 10;
        var_8c226014 = 1;
    }
    dst = var_8c226014;
    for (i = 0; i < var_8c22600c; i++) {
        var_8c226018[dst] = var_8c225fe4[i];
        dst++;
    }
    for (p = &var_8c226018[dst]; p < &var_8c226018[12]; p++) {
        *p = 0xb;
    }
    var_8c226014 += var_8c22600c;

    LOG_DEBUG(("[FILE_MENU] buildFileList_8c018a22: built file list (%d cards)\n", var_8c226014));
}

/*
 * Draws a non-negative integer as digit sprites (glyph 15 + digit), right to
 * left from (x, y), stepping 10px left per digit. Priority -4.0.
 */
STATIC void drawNumber_8c018aa2(int value, float x, float y)
{
    do {
        TxtDrawSprite_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupA_0x00,
            15 + value % 10,
            x,
            y,
            -4.0
        );
        x -= 10.0;
    } while (value /= 10);
}

/*
 * Renders one FILE SELECT card at column x. Kind 0xa draws the NEW FILE card;
 * otherwise the loaded save at var_8c225fe0 is drawn: date (day digits + weekday
 * icon), the 3x3 course-icon grid, event/EXP counts, then rank markers -- rows of
 * up to five, consuming the rank-3, then rank-2, then rank-1 course tallies. The
 * card advances var_8c225fe0 by one 0x600 save image.
 */
STATIC void drawFileCard_8c018b4c(int kind, float x)
{
    PlayerProgress *save;
    int cnt1;
    int cnt2;
    int cnt3;
    int i;
    float y;
    float xoff;

    if (kind == 0xa) {
        TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0x13, x, 0.0, -4.0);
        return;
    }

    save = var_8c225fe0;

    drawNumber_8c018aa2(save->days_0x00, x + (save->days_0x00 >= 10 ? 63.0 : 52.0), 122.0);
    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00,
        6 + (save->days_0x00 + 1) % 7,
        x + 84.0, 122.0, -4.0);

    cnt1 = 0;
    cnt2 = 0;
    cnt3 = 0;
    for (i = 0; i < 9; i++) {
        if (save->courses_0x44[i].unlocked_0x00 != 0) {
            TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0x26 + i, x, 0.0, -4.0);
        }
        switch (save->courses_0x44[i].storySpriteNo_0x03) {
        case 3: cnt3++; break;
        case 2: cnt2++; break;
        case 1: cnt1++; break;
        }
    }

    drawNumber_8c018aa2(save->field_0x8c, x + 77.0, 244.0);
    drawNumber_8c018aa2(save->exp_0x90, x + 77.0, 264.0);

    y = 279.0;
    while (y <= 303.0) {
        xoff = 13.0;
        while (1) {
            int glyph;
            if (cnt3-- != 0) {
                glyph = 0x23;
            } else if (cnt2-- != 0) {
                glyph = 0x24;
            } else if (cnt1-- != 0) {
                glyph = 0x25;
            } else {
                y = 65536.0;
                break;
            }
            TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, glyph, x + xoff, y, -4.5);
            xoff += 24.0;
            if (xoff > 109.0) {
                break;
            }
        }
        y += 24.0;
    }

    TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0x12, x, 0.0, -4.5);
    var_8c225fe0 = (char *)var_8c225fe0 + 0x600;
}

/*
 * Draws the FILE SELECT screen: positions var_8c225fe0 at the first visible save
 * image (page var_8c226018[0]==0xa NEW-FILE column shifts the save index by one),
 * lays out up to a page of cards left to right (182px apart, x 55..419) via
 * drawFileCard, then the selection cursor (0x2f at the highlighted column), the
 * BACK/NEXT arrows (enabled sprites when a previous/next page exists), and the
 * static frame sprites.
 */
STATIC void drawFileSelect_8c018d46(void)
{
    int i;
    float x;
    void *dst;

    if (var_8c226018[0] == 0xa) {
        dst = var_8c1ba2e0;
        if (var_menuState_8c1bc7a8.field_0x3c != 0) {
            dst = (char *)var_8c1ba2e0 + (var_menuState_8c1bc7a8.field_0x3c - 1) * 0x600;
        }
    } else {
        dst = (char *)var_8c1ba2e0 + var_menuState_8c1bc7a8.field_0x3c * 0x600;
    }
    var_8c225fe0 = dst;

    x = 55.0;
    for (i = var_menuState_8c1bc7a8.field_0x3c;
         x <= 419.0 && var_8c226018[i] != 0xb;
         i++) {
        drawFileCard_8c018b4c(var_8c226018[i], x);
        x += 182.0;
    }

    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0x2f,
        182.0 * (float)var_menuState_8c1bc7a8.selected_0x38 + 45.0, 0.0, -3.0);

    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_menuState_8c1bc7a8.field_0x3c != 0 ? 0x16 : 0x15,
        0.0, 0.0, -3.0);

    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_8c226018[var_menuState_8c1bc7a8.field_0x3c + 3] == 0xb ? 0x17 : 0x18,
        0.0, 0.0, -3.0);

    TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0x14, 0.0, 0.0, -4.0);
    TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 1, 0.0, 0.0, -4.3);
    TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 0, 0.0, 0.0, -5.0);
}

/*
 * FILE SELECT screen task: the state machine driving card selection, the
 * new-file / load confirmation prompts, and the transition back to VM SELECT or
 * the main menu. Dispatches on menuState.state_0x18; most states redraw via
 * drawFileSelect on exit.
 */
STATIC void fileSelectTask_8c018e7e(Task *task)
{
    unsigned int press = var_peripherals_8c1ba35c[0].press;
    int i;

    switch (var_menuState_8c1bc7a8.state_0x18) {
    case FILE_MENU_STATE_LOADING:
        if (var_8c226010 == 0) {
            menuTextboxText_8c02af1c(0xff);
        } else if (var_8c226010 == 1) {
            var_vmBusy_8c157a7c = 0;
            VmGameSetLcdSlot_8c01c8fc(0);
            var_8c225fe0 = var_8c1ba2e0;
            for (i = 0; i < var_8c22600c; i++) {
                if (!FileMenuIsSaveValid_8c018804((int *)var_8c225fe0)) {
                    LOG_WARN(("[FILE_MENU] fileSelectTask_8c018e7e: save %d failed validation\n", i));
                    FileMenuFreeBuffers_8c0187d0();
                    swapMessageBoxFor_8c02aefc(MSG_LOAD_FAIL);
                    CHANGE_STATE(FILE_MENU_STATE_LOAD_ERROR);
                    break;
                }
                var_8c225fe0 = (char *)var_8c225fe0 + 0x600;
            }
            if (i >= var_8c22600c) {
                buildFileList_8c018a22();
                CHANGE_STATE(FILE_MENU_STATE_READY);
            }
        } else if (var_8c226010 == 2) {
            LOG_WARN(("[FILE_MENU] fileSelectTask_8c018e7e: file load failed\n"));
            var_vmBusy_8c157a7c = 0;
            FileMenuFreeBuffers_8c0187d0();
            swapMessageBoxFor_8c02aefc(MSG_LOAD_FAIL);
            CHANGE_STATE(FILE_MENU_STATE_LOAD_ERROR);
            VmGameSetLcdSlot_8c01c8fc(0);
        }
        TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 1, 0.0, 0.0, -4.3);
        TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 0, 0.0, 0.0, -5.0);
        break;

    case FILE_MENU_STATE_LOAD_ERROR:
        if (press & PDD_DGT_TA) {
            CHANGE_STATE(FILE_MENU_STATE_ERROR_FADE_OUT);
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            FadePushOut_8c022b60(10);
            return;
        }
        menuTextboxText_8c02af1c(0xff);
        TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 1, 0.0, 0.0, -4.3);
        TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 0, 0.0, 0.0, -5.0);
        break;

    case FILE_MENU_STATE_ERROR_FADE_OUT:
        if (var_isFading_8c226568 == 0) {
            VmMenuSwitchFromTask_8c019e44(task);
            return;
        }
        menuTextboxText_8c02af1c(0xff);
        TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 1, 0.0, 0.0, -4.3);
        TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 0, 0.0, 0.0, -5.0);
        break;

    case FILE_MENU_STATE_READY:
        if (press & PDD_DGT_KL) {
            if (var_menuState_8c1bc7a8.selected_0x38 != 0) {
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                var_menuState_8c1bc7a8.selected_0x38--;
            } else if (var_menuState_8c1bc7a8.field_0x3c > 0) {
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                var_menuState_8c1bc7a8.field_0x3c--;
            }
        } else if (press & PDD_DGT_KR) {
            if (var_menuState_8c1bc7a8.selected_0x38 >= 2) {
                if (var_8c226018[var_menuState_8c1bc7a8.field_0x3c + 3] != 0xb) {
                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                    var_menuState_8c1bc7a8.field_0x3c++;
                }
            } else if (var_8c226018[var_menuState_8c1bc7a8.field_0x3c +
                                    var_menuState_8c1bc7a8.selected_0x38 + 1] != 0xb) {
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                var_menuState_8c1bc7a8.selected_0x38++;
            }
        } else if (press & PDD_DGT_TB) {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
            CHANGE_STATE(FILE_MENU_STATE_CANCEL_FADE_OUT);
            FadePushOut_8c022b60(10);
        } else if (press & PDD_DGT_TA) {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            if (var_8c226018[var_menuState_8c1bc7a8.field_0x3c +
                             var_menuState_8c1bc7a8.selected_0x38] == 0xa) {
                swapMessageBoxFor_8c02aefc(MSG_CONFIRM_NEW_FILE);
            } else {
                swapMessageBoxFor_8c02aefc(MSG_CONFIRM_FILE);
            }
            var_menuState_8c1bc7a8.field_0x40 = 0;
            CHANGE_STATE(FILE_MENU_STATE_CONFIRM);
        }
        drawFileSelect_8c018d46();
        break;

    case FILE_MENU_STATE_CONFIRM:
        if (press & PDD_DGT_TA) {
            int cardValue;

            CHANGE_STATE(FILE_MENU_STATE_CONFIRM_FADE_OUT);
            cardValue = var_8c226018[var_menuState_8c1bc7a8.field_0x3c +
                                     var_menuState_8c1bc7a8.selected_0x38];
            if (cardValue == 0xa) {
                int slot = 0;
                int idx = 1;

                LOG_DEBUG(("[FILE_MENU] fileSelectTask_8c018e7e: creating new file\n"));
                FileMenuResetNewGame_8c01895e();
                while (idx < var_8c226014 && var_8c226018[idx] == slot) {
                    slot++;
                    idx++;
                }
                var_8c1ba350 = slot;
            } else {
                int idx = 0;

                LOG_DEBUG(("[FILE_MENU] fileSelectTask_8c018e7e: loading file (slot %d)\n", cardValue));
                while (var_8c225fe4[idx] != cardValue) {
                    idx++;
                }
                njMemCopy(&var_progress_8c1ba1cc, (char *)var_8c1ba2e0 + idx * 0x600, 0xe8);
                SystemMenuApplyLoadedProgress_8c01b19c();
                var_8c1ba350 = cardValue;
                FileMenuApplySoundSettings_8c0189fc();
            }
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            SndStartAdxFadeOut_8c010bae(0);
            SndStartAdxFadeOut_8c010bae(1);
            FadePushOut_8c022b60(10);
        } else if (press & PDD_DGT_TB) {
            CHANGE_STATE(FILE_MENU_STATE_READY);
            swapMessageBoxFor_8c02aefc("");
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
        }
        drawFileSelect_8c018d46();
        menuTextboxText_8c02af1c(0xff);
        break;

    case FILE_MENU_STATE_CONFIRM_FADE_OUT:
        if (var_isFading_8c226568 != 0) {
            drawFileSelect_8c018d46();
            break;
        }
        FileMenuFreeBuffers_8c0187d0();
        VmMenuUnmountVms_8c0194de();
        CHANGE_STATE(FILE_MENU_STATE_MOUNTING);
        break;

    case FILE_MENU_STATE_MOUNTING:
        if (var_vmMountBusy_8c22606c != 0 || init_8c03bd80 != 0) {
            break;
        }
        var_currentSysResGroupInfo_8c225fb0 = (void *)-1;
        MainMenuSwitchFromTask_8c01a09a(task, 0);
        return;

    case FILE_MENU_STATE_CANCEL_FADE_OUT:
        if (var_isFading_8c226568 != 0) {
            drawFileSelect_8c018d46();
            break;
        }
        FileMenuFreeBuffers_8c0187d0();
        VmMenuSwitchFromTask_8c019e44(task);
        return;

    case FILE_MENU_STATE_UNMOUNTING:
        if (var_vmMountBusy_8c22606c != 0) {
            break;
        }
        VmMenuSwitchFromTask_8c019e44(task);
        return;
    }
}

/*
 * Enters the FILE SELECT screen: installs fileSelectTask on the caller's task.
 * When the selected VMU is mid-operation (status 5/6) it shows the "load in
 * progress" box, lights the VMS LCD, and kicks off the load; otherwise it builds
 * the file list. Fades in either way.
 */
void FileMenuSwitchFromTask_8c019334(Task *task)
{
    TaskSetAction_8c014b3e(task, fileSelectTask_8c018e7e);
    var_8c22600c = 0;
    if (var_vmuStatus_8c226048[var_selectedVm_8c1ba34c] == 5 ||
        var_vmuStatus_8c226048[var_selectedVm_8c1ba34c] == 6) {
        CHANGE_STATE(FILE_MENU_STATE_LOADING);
        swapMessageBoxFor_8c02aefc(MSG_LOADING_NO_POWER_OFF);
        VmGameSetLcdSlot_8c01c8fc(1);
        startVmLoad_8c018784();
        var_vmBusy_8c157a7c = 1;
    } else {
        buildFileList_8c018a22();
        CHANGE_STATE(FILE_MENU_STATE_READY);
    }
    var_menuState_8c1bc7a8.field_0x3c = 0;
    var_menuState_8c1bc7a8.selected_0x38 = 0;
    FadePushIn_8c022a9c(10);
}
