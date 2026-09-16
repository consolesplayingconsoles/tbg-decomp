/* @unit Psp */
#include <shinobi.h>
#include "010e90.h"
#include "011120_asset_queues.h"
#include "012324_peripheral_support.h"
#include "012504_input.h"
#include "01bb48_vm_game.h"
#include "sectionB.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#define DGT_ABXY (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)
#define DGT_UDLR (PDD_DGT_KU | PDD_DGT_KD | PDD_DGT_KL | PDD_DGT_KR)

/* Stick deflection that counts as a dpad press. */
#define STICK_THRESHOLD 64

#define REPEAT_DELAY    15 /* frames held before the first repeat */
#define REPEAT_PERIOD    6 /* and between repeats after that */
#define REPEAT_RAMP     30 /* held this long, it goes to every frame */

/* =======================
 * Non-initialized Globals
 * =======================
 */

KeyRepeat var_keyRepeat_8c157ad4;
int var_stickLatchX_8c157ae4;
int var_stickLatchY_8c157ae8;

/* =========
 * Functions
 * =========
 */

/*
 * Menu-side input: publishes port 0 verbatim as var_peripherals_8c1ba35c[0],
 * then adds the two things menus need that the pad does not report -- dpad
 * presses synthesised from the analog stick, and key repeat on all four
 * directions. InputPushTask_8c0128cc(1) swaps this out for the drive's own
 * remapping task.
 */
void PspTask_8c012324()
{
    int support;

    var_resetRequested_8c157a78 = 0;
    var_peripheral_8c1ba358 = pdGetPeripheral(0);

    support = var_peripheral_8c1ba358->support & BT_CONTROLLER;
    if (
        (var_peripheral_8c1ba358->info->type & PDD_DEVTYPE_CONTROLLER) &&
        (support == BT_CONTROLLER || support == BT_RACING)
    ) {
        var_peripherals_8c1ba35c[0] = *var_peripheral_8c1ba358;
        if (support == BT_CONTROLLER) {
            var_activeCtrlType_8c157a70 = BT_CONTROLLER;
            /* The racing controller has no stick to fold in. */
            if (var_peripherals_8c1ba35c[0].x1 < -STICK_THRESHOLD) {
                var_peripherals_8c1ba35c[0].on |= PDD_DGT_KL;
                if (!(var_stickLatchX_8c157ae4 & PDD_DGT_KL)) {
                    var_peripherals_8c1ba35c[0].press |= PDD_DGT_KL;
                }
                var_stickLatchX_8c157ae4 = PDD_DGT_KL;
            }
            else if (var_peripherals_8c1ba35c[0].x1 > STICK_THRESHOLD) {
                var_peripherals_8c1ba35c[0].on |= PDD_DGT_KR;
                if (!(var_stickLatchX_8c157ae4 & PDD_DGT_KR)) {
                    var_peripherals_8c1ba35c[0].press |= PDD_DGT_KR;
                }
                var_stickLatchX_8c157ae4 = PDD_DGT_KR;
            }
            else {
                var_stickLatchX_8c157ae4 = 0;
            }

            if (var_peripherals_8c1ba35c[0].y1 < -STICK_THRESHOLD) {
                var_peripherals_8c1ba35c[0].on |= PDD_DGT_KU;
                if (!(var_stickLatchY_8c157ae8 & PDD_DGT_KU)) {
                    var_peripherals_8c1ba35c[0].press |= PDD_DGT_KU;
                }
                var_stickLatchY_8c157ae8 = PDD_DGT_KU;
            }
            else if (var_peripherals_8c1ba35c[0].y1 > STICK_THRESHOLD) {
                var_peripherals_8c1ba35c[0].on |= PDD_DGT_KD;
                if (!(var_stickLatchY_8c157ae8 & PDD_DGT_KD)) {
                    var_peripherals_8c1ba35c[0].press |= PDD_DGT_KD;
                }
                var_stickLatchY_8c157ae8 = PDD_DGT_KD;
            }
            else {
                var_stickLatchY_8c157ae8 = 0;
            }

            /* Sega's mandatory soft-reset combo. */
            if (
                (var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
                ((var_peripheral_8c1ba358->on & DGT_ABXY) == DGT_ABXY)
            ) {
                var_resetRequested_8c157a78 = 1;
            }
        } else if (support == BT_RACING) {
            var_activeCtrlType_8c157a70 = BT_RACING;
            /* Same combo, even though the wheel has no X/Y to hold -- so it is
               unreachable here, unlike inputManualTask_8c012504's wheel path. */
            if (
                (var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
                ((var_peripheral_8c1ba358->on & DGT_ABXY) == DGT_ABXY)
            ) {
                var_resetRequested_8c157a78 = 1;
            }
        }
    }
    else {
        *var_peripherals_8c1ba35c = const_peripheral_8c033318;
        var_vibport_8c1ba354 = -1;
        var_activeCtrlType_8c157a70 = -1;
    }

    if (var_keyRepeat_8c157ad4.active_0x00 == 0) {
        if (var_peripherals_8c1ba35c[0].on & DGT_UDLR) {
            var_keyRepeat_8c157ad4.active_0x00 = 1;
            var_keyRepeat_8c157ad4.counter_0x04 = 0;
            var_keyRepeat_8c157ad4.period_0x08 = REPEAT_DELAY;
            var_keyRepeat_8c157ad4.totalFrames_0x0c = 0;
        }
    } else if (var_keyRepeat_8c157ad4.active_0x00 == 1) {
        if (!(var_peripherals_8c1ba35c[0].on & DGT_UDLR)) {
            var_keyRepeat_8c157ad4.active_0x00 = 0;
        } else {
            if (var_keyRepeat_8c157ad4.period_0x08 <= ++var_keyRepeat_8c157ad4.counter_0x04) {
                var_keyRepeat_8c157ad4.counter_0x04 = 0;
                var_keyRepeat_8c157ad4.period_0x08 = REPEAT_PERIOD;
                var_peripherals_8c1ba35c[0].press |=
                    var_peripherals_8c1ba35c[0].on & DGT_UDLR;
            }
            if (++var_keyRepeat_8c157ad4.totalFrames_0x0c > REPEAT_RAMP) {
                var_keyRepeat_8c157ad4.period_0x08 = 1;
            }
        }
    }

    VmGameUpdateLcd_8c01c910();
    SndUpdateAdxVolFade_8c010a40();
}
