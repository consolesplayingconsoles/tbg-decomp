/* @unit Input */
#include <shinobi.h>
#include "010e90.h"
#include "011120_asset_queues.h"
#include "012324_peripheral_support.h"
#include "014a9c_tasks.h"
#include "012504_input.h"
#include "01bb48_vm_game.h"
#include "sectionB.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"

/* =======================
 * Non-initialized Globals
 * =======================
 */

STATIC char var_bootSentinel_8c157aec[12];

/* ===================
 * Initialized Globals
 * ===================
 */

STATIC const char *init_fortyFive_8c03bf40 = "FortyFive";

/* =========
 * Functions
 * =========
 */

/*
 * Per-frame input for a manual-transmission drive (var_driveMode_8c1bb8c8 ==
 * 0): folds port 0 into the logical var_peripherals_8c1ba35c[0] through the
 * configurable remap tables, and watches for the reset combo.
 */
STATIC void inputManualTask_8c012504(void)
{
    int support;
    int i;

    var_resetRequested_8c157a78 = 0;
    var_peripheral_8c1ba358 = pdGetPeripheral(0);

    support = var_peripheral_8c1ba358->support & BT_CONTROLLER;
    if (
        !(var_peripheral_8c1ba358->info->type & PDD_DEVTYPE_CONTROLLER) ||
        (support != BT_CONTROLLER && support != BT_RACING)
    ) {
        LOG_TRACE(("[INPUT] inputManualTask_8c012504: no supported controller on port 0\n"));
        *var_peripherals_8c1ba35c = const_peripheral_8c033318;
        var_vibport_8c1ba354 = -1;
        var_activeCtrlType_8c157a70 = -1;
        VmGameUpdateLcd_8c01c910();
        return;
    }

    var_peripherals_8c1ba35c[0].r = var_peripheral_8c1ba358->r;
    var_peripherals_8c1ba35c[0].l = var_peripheral_8c1ba358->l;
    var_peripherals_8c1ba35c[0].x1 = var_peripheral_8c1ba358->x1;
    var_peripherals_8c1ba35c[0].on = 0;
    var_peripherals_8c1ba35c[0].press = 0;
    var_activeCtrlType_8c157a70 = support;

    if (support == BT_CONTROLLER) {
        LOG_TRACE(("[INPUT] inputManualTask_8c012504: standard controller\n"));
        for (i = 0; i < 7; i++) {
            if (var_peripheral_8c1ba358->on & init_btnRemap_8c03be80[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].on |= init_btnRemap_8c03be80[i].logical_0x04;
            }
        }
        for (i = 0; i < 7; i++) {
            if (var_peripheral_8c1ba358->press & init_btnRemap_8c03be80[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].press |= init_btnRemap_8c03be80[i].logical_0x04;
            }
        }
        /* Sega's mandatory soft-reset combo. */
        if ((var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
            (var_peripheral_8c1ba358->on & (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)) ==
                (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)) {
            LOG_DEBUG(("[INPUT] inputManualTask_8c012504: soft-reset combo held\n"));
            var_resetRequested_8c157a78 = 1;
        }
    } else if (support == BT_RACING) {
        LOG_TRACE(("[INPUT] inputManualTask_8c012504: racing wheel\n"));
        for (i = 0; i < 5; i++) {
            if (var_peripheral_8c1ba358->on & init_btnRemapWheel_8c03bef0[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].on |= init_btnRemapWheel_8c03bef0[i].logical_0x04;
            }
        }
        for (i = 0; i < 5; i++) {
            if (var_peripheral_8c1ba358->press & init_btnRemapWheel_8c03bef0[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].press |= init_btnRemapWheel_8c03bef0[i].logical_0x04;
            }
        }
        /* Forward/reverse selector: at a standstill with the brake past
           half, Y becomes the D-pad press updateGearSelector_8c0242ce reads --
           Up to leave reverse, Down to enter it. Swallows Y, so the reset
           combo is unreachable this frame. */
        if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_TY) &&
            var_busState_8c1bb9d0.speed_0x27c == 0.0f &&
            var_peripherals_8c1ba35c[0].l >= 0x81) {
            LOG_DEBUG(("[INPUT] inputManualTask_8c012504: gear selector (gear %d)\n", var_busState_8c1bb9d0.gear_0x2f4));
            if (var_busState_8c1bb9d0.gear_0x2f4 == 5) {
                var_peripherals_8c1ba35c[0].press ^= (PDD_DGT_TY | PDD_DGT_KU);
            } else if (var_busState_8c1bb9d0.gear_0x2f4 == 0) {
                var_peripherals_8c1ba35c[0].press ^= (PDD_DGT_TY | PDD_DGT_KD);
            }
        } else {
            /* The wheel has no X/Y, so the combo is Start + A + B. */
            if ((var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
                (var_peripheral_8c1ba358->on & (PDD_DGT_TA | PDD_DGT_TB)) ==
                    (PDD_DGT_TA | PDD_DGT_TB)) {
                LOG_DEBUG(("[INPUT] inputManualTask_8c012504: soft-reset combo held (wheel)\n"));
                var_resetRequested_8c157a78 = 1;
            }
        }
    }

    if (pdGetPeripheral(1)->info->type & PDD_DEVTYPE_VIBRATION) {
        var_vibport_8c1ba354 = 1;
    } else if (pdGetPeripheral(2)->info->type & PDD_DEVTYPE_VIBRATION) {
        var_vibport_8c1ba354 = 2;
    } else {
        var_vibport_8c1ba354 = -1;
    }

    VmGameUpdateLcd_8c01c910();
}

/*
 * The same for an automatic drive (var_driveMode_8c1bb8c8 != 0): its own pair
 * of remap tables, and no forward/reverse selector.
 */
STATIC void inputAutoTask_8c012718(void)
{
    int support;
    int i;

    var_resetRequested_8c157a78 = 0;
    var_peripheral_8c1ba358 = pdGetPeripheral(0);

    support = var_peripheral_8c1ba358->support & BT_CONTROLLER;
    if (
        !(var_peripheral_8c1ba358->info->type & PDD_DEVTYPE_CONTROLLER) ||
        (support != BT_CONTROLLER && support != BT_RACING)
    ) {
        LOG_TRACE(("[INPUT] inputAutoTask_8c012718: no supported controller on port 0\n"));
        *var_peripherals_8c1ba35c = const_peripheral_8c033318;
        var_vibport_8c1ba354 = -1;
        var_activeCtrlType_8c157a70 = -1;
        VmGameUpdateLcd_8c01c910();
        return;
    }

    var_peripherals_8c1ba35c[0].r = var_peripheral_8c1ba358->r;
    var_peripherals_8c1ba35c[0].l = var_peripheral_8c1ba358->l;
    var_peripherals_8c1ba35c[0].x1 = var_peripheral_8c1ba358->x1;
    var_peripherals_8c1ba35c[0].on = 0;
    var_peripherals_8c1ba35c[0].press = 0;
    var_activeCtrlType_8c157a70 = support;

    if (support == BT_CONTROLLER) {
        LOG_TRACE(("[INPUT] inputAutoTask_8c012718: standard controller\n"));
        for (i = 0; i < 7; i++) {
            if (var_peripheral_8c1ba358->on & init_btnRemapAlt_8c03beb8[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].on |= init_btnRemapAlt_8c03beb8[i].logical_0x04;
            }
        }
        for (i = 0; i < 7; i++) {
            if (var_peripheral_8c1ba358->press & init_btnRemapAlt_8c03beb8[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].press |= init_btnRemapAlt_8c03beb8[i].logical_0x04;
            }
        }
        /* Sega's mandatory soft-reset combo. */
        if ((var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
            (var_peripheral_8c1ba358->on & (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)) ==
                (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)) {
            LOG_DEBUG(("[INPUT] inputAutoTask_8c012718: soft-reset combo held\n"));
            var_resetRequested_8c157a78 = 1;
        }
    } else if (support == BT_RACING) {
        LOG_TRACE(("[INPUT] inputAutoTask_8c012718: racing wheel\n"));
        for (i = 0; i < 5; i++) {
            if (var_peripheral_8c1ba358->on & init_btnRemapWheelAlt_8c03bf18[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].on |= init_btnRemapWheelAlt_8c03bf18[i].logical_0x04;
            }
        }
        for (i = 0; i < 5; i++) {
            if (var_peripheral_8c1ba358->press & init_btnRemapWheelAlt_8c03bf18[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].press |= init_btnRemapWheelAlt_8c03bf18[i].logical_0x04;
            }
        }
        /* The wheel has no X/Y, so the combo is Start + A + B. */
        if ((var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
            (var_peripheral_8c1ba358->on & (PDD_DGT_TA | PDD_DGT_TB)) ==
                (PDD_DGT_TA | PDD_DGT_TB)) {
            LOG_DEBUG(("[INPUT] inputAutoTask_8c012718: soft-reset combo held (wheel)\n"));
            var_resetRequested_8c157a78 = 1;
        }
    }

    if (pdGetPeripheral(1)->info->type & PDD_DEVTYPE_VIBRATION) {
        var_vibport_8c1ba354 = 1;
    } else if (pdGetPeripheral(2)->info->type & PDD_DEVTYPE_VIBRATION) {
        var_vibport_8c1ba354 = 2;
    } else {
        var_vibport_8c1ba354 = -1;
    }

    VmGameUpdateLcd_8c01c910();
}

/* Installs the task that fills var_peripherals_8c1ba35c each frame: 0 for the
 * menus (PspTask_8c012324, with its stick and repeat state reset), 1 for a
 * drive. Any other value installs nothing. */
void InputPushTask_8c0128cc(int param)
{
    void (*action)(void);
    void *created_state;

    if (param == 0) {
        LOG_DEBUG(("[INPUT] InputPushTask_8c0128cc: queueing peripheral-support task\n"));
        TaskPush_8c014ae8(var_tasks_8c1ba3c8, PspTask_8c012324,
                          &var_8c157a74, &created_state, 0);
        var_stickLatchX_8c157ae4 = 0;
        var_stickLatchY_8c157ae8 = 0;
        var_keyRepeat_8c157ad4.active_0x00 = 0;
    } else if (param == 1) {
        if (var_driveMode_8c1bb8c8 == 0) {
            action = inputManualTask_8c012504;
        } else {
            action = inputAutoTask_8c012718;
        }
        LOG_DEBUG(("[INPUT] InputPushTask_8c0128cc: queueing input handler (%s)\n",
                   var_driveMode_8c1bb8c8 == 0 ? "inputManualTask_8c012504" : "inputAutoTask_8c012718"));
        TaskPush_8c014ae8(var_tasks_8c1ba3c8, action,
                          &var_8c157a74, &created_state, 0);
    }
}

/* One frame of input, run straight after GameEnterDrive_8c01306e (013ae8) so
 * the drive's first frame already sees mapped buttons. */
void InputDispatchTask_8c012970(void)
{
    if (var_driveMode_8c1bb8c8 == 0) {
        LOG_TRACE(("[INPUT] InputDispatchTask_8c012970: dispatch inputManualTask_8c012504\n"));
        inputManualTask_8c012504();
    } else {
        LOG_TRACE(("[INPUT] InputDispatchTask_8c012970: dispatch inputAutoTask_8c012718\n"));
        inputAutoTask_8c012718();
    }
}

/* True only on the first call since the program started: the sentinel lives
 * in BSS, so a return to the title leaves it standing but a reboot clears it. */
int InputCheckColdBoot_8c012984(void)
{
    if (strcmp(var_bootSentinel_8c157aec, init_fortyFive_8c03bf40) == 0) {
        LOG_DEBUG(("[INPUT] InputCheckColdBoot_8c012984: name already \"%s\"\n", var_bootSentinel_8c157aec));
        return 0;
    }
    LOG_DEBUG(("[INPUT] InputCheckColdBoot_8c012984: setting name to \"%s\"\n", init_fortyFive_8c03bf40));
    strcpy(var_bootSentinel_8c157aec, init_fortyFive_8c03bf40);
    return 1;
}
