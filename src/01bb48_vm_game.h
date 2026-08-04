#ifndef _VM_GAME_H
#define _VM_GAME_H

#include <shinobi.h>
#include "015ab8_title.h"

// TODO: Most of these declarations could be
// private once sectionB.h is merged into units

/* =================
 * Type Declarations
 * =================
 */

typedef struct {
    Uint8 data[0x600];
} LcdFrame;

typedef struct {
    Uint32 count;
    LcdFrame data[1]; // Placeholder for variable-length data
} LcdFrames;

/*
 * VMU LCD icon animation.
 */
typedef struct {
    LcdFrames *frames;
    int delay;
    int frameIdx;
} LcdAnim;

/**
 * Async backup operation phase
 */
enum VmGameBupPhase {
    VMGAME_BUP_ERROR        = -1,
    VMGAME_BUP_IDLE         = 0,
    VMGAME_BUP_SAVE_BUSY    = 18,
    VMGAME_BUP_SAVE_DONE    = 19,
    VMGAME_BUP_DEFRAG_BUSY  = 20,
    VMGAME_BUP_DEFRAG_DONE  = 21,
    VMGAME_BUP_LOAD_BUSY    = 22,
    VMGAME_BUP_LOAD_DONE    = 23,
    VMGAME_BUP_REWRITE_BUSY = 24,
    VMGAME_BUP_REWRITE_DONE = 25
};

/* =========
 * Functions
 * =========
 */

void VmGameSwitchToTopMenu_8c01c880(Task *task);
void VmGameResetLcdAnims_8c01c8dc(void);
void VmGameSetLcdSlot_8c01c8fc(int slot);
void VmGameUpdateLcd_8c01c910(void);

#endif // _VM_GAME_H
