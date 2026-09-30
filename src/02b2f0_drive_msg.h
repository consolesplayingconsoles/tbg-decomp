/* Drive messages: the banners that call out a driving penalty while the HUD
 * is up. */
#ifndef _02B2F0_DRIVE_MSG_H
#define _02B2F0_DRIVE_MSG_H

/* DrawCallback1 (022464_render.h) for the banner stack: draws every
 * var_driveMsgQueue_8c228564 slot (sectionB.h) still holding as a row of
 * 32x32 glyphs, the newest at y=192 and each older one 32 above it. Once
 * var_runState_8c2285c4.runPassed_0x04 is set it draws the run-passed mark instead, and
 * nothing else. Pushed per frame by taskCallback_8c02c072 (02b464) via
 * RenderPushCall1_8c0223ea. */
void DriveMsgDraw_8c02b388(int unused);

#endif // _02B2F0_DRIVE_MSG_H
