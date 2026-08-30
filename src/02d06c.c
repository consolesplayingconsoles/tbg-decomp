#include <shinobi.h>
#include "sectionB.h"

/* ====================
 * Functions
 * ====================
 */

/* Draws every already-picked waiting passenger's sprite for the frame
 * (var_8c228798, count var_8c228794 -- see pickWaitingPassengers_8c02c8ae in
 * 02c884). arg0 selects which of two facing sprites to use (0 or 1, matching
 * the two calls FadeCmdPushCall1_8c0223ea registers this under in
 * pedestriansTask_8c0293f6's mirror-view state). A passenger whose stop
 * index (the byte at *spot_0x00) is out of range or whose asset slot has no
 * texlist loaded yet is skipped. */
void FUN_8c02d06c(int arg0)
{
    int i;
    Sint8 val;
    NJS_TEXLIST *tlist;
    int spriteIndex;

    for (i = 0; i < var_8c228794; i++) {
        val = *(Sint8 *)var_8c228798[i].spot_0x00;
        if (val < 0x41) {
            tlist = var_pedestrianAssets_8c1bbfdc[(int)val].texlist_0x08;
            if (tlist != (NJS_TEXLIST *)-1) {
                var_8c2288d8.tlist = tlist;
                var_8c2288d8.p = var_8c228798[i].pos_0x04;
                spriteIndex = (arg0 == 0) ? 0x23 : 0x22;
                njDrawSprite3D(&var_8c2288d8, spriteIndex, 0x30);
            }
        }
    }
}

/* Installed as a FadeCallback1 (via literal-pool pointer in still-asm
 * 02d19c); ignores its arg. Sibling of setSimpleLightCallback_8c02a5d0
 * (028258_objects) but always uses the layer-0 light direction and adds the
 * njControl3D/constant-attr/constant-material setup for the bus-stop
 * anchor-point draws that follow. */
void FUN_8c02d0fc(int arg0)
{
    njCnkSetSimpleLight(var_fadeLightDir0_8c2264d8[0], var_fadeLightDir0_8c2264d8[1], var_fadeLightDir0_8c2264d8[2]);
    njCnkSetSimpleLightIntensity(var_fadeLightIntensity_8c2264f0[0], var_fadeLightIntensity_8c2264f0[1]);
    njCnkSetSimpleLightColor(var_fadeLightColor_8c2264f8[0], var_fadeLightColor_8c2264f8[1], var_fadeLightColor_8c2264f8[2]);
    njControl3D(0x120);
    njSetConstantAttr(0xffffffff, 0x100000);
    njSetConstantMaterial((NJS_ARGB *)var_8c228960);
}

/* Installed as a FadeCallback1 (via literal-pool pointer in still-asm
 * 02d19c); ignores its arg. Restores njControl3D's default flags after
 * FUN_8c02d0fc's draw. */
void FUN_8c02d146(int arg0)
{
    njControl3D(0x100);
}
