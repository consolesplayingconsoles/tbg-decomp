#ifndef OPTION_01A148_H
#define OPTION_01A148_H

#include "014a9c_tasks.h"

extern Sint8 var_soundMode_8c226070;

/*
 * The OPTION menu's only external entry point: the main menu calls this to open
 * the OPTION top menu. Every other function in the unit (the per-screen tasks,
 * the switch-in wrappers, the value/draw helpers) is private -- see the .c.
 */
void OptionSwitchToTopMenu_8c01b122(Task *task, int row);

#endif
