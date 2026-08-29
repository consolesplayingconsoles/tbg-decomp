#ifndef _024B4C_H
#define _024B4C_H

/* Still-undecompiled unit; only declaring what 02b464_drive_points.c
 * references. */
void FUN_8c024b4c(void);

/* Called by BusTask_8c022bdc (022bdc) with no arguments each frame outside
 * demo playback (var_playMode_8c1bb8d0 != PLAY_MODE_DEMO); updates the
 * gameplay camera to follow the player's bus. */
void gameplayRenderBusUpdateCamera_8c025078(void);

/* Called last by BusTask_8c022bdc (022bdc) with no arguments, once per frame;
 * role unclear. */
void FUN_8c025604(void);

#endif // _024B4C_H
