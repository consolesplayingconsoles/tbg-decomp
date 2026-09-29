/* @unit DriveCue */
#include "014a9c_tasks.h"
#include "020214_drive_cue_task.h"
#include "sectionB.h"
#include "1ba1c8_globals.h"
#include "020528_drive_cue_init.h"

/* ====================
 * Functions
 * ====================
 */

/* See 020528_drive_cue_init.h. */
void DriveCueInit_8c020528()
{
    Task* created_task;
    void* created_state;

    if (var_playMode_8c1bb8d0 != PLAY_MODE_DEMO) {
        TaskPush_8c014ae8(var_tasks_8c1ba5e8, &DriveCueTask_8c020214, &created_task, &created_state, 0);
        var_driveCueState_8c2264b8.idleChimeState_0x00 = 0;
        var_driveCueState_8c2264b8.idleChimeTimer_0x04 = AsqGetRandomInRangeB_8c0121be(300) + 150;
        var_driveCueState_8c2264b8.stopAnnounceState_0x08 = 3;
        var_driveCueState_8c2264b8.nearStopLatch_0x0c = 1;
        var_driveCueState_8c2264b8.nearStopChimeLatch_0x14 = 0;
        var_driveCueState_8c2264b8.firstChimeArmed_0x18 = 0;
    }
}
