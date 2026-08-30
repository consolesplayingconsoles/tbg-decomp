/* @unit FadeCmd */
#include <shinobi.h>

#include "011120_asset_queues.h" /* LoadedModel */
#include "013ae8_route_load.h" /* CourseSceneParams */
#include "014a9c_tasks.h"
#include "021b9c.h"
#include "022464_fade.h" /* FadeDrawCommand, FadeCallback1, FadeCallback2 */
#include "sectionB.h"

/* ==========
 * Functions
 * ==========
 */

void FUN_8c0222dc(void)
{
    Task *task;
    LoadedModel *state;

    TaskPush_8c014ae8(var_tasks_8c1ba5e8, TileDrawEnqueueTask_8c0221d0, &task, (void **)&state, 8);
    state->texlist = var_segmentModels_8c1bc3f0->texlist;
    state->njDest = var_segmentModels_8c1bc3f0->njDest;

    var_8c226544[0] = var_sceneParams_8c18ad24->rec1_0x54[0];
    var_8c226544[1] = var_sceneParams_8c18ad24->rec1_0x54[1];
    var_8c22654c[0] = var_sceneParams_8c18ad24->rec1_0x54[2];
    var_8c22654c[1] = var_sceneParams_8c18ad24->rec1_0x54[3];
    var_8c22654c[2] = var_sceneParams_8c18ad24->rec1_0x54[4];

    var_fadeLightIntensity_8c2264f0[0] = var_sceneParams_8c18ad24->rec2_0x74[0];
    var_fadeLightIntensity_8c2264f0[1] = var_sceneParams_8c18ad24->rec2_0x74[1];
    var_fadeLightColor_8c2264f8[0] = var_sceneParams_8c18ad24->rec2_0x74[2];
    var_fadeLightColor_8c2264f8[1] = var_sceneParams_8c18ad24->rec2_0x74[3];
    var_fadeLightColor_8c2264f8[2] = var_sceneParams_8c18ad24->rec2_0x74[4];
}

void FUN_8c02239c(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        var_fadeDrawCommandCount_8c226570[i] = 0;
    }
}

/* Queues a FADE_CMD_5_CALL1 entry for layer (0-2), dropped once that layer's
 * queue (var_fadeDrawCommandCount_8c226570/var_fadeDrawCommands_8c22657c) is full. */
void FadeCmdPushCall1_8c0223ea(int layer, FadeCallback1 fn, int arg0)
{
    FadeDrawCommand *cmd;

    if (var_fadeDrawCommandCount_8c226570[layer] < 0x80) {
        cmd = (FadeDrawCommand *)var_fadeDrawCommands_8c22657c[layer]
            + var_fadeDrawCommandCount_8c226570[layer];
        cmd->type = FADE_CMD_5_CALL1;
        cmd->u.call1.fn = fn;
        cmd->u.call1.arg0 = arg0;
        var_fadeDrawCommandCount_8c226570[layer]++;
    }
}

/* Same as FadeCmdPushCall1_8c0223ea, but for a FADE_CMD_6_CALL2 entry. */
void FadeCmdPushCall2_8c022420(int layer, FadeCallback2 fn, int arg0, int arg1)
{
    FadeDrawCommand *cmd;

    if (var_fadeDrawCommandCount_8c226570[layer] < 0x80) {
        cmd = (FadeDrawCommand *)var_fadeDrawCommands_8c22657c[layer]
            + var_fadeDrawCommandCount_8c226570[layer];
        cmd->type = FADE_CMD_6_CALL2;
        cmd->u.call2.fn = fn;
        cmd->u.call2.arg0 = arg0;
        cmd->u.call2.arg1 = arg1;
        var_fadeDrawCommandCount_8c226570[layer]++;
    }
}
