/* 8c02b464: undecompiled */
#ifndef _02B464_H
#define _02B464_H

/* Called from BusStopUpdateArrival_8c02ce48 on stop completion; return
 * nonzero to also request FUN_8c02c586's (unclear) extra side effect. */
int FUN_8c02c586(void);

/* Installed as var_fadeCompleteCallback_8c22656c by
 * BusStopUpdateArrival_8c02ce48 when a stop completes. */
void FUN_8c02c784(void);

#endif // _02B464_H
