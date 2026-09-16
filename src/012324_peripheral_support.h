/* 8c012324 - Peripheral Support */
#ifndef _PERIPHERAL_SUPPORT_H
#define _PERIPHERAL_SUPPORT_H

/* =================
 * Type Declarations
 * =================
 */

/* Auto-repeat for the dpad, applied to var_peripherals_8c1ba35c[0].press.
 * period_0x08 starts long and shortens the longer a direction is held. */
typedef struct {
    int active_0x00;
    int counter_0x04;
    int period_0x08;
    int totalFrames_0x0c;
} KeyRepeat;

extern KeyRepeat var_keyRepeat_8c157ad4;

/* Last dpad direction the analog stick stood for, so a held stick presses once.
 * Only PspTask_8c012324 synthesises these; the drive's own input tasks do not. */
extern int var_stickLatchX_8c157ae4;
extern int var_stickLatchY_8c157ae8;

void PspTask_8c012324();

#endif // _PERIPHERAL_SUPPORT_H
