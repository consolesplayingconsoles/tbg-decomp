/* @unit Hud */
#include <shinobi.h>

#include "01fa78.h"
#include "014a9c_tasks.h"
#include "014f54_text.h"
#include "0222dc_fadecmd.h"
#include "028258_objects.h"
#include "sectionB.h"
#include "serial_debug.h"

/* ====================
 * Initialized Globals
 * ====================
 */

/* Driver-points meter fill quad: 4 DrawVertex8c226478-shaped vertices
 * (x,y,z,color), left edge fixed at x=38.0 (vertices 0/1), right edge
 * (vertices 2/3, offsets 0x20/0x30) overwritten every frame by
 * drawHud_8c01fbac. Kept as raw bytes to stay byte-identical with the asm. */
STATIC Uint8 init_8c045334[] = {
    0x00, 0x00, 0x18, 0x42, 0x00, 0x00, 0xD8, 0x43,
    0xFB, 0x91, 0x53, 0x3F, 0x00, 0xF4, 0x00, 0xFF,
    0x00, 0x00, 0x18, 0x42, 0x00, 0x00, 0xDE, 0x43,
    0xFB, 0x91, 0x53, 0x3F, 0x00, 0xF4, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x43,
    0xFB, 0x91, 0x53, 0x3F, 0x00, 0xF4, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDE, 0x43,
    0xFB, 0x91, 0x53, 0x3F, 0x00, 0xF4, 0x00, 0xFF,
};

STATIC Uint8 init_8c045374[] = {
    0x00, 0x00, 0x18, 0x42, 0x00, 0x00, 0xD8, 0x43,
    0x4D, 0x21, 0x50, 0x3F, 0x00, 0x00, 0xB8, 0xFF,
    0x00, 0x00, 0x18, 0x42, 0x00, 0x00, 0xDE, 0x43,
    0x4D, 0x21, 0x50, 0x3F, 0x00, 0x00, 0xB8, 0xFF,
    0x00, 0x00, 0x70, 0x43, 0x00, 0x00, 0xD8, 0x43,
    0x4D, 0x21, 0x50, 0x3F, 0x00, 0x00, 0xB8, 0xFF,
    0x00, 0x00, 0x60, 0x43, 0x00, 0x00, 0xDE, 0x43,
    0x4D, 0x21, 0x50, 0x3F, 0x00, 0x00, 0xB8, 0xFF,
};

STATIC Uint8 init_8c0453b4[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xD4, 0x43,
    0x9D, 0x73, 0x4E, 0x3F, 0x00, 0x00, 0x00, 0x80,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xEF, 0x43,
    0x9D, 0x73, 0x4E, 0x3F, 0x00, 0x00, 0x00, 0x80,
    0x00, 0x00, 0x72, 0x43, 0x00, 0x80, 0xD4, 0x43,
    0x9D, 0x73, 0x4E, 0x3F, 0x00, 0x00, 0x00, 0x80,
    0x00, 0xC0, 0x1F, 0x44, 0x00, 0x80, 0xEF, 0x43,
    0x9D, 0x73, 0x4E, 0x3F, 0x00, 0x00, 0x00, 0x80,
    0x00, 0x80, 0x90, 0x43, 0x00, 0x00, 0xC1, 0x43,
    0x9D, 0x73, 0x4E, 0x3F, 0x00, 0x00, 0x00, 0x80,
    0x00, 0xC0, 0x1F, 0x44, 0x00, 0x00, 0xC1, 0x43,
    0x9D, 0x73, 0x4E, 0x3F, 0x00, 0x00, 0x00, 0x80,
};

/* 3 NJS_POINT3 local-space corners of the rotating tachometer needle triangle
 * (drawHud_8c01fbac), njCalcPoint'd through var_8c1bc46c into var_8c226478. */
STATIC Uint8 init_8c045414[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0,
    0x08, 0xD6, 0x51, 0x3F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x40, 0x08, 0xD6, 0x51, 0x3F,
    0x00, 0x00, 0x20, 0xC2, 0x00, 0x00, 0x00, 0x00,
    0x08, 0xD6, 0x51, 0x3F,
};

/* ====================
 * Forward Declarations
 * ====================
 */

STATIC void drawSpeedAndTimers_8c01fe84(Sint32 speed);

/* ====================
 * Functions
 * ====================
 */

STATIC void showMark_8c01fa78(int a, int b) {
    var_8c2264a8.field_0x00 = a;
    var_8c2264a8.field_0x08 = b;
}

/* Draws a 6-digit HH:MM:SS readout from a 30fps frame count (108000 = 1h,
 * 1800 = 1min, 30 = 1sec), tens-then-units per field. */
STATIC void drawTimeDigits_8c01fa80(int frames, float y, int spriteBase) {
    int hours = frames / 108000;
    int minutes = (frames % 108000) / 1800;
    int seconds = (frames % 1800) / 30;

    TxtDrawSprite_8c014f54(&var_busStopTexlist_8c1bc424, spriteBase + hours % 10, 448.0f, y, -1.21f);
    TxtDrawSprite_8c014f54(&var_busStopTexlist_8c1bc424, spriteBase + hours / 10, 436.0f, y, -1.21f);
    TxtDrawSprite_8c014f54(&var_busStopTexlist_8c1bc424, spriteBase + minutes % 10, 477.0f, y, -1.21f);
    TxtDrawSprite_8c014f54(&var_busStopTexlist_8c1bc424, spriteBase + minutes / 10, 465.0f, y, -1.21f);
    TxtDrawSprite_8c014f54(&var_busStopTexlist_8c1bc424, spriteBase + seconds % 10, 506.0f, y, -1.21f);
    TxtDrawSprite_8c014f54(&var_busStopTexlist_8c1bc424, spriteBase + seconds / 10, 494.0f, y, -1.21f);
}

/* Per-frame in-drive HUD renderer: driver-comment popup icons (var_8c2264a8,
 * plus arg0 -- the message code hudUpdateTask_8c01ff48 stages through the fade-command
 * queue), the next-stop icon (blinking once armed), the driver-points meter
 * bar, both turn-signal icons, a rotating tachometer needle driven by the
 * steering/lane state machine (var_engineRpm_8c226468's 500-unit ramp), and the speed
 * readout + two timers drawn by the drawSpeedAndTimers_8c01fe84 tail. Installed as a
 * FadeCallback1 and as a TaskPush_8c014ae8 action, so it must keep this
 * exact signature. */
STATIC void drawHud_8c01fbac(int arg0) {
    float barWidth, barWidthInner;
    Angle angle;
    Sint32 speed;

    if (var_playMode_8c1bb8d0 == PLAY_MODE_DEMO) {
        return;
    }

    if (var_8c2264a8.field_0x08 != 0) {
        TxtDrawSprite_8c014f54(&var_markTexlist_8c1bc418, var_8c2264a8.field_0x00, 0.0f, 0.0f, -1.21f);
    }
    if (var_8c2264a8.field_0x04 != 0) {
        TxtDrawSprite_8c014f54(&var_markTexlist_8c1bc418, var_8c2264a8.field_0x04, 0.0f, 0.0f, -1.2f);
    }
    if (arg0 != 0) {
        TxtDrawSprite_8c014f54(&var_markTexlist_8c1bc418, arg0, 0.0f, 0.0f, -1.21f);
    }

    /* var_8c226450/var_8c226454 (next-stop icon + its blink timer) are owned by
     * BusStopUpdateArrival_8c02ce48 (02c884); blink once armed for >1s or
     * every few frames (bits 1/2 of the timer). */
    if (var_stopPhase_8c2285e4 == 0 || var_stopPhase_8c2285e4 == 4) {
        if (var_8c226450 != -1 && (60 < (Sint32)var_8c226454 || (var_8c226454 & 6) != 0)) {
            TxtDrawSprite_8c014f54(&var_busStopTexlist_8c1bc424, var_8c226450, 0.0f, 0.0f, -1.2f);
        }
    } else if (var_stopPhase_8c2285e4 == 1) {
        if (var_8c226450 != -1 && (60 < (Sint32)var_8c226454 || (var_8c226454 & 6) != 0)) {
            TxtDrawSprite_8c014f54(&var_busStopTexlist_8c1bc424, 0x1e, 0.0f, 0.0f, -1.2f);
        }
    } else if (var_stopPhase_8c2285e4 == 2) {
        if (60 < (Sint32)var_8c226454 || (var_8c226454 & 6) != 0) {
            TxtDrawSprite_8c014f54(&var_busStopTexlist_8c1bc424, 0x1f, 0.0f, 0.0f, -1.2f);
        }
    }

    /* Moving right edge (vertices 2/3) of the driver-points meter's fill
     * quad -- init_8c045334's own vertex 0/1 (x=38.0) is the fixed left
     * edge; a second, narrower quad trails 16px behind, clamped to the
     * same left edge. */
    barWidth = (var_8c226458.field_0x00 * 202.0f) / (float)var_8c2285d4 + 38.0f;
    barWidthInner = barWidth - 16.0f;
    if (barWidthInner < 38.0f) {
        barWidthInner = 38.0f;
    }
    *(float *)&init_8c045334[0x20] = barWidth;
    *(float *)&init_8c045334[0x30] = barWidthInner;
    njDrawPolygon((NJS_POLYGON_VTX *)init_8c045334, 4, 0);
    njDrawPolygon((NJS_POLYGON_VTX *)init_8c045374, 4, 0);
    njDrawPolygon((NJS_POLYGON_VTX *)init_8c0453b4, 6, 1);

    if (var_busState_8c1bb9d0.blinker_0x080 & 2) {
        TxtDrawSprite_8c014f54(&var_busStopTexlist_8c1bc424, 0x25, 0.0f, 0.0f, -1.21f);
    }
    if (var_busState_8c1bb9d0.blinker_0x080 & 4) {
        TxtDrawSprite_8c014f54(&var_busStopTexlist_8c1bc424, 0x26, 0.0f, 0.0f, -1.21f);
    }

    /* Rotating tachometer needle position: field_0x2e0 selects a mode (0 = relax
     * toward 0, 1 = settle to 500, 2 = ramp toward target_0x2e8),
     * each stepping var_engineRpm_8c226468 by 200.0f/frame and clamping on
     * overshoot. */
    switch (var_busState_8c1bb9d0.field_0x2e0) {
    case 0:
        if (var_engineRpm_8c226468 > 0.0f) {
            var_engineRpm_8c226468 -= 200.0f;
            if (var_engineRpm_8c226468 < 0.0f) {
                var_engineRpm_8c226468 = 0.0f;
            }
        }
        break;

    case 1:
        if (var_engineRpm_8c226468 > 500.0f) {
            var_engineRpm_8c226468 -= 200.0f;
            if (var_engineRpm_8c226468 >= 500.0f) break;
            var_engineRpm_8c226468 = 500.0f;
        } else {
            var_engineRpm_8c226468 += 200.0f;
            if (var_engineRpm_8c226468 <= 500.0f) break;
            var_engineRpm_8c226468 = 500.0f;
        }
        break;

    case 2: {
        float target = var_busState_8c1bb9d0.target_0x2e8;
        if (target > 500.0f) {
            if (target < var_engineRpm_8c226468) {
                var_engineRpm_8c226468 -= 200.0f;
                if (target <= var_engineRpm_8c226468) break;
            } else {
                var_engineRpm_8c226468 += 200.0f;
                if (var_engineRpm_8c226468 <= target) break;
            }
            var_engineRpm_8c226468 = target;
            break;
        }
        var_engineRpm_8c226468 = 500.0f;
        break;
    }

    default:
        break;
    }

    njUnitMatrix(&var_8c1bc46c);
    njTranslate(&var_8c1bc46c, 320.0f, 436.0f, 0.0f);
    angle = ((Sint32)var_engineRpm_8c226468 * 32768) / 6000;
    njRotateZ(&var_8c1bc46c, angle);
    njCalcPoint(&var_8c1bc46c, (NJS_POINT3 *)&init_8c045414[0], (NJS_POINT3 *)&var_8c226478[0]);
    njCalcPoint(&var_8c1bc46c, (NJS_POINT3 *)&init_8c045414[12], (NJS_POINT3 *)&var_8c226478[1]);
    njCalcPoint(&var_8c1bc46c, (NJS_POINT3 *)&init_8c045414[24], (NJS_POINT3 *)&var_8c226478[2]);
    njDrawPolygon((NJS_POLYGON_VTX *)var_8c226478, 3, 0);

    /* Speed readout (var_busState_8c1bb9d0.speed_0x27c converted to a
     * display unit) plus the two elapsed/remaining timers, split out to
     * drawSpeedAndTimers_8c01fe84 -- see that function for the digit layout. */
    speed = (Sint32)((var_busState_8c1bb9d0.speed_0x27c * 108000.0f) / 1000.0f);
    drawSpeedAndTimers_8c01fe84(speed);
}

/* Tail of drawHud_8c01fbac's HUD render, split out because the original asm
 * falls through into this label from drawHud_8c01fbac with no other reference
 * to it anywhere in the tree (see drawHud_8c01fbac). Draws the 2-digit speed
 * value (its sign discarded) and the two elapsed-time readouts.
 *
 * No standalone test file: the asm label has no real prologue of its own
 * (it inherits drawHud_8c01fbac's), so a direct call in the .src object reads
 * its argument from a stale R9 instead of R4 and its epilogue restores an
 * unestablished stack frame -- there is no way to invoke it in isolation
 * against that object. drawHud_8c01fbac's own tests exercise this function's
 * real behavior in both objects: as a genuine call in the C object, and
 * (since the .src object can't have it mocked away either) via inline
 * assertions in that same file's assertSpeedTail() for the .src object. */
STATIC void drawSpeedAndTimers_8c01fe84(Sint32 speed) {
    Sint32 units, tens;

    if (speed < 0) {
        speed = -speed;
    }

    units = speed % 10;
    TxtDrawSprite_8c014f54(&var_busStopTexlist_8c1bc424, units, 320.0f, 420.0f, -1.21f);
    tens = speed / 10;
    TxtDrawSprite_8c014f54(&var_busStopTexlist_8c1bc424, tens, 308.0f, 420.0f, -1.21f);

    drawTimeDigits_8c01fa80(var_8c2285d8, 402.0f, 10);
    drawTimeDigits_8c01fa80(var_8c2285dc, 423.0f, 20);

    TxtDrawSprite_8c014f54(&var_busStopTexlist_8c1bc424, 0x24, 0.0f, 0.0f, -1.23f);
}

/* Per-frame driver-comment popup logic: turns bus-state bit changes (turn
 * signal, wipers, gear, lane-change, headlights) into message codes staged
 * via showMark_8c01fa78, ramps the driver-points meter display, and tail-calls
 * drawHud_8c01fbac (the popup/meter renderer) through the fade-command queue. */
STATIC void hudUpdateTask_8c01ff48() {
    unsigned int blinker = var_busState_8c1bb9d0.field_0x3b0 & 7;
    unsigned int wiper;
    unsigned int gear;
    unsigned int lane;
    int messageArg = 0;

    if (var_8c22643c.field_0x10 == 0) {
        if (blinker != 0) {
            var_8c226450 = blinker + 0x1f;
            var_8c22643c.field_0x10 = 1;
            var_8c226454 = 0;
        }
    } else if (blinker == 0) {
        var_8c22643c.field_0x10 = 0;
    }

    wiper = var_busState_8c1bb9d0.field_0x3b0 & 0x30000;
    if (wiper != 0) {
        if (wiper == 0x20000) {
            showMark_8c01fa78(0x18, 300);
        } else if (wiper == 0x30000) {
            showMark_8c01fa78(0x19, 300);
        } else if (wiper == 0x10000) {
            showMark_8c01fa78(0x17, 300);
        }
    }

    gear = var_busState_8c1bb9d0.field_0x3b0 & 0xc0000;
    if (var_gearLatch_8c226470 == 0 && gear != 0) {
        /* A fresh gear change hides the headlight and lane-change marks
         * for this frame; they only show once the latch clears again. */
        if (gear == 0x40000) {
            showMark_8c01fa78(0x1c, 0xb4);
        } else if (gear == 0x80000) {
            showMark_8c01fa78(0x1d, 0xb4);
        }
        var_gearLatch_8c226470 = 1;
    } else {
        if (var_gearLatch_8c226470 != 0 && gear == 0) {
            var_gearLatch_8c226470 = 0;
        }

        if ((var_busState_8c1bb9d0.field_0x3b0 & 0x100) != 0) {
            showMark_8c01fa78(0x1b, 0xb4);
        }

        lane = var_busState_8c1bb9d0.field_0x3b8 & 0xff0000;
        if (var_laneLatch_8c226474 == 0) {
            if (lane != 0) {
                showMark_8c01fa78((short)(lane >> 0x10) + 0x1e, 0x78);
                var_laneLatch_8c226474 = 1;
                var_8c2264a8.field_0x0c = 0;
            }
        } else if (lane == 0) {
            var_laneLatch_8c226474 = 0;
        }
    }

    if (var_8c2264a8.field_0x00 < 0x1e || 0x50 < var_8c2264a8.field_0x00 || var_8c2264a8.field_0x08 == 0) {
        var_8c2264a8.field_0x04 = 0;
    } else {
        var_8c2264a8.field_0x0c += 1;
        if ((var_8c2264a8.field_0x0c & 4) == 0) {
            var_8c2264a8.field_0x04 = 0;
        } else {
            var_8c2264a8.field_0x04 = (Uint8)var_busState_8c1bb9d0.field_0x3bc + 0x51;
        }
    }

    if ((var_busState_8c1bb9d0.field_0x3b4 & 0xff000000U) != 0) {
        int index = (Sint8)((unsigned int)var_busState_8c1bb9d0.field_0x3b4 >> 0x18);
        TrafficSignal *sig = ObjectsGetTrafficSignal_8c0288b2(index);
        int frame = ObjectsGetTrafficSignalFrame_8c028900(index);
        messageArg = frame + 1;

        if (sig->attached_0xbc[0] == 0) {
            if (sig->attached_0xbc[2] != 0) {
                messageArg = frame + 4;
                if (frame == 0) {
                    frame = sig->attached_0xbc[2]->frame_0x0c;
                    if (frame == 1) {
                        messageArg = 4 + 3;
                    }
                }
            }
        } else {
            messageArg = frame + 8;
            if (frame == 0) {
                frame = sig->attached_0xbc[0]->frame_0x0c;
                if (frame == 1) {
                    messageArg = 8 + 3;
                }
            }
        }
    }

    if (var_8c2264a8.field_0x08 != 0) {
        var_8c2264a8.field_0x08 -= 1;
    }
    var_8c226454 += 1;

    {
        float driverPointsF = (float)var_driverPoints_8c2285d0;
        if (driverPointsF != var_8c226458.field_0x00) {
            if (driverPointsF != var_8c226458.field_0x04) {
                var_8c226458.field_0x08 = (driverPointsF - var_8c226458.field_0x04) / 20.0f;
                var_8c226458.field_0x04 = driverPointsF;
            }
            var_8c226458.field_0x00 += var_8c226458.field_0x08;
            if ((var_8c226458.field_0x08 > 0.0f && driverPointsF < var_8c226458.field_0x00) ||
                (var_8c226458.field_0x08 < 0.0f && var_8c226458.field_0x00 < driverPointsF)) {
                var_8c226458.field_0x00 = driverPointsF;
            }
        }
    }

    FadeCmdPushCall1_8c0223ea(0, drawHud_8c01fbac, messageArg);
}

/* Installs hudUpdateTask_8c01ff48 as a per-frame task and resets the violation-checker
 * and popup/meter scratch state for a fresh run. */
void HudReset_8c02018c(void) {
    Task *createdTask;
    void *createdState;

    TaskPush_8c014ae8(var_tasks_8c1ba5e8, hudUpdateTask_8c01ff48, &createdTask, &createdState, 0);

    var_8c22643c.field_0x00 = 0;
    var_8c22643c.field_0x04 = 0;
    var_8c22643c.field_0x10 = 0;
    var_8c226450 = -1;
    var_8c226454 = 0;

    var_8c226458.field_0x04 = (float)var_driverPoints_8c2285d0;
    var_8c226458.field_0x00 = (float)var_driverPoints_8c2285d0;
    var_8c226458.field_0x0c = 1.0f;

    var_engineRpm_8c226468 = 0.0f;
    var_8c22646c = 0;
    var_gearLatch_8c226470 = 0;
    var_laneLatch_8c226474 = 0;

    var_8c226478[0].color = 0xffff0000;
    var_8c226478[1].color = 0xffff0000;
    var_8c226478[2].color = 0xffff0000;
}
