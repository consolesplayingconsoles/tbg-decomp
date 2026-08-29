/* @unit: not stamped -- this TU bundles three unrelated jobs: bus blinker-
 * light state (FUN_8c027958, FUN_8c028022), traffic-signal draw
 * callbacks for 028258_objects (FUN_8c0281ac, FUN_8c028206), and a ground-
 * alignment matrix helper for 025b98 (FUN_8c027c3c). */
#ifndef _027958_H
#define _027958_H

#include "sectionB.h"           /* BusState */
#include "026710_traffic.h"     /* TrafficEntry */

/* Called by FUN_8c024bb8 (024b4c) with the player's BusState each frame.
 * Sets the visibility flag on the bus's blinker-arrow light models from
 * blinker_0x080, and on a turn-maneuver-phase light from field_0x000. */
void FUN_8c027958(BusState *bus);

/* Called by BusTask_8c022bdc (022bdc) with the player's BusState
 * (var_8c1bbd9c) each frame at night (var_timeOfDay_8c18ad20 == 2). Forces
 * blinker_0x080 bits 0x08/0x10 on, and crossfades field_0x0c4[0..4] (the
 * bus's directional-light coefficient row) between its cached day/night
 * values over 20 frames, gated by field_0x2dc. Despite the name, this isn't
 * itself the probability roll -- see the .c file. */
void FUN_8c028022(BusState *bus);

/* FadeCmdPushCall2_8c022420 callback (FadeCallback2, hence the (int, int)
 * signature): draws a TrafficSignal's lamp model (frames_0x10) via
 * tlist_0xb4/model_0xb8 at matrixArg (an NJS_MATRIX*, multiplied onto
 * identity), showing the frame for obj->frame_0x0c (0/1/2) and hiding the
 * other two. objArg is the TrafficSignal*. */
void FUN_8c0281ac(int objArg, int matrixArg);

/* FadeCmdPushCall2_8c022420 callback used for a type 2/3/4 TrafficSignal's
 * attachment: same draw as FUN_8c0281ac but toggles frames_0x10[0]'s
 * NJD_EVAL_HIDE bit from obj->drawA_0xc8 rather than switching on
 * frame_0x0c. */
void FUN_8c028206(int objArg, int matrixArg);

/* Called by FUN_8c025b98 (025b98, still raw asm) once per frame for each
 * moving traffic entity, passing the entity itself and its current heading
 * (as a 0..1 turn fraction, scaled by 45000.0 below to match acc_0x078's
 * fixed-point units). Two jobs:
 *
 * 1. Registers this entity's draw for the current frame via
 *    FadeCmdPushCall2_8c022420: a near test (200m range, ~80-degree cone
 *    around var_busState_8c1bb9d0's last move-delta -- i.e. "ahead of the
 *    player") selects busDrawSimpleCb_8c027a88 (which additionally draws
 *    bodyModel_0x14 up close); a separate, tighter far test (50m range,
 *    ~15-degree cone around what looks like the rear-view mirror camera's
 *    world position/direction, field_0x318.._0x330) selects
 *    busDrawSimpleCb_8c027bac. Either can fire independently (both draw
 *    calls can be queued the same frame); each callback also picks a near
 *    vs. far LOD/night-vs-day light variant from the distance and
 *    var_timeOfDay_8c18ad20.
 *
 * 2. Advances this entity's suspension-lean state (field_0x070/074/078/07c,
 *    mirroring BusState's distance_traveled_0x070/ang_0x074/acc_0x078/
 *    ang_0x07c -- the same fields FUN_8c027958 reads for blinker-light
 *    placement) and, unless skipped (field_0x494 already 1 and speed
 *    field_0x27c is 0 -- i.e. stationary and already aligned), re-probes 3
 *    ground points (field_0x118/0x124/0x100, each an (x,y,z) triple) through
 *    entity->field_0x2c8 (a per-entity ground-probe function pointer) into
 *    3 scratch GroundQueryResult buffers at entity+0x190/0x1a0/0x1b0,
 *    interpolates height at 2 of them into posY_0xf8, and re-aligns the
 *    entity's world matrix (worldMatrix_0x84) to the ground via
 *    move_bus_model_8c020594. field_0x2b4 == 1 temporarily swaps the active
 *    ground grid to the fallback grid for these probes. */
void FUN_8c027c3c(TrafficEntry *entity, float heading);

#endif // _027958_H
