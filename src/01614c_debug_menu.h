/* 8c01614c */
#ifndef _01614C_H
#define _01614C_H

#include "014a9c_tasks.h"
#include "014b8c_backup.h"

void DebugMenuOpen_8c01673a(void);
void DebugMenuDemoRecordTask_8c01677e(Task *task, void *state);

/* course-start params: tail of a DebugMenuEntry, handed to 0129cc_game.c's
 * course start through var_debugMenuCourseSel_8c1bc824. */
typedef struct {
    int courseId_0x00;
    int startStopIndex_0x04;
    int driveMode_0x08;
} DebugMenuCourseSel;

/* States of saveMenuTask_8c01628c, the VISUAL_MEMORY entry's VMU picker: it
 * only records which drive a later replay save should use. */
typedef enum {
    DEBUG_SAVE_MENU_INIT = 0,        /* 11-frame settle, then scan for usable drives */
    DEBUG_SAVE_MENU_SELECT = 1,      /* choose a VMU slot */
    DEBUG_SAVE_MENU_CHECK = 2,       /* wait for card ready, validate format/space */
    DEBUG_SAVE_MENU_CONFIRM = 3,     /* free/total blocks; A commits the drive */
    DEBUG_SAVE_MENU_NO_SAVING = 4,   /* "NO SAVING OK?" confirm */
    DEBUG_SAVE_MENU_UNFORMATTED = 5, /* card is not formatted */
    DEBUG_SAVE_MENU_NO_SPACE = 6,    /* not enough free area */
    DEBUG_SAVE_MENU_NO_VMU = 7,      /* no drive can take the save */
    DEBUG_SAVE_MENU_EXIT = 8         /* leave the save flow */
} DebugSaveMenuStateId;

typedef struct {
    DebugSaveMenuStateId state_0x00;
    int selectedVmu_0x04;           /* cursor over slots 0-7 plus the NO SAVING row (8) */
    int port_0x08;                  /* drive chosen with A */
    int frameCounter_0x0c;          /* counts up to 11 in DEBUG_SAVE_MENU_INIT */
    const BACKUPINFO *bupInfo_0x10; /* BupGetInfo result for the selected port */
} SaveMenuState;

void DebugMenuFreeDriveTasks_8c01614c(void);
void DebugMenuFreeSessionAssets_8c016182(void);
void DebugMenuResetDemoCursor_8c016770(void);

#endif // _01614C_H
