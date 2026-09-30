/* Drive messages: the banners that call out a driving penalty while the HUD
 * is up. */
#ifndef _02B2F0_DRIVE_MSG_H
#define _02B2F0_DRIVE_MSG_H

/* One driver-comment banner. `count`/`ids` point into a {count, id...} row
 * of init_penaltyMsgGlyphs_8c04c35c (02b464); the ids are 16x16-atlas glyph
 * indices. [0] is the newest message, [1..3] older ones shifted back as each
 * new one arrives. Typed out one glyph per two frames, then held 60. */
typedef struct {
    int count;
    int *ids;
    float x;          /* row's left edge, centered: (640 - 32 * count) / 2 */
    int revealed;     /* glyphs typed out so far; revealCounter >> 1 */
    int revealCounter;
    int holdFrames;   /* counts down once fully revealed; 0 = slot free */
} DriveMsgSlot;
extern DriveMsgSlot var_driveMsgQueue_8c228564[4];

/* DrawFn (022464_render.h) for the banner stack: draws every
 * var_driveMsgQueue_8c228564 slot still holding as a row of
 * 32x32 glyphs, the newest at y=192 and each older one 32 above it. Once
 * var_runState_8c2285c4.runPassed_0x04 is set it draws the run-passed mark instead, and
 * nothing else. Pushed per frame by taskCallback_8c02c072 (02b464) via
 * RenderQueueDraw_8c0223ea. */
void DriveMsgDraw_8c02b388(int unused);

#endif // _02B2F0_DRIVE_MSG_H
