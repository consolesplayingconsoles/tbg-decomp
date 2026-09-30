/* 8c018644: FILE SELECT screen, plus the PlayerProgress reset helpers OPTION shares. */
#ifndef _018644_FILE_SELECT_H
#define _018644_FILE_SELECT_H

#include "014a9c_tasks.h"

void FileSelectFreeBuffers_8c0187d0(void);
int FileSelectIsSaveValid_8c018804(int *save);
void FileSelectResetSettingDefaults_8c018862(void);
void FileSelectResetKeyConfigDefaults_8c0188bc(void);
void FileSelectResetSoundDefaults_8c0188dc(void);
void FileSelectResetProgress_8c01890a(void);
void FileSelectResetNewGame_8c01895e(void);
void FileSelectResetOptionDefaults_8c0189d2(void);
void FileSelectApplySoundSettings_8c0189fc(void);
void FileSelectSwitchFromTask_8c019334(Task *task);

#endif // _018644_FILE_SELECT_H
