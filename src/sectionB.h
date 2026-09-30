/* 8c228564: undecompiled data section */
#ifndef _228564_SECTIONB_H
#define _228564_SECTIONB_H

#include <shinobi.h>
#include "01614c_replay_menu.h"
#include "013ae8_route_load.h"
#include "02af78_event.h"
#include "011120_asset_queues.h"
#include "014a9c_tasks.h"
#include "014b8c_backup.h"
#include "015034_text.h"

/* =================
 * Type Declarations
 * =================
 */

/* One driver-comment banner. `count`/`ids` point into a {count, id...} row
 * of init_penaltyMsgGlyphs_8c04c35c (02b464); the ids are 16x16-atlas glyph
 * indices. [0] is the newest message, [1..3] older ones shifted back as each
 * new one arrives. Typed out one glyph per two frames, then held 60. */
typedef struct {
    int count;
    int *ids;
    float x;          /* row's left edge, centered: (640 - 32 * count) / 2 */
    int revealed;     /* glyphs typed out so far; revealCounter >> 1 */
    int revealCounter;
    int holdFrames;   /* counts down once fully revealed; 0 = slot free */
} DriveMsgSlot;
extern DriveMsgSlot var_driveMsgQueue_8c228564[4];

#endif // _228564_SECTIONB_H
