#ifndef _VM_MENU_H
#define _VM_MENU_H

#include "sg_xpt.h"
#include "014a9c_tasks.h"

enum VMU_STATUS {
    VMU_STATUS_NOT_CONNECTED = 0,
    VMU_STATUS_NOT_AVAILABLE = 1,
    VMU_STATUS_NOT_ENOUGH_SPACE = 2,
    VMU_STATUS_PROCEED_WITHOUT_SAVING = 3,
    VMU_STATUS_SAVING_POSSIBLE = 4,
    VMU_STATUS_SAVE_EXISTS = 5,
    VMU_STATUS_SAVE_EXISTS_NO_SPACE = 6
};

extern char* init_saveNames_8c044d50[11];

void VmMenuMountVms_8c01940e();
void VmMenuUnmountVms_8c0194de();
void VmMenuFreeAndClear_8c019504(void);
int VmMenuUpdateVmusStatus_8c019550(char **saveNames, Uint16 blocks);
void VmMenuUpdateVmuStatus_8c01967c(Sint32 drive, char *saveName, Uint16 blocks);
void VmMenuSwitchFromTask_8c019e44(Task *task);

#endif // _VM_MENU_H
