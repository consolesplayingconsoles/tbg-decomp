/* 8c018644: FILE SELECT screen, plus the PlayerProgress reset helpers OPTION shares. */
#ifndef _018644_FILE_MENU_H
#define _018644_FILE_MENU_H

#include "014a9c_tasks.h"

void FileMenuFreeBuffers_8c0187d0(void);
int FileMenuIsSaveValid_8c018804(int *save);
void FileMenuResetSettingDefaults_8c018862(void);
void FileMenuResetKeyConfigDefaults_8c0188bc(void);
void FileMenuResetSoundDefaults_8c0188dc(void);
void FileMenuResetProgress_8c01890a(void);
void FileMenuResetNewGame_8c01895e(void);
void FileMenuResetOptionDefaults_8c0189d2(void);
void FileMenuApplySoundSettings_8c0189fc(void);
void FileMenuSwitchFromTask_8c019334(Task *task);

#endif // _018644_FILE_MENU_H
