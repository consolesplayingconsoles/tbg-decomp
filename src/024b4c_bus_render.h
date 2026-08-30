#ifndef _024B4C_H
#define _024B4C_H

#include <shinobi.h>

/* Shifts 7 prev/current value pairs (var_8c227d9c->da0, da4->da8,
 * dd8->ddc, de0->de4, de8->dec, df0->df4, df8->dfc) one frame forward.
 * Called by 02d19c after checking var_8c227d9c/da4. */
void FUN_8c024b4c(void);

/* Reverse-direction mirror of FUN_8c024b4c; tail-calls FUN_8c024f32. */
void FUN_8c024b86(void);

/* Re-seats busState's camera draw position (posX_0x2fc/posZ_0x304) by a
 * turn-rate offset picked from var_8c227d9c. */
void FUN_8c024f32(void);

/* Lights, textures and draws the third-person bus model (with door/etc
 * shape motion, always busState.field_0x00c); altLight only selects the
 * light direction (non-NULL -> var_8c227dc4). Called by 025870. */
void FUN_8c024bb8(void *altLight);

/* Updates the gameplay camera to follow the player's bus, outside demo
 * playback (var_playMode_8c1bb8d0 != PLAY_MODE_DEMO). */
void BusRenderUpdateCamera_8c025078(void);

/* No-op unless busState.mirror_0x268 is nonzero. Otherwise picks a
 * local mirror-camera offset/interest by mirror_0x268 (1/2/3, else stale),
 * rotates the offset into world space by the bus's world matrix
 * (field_0x084), positions the separate mirror camera (var_8c1bb944)
 * there, points its interest at the same-rotated per-mode interest
 * vector, rolls it by recent Y waypoint history, activates it, and queues
 * FUN_8c024bb8 on fade layer 1 with the alt light direction
 * (var_8c227dc4, recomputed here from the course's primary light). */
void BusRenderUpdateMirrorCamera_8c025604(void);

#endif // _024B4C_H
