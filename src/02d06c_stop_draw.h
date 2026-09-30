#ifndef _02D06C_STOP_DRAW_H
#define _02D06C_STOP_DRAW_H

/* Draws the passengers waiting at the upcoming stop
 * (var_waitingPassengers_8c228798, count var_waitingPassengerCount_8c228794).
 * Registered once per layer by pedestriansTask_8c0293f6 (028258), which
 * passes the layer through as the argument as well. */
void StopDrawWaitingPassengers_8c02d06c(
    /* 0 for the main view, 1 for the mirror; picks the passenger's facing */
    int layer
);

/* DrawCallback1 pair bracketing the passenger draws: sets up njCnk's simple
 * light and the constant material that fades passengers in and out, then
 * restores njControl3D. Both ignore their arg, and both are reached only by
 * literal-pool pointer from 02d19c_passenger. */
void StopDrawLightBegin_8c02d0fc(int arg0);
void StopDrawLightEnd_8c02d146(int arg0);

#endif // _02D06C_STOP_DRAW_H
