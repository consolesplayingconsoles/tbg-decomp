/* @unit StopSpawn */

#include <shinobi.h>
#include "serial_debug.h"
#include "sectionB.h"
#include "014a9c_tasks.h"
#include "02d19c.h"
#include "025870.h"

/* ====================
 * Type Declarations
 * ====================
 */

/* local_44's element type: one entry per var_8c228718 slot sharing the
 * bus's current segment, built up before being Fisher-Yates shuffled and
 * spawned via BusRiderAlightTask_8c02d46c. */
typedef struct {
    void *entry_0x00;
    int index_0x04;
} SegMatchEntry;

/* ====================
 * Initialized Globals
 * ====================
 */

/* Per-stop-type animation-variant lookup, indexed by a scripted schedule
 * entry's type byte; consumed by StopSpawnInit_8c02d968 to offset into a passenger
 * sprite/anim table (added to 0x32 or 0x36 depending on spawn path). */
STATIC Uint8 init_8c04c4dc[] = {
    0x03, 0x03, 0x02, 0x01, 0x00, 0x00, 0x02, 0x01,
    0x02, 0x01, 0x00, 0x03, 0x02, 0x00, 0x00, 0x00,
    0x02, 0x03, 0x01, 0x00, 0x01, 0x01, 0x02, 0x00,
    0x01, 0x03, 0x03, 0x03, 0x03, 0x02, 0x02, 0x00,
    0x00, 0x02, 0x02, 0x03, 0x02, 0x02, 0x01, 0x02,
    0x02, 0x03, 0x03, 0x03, 0x01, 0x01, 0x02, 0x01,
    0x02, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01,
    0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01,
    0x03, 0x00, 0x00, 0x00,
};

/* ====================
 * Functions
 * ====================
 */

/* Course-start setup for the bus-stop passenger subsystem: sets up the six
 * bus-interior anchor points (var_8c228910/91c/928/934/940/94c), then
 * spawns one task per scripted stop-schedule slot (var_8c228718) and one
 * per already-picked waiting passenger (var_8c228798). Called once by
 * FUN_8c012f44 (012f44_game) at course start. */
void StopSpawnInit_8c02d968(void)
{
    Task *task;
    StopScheduleState *state;
    Uint32 *rawState;
    int i, n, count, quadrant, randIdx;
    void *scriptEntry;
    void *matchedBuf;
    SegMatchEntry *matched;
    SegMatchEntry tmp;

    if (var_playMode_8c1bb8d0 == 1 && (var_8c226410 & 8) != 8) {
        TaskPush_8c014ae8(var_tasks_8c1ba5e8, &BusRiderSkipStopTask_8c02d8f0, &task, (void **)&state, 0);
        var_8c2285c4[0] = 2;
        return;
    }

    TaskPush_8c014ae8(var_tasks_8c1ba5e8, &BusRiderStopSceneTask_8c02d644, &task, (void **)&rawState, 8);
    rawState[0] = 0;
    rawState[1] = 0;

    njSetTexture(var_interiorTexlist_8c1bc438);
    njLoadCacheTexture(var_interiorTexlist_8c1bc438);
    FUN_8c025870();

    /* Six bus-interior anchor points, set up in bus-local space and (for
     * var_8c228928 and var_8c22894c only) immediately transformed to world
     * space in place. Each point is written x, then y, then z -- verified
     * against the real instruction order, which does NOT match Ghidra's
     * statement order for this block. */
    if (var_route_8c18ad1c == ROUTE_SHINJUKU || var_route_8c18ad1c == ROUTE_WANGAN) {
        var_8c228928.x = -1.6f;
        var_8c228928.y = 0.0f;
        var_8c228928.z = 0.9f;
        njCalcPoint(&var_busWorldMatrix_8c1bba54, &var_8c228928, &var_8c228928);

        var_8c228910.x = 0.15f;
        var_8c228910.y = 0.68f;
        var_8c228910.z = 0.27f;

        var_8c22891c.x = 0.128f;
        var_8c22891c.y = 0.68f;
        var_8c22891c.z = 0.76f;

        var_8c228934.x = 0.35f;
        var_8c228934.y = 0.68f;
        var_8c228934.z = 3.0f;

        var_8c228940.x = 1.0f;
        var_8c228940.y = 0.35f;
        var_8c228940.z = 2.8f;

        var_8c22894c.x = -1.39f;
        var_8c22894c.y = 0.0f;
        var_8c22894c.z = 2.63f;
    } else if (var_route_8c18ad1c == ROUTE_OME) {
        var_8c228928.x = -1.39f;
        var_8c228928.y = 0.0f;
        var_8c228928.z = 2.63f;
        njCalcPoint(&var_busWorldMatrix_8c1bba54, &var_8c228928, &var_8c228928);

        var_8c228910.x = 1.0f;
        var_8c228910.y = 0.35f;
        var_8c228910.z = 2.8f;

        var_8c22891c.x = 0.35f;
        var_8c22891c.y = 0.68f;
        var_8c22891c.z = 3.0f;

        var_8c228934.x = 0.128f;
        var_8c228934.y = 0.68f;
        var_8c228934.z = 0.76f;

        var_8c228940.x = 0.15f;
        var_8c228940.y = 0.68f;
        var_8c228940.z = 0.27f;

        var_8c22894c.x = -1.6f;
        var_8c22894c.y = 0.0f;
        var_8c22894c.z = 0.9f;
    } else {
        goto skip_anchor_points;
    }

    njCalcPoint(&var_busWorldMatrix_8c1bba54, &var_8c22894c, &var_8c22894c);

skip_anchor_points:
    var_8c22895c = 0;
    var_8c228960[0] = 0.0f;
    var_8c228960[1] = 1.0f;
    var_8c228960[2] = 1.0f;
    var_8c228960[3] = 1.0f;

    /* Count how many scripted schedule slots are in use, to size the task
     * group allocation for both this loop and the var_8c228798 loop below. */
    n = 0;
    for (i = 0; i < 31; i++) {
        if (var_8c228718[i] != -1) {
            n++;
        }
    }

    var_stopTaskGroup_8c2288f8 = (void *)syMalloc((var_8c228794 + n + 1) * 0x20);
    TaskClear_8c014a9c(var_stopTaskGroup_8c2288f8, var_8c228794 + n);

    /* Spawn a task for every already-picked waiting passenger. */
    for (i = 0; i < var_8c228794; i++) {
        TaskPush_8c014ae8((Task *)var_stopTaskGroup_8c2288f8, &BusRiderBoardTask_8c02d21c, &task,
                           (void **)&state, sizeof(StopScheduleState));
        state->entry_0x00 = var_8c228798[i].spot_0x00;
        state->state_0x04 = 1;
        state->pos_0x08 = var_8c228798[i].pos_0x04;
        state->jitterX_0x18 = ((float)rand() / 32768.0f) * 0.2f;
        state->jitterZ_0x1c = ((float)rand() / 32768.0f) * 0.2f;
        state->delay_0x20 = i;
        state->isSeated_0x28 = 0;
        quadrant = i % 4;
        state->voice_0x2c = quadrant;
        state->soundIdA_0x30 = init_8c04c4dc[*(char *)state->entry_0x00] + 0x32;
        state->soundIdB_0x34 = state->soundIdA_0x30 + 4;
        var_passengerCount_8c1bb8e4++;
    }

    /* Walk the scripted schedule table. A slot whose stop segment differs
     * from the bus's current segment is spawned immediately (BusRiderSeatedTask_8c02d5ca);
     * a slot that matches is collected into matchedBuf to be shuffled and
     * spawned below (BusRiderAlightTask_8c02d46c) instead. */
    matchedBuf = (void *)syMalloc(0xf8);
    matched = (SegMatchEntry *)matchedBuf;
    count = 0;
    for (i = 0; i < 0x1f; i++) {
        if (var_8c228718[i] == -1) {
            continue;
        }
        scriptEntry = (void *)var_8c228718[i];
        if (*((char *)scriptEntry + 1) == (char)var_currentSegment_8c228708) {
            matched[count].entry_0x00 = scriptEntry;
            matched[count].index_0x04 = i;
            count++;
        } else {
            TaskPush_8c014ae8((Task *)var_stopTaskGroup_8c2288f8, &BusRiderSeatedTask_8c02d5ca, &task,
                               (void **)&state, sizeof(StopScheduleState));
            state->entry_0x00 = scriptEntry;
            state->pos_0x08.x = init_seatPositions_8c04c3e4[i].x;
            state->pos_0x08.y = var_8c228934.y;
            state->pos_0x08.z = init_seatPositions_8c04c3e4[i].z;
            if (i < 0x14) {
                state->spriteNo_0x14 = 0x20;
                state->pos_0x08.y -= 0.18f;
            } else if (i < 0x1a) {
                state->spriteNo_0x14 = 0x22;
            } else {
                state->spriteNo_0x14 = 0x23;
            }
            state->isSeated_0x28 = 1;
        }
    }

    /* Fisher-Yates shuffle the matched entries. */
    for (i = 0; i < count; i++) {
        randIdx = AsqGetRandomInRangeA_8c012178(count);
        tmp = matched[i];
        matched[i] = matched[randIdx];
        matched[randIdx] = tmp;
    }

    /* Spawn the shuffled matched-segment slots. */
    for (i = 0; i < count; i++) {
        TaskPush_8c014ae8((Task *)var_stopTaskGroup_8c2288f8, &BusRiderAlightTask_8c02d46c, &task,
                           (void **)&state, sizeof(StopScheduleState));
        state->entry_0x00 = matched[i].entry_0x00;
        state->state_0x04 = 0;
        state->pos_0x08.x = init_seatPositions_8c04c3e4[i].x;
        state->pos_0x08.y = var_8c228934.y;
        state->pos_0x08.z = init_seatPositions_8c04c3e4[i].z;
        if (i < 0x14) {
            state->spriteNo_0x14 = 0x20;
            state->pos_0x08.y -= 0.18f;
        } else if (i < 0x1a) {
            state->spriteNo_0x14 = 0x22;
        } else {
            state->spriteNo_0x14 = 0x23;
        }
        state->jitterX_0x18 = ((float)rand() / 32768.0f) * 0.2f;
        state->jitterZ_0x1c = ((float)rand() / 32768.0f) * 0.2f;
        state->delay_0x20 = i;
        state->slotIndex_0x24 = matched[i].index_0x04;
        state->isSeated_0x28 = 1;
        quadrant = i % 4;
        state->voice_0x2c = quadrant;
        state->soundIdA_0x30 = init_8c04c4dc[*(char *)state->entry_0x00] + 0x36;
        state->soundIdB_0x34 = state->soundIdA_0x30 - 8;
    }

    syFree(matchedBuf);
}
