/* @unit Vib */
/* 8c010e90 */
#include <shinobi.h>
#include "010e90.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"

/* === Workarounds === */
/* TODO */
const PDS_PERIPHERAL const_peripheral_8c033318 = {0};

/* === Structs === */
struct VibPlaybackState {
    int index_0x00;
    int frameCounter_0x04;
    int index_0x08;
    int active_0x0c;
}
typedef VibPlaybackState;

struct VibStep {
    int durationFrames_0x00;
    Uint8 flag;
    Uint8 power;
    Uint8 freq;
}
typedef VibStep;

VibStep init_vib_8c03bdac[] = {
    {0x14, 0x01, 0xFF, 0x0F},
    {0x64, 0x01, 0xFE, 0x1E},
    {0x00, 0x00, 0x00, 0x00}
};

VibStep init_vib_8c03bdc4[] = {
    {0x1E, 0x01, 0x03, 0x1E},
    {0x14, 0x01, 0x00, 0x00},
    {0x1E, 0x01, 0x03, 0x1E},
    {0x00, 0x00, 0x00, 0x00}
};

VibStep init_vib_8c03bde4[] = {
    {0x14, 0x01, 0x03, 0x1E},
    {0x00, 0x00, 0x00, 0x00}
};

VibStep init_vib_8c03bdf4[] = {
    {0x0C, 0x01, 0xF9, 0x0F},
    {0x00, 0x00, 0x00, 0x00}
};

VibStep init_vib_8c03be04[] = {
    {0x0A, 0x01, 0x03, 0x0F},
    {0x0A, 0x01, 0x02, 0x14},
    {0x00, 0x00, 0x00, 0x00}
};

VibStep init_vib_8c03be1c[] = {
    {0x0F, 0x01, 0x04, 0x1E},
    {0x0F, 0x01, 0x03, 0x14},
    {0x00, 0x00, 0x00, 0x00}
};

VibStep init_vib_8c03be34[] = {
    {0x19, 0x01, 0x07, 0x28},
    {0x0A, 0x01, 0x05, 0x1E},
    {0x00, 0x00, 0x00, 0x00}
};

VibStep init_vib_8c03be4c[] = {
    {0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00}
};

/* === External vars === */
VibStep* init_unknownVibStructBArray_8c03be5c[] = {
    init_vib_8c03bdac,
    init_vib_8c03bdc4,
    init_vib_8c03bde4,
    init_vib_8c03bdf4,
    init_vib_8c03be04,
    init_vib_8c03be1c,
    init_vib_8c03be34,
    init_vib_8c03be4c
};

/* === Uninitialized vars === */
VibPlaybackState var_unknownVibStructA_8c157a48;

NM_STATIC void vib_8c010e90(int port) {
    PDS_VIBPARAM param;
    VibStep* unknownVibStructB;

    unknownVibStructB = init_unknownVibStructBArray_8c03be5c[var_unknownVibStructA_8c157a48.index_0x00];

    if (!var_unknownVibStructA_8c157a48.index_0x08) {
        param.unit = 1;

        param.flag = unknownVibStructB[var_unknownVibStructA_8c157a48.index_0x08].flag;
        param.power = unknownVibStructB[var_unknownVibStructA_8c157a48.index_0x08].power;
        param.freq = unknownVibStructB[var_unknownVibStructA_8c157a48.index_0x08].freq;

        param.inc = 0;

        pdVibMxStart(port, &param);

        var_unknownVibStructA_8c157a48.index_0x08++;
        var_unknownVibStructA_8c157a48.active_0x0c = 1;
    } else if (unknownVibStructB[var_unknownVibStructA_8c157a48.index_0x08 - 1].durationFrames_0x00 < var_unknownVibStructA_8c157a48.frameCounter_0x04) {
        pdVibMxStop(port);

        var_unknownVibStructA_8c157a48.frameCounter_0x04 = 0;

        if (unknownVibStructB[var_unknownVibStructA_8c157a48.index_0x08].durationFrames_0x00 == 0) {
            VibClear_8c010fbe();
        } else {
            param.unit = 1;

            param.flag = unknownVibStructB[var_unknownVibStructA_8c157a48.index_0x08].flag;
            param.power = unknownVibStructB[var_unknownVibStructA_8c157a48.index_0x08].power;
            param.freq = unknownVibStructB[var_unknownVibStructA_8c157a48.index_0x08].freq;

            param.inc = 0;

            while (1) {
                int r = pdVibMxStart(port, &param);
                if (!r)
                    break;
            }

            var_unknownVibStructA_8c157a48.index_0x08++;
        }
    }

    if (var_unknownVibStructA_8c157a48.active_0x0c == 1) {
        var_unknownVibStructA_8c157a48.frameCounter_0x04++; 
    }
}

void VibStart_8c010f7a(int param) {
    if (param < 8) {
        if (var_unknownVibStructA_8c157a48.active_0x0c == 1) {
            if (param > var_unknownVibStructA_8c157a48.index_0x00) {
                var_unknownVibStructA_8c157a48.index_0x00 = param;
            }
        } else if (var_unknownVibStructA_8c157a48.active_0x0c == 0) {
            VibClear_8c010fbe();
            var_unknownVibStructA_8c157a48.index_0x00 = param;
        }
    }
}

void VibStop_8c010fae(int port) {
    if (var_unknownVibStructA_8c157a48.index_0x00 != 7) {
        vib_8c010e90(port);
    }
}

void VibClear_8c010fbe() {
    memset(&var_unknownVibStructA_8c157a48, 0, sizeof(VibPlaybackState));
    var_unknownVibStructA_8c157a48.index_0x00 = 7;
}
