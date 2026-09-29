/* @unit StopDraw */

#include <shinobi.h>
#include "sectionB.h"
#include "1ba1c8_globals.h"
#include "02d19c_passenger.h"
#include "02c884_bus_stop.h"

/* ====================
 * Functions
 * ====================
 */

/* The slots come from pickWaitingPassengers_8c02c8ae (02c884). 0x22 and 0x23
 * are the same standing-passenger pair drawPassengerSprite_8c02d19c uses for
 * the two aisle columns. A passenger whose stop index (the byte at
 * *spot_0x00) is past the asset table or whose slot has no texlist loaded
 * yet is skipped. */
void StopDrawWaitingPassengers_8c02d06c(int layer)
{
    int i;
    Sint8 val;
    NJS_TEXLIST *tlist;
    int spriteIndex;

    for (i = 0; i < var_waitingPassengerCount_8c228794; i++) {
        val = *(Sint8 *)var_waitingPassengers_8c228798[i].spot_0x00;
        if (val < 0x41) {
            tlist = var_pedestrianAssets_8c1bbfdc[(int)val].texlist_0x08;
            if (tlist != (NJS_TEXLIST *)-1) {
                var_passengerSprite_8c2288d8.tlist = tlist;
                var_passengerSprite_8c2288d8.p = var_waitingPassengers_8c228798[i].pos_0x04;
                spriteIndex = (layer == 0) ? 0x23 : 0x22;
                njDrawSprite3D(&var_passengerSprite_8c2288d8, spriteIndex, 0x30);
            }
        }
    }
}

/* Sibling of setSimpleLightCallback_8c02a5d0 (028258_objects), but always
 * takes the layer-0 light direction and adds the constant-attr/material
 * setup. var_passengerFadeColor_8c228960's alpha is what makes passengers
 * fade out and back in between waypoints. */
void StopDrawLightBegin_8c02d0fc(int arg0)
{
    njCnkSetSimpleLight(var_fadeLightDir0_8c2264d8[0], var_fadeLightDir0_8c2264d8[1],
                        var_fadeLightDir0_8c2264d8[2]);
    njCnkSetSimpleLightIntensity(var_fadeLightIntensity_8c2264f0[0],
                                 var_fadeLightIntensity_8c2264f0[1]);
    njCnkSetSimpleLightColor(var_fadeLightColor_8c2264f8[0], var_fadeLightColor_8c2264f8[1],
                             var_fadeLightColor_8c2264f8[2]);
    njControl3D(0x120);
    njSetConstantAttr(0xffffffff, 0x100000);
    njSetConstantMaterial((NJS_ARGB *)var_passengerFadeColor_8c228960);
}

void StopDrawLightEnd_8c02d146(int arg0)
{
    njControl3D(0x100);
}
