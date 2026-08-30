#ifndef _025870_H
#define _025870_H

/* Sets up the fade camera (var_8c1bb984, owned by 022464) at a fixed vantage
 * point -- likely a canned close-up shot used by a fade/transition. */
void FUN_8c025870(void);

/* Called by BusTask_8c022bdc (022bdc) with no arguments each frame during
 * demo playback (var_playMode_8c1bb8d0 == 2); updates the camera from the
 * recorded demo instead of live input. */
void DemoUpdateCamera_8c025906(void);

/* Opens the "next stop" textbox and arms stopTextboxTask_8c0259e8. Picks
 * the current route's stop table (var_8c227e0c) among the three
 * per-route init_stopsShinjuku_8c045674/init_stopsWangan_8c045b60/
 * init_stopsOme_8c045ee4 tables. */
void FUN_8c025af4(void);

#endif // _025870_H
