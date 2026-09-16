#ifndef _01FA78_HUD_H
#define _01FA78_HUD_H

/* Starts the in-drive HUD for a new run: installs its per-frame task
 * (hudUpdateTask_8c01ff48, which queues the renderer through the
 * fade-command queue) and clears the popup, instruction-slot and
 * driver-points-meter state in section B. */
void HudReset_8c02018c(void);

#endif // _01FA78_HUD_H
