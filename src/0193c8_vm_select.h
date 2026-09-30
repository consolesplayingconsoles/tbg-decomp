#ifndef _0193C8_VM_SELECT_H
#define _0193C8_VM_SELECT_H

#include "sg_xpt.h"
#include "014a9c_tasks.h"

/* var_vmuStatus_8c226048[drive]. Index 8 is the PROCEED WITHOUT SAVING
 * pseudo-slot. Both SAVE_EXISTS values mean the file is there; _NO_SPACE
 * adds that there is no room left for another. */
enum VMU_STATUS {
    VMU_STATUS_NOT_CONNECTED = 0,
    VMU_STATUS_NOT_AVAILABLE = 1,
    VMU_STATUS_NOT_ENOUGH_SPACE = 2,
    VMU_STATUS_PROCEED_WITHOUT_SAVING = 3,
    VMU_STATUS_SAVING_POSSIBLE = 4,
    VMU_STATUS_SAVE_EXISTS_NO_SPACE = 5,
    VMU_STATUS_SAVE_EXISTS = 6
};

extern char* init_saveNames_8c044d50[11];
extern int var_vmuStatus_8c226048[9];
extern int var_vmMountBusy_8c22606c;

void VmSelectMountAll_8c01940e();
void VmSelectUnmountAll_8c0194de();
void VmSelectFreeAndClear_8c019504(void);
int VmSelectUpdateAllStatus_8c019550(char **saveNames, Uint16 blocks);
void VmSelectUpdateStatus_8c01967c(Sint32 drive, char *saveName, Uint16 blocks);
void VmSelectSwitchFromTask_8c019e44(Task *task);

#endif // _0193C8_VM_SELECT_H
