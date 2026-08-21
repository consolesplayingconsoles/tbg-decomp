/* @unit SystemMenu */
#include <shinobi.h>
#include "01b19c_system_menu.h"
#include "sectionB.h"
#include "013ae8_route_load.h"
#include "01c980_profile_file.h"
#include "0193c8_vm_menu.h"
#include "014f54_text.h"
#include "014b8c_backup.h"
#include "011120_asset_queues.h"
#include "028258.h"
#include "022464.h"
#include "016c58_prompt.h"
#include "016d2c_course_menu.h"
#include "018644_file_menu.h"
#include "01bb48_vm_game.h"
#include "015ab8_title.h"
#include "01614c_debug_menu.h"
#include "0100bc_sound.h"
#include "serial_debug.h"

/* ====================
 * Type Declarations
 * ====================
 */

/* Outer state machine, held in menuState.state_0x18. */
typedef enum SaveState {
    SAVE_STATE_WAIT_PVM       = 0, /* wait for route PVM, then free asset queues */
    SAVE_STATE_FADE_IN        = 1, /* wait for the fade-in to finish */
    SAVE_STATE_TOP_MENU       = 2, /* load / save / back / quit selection */
    SAVE_STATE_LOAD           = 3, /* load: confirm, then read the VMU */
    SAVE_STATE_LOAD_FAILED    = 4, /* corrupt/incompatible save notice */
    SAVE_STATE_SAVE           = 5, /* save: confirm, then write the VMU */
    SAVE_STATE_QUIT_CONFIRM   = 6, /* confirm quit to title */
    SAVE_STATE_EXIT_TO_COURSE = 7, /* fade out, hand back to the course menu */
    SAVE_STATE_EXIT_TO_TITLE  = 8  /* fade out, hand back to the title screen */
} SaveState;

/* Top-menu selection, held in menuState.selected_0x38. */
typedef enum SaveMenuItem {
    SAVE_MENU_LOAD = 0,
    SAVE_MENU_SAVE = 1,
    SAVE_MENU_BACK = 2, /* leave the menu, resume the course (-> EXIT_TO_COURSE) */
    SAVE_MENU_QUIT = 3  /* quit to title, with confirm (-> QUIT_CONFIRM) */
} SaveMenuItem;

/* Load/save sub-phase, held in menuState.field_0x1c. */
typedef enum SavePhase {
    SAVE_PHASE_CONFIRM     = 0, /* yes/no prompt is on screen */
    SAVE_PHASE_IN_PROGRESS = 1  /* VMU read/write is underway */
} SavePhase;

/* ====================
 * Forward Declarations
 * ====================
 */

STATIC void writeDecimalDigits_8c01b1c0(char *dst, int value);
STATIC void updateVmuIconText_8c01b206(void);

/* ====================
 * Functions
 * ====================
 */

/* Restore the runtime session values from a just-loaded save. */
void SystemMenuApplyLoadedProgress_8c01b19c(void)
{
    var_8c1bb8b8 = var_progress_8c1ba1cc.field_0xd8;
    var_8c1bb8bc = var_progress_8c1ba1cc.field_0xdc;
    var_8c1bb8dc = var_progress_8c1ba1cc.field_0xe0;
    /* only the low byte is written; upper 3 bytes of the int slot are left alone */
    *(signed char *)&var_award_8c1bb8f8 = var_progress_8c1ba1cc.award_0xe4;
}

/* itoa: write value's decimal digits (MSD first) to dst, no terminator. */
STATIC void writeDecimalDigits_8c01b1c0(char *dst, int value)
{
    char buf[8];
    char *p;
    int count;

    p = &buf[7];
    count = 0;
    do {
        *p = (char)(value % 10) + '0';
        p--;
        count++;
        value /= 10;
    } while (value != 0);

    for (; count > 0; count--) {
        p++;
        *dst = *p;
        dst++;
    }
}

/* Rebuild the "9/DD  EXP EEE" status line shown on the VMU icon. */
STATIC void updateVmuIconText_8c01b206(void)
{
    int i;

    for (i = 0; i < 0x10; i++) {
        var_8c226098[i] = ' ';
    }
    strcpy(var_8c226098, "9/   EXP ");
    writeDecimalDigits_8c01b1c0(&var_8c226098[2], var_progress_8c1ba1cc.days_0x00);
    writeDecimalDigits_8c01b1c0(&var_8c226098[9], var_exp_8c1ba25c);
}

/* Stage the session values into the progress struct, build the VMU backup
 * file image around it, and write it to the selected VMU. */
void SystemMenuWriteToVmu_8c01b26c(void)
{
    int size;

    var_progress_8c1ba1cc.field_0xd8 = var_8c1bb8b8;
    var_progress_8c1ba1cc.field_0xdc = var_8c1bb8bc;
    var_progress_8c1ba1cc.field_0xe0 = var_8c1bb8dc;
    var_progress_8c1ba1cc.award_0xe4 = *(signed char *)&var_award_8c1bb8f8;
    ProfileFileUpdateUnlocks_8c01c980();
    var_progress_8c1ba1cc.field_0x8c = var_profileUnlockedCount_8c2263a4;
    updateVmuIconText_8c01b206();

    memset(&var_8c1ba2e4, 0, 0x60);
    njMemCopy(&var_8c1ba2e4, var_8c226098, 0x10);
    /* VMU file comment: "Tokyo Bus Guide data" */
    strcpy(var_8c1ba2e4.btr_comment, "東京バス案内　データ");
    njMemCopy(var_8c1ba2e4.game_name, init_8c04410c, 0x10);
    var_8c1ba2e4.icon_palette = var_8c1ba344;
    var_8c1ba2e4.icon_data = (char *)var_8c1ba344 + 0x20;
    var_8c1ba2e4.icon_num = 1;
    var_8c1ba2e4.icon_speed = 1;
    var_8c1ba2e4.save_data = &var_progress_8c1ba1cc;
    var_8c1ba2e4.save_size = 0xe8;

    size = buCalcBackupFileSize(var_8c1ba2e4.icon_num, var_8c1ba2e4.visual_type,
                                var_8c1ba2e4.save_size);
    var_8c1ba348 = syMalloc(size << 9);
    buMakeBackupFileImage(var_8c1ba348, &var_8c1ba2e4);
    BupSave_8c014bcc(var_selectedVm_8c1ba34c,
                     init_saveNames_8c044d50[var_8c1ba350], var_8c1ba348, 3);
    var_vmBusy_8c157a7c = 1;
}

/* Per-frame update for the save/load menu task. Dispatches on the menu state
 * machine (0-8) and draws the current screen. */
STATIC void saveTask_8c01b3ac(Task *task, void *state)
{
    int vmStatus;
    int result;
    int drawPrompt = 0;   /* states 3/5/6 draw the yes/no highlight before the tail */

    if (var_menuState_8c1bc7a8.state_0x18 < SAVE_STATE_EXIT_TO_COURSE) {
        if (var_selectedVm_8c1ba34c == -1) {
            vmStatus = VMU_STATUS_PROCEED_WITHOUT_SAVING;
        } else {
            VmMenuUpdateVmuStatus_8c01967c(var_selectedVm_8c1ba34c,
                                           init_saveNames_8c044d50[var_8c1ba350], 3);
            vmStatus = var_vmuStatus_8c226048[var_selectedVm_8c1ba34c];
        }
    }

    switch (var_menuState_8c1bc7a8.state_0x18) {
    case SAVE_STATE_WAIT_PVM:
        if (RouteLoadIsPvmReady_8c01432a() != 0) {
            return;
        }
        AsqFreeQueues_8c011f7e();
        var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_FADE_IN;
        return;

    case SAVE_STATE_FADE_IN:
        if (var_isFading_8c226568 == 0) {
            var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_TOP_MENU;
        }
        break;

    case SAVE_STATE_TOP_MENU:
        PromptHandleMultiple_8c016c58(&var_menuState_8c1bc7a8.selected_0x38, 4);
        if ((var_peripherals_8c1ba35c[0].press & (PDD_DGT_KR | PDD_DGT_KL)) != 0) {
            swapMessageBoxFor_8c02aefc("");
        }
        if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) == 0) {
            /* A not pressed */
            if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_TB) == 0) {
                break;
            }
            /* B = cancel */
            var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_EXIT_TO_COURSE;
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
            VmMenuUnmountVms_8c0194de();
            push_fadeout_8c022b60(10);
            break;
        }
        /* A = confirm */
        var_menuState_8c1bc7a8.field_0x3c = 0;
        var_menuState_8c1bc7a8.field_0x1c = SAVE_PHASE_CONFIRM;
        switch (var_menuState_8c1bc7a8.selected_0x38) {
        case SAVE_MENU_LOAD:
            switch (vmStatus) {
            case VMU_STATUS_SAVE_EXISTS:
            case VMU_STATUS_SAVE_EXISTS_NO_SPACE:
                var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_LOAD;
                /* "Load the file. Are you sure?" */
                swapMessageBoxFor_8c02aefc("ファイルをロードします。<E>よろしいですか？");
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                break;
            case VMU_STATUS_NOT_ENOUGH_SPACE:
            case VMU_STATUS_SAVING_POSSIBLE:
                /* "The set file does not exist, so it cannot be loaded" */
                swapMessageBoxFor_8c02aefc("設定されたファイルが無いため<E>ロードできません");
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                break;
            case VMU_STATUS_NOT_CONNECTED:
                /* "No VM is connected to the set port" */
                swapMessageBoxFor_8c02aefc("設定されたポートにＶＭが<E>接続されていません");
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                break;
            case VMU_STATUS_NOT_AVAILABLE:
                /* "This VM is in a state that cannot be loaded" */
                swapMessageBoxFor_8c02aefc("このＶＭはロードできない状態です");
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                break;
            case VMU_STATUS_PROCEED_WITHOUT_SAVING:
                /* "No file has been set, so it cannot be loaded" */
                swapMessageBoxFor_8c02aefc("ファイルが設定されていないため<E>ロードできません");
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                break;
            default:
                break;
            }
            break;
        case SAVE_MENU_SAVE:
            switch (vmStatus) {
            case VMU_STATUS_SAVING_POSSIBLE:
                /* "Create a file. Are you sure?" */
                swapMessageBoxFor_8c02aefc("ファイルを作成します。<E>よろしいですか？");
                var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_SAVE;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                break;
            case VMU_STATUS_SAVE_EXISTS:
            case VMU_STATUS_SAVE_EXISTS_NO_SPACE:
                /* "The file will be overwritten. Are you sure?" */
                swapMessageBoxFor_8c02aefc("ファイルが上書きされます。<E>よろしいですか？");
                var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_SAVE;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                break;
            case VMU_STATUS_NOT_ENOUGH_SPACE:
                /* "Not enough free blocks; saving needs 3 blocks" */
                swapMessageBoxFor_8c02aefc("空きブロックが不足しています<E>セーブには３ブロック必要です");
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                break;
            case VMU_STATUS_NOT_AVAILABLE:
                /* "This VM is in a state that cannot be saved" */
                swapMessageBoxFor_8c02aefc("このＶＭはセーブできない状態です");
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                break;
            case VMU_STATUS_NOT_CONNECTED:
                /* "No VM is connected; saving needs 3 blocks" */
                swapMessageBoxFor_8c02aefc("ＶＭが接続されていません<E>セーブには３ブロック必要です");
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                break;
            case VMU_STATUS_PROCEED_WITHOUT_SAVING:
                /* "No file has been set, so it cannot be saved" */
                swapMessageBoxFor_8c02aefc("ファイルが設定されていないため<E>セーブできません");
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                break;
            default:
                break;
            }
            break;
        case SAVE_MENU_BACK:
            var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_EXIT_TO_COURSE;
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            VmMenuUnmountVms_8c0194de();
            push_fadeout_8c022b60(10);
            break;
        case SAVE_MENU_QUIT:
            var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_QUIT_CONFIRM;
            /* story: "Exit story mode. Are you sure?" */
            /* free:  "Exit free-run mode. Are you sure?" */
            if (var_gameMode_8c1bb8fc == 0) {
                swapMessageBoxFor_8c02aefc("ストーリーモードを終了します。<E>よろしいですか？");
            } else {
                swapMessageBoxFor_8c02aefc("フリーランモードを終了します。<E>よろしいですか？");
            }
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            break;
        default:
            break;
        }
        break;

    case SAVE_STATE_LOAD: /* LOAD confirm/progress, sub-state in field_0x1c */
        switch (var_menuState_8c1bc7a8.field_0x1c) {
        case SAVE_PHASE_CONFIRM:
            result = PromptHandleBinary_8c016caa(&var_menuState_8c1bc7a8.field_0x3c);
            if (result == 1) {
                /* confirmed -- kick off the VMU read */
                BupLoad_8c014bc6(var_selectedVm_8c1ba34c,
                                 init_saveNames_8c044d50[var_8c1ba350], var_8c1ba2e0);
                var_vmBusy_8c157a7c = 1;
                var_menuState_8c1bc7a8.field_0x1c = SAVE_PHASE_IN_PROGRESS;
                /* "Loading in progress; do not turn off the power" */
                swapMessageBoxFor_8c02aefc("ロード実行中です<E>電源を切らないで下さい");
                VmGameSetLcdSlot_8c01c8fc(1);
            } else if (result == 2) {
                /* cancelled */
                var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_TOP_MENU;
                swapMessageBoxFor_8c02aefc("");
            } else if (vmStatus != VMU_STATUS_SAVE_EXISTS &&
                       vmStatus != VMU_STATUS_SAVE_EXISTS_NO_SPACE) {
                /* "There is no file to load" */
                swapMessageBoxFor_8c02aefc("ロードするファイルがありません");
                var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_TOP_MENU;
            }
            break;
        case SAVE_PHASE_IN_PROGRESS: /* wait for the read to complete */
            if (buStat(var_selectedVm_8c1ba34c) != 0) {
                break;
            }
            if (buGetLastError(var_selectedVm_8c1ba34c) == 0) {
                buAnalyzeBackupFileImage(&var_8c1ba2e4, var_8c1ba2e0);
                njMemCopy(&var_progress_8c1ba1cc, var_8c1ba33c, 0xe8);
                if (FileMenuIsSaveValid_8c018804((int *)&var_progress_8c1ba1cc) != 0) {
                    SystemMenuApplyLoadedProgress_8c01b19c();
                    FileMenuApplySoundSettings_8c0189fc();
                    /* "Loading complete" */
                    swapMessageBoxFor_8c02aefc("ロードが終了しました");
                } else {
                    /* corrupt/incompatible save -- "Load failed; aborting the game" */
                    swapMessageBoxFor_8c02aefc("ロードが失敗しました<E>ゲームを中断します");
                    var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_LOAD_FAILED;
                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                    break;
                }
            } else {
                /* "Load failed" */
                swapMessageBoxFor_8c02aefc("ロードが失敗しました");
            }
            var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_TOP_MENU;
            var_vmBusy_8c157a7c = 0;
            VmGameSetLcdSlot_8c01c8fc(0);
            break;
        default:
            break;
        }
        drawPrompt = 1;
        break;

    case SAVE_STATE_LOAD_FAILED: /* corrupt-save notice, wait for A to bail to title */
        if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) != 0) {
            FUN_8c016182();
            TitlePushTitle_8c015fd6(0);
            return;
        }
        TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                               4, 0.0f, 0.0f, -5.0f);
        if (menuTextboxText_8c02af1c(0xff) != 0) {
            TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00,
                                   1, 0.0f, 0.0f, -5.0f);
        }
        return;

    case SAVE_STATE_SAVE: /* SAVE confirm/progress, sub-state in field_0x1c */
        switch (var_menuState_8c1bc7a8.field_0x1c) {
        case SAVE_PHASE_CONFIRM:
            result = PromptHandleBinary_8c016caa(&var_menuState_8c1bc7a8.field_0x3c);
            if (result == 1) {
                /* confirmed -- write and wait */
                SystemMenuWriteToVmu_8c01b26c();
                var_menuState_8c1bc7a8.field_0x1c = SAVE_PHASE_IN_PROGRESS;
                /* "Saving in progress; do not turn off the power" */
                swapMessageBoxFor_8c02aefc("セーブ実行中です<E>電源を切らないで下さい");
                VmGameSetLcdSlot_8c01c8fc(1);
                TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00,
                                       var_menuState_8c1bc7a8.field_0x3c + 2,
                                       228.0f, 266.0f, -4.0f);
            } else if (result == 2) {
                /* cancelled */
                var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_TOP_MENU;
                swapMessageBoxFor_8c02aefc("");
                TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00,
                                       var_menuState_8c1bc7a8.field_0x3c + 2,
                                       228.0f, 266.0f, -4.0f);
            } else if (vmStatus != VMU_STATUS_SAVING_POSSIBLE &&
                       vmStatus != VMU_STATUS_SAVE_EXISTS &&
                       vmStatus != VMU_STATUS_SAVE_EXISTS_NO_SPACE) {
                /* "Cannot save" */
                swapMessageBoxFor_8c02aefc("セーブできません");
                var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_TOP_MENU;
            }
            break;
        case SAVE_PHASE_IN_PROGRESS: /* wait for the write to complete */
            BupGetInfo_8c014bba(var_selectedVm_8c1ba34c);
            if (buStat(var_selectedVm_8c1ba34c) != 0) {
                break;
            }
            /* on success "Save complete", else "Save failed" */
            if (buGetLastError(var_selectedVm_8c1ba34c) == 0) {
                swapMessageBoxFor_8c02aefc("セーブ終了");
            } else {
                swapMessageBoxFor_8c02aefc("セーブが失敗しました");
            }
            var_vmBusy_8c157a7c = 0;
            syFree(var_8c1ba348);
            var_8c1ba348 = (void *)-1;
            var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_TOP_MENU;
            VmGameSetLcdSlot_8c01c8fc(0);
            break;
        default:
            break;
        }
        drawPrompt = 1;
        break;

    case SAVE_STATE_QUIT_CONFIRM: /* QUIT-to-title confirm */
        result = PromptHandleBinary_8c016caa(&var_menuState_8c1bc7a8.field_0x3c);
        if (result == 1) {
            var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_EXIT_TO_TITLE;
            VmMenuUnmountVms_8c0194de();
            push_fadeout_8c022b60(10);
        } else if (result == 2) {
            var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_TOP_MENU;
            swapMessageBoxFor_8c02aefc("");
        }
        drawPrompt = 1;
        break;

    case SAVE_STATE_EXIT_TO_COURSE: /* drive-exit -- fade out then hand back to course */
        if (var_isFading_8c226568 != 0 || var_vmMountBusy_8c22606c != 0) {
            break;
        }
        var_menuState_8c1bc7a8.field_0x3c = 1;
        var_menuState_8c1bc7a8.field_0x40 = 0;
        FileMenuFreeBuffers_8c0187d0();
        CourseMenuSwitchFromTask_8c017e18(task);
        return;

    case SAVE_STATE_EXIT_TO_TITLE: /* title-exit -- fade out then hand back to title */
        if (var_isFading_8c226568 != 0 || var_vmMountBusy_8c22606c != 0) {
            break;
        }
        FUN_8c016182();
        TitlePushTitle_8c015fd6(1);
        return;

    default:
        break;
    }

    if (drawPrompt) {
        /* highlight over the active yes/no prompt option */
        TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00,
                               var_menuState_8c1bc7a8.field_0x3c + 2,
                               228.0f, 266.0f, -4.0f);
    }

    /* common draw tail */
    TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                           var_menuState_8c1bc7a8.selected_0x38 + 5, 0.0f, 0.0f, -4.0f);
    TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                           4, 0.0f, 0.0f, -5.0f);
    if (menuTextboxText_8c02af1c(0xff)) {
        TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00,
                               1, 0.0f, 0.0f, -5.0f);
    }
    TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00,
                           0, 0.0f, 0.0f, -7.0f);
}

/* Course-menu "onSelect" that switches the running task into the save/load
 * flow: install the update action, reset its state, and kick off loading the
 * VMU header from \SYSTEM\bus_mem.VMI. */
void SystemMenuSwitchFromTask_8c01ba64(Task *task)
{
    TaskSetAction_8c014b3e(task, saveTask_8c01b3ac);
    var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_WAIT_PVM;
    var_menuState_8c1bc7a8.selected_0x38 = SAVE_MENU_LOAD;
    var_8c1ba2e0 = syMalloc(0x600);
    VmMenuUpdateVmuStatus_8c01967c(var_selectedVm_8c1ba34c,
                                   init_saveNames_8c044d50[var_8c1ba350], 3);
    swapMessageBoxFor_8c02aefc("");
    push_fadein_8c022a9c(10);
    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();
    AsqRequestDat_8c011182("\\SYSTEM", "bus_mem.VMI", &var_8c1ba344);
    RouteLoadSetPvmReady_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteLoadResetPvmReady_8c014322);
}
