/* 8c027958: undecompiled */
#ifndef _027958_H
#define _027958_H

void FUN_8c0281ac(int arg0, int arg1);
void FUN_8c028206(int arg0, int arg1);

/* Called by BusTask_8c022bdc (022bdc) with the player's BusState
 * (var_8c1bbd9c) each frame at night (var_timeOfDay_8c18ad20 == 2);
 * probabilistically toggles the bus's blinkers. */
void prob_blinker_8c028022(void *bus);

#endif // _027958_H
