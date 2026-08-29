/* 8c02b2f0 */
#ifndef _02B2F0_H
#define _02B2F0_H

/* FadeCallback1 (022464_fade.h) for the drive-message HUD banner. Draws all
 * 4 var_driveMsgQueue_8c228564 slots (sectionB.h) whose holdFrames is
 * nonzero, as a row of 32x32 glyph quads each, one row per slot 32 units
 * apart -- unless var_8c2285c8 is set, in which case it instead draws
 * var_markTexlist_8c1bc418's sprite (some other UI element temporarily owns
 * the screen). Pushed as taskCallback_8c02c072's per-frame HUD draw call
 * (02b464) via FadeCmdPushCall1_8c0223ea. */
void DriveMsgDraw_8c02b388(int unused);

#endif // _02B2F0_H
