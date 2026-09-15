/* @unit BusStop */

#include <shinobi.h>
#include "includes.h" /* STATIC */

#include "sectionB.h"
#include "028258_objects.h"
#include "013ae8_route_load.h" /* enum ROUTE */
#include "011120_asset_queues.h" /* AsqGetRandomInRangeA_8c012178 */
#include "014a9c_tasks.h" /* TaskFreeGroup_8c014ab4 */
#include "020914_ground_query.h" /* GroundQueryFindPolygon_8c020914, GroundQueryResult */
#include "020b6c_ground_probe.h"
#include "02fb50_sh4nlfzn.h" /* rand */
#include "02af78_event.h" /* EventScanCandidates_8c02b03c */
#include "01614c_debug_menu.h" /* FUN_8c016770 */
#include "02c884_bus_stop.h" /* BusStopGetSegment_8c02cd6a */
#include "0222dc_fadecmd.h" /* FadeCmdPushCall1_8c0223ea */
#include "0100bc_sound.h" /* SndStartAdxFadeOut_8c010bae */
#include "02b464_drive_points.h" /* FUN_8c02c586, FUN_8c02c784 */

/* ====================
 * Functions
 * ====================
 */

/* Clears the 31-slot scripted waiting-passenger schedule (-1 = unused) and
 * resets the shared waiting-passenger sprite's scale/angle/animation ahead
 * of a new stop. */
STATIC void resetStopState_8c02c884(void)
{
    int i;

    for (i = 0; i < 31; i++) {
        var_stopSchedule_8c228718[i] = -1;
    }

    var_8c2288d8.sx = 0.014f;
    var_8c2288d8.sy = 0.014f;
    var_8c2288d8.ang = 0;
    var_8c2288d8.tanim = init_pedestrianTexAnims_8c04623c;
}

CourseSegment *BusStopGetSegment_8c02cd6a(int segmentIndex)
{
    return &var_currentCourseConfig_8c18ad18->segments_0x08[segmentIndex];
}

/* Advances prevStopSegment to the just-armed stop and reloads the
 * event-gate threshold (var_8c2285d8) from the course's per-segment gate
 * table (courseConfig's ukn_0x0c). */
STATIC void advanceStopSegment_8c02ccae(void)
{
    var_prevStopSegment_8c22870c++;
    var_8c2285d8 = ((int *)var_currentCourseConfig_8c18ad18->ukn_0x0c)[var_prevStopSegment_8c22870c];
}

StopAreaRecord *BusStopGetStopArea_8c02cd7a(int segmentIndex)
{
    CourseSegment *seg = BusStopGetSegment_8c02cd6a(segmentIndex);
    return *(StopAreaRecord **)((char *)var_stopAreaTable_8c1bb870 + seg->stopAreaId_0x02 * 8);
}

/* Locks in the current stop's heading angle (var_8c2288fc, masked to an
 * unsigned 16-bit njArcTan2 angle) from the segment var_nextStopSegment_8c228710
 * was still pointing at, then advances var_nextStopSegment_8c228710 to the
 * next segment with an active-stop flag (var_8c2286a4) and primes that
 * upcoming stop's position (var_8c228900.x/.z) and heading angle
 * (var_8c228714, sign-extended back to a full Angle) from its stop-area
 * record (BusStopGetStopArea_8c02cd7a). */
void BusStopUpdateStopHeadings_8c02ccc6(void)
{
    StopAreaRecord *rec;
    int seg;
    int angle;

    var_currentSegment_8c228708 = var_nextStopSegment_8c228710;
    rec = BusStopGetStopArea_8c02cd7a(var_nextStopSegment_8c228710);
    var_8c2288fc = (int)(njArcTan2(rec->dx_0x0c, rec->dz_0x10) + 0x8000) & 0xffff;

    seg = var_currentSegment_8c228708;
    do {
        seg++;
        var_nextStopSegment_8c228710 = seg;
    } while (var_8c2286a4[seg] == 0);

    rec = BusStopGetStopArea_8c02cd7a(var_nextStopSegment_8c228710);
    var_8c228900.x = rec->x_0x04;
    var_8c228900.z = rec->z_0x08;
    angle = njArcTan2(rec->dx_0x0c, rec->dz_0x10) + 0x8000;
    var_8c228714 = angle;
    if ((angle & 0x8000) != 0) {
        var_8c228714 = angle | 0xffff0000;
    }
}

/* Tears down the bus-stop task group (waiting-passenger/departure tasks),
 * freeing its Task array and resetting the handle to "not allocated". */
void BusStopFreeTaskGroup_8c02ca96(void)
{
    if (var_stopTaskGroup_8c2288f8 != (void *)-1) {
        TaskFreeGroup_8c014ab4((Task *)var_stopTaskGroup_8c2288f8);
        syFree(var_stopTaskGroup_8c2288f8);
        var_stopTaskGroup_8c2288f8 = (void *)-1;
    }
}

/* Picks the waiting passengers for the upcoming stop (var_nextStopSegment_8c228710):
 * snaps its ground position, collects the segment's candidate stop spots that
 * have an active-stop flag (var_8c2286a4) into a scratch list, then randomly
 * picks 1-16 of them without replacement into var_waitingPassengers_8c228798, positioned along
 * the stop's spawn-area strip (var_8c1bb894) with per-passenger jitter --
 * except on ROUTE_OME, which skips the jitter. */
STATIC void pickWaitingPassengers_8c02c8ae(void)
{
    GroundQueryResult ground;
    void **candidates;
    CourseSegment *seg;
    char *list;
    void **slot;
    int candidateCount;
    int pick;
    int i;
    float x0, z0, dx, dz;
    float counter;

    var_waitingPassengerCount_8c228794 = 0;
    var_activeGroundGrid_8c2264d4 = var_groundGridFallback_8c1bb86c;

    GroundQueryFindPolygon_8c020914(var_8c228900.x, var_8c228900.y, var_8c228900.z, &ground);
    GroundProbeInterpolateHeight_8c020f7e(&ground, (float *)&var_8c228900);

    seg = BusStopGetSegment_8c02cd6a(var_nextStopSegment_8c228710);
    candidates = (void **)syMalloc(0x40);

    list = (char *)seg->stopCandidates_0x08;
    candidateCount = 0;
    if (list != 0) {
        for (; list[1] != 0; list += 2) {
            if (var_8c2286a4[(int)list[1]] != 0) {
                candidates[candidateCount++] = list;
            }
        }

        if (candidateCount != 0) {
            var_8c22890c = *(char **)((char *)var_8c1bb894 + seg->ukn_0x06 * 0xc);

            var_waitingPassengerCount_8c228794 = AsqGetRandomInRangeA_8c012178(candidateCount) + 1;
            if (var_waitingPassengerCount_8c228794 > 0x10) {
                var_waitingPassengerCount_8c228794 = 0x10;
            }

            x0 = *(float *)(var_8c22890c + 4);
            z0 = *(float *)(var_8c22890c + 8);
            dx = *(float *)(var_8c22890c + 0xc);
            dz = *(float *)(var_8c22890c + 0x10);

            counter = 0.0f;
            for (i = 0; i < var_waitingPassengerCount_8c228794; i++) {
                do {
                    pick = AsqGetRandomInRangeA_8c012178(candidateCount);
                    slot = &candidates[pick];
                } while (*slot == (void *)-1);

                var_waitingPassengers_8c228798[i].spot_0x00 = *slot;

                if (var_route_8c18ad1c == ROUTE_OME) {
                    var_waitingPassengers_8c228798[i].pos_0x04.x = counter * dx + x0;
                    var_waitingPassengers_8c228798[i].pos_0x04.z = counter * dz + z0;
                } else {
                    var_waitingPassengers_8c228798[i].pos_0x04.x =
                        counter * dx + x0 + (float)rand() / 32768.0f - 0.5f;
                    var_waitingPassengers_8c228798[i].pos_0x04.z =
                        counter * dz + z0 + (float)rand() / 32768.0f - 0.5f;
                }

                GroundQueryFindPolygon_8c020914(var_waitingPassengers_8c228798[i].pos_0x04.x, 0.0f,
                                                var_waitingPassengers_8c228798[i].pos_0x04.z, &ground);
                GroundProbeInterpolateHeight_8c020f7e(
                    &ground, (float *)&var_waitingPassengers_8c228798[i].pos_0x04);

                var_waitingPassengers_8c228798[i].index_0x10 = counter;
                counter += 1.0f;

                *slot = (void *)-1;
            }
        }
    }

    syFree(candidates);
}

/* Per-run bus-stop setup, called right after the course loads. Clears the
 * active-stop flags, flags one segment per candidate story event, then
 * forces a stop at every type-2 segment and randomly flags additional
 * segments (never type-3) until the course's randomized total stop count
 * (courseConfig's [randomStopCountMin_0x14, randomStopCountMax_0x18) range)
 * is reached. Finishes by priming the next/prev stop segment and the
 * upcoming stop's passengers, and resetting a couple of run-scoped
 * timers/thresholds (var_driverPoints_8c2285d0/var_8c2285d4,
 * var_8c2285d8/var_8c2285dc) based on var_8c1ba290[0] and play mode. */
void BusStopSetup_8c02caba(void)
{
    int i;
    int candidateId;
    int activeStopCount;
    int extraStopCount;
    int totalSegments;
    int pick;
    CourseSegment *segments;

    for (i = 0; i < (int)(sizeof(var_8c2286a4) / sizeof(var_8c2286a4[0])); i++) {
        var_8c2286a4[i] = 0;
    }

    EventScanCandidates_8c02b03c();
    for (i = 0; i < var_eventCandidateCount_8c228560; i++) {
        candidateId = var_eventCandidates_8c228520[i];
        var_8c2286a4[var_routeEvents_8c22851c[candidateId].segmentId_0x02] = 1;
    }

    segments = var_currentCourseConfig_8c18ad18->segments_0x08;
    activeStopCount = 0;
    totalSegments = 0;
    while (segments->type_0x00 != 0) {
        if (var_8c2286a4[totalSegments] != 0) {
            activeStopCount++;
        } else if (segments->type_0x00 == 2) {
            var_8c2286a4[totalSegments] = 1;
            activeStopCount++;
        }
        totalSegments++;
        segments++;
    }

    extraStopCount = AsqGetRandomInRangeA_8c012178(
                         var_currentCourseConfig_8c18ad18->randomStopCountMax_0x18 -
                         var_currentCourseConfig_8c18ad18->randomStopCountMin_0x14) +
                     var_currentCourseConfig_8c18ad18->randomStopCountMin_0x14 -
                     activeStopCount;

    /* segments now points at the scan's terminator record (courseConfig's
     * segments array + totalSegments), not its start -- and the reroll loop
     * below indexes segments[pick] off that same (unreset) pointer, so its
     * type_0x00 check actually lands totalSegments+pick records past the
     * course's segment table start. Preserved as-is: real asm behavior. */
    while (extraStopCount > 0) {
        pick = AsqGetRandomInRangeA_8c012178(totalSegments);
        if (segments[pick].type_0x00 != 3 && var_8c2286a4[pick] == 0) {
            extraStopCount--;
            var_8c2286a4[pick] = 1;
        }
    }

    FUN_8c016770();

    if (var_playMode_8c1bb8d0 != PLAY_MODE_DEMO) {
        var_activeTrafficPreset_8c227e14 = 0;
        var_activePedPreset_8c22822c = 0;
    }

    var_nextStopSegment_8c228710 = var_startStopIndex_8c228704;
    var_prevStopSegment_8c22870c = var_startStopIndex_8c228704 - 1;

    resetStopState_8c02c884();
    pickWaitingPassengers_8c02c8ae();
    BusStopUpdateStopHeadings_8c02ccc6();
    advanceStopSegment_8c02ccae();

    /* var_8c1ba290[0] is the DIFFICULTY setting byte (see sectionB.h). */
    if (var_8c1ba290[0] < 1 && var_playMode_8c1bb8d0 != PLAY_MODE_PRACTICE) {
        var_8c2285d4 = 200;
    } else {
        var_8c2285d4 = 100;
    }
    var_driverPoints_8c2285d0 = var_8c2285d4;

    if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE && (var_8c226410 & 2) != 2) {
        var_8c2285dc = var_8c2285d8;
        var_8c2285d8 = 0;
    } else {
        var_8c2285dc = var_8c2285d8 - 0x1c2;
    }
}

/* Draws the "fuu" stop-marker model at the upcoming stop, facing its
 * heading (var_8c228714) and animated by the frame counter var_8c1bc44c.
 * Installed as a FadeCallback1 by BusStopUpdateArrival_8c02ce48 while
 * approaching a stop; the callback arg is unused. */
STATIC void drawStopMarker_8c02cd92(int arg0)
{
    float frame;

    njUnitMatrix(&var_8c1bc46c);
    frame = var_8c1bc44c;
    njTranslate(&var_8c1bc46c, var_8c228900.x, var_8c228900.y, var_8c228908);
    njRotateY(&var_8c1bc46c, var_8c228714);
    njMultiMatrix(0, &var_8c1bc46c);
    njSetTexture(var_8c1bc440);
    njCnkSimpleDrawMotion(var_8c1bc444, var_loadedFooNjm_8c1bc448, frame);
}

/* Per-frame bus-stop arrival state machine, called once per frame by
 * 02b464_drive_points. States:
 * 0 = cruising -- watches var_busState_8c1bb9d0.markCueByte_0x3b4 for a
 *     just-crossed segment (its high byte) matching the next or previous
 *     stop segment, arming state 2 (approach) or 1 (post-departure wait);
 * 1 = waits for markCueByte_0x3b4's low byte to clear, then re-arms cruising and
 *     advances to the next stop segment;
 * 2 = approach -- tracks the running minimum straight-line distance from
 *     the bus (busState's posX_0x0f4/posZ_0x0fc) to the upcoming stop
 *     (var_8c228900.x/var_8c228908) and drives the marker's animation
 *     frame/draw callback, transitioning to "stopped" (3, via bus_state 3)
 *     once close enough and halted (var_8c1bbc4c == 0), or once the stop
 *     segment is reached outright (bus_state 4);
 * 3 = idle, waiting for something external to request the finish;
 * 4 = finishes the stop once markDriveFlags_0x3b0's top byte is set, installing the
 *     fade-complete callback and starting both ADX fade-outs. */
void BusStopUpdateArrival_8c02ce48(void)
{
    int crossedSegment;
    float distance;
    float dx, dz;

    if (var_stopPhase_8c2285e4 == 0) {
        if ((var_busState_8c1bb9d0.markCueByte_0x3b4 & 0xff00) != 0) {
            crossedSegment = (var_busState_8c1bb9d0.markCueByte_0x3b4 & 0xff00) >> 8;
            if (crossedSegment == var_nextStopSegment_8c228710) {
                var_stopPhase_8c2285e4 = 2;
                var_stopMinDistance_8c2285ec = 9999.0f;
                var_8c226454 = 0;
                pickWaitingPassengers_8c02c8ae();
                var_8c1bc44c = 0.0f;
            } else if (crossedSegment == var_prevStopSegment_8c22870c) {
                var_stopPhase_8c2285e4 = 1;
                var_8c226454 = 0;
            }
        }
    } else if (var_stopPhase_8c2285e4 == 1) {
        if ((var_busState_8c1bb9d0.markCueByte_0x3b4 & 0xff) != 0) {
            var_stopPhase_8c2285e4 = 0;
            if (var_8c226450 != -1) {
                var_8c228640 = 1;
            }
            var_8c2264c4 = 0;
            advanceStopSegment_8c02ccae();
        }
    } else if (var_stopPhase_8c2285e4 == 2) {
        dx = var_8c228900.x - var_busState_8c1bb9d0.posX_0x0f4;
        dz = var_8c228908 - var_busState_8c1bb9d0.posZ_0x0fc;
        distance = njSqrt(dx * dx + dz * dz);
        var_8c1bc44c += 1.0f;
        if (var_8c1bc450 <= var_8c1bc44c) {
            var_8c1bc44c = 0.0f;
        }
        FadeCmdPushCall1_8c0223ea(0, drawStopMarker_8c02cd92, 0);
        if (distance < var_stopMinDistance_8c2285ec) {
            var_stopMinDistance_8c2285ec = distance;
        }
        if ((distance < 3.0f && var_8c1bbc4c == 0.0f) ||
            (var_stopMinDistance_8c2285ec < 3.0f && var_8c1bbc4c == 0.0f)) {
            var_busState_8c1bb9d0.bus_state_0x2b4 = 3;
            var_stopPhase_8c2285e4 = 3;
            var_8c2285c4[0] = 3;
            var_8c2285e8 = 0;
        } else if ((var_busState_8c1bb9d0.markCueByte_0x3b4 & 0xff) == var_nextStopSegment_8c228710) {
            var_busState_8c1bb9d0.bus_state_0x2b4 = 4;
            var_stopPhase_8c2285e4 = 3;
            var_8c2285c4[0] = 3;
            var_8c2285e8 = 2;
        }
    } else if (var_stopPhase_8c2285e4 == 4) {
        if ((var_busState_8c1bb9d0.markDriveFlags_0x3b0 & 0xff000000) != 0) {
            var_busState_8c1bb9d0.bus_state_0x2b4 = 4;
            var_stopPhase_8c2285e4 = 3;
            var_8c2285e8 = 0;
            var_8c2285c4[0] = 4;
            var_8c2285cc = 0x1e;
            var_fadeCompleteCallback_8c22656c = FUN_8c02c784;
            if (0 < var_driverPoints_8c2285d0 && FUN_8c02c586() != 0) {
                var_8c2285c8 = 1;
            }
            SndStartAdxFadeOut_8c010bae(0);
            SndStartAdxFadeOut_8c010bae(1);
        }
    }
}
