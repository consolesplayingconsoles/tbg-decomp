/* 8c228478: undecompiled data section */
#ifndef _228478_SECTIONB_H
#define _228478_SECTIONB_H

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

extern int var_messageBoxActive_8c22847c;

/* One entry of var_eventSlides_8c228480[event]/state->slide_0x10, terminated
 * by an entry whose layers_0x00 is (unsigned short *)-1. layers_0x00 is a base
 * image id plus 0-2 overlay ids, all drawn in the same frame in `pr` order.
 * lineListIndex_0x04 selects the line list out of var_messageTextDat_8c228518
 * (an array of EventLine*). */
typedef struct {
    unsigned short *layers_0x00;
    int lineListIndex_0x04;
} EventSlide;

/* Per-event slide table for the route selected by
 * MessageBoxRequestAssets_8c02aa36: init_shinjukuEvents_8c049a6c / init_wanganEvents_8c04843c /
 * init_omeEvents_8c04a9c8, indexed by var_selectedEventEntry_8c228478. */
extern EventSlide **var_eventSlides_8c228480;

/* Dedup table of message pvm/dat assets requested by
 * MessageBoxRequestAssets_8c02aa36, one entry per distinct id seen
 * across the selected event's slides; count in var_messageAssetCount_8c228514. */
typedef struct {
    int id_0x00;
    void *pvm_0x04;
    void *dat_0x08;
} MessageAssetEntry;
extern MessageAssetEntry var_messageAssets_8c228484[12];

extern int var_messageAssetCount_8c228514;

/* One line of dialogue within a slide, walked by state->line_0x18; terminated
 * by an entry whose text_0x00 points at an empty string. */
typedef struct {
    char *text_0x00;
    int voiceId_0x04;
} EventLine;
extern EventLine **var_messageTextDat_8c228518;

/* unlock-candidate scratch list built by EventScanCandidates_8c02b03c;
 * var_routeEvents_8c22851c points at the active route's EventEntry table */
extern EventEntry* var_routeEvents_8c22851c;
extern int var_eventCandidates_8c228520[];
extern int var_eventCandidateCount_8c228560;

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

/* table index (into the EventEntry array pointed to by var_routeEvents_8c22851c)
 * chosen by EventPickForSegment_8c02b170, consumed by
 * EventApplyFlags_8c02b292 */
extern int var_selectedEventEntry_8c228478;

#endif // _228478_SECTIONB_H
