/* 8c0222dc */
#ifndef _0222DC_FADECMD_H
#define _0222DC_FADECMD_H

#include "022464_fade.h" /* FadeCallback1, FadeCallback2 */

void FadeCmdPushTileDrawTask_8c0222dc(void);
void FadeCmdResetQueues_8c02239c(void);
void FadeCmdPushCall1_8c0223ea(int layer, FadeCallback1 fn, int arg0);
void FadeCmdPushCall2_8c022420(int layer, FadeCallback2 fn, int arg0, int arg1);

#endif // _0222DC_FADECMD_H
