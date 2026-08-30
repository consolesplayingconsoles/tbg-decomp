#ifndef _02D06C_H
#define _02D06C_H

/* Draws every already-picked waiting passenger's sprite (var_8c228798,
 * count var_8c228794); arg0 selects which of two facing sprites to use. */
void StopDrawWaitingPassengers_8c02d06c(int arg0);

/* FadeCallback1 pair bracketing StopDrawWaitingPassengers_8c02d06c's draw: sets up njCnk's simple
 * light + constant material, and restores njControl3D afterward. Each
 * ignores its own arg; referenced only via literal-pool pointer from
 * still-asm 02d19c. */
void StopDrawLightBegin_8c02d0fc(int arg0);
void StopDrawLightEnd_8c02d146(int arg0);

#endif // _02D06C_H
