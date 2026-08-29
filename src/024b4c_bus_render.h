#ifndef _024B4C_H
#define _024B4C_H

#include <shinobi.h>

/* Shifts 7 prev/current value pairs (var_8c227d9c->da0, da4->da8,
 * dd8->ddc, de0->de4, de8->dec, df0->df4, df8->dfc) one frame forward.
 * Called by 02d19c after checking var_8c227d9c/da4. */
void FUN_8c024b4c(void);

/* Reverse-direction mirror of FUN_8c024b4c; tail-calls FUN_8c024f32. */
void FUN_8c024b86(void);

/* Turn-blink state machine driven by var_8c227d9c; called by FUN_8c024b86
 * with no arguments. */
void FUN_8c024f32(void);

/* Lights, textures and draws the third-person bus model (with door/etc
 * shape motion, always busState.field_0x00c); altLight only selects the
 * light direction (non-NULL -> var_8c227dc4). Called by 025870
 * (undecompiled). */
void FUN_8c024bb8(void *altLight);

/* Moves busState's draw position (posX_0x2fc/posY_0x300/posZ_0x304) dist
 * along the unit vector from the current position (posX_0x0f4/posZ_0x0fc)
 * toward it, dyOffset in Y; when the resulting bearing change from the
 * bus's stored heading (field_0x230/field_0x238) exceeds ~5 degrees, clamps
 * the turn rate and rotates the move by the clamped amount instead. Then
 * points the camera at the new position, with its interest aimed at the
 * unmoved position offset by interestDyOffset in Y. Called by
 * BusRenderUpdateCamera_8c025078. */
void BusRenderPositionCamera_8c024d6c(float dist, float dyOffset, float interestDyOffset);

/* Called by BusTask_8c022bdc (022bdc) with no arguments each frame outside
 * demo playback (var_playMode_8c1bb8d0 != PLAY_MODE_DEMO); updates the
 * gameplay camera to follow the player's bus. */
void BusRenderUpdateCamera_8c025078(void);

/* Called last by BusTask_8c022bdc (022bdc) with no arguments, once per
 * frame. No-op unless busState.mirror_0x268 is nonzero. Otherwise picks a
 * local mirror-camera offset/interest by mirror_0x268 (1/2/3, else stale),
 * rotates the offset into world space by the bus's world matrix
 * (field_0x084), positions the separate mirror camera (var_8c1bb944)
 * there, points its interest at the same-rotated per-mode interest
 * vector, rolls it by recent Y waypoint history, activates it, and queues
 * FUN_8c024bb8 on fade layer 1 with the alt light direction
 * (var_8c227dc4, recomputed here from the course's primary light). */
void BusRenderUpdateMirrorCamera_8c025604(void);

#endif // _024B4C_H
