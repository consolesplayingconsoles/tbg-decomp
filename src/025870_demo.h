/* 8c025870 */
#ifndef _025870_DEMO_H
#define _025870_DEMO_H

/* Per-course opening shot for the attract-mode tour, indexed by
 * courseId_0x00 - 0x26. */
extern char init_demoFirstShot_8c0460b0[];

/* Aims var_cabinCamera_8c1bb984 from inside the bus at eye height, looking
 * forward down the aisle. Despite the unit prefix this is not
 * demo playback: its only caller is StopSpawnInit_8c02d968, which
 * runs in normal play too -- it is the passenger boarding shot, and it shares
 * a unit with the tour below only because both aim a camera the player is not
 * driving. */
void DemoBoardingCamera_8c025870(void);

/* Called each frame by BusTask_8c022bdc (022bdc) while
 * var_playMode_8c1bb8d0 is PLAY_MODE_DEMO. Places the main camera for the
 * three demo modes of var_cameraMode_8c227d9c (024b4c_bus_camera.h): the shot
 * position for 5 and 6 was already resolved by applyShotPosition_8c0258ba,
 * mode 7 re-resolves it every frame so the camera rides along. Also refreshes
 * the bus's move delta from camera to bus, which is what 027958_bus_draw's
 * forward cone tests against. */
void DemoUpdateCamera_8c025906(void);

/* Starts the attract-mode camera tour: selects this route's shot table into
 * var_demoShots_8c227e0c, spawns demoShotTask_8c0259e8 and opens the caption
 * textbox. Called by 0129cc_game.c only in PLAY_MODE_DEMO, which is the whole
 * tour's entry point. The tour is a sequence of camera cues painted along the
 * route, not one shot per bus stop -- most cues are unnamed framings, and
 * several in a row can sit at one spot to cut between angles. */
void DemoStartTour_8c025af4(void);

#endif // _025870_DEMO_H
