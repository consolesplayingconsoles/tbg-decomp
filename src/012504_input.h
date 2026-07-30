/* 8c012504 */
#ifndef _INPUT_H
#define _INPUT_H

/*
 * Peripheral device-support masks identifying controller type (PDS info->support,
 * mirrored into var_activeCtrlType_8c157a70): BT_CONTROLLER = 0xf06fe (Dreamcast
 * pad), BT_RACING = 0x700fe (racing controller).
 */
#define BT_CONTROLLER   (PDD_DEV_SUPPORT_TA | PDD_DEV_SUPPORT_TB | PDD_DEV_SUPPORT_TX | \
                         PDD_DEV_SUPPORT_TY | PDD_DEV_SUPPORT_ST | PDD_DEV_SUPPORT_KU | \
                         PDD_DEV_SUPPORT_KD | PDD_DEV_SUPPORT_KL | PDD_DEV_SUPPORT_KR | \
                         PDD_DEV_SUPPORT_AX1 | PDD_DEV_SUPPORT_AY1 | PDD_DEV_SUPPORT_AL | \
                         PDD_DEV_SUPPORT_AR)

#define BT_RACING       (PDD_DEV_SUPPORT_TA | PDD_DEV_SUPPORT_TB | PDD_DEV_SUPPORT_ST | \
                         PDD_DEV_SUPPORT_KU | PDD_DEV_SUPPORT_KD | PDD_DEV_SUPPORT_KL | \
                         PDD_DEV_SUPPORT_KR | PDD_DEV_SUPPORT_AX1 | PDD_DEV_SUPPORT_AL | \
                         PDD_DEV_SUPPORT_AR)

void InputPushTask_8c0128cc(int param);
void InputDispatchTask_8c012970(void);
int InputSetName_8c012984(void);

#endif // _INPUT_H
