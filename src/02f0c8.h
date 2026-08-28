/* Minimal external declarations for still-undecompiled unit 02f0c8. */
#ifndef _02F0C8_H
#define _02F0C8_H

/* Returns the next traffic entry in some global iteration cursor; takes no
 * arguments -- the cursor itself is maintained elsewhere. */
void *FUN_8c02f212(void);

/* Called by spawnEntry_8c0272b8 (026710_traffic.h) right after task
 * allocation, with the new Task, its entry, the first element of the
 * entry's resolved script-arg array (entry+0x304), and a constant 0.
 * Nonzero means "reject" -- the caller frees the task and aborts the spawn. */
Sint32 FUN_8c02f0c8(void *task, void *entry, Sint32 firstScriptArg, Sint32 flag);

#endif // _02F0C8_H
