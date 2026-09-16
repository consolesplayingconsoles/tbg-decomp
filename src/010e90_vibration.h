/* 8c010e90 - Vibration */
#ifndef _010E90_VIBRATION_H
#define _010E90_VIBRATION_H

#include <shinobi.h>

/* An all-zero PDS_PERIPHERAL, copied over the pad state by the input units
 * when the controller is gone. */
extern const PDS_PERIPHERAL const_peripheralZero_8c033318;

void VibClear_8c010fbe();
/* Queue rumble pattern 0-7 (see init_vibPatterns_8c03be5c); a request only
 * displaces one already playing if its index is higher. */
void VibStart_8c010f7a(int pattern);
/* Advances the queued pattern one frame -- without it nothing rumbles. */
void VibUpdate_8c010fae(int port);

#endif // _010E90_VIBRATION_H
