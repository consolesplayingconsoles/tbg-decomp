<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // var_8c2285c4 is the base of a large scratch region this unit
    // addresses partly through their own exported names (var_8c2285fc,
    // var_8c22861c) and partly through raw var_8c2285c4[N] offsets --
    // var_8c228640/[N=31], and the totally unnamed [33]/[35]/[36]/[37]/[38]
    // (0x228648/50/54/58/5c) -- depending on which the original compiler
    // had loaded in a register at that point. Both paths must resolve to
    // the same address here, like the real ROM.
    private function resolveSymbols(): int
    {
        $this->setSize('_var_8c2285c4', 0xa0);
        $base = $this->addressOf('_var_8c2285c4');

        $this->rellocate('_var_8c2285fc', $base + 0x38);
        $this->rellocate('_var_scheduleTime_8c2285d8', $base + 0x14);
        $this->rellocate('_var_runClock_8c2285dc', $base + 0x18);
        $this->rellocate('_var_8c22861c', $base + 0x58);

        $this->setSize('_var_busState_8c1bb9d0', 0x400);
        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $this->setSize('_var_prevLane_8c228684', 4);
        $this->setSize('_var_frameSpeed_8c22866c', 4);
        $this->setSize('_var_driveMode_8c1bb8c8', 4);
        $this->setSize('_var_playerBus_8c1bbd9c', 4); // BusState*, allocated via alloc()
        $this->setSize('_var_padTriggerR_8c1ba374', 2);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_practiceRules_8c226410', 4);
        $this->setSize('_var_stopPhase_8c2285e4', 4);
        $this->setSize('_var_driveCueState_8c2264b8', 0x1c);
        $this->setSize('_var_hudDriveMarkIcon_8c226450', 4);
        $this->setSize('_BusStopUpdateArrival_8c02ce48', 4);
        $this->setSize('_VibStart_8c010f7a', 4);

        return $base;
    }

    private function initFloat(int $address, float $value): void
    {
        $raw = unpack('L', pack('f', $value))[1];
        $this->initUint32($address, $raw);
    }

    // "Everything idle/off" baseline so only the behavior under test fires.
    private function baseline(int $base): int
    {
        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($busState + 0x25c, 0);
        $this->initUint32($busState + 0x268, 0);
        $this->initUint32($busState + 0x34c, 0);
        $this->initUint32($busState + 0x384, 0);
        $this->initUint32($busState + 0x2b4, 0); // bus_state, not "driving" (!=1)
        $this->initFloat($busState + 0x27c, 0.0); // speed
        $this->initUint32($busState + 0x258, 0); // ang
        $this->initUint32($busState + 0x3c0, 0); // bus_substate
        $this->initUint32($busState + 0x3c4, 0);

        $this->initUint32($base + 0x58, 0); // var_8c22861c[0]
        $this->initUint32($base + 0x5c, 1); // var_8c22861c[1] armed -- skip idle timer branch
        $this->initUint32($base + 0x60, 0); // var_8c22861c[2]
        $this->initUint32($base + 0x64, 0); // var_8c22861c[3] sig state
        $this->initUint32($base + 0x68, 0); // var_8c22861c[4]
        $this->initUint32($base + 0x6c, 0); // var_8c22861c[5]
        $this->initUint32($this->addressOf('_var_prevLane_8c228684'), 0);

        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 0.0);
        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 1); // skip speed*ang check

        $busPtr = $this->alloc(0x2b8);
        $this->initUint32($busPtr + 0x25c, 0);
        $this->initUint32($busPtr + 0x268, 0);
        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), $busPtr);

        $this->initUint32($base + 0x84, 0); // var_8c2285c4[33]
        $this->initUint32($base + 0x88, 0); // var_8c2285c4[34] i.e. var_firstUpshift_8c22864c, "armed" flag
        $this->initUint32($base + 0x8c, 0); // var_8c2285c4[35]
        $this->initUint32($base + 0x94, 0); // var_8c2285c4[37]
        $this->initFloat($base + 0x90, 0.0); // var_8c2285c4[36]
        $this->initUint32($base + 0x98, 0); // var_8c2285c4[38]
        $this->initUint32($base + 0x7c, 0); // var_8c2285c4[31] i.e. var_8c228640

        $this->initUint16($this->addressOf('_var_padTriggerR_8c1ba374'), 0);

        $this->initUint32($base + 0x54, 0); // var_8c2285fc[7]
        $this->initUint32($base + 0x4c, 0); // var_8c2285fc[5]

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0);
        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 1);
        $this->initUint32(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 0);

        $this->initUint32($base + 0x18, 0); // var_runClock_8c2285dc
        $this->initUint32($base + 0x14, 0); // var_scheduleTime_8c2285d8
        $this->initUint32($this->addressOf('_var_hudDriveMarkIcon_8c226450'), -1);

        return $busPtr;
    }

    public function test_lane_signal_reset_when_lane_delta_negative_and_left_signal_on(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x6c, 4); // var_8c22861c[5]
        $this->initUint32($this->addressOf('_var_prevLane_8c228684'), 10); // laneDelta = 4-10 < 0
        $this->initUint32($busPtr + 0x25c, 1); // left signal on

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($busPtr + 0x25c, 0);
        $this->shouldWriteLong($busPtr + 0x268, 0);
        $this->shouldWriteLong($base + 0x68, 0); // var_8c22861c[4], hold-timer no-op reset
        $this->shouldWriteLong($base + 0x58, 1); // var_8c22861c[0], signalSide_0x25c-unchanged counter

        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');

        $this->shouldCall('_adjust_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1); // var_runClock_8c2285dc incremented
    }

    public function test_lane_signal_untouched_when_delta_negative_and_right_signal_on(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x6c, 4);
        $this->initUint32($this->addressOf('_var_prevLane_8c228684'), 10); // laneDelta < 0
        $this->initUint32($busPtr + 0x25c, 2); // right signal on -- mismatched side, no reset

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0); // var_8c22861c[4], hold-timer no-op reset
        $this->shouldWriteLong($base + 0x58, 1); // var_8c22861c[0], signalSide_0x25c-unchanged counter

        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');

        $this->shouldCall('_adjust_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1); // var_runClock_8c2285dc incremented
    }

    // sigState 1 -> (turning) -> 2 -> (still turning, stopped) -> 5
    public function test_turn_signal_state_1_to_2_to_5_when_stopped(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x64, 1); // var_8c22861c[3] sig state = 1
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x34c, 0x10000000); // turning left

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x64, 2); // sigState -> 2 (still turning)
        $this->shouldWriteLong($base + 0x64, 5); // sigState -> 5 (turning, stopped)
        $this->shouldWriteLong($base + 0x68, 0); // var_8c22861c[4] hold-timer reset

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjust_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // sigState 2, turn signal cancelled (turnBits no longer 0x10000000):
    // penalty for cancelling early, sigState -> 1.
    public function test_turn_signal_state_2_cancelled_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x64, 2); // var_8c22861c[3] sig state = 2

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldCall('_adjust_8c02b464')->with(0x14, 0xffffffe2); // -30
        $this->shouldWriteLong($base + 0x64, 1); // sigState -> 1
        $this->shouldWriteLong($base + 0x68, 0); // var_8c22861c[4] hold-timer reset

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjust_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // sigState 2, still turning and moving: no state change, hold timer
    // skipped this frame (var_8c22861c[4] reset only).
    public function test_turn_signal_state_2_still_turning_while_moving(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x64, 2); // var_8c22861c[3] sig state = 2
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x34c, 0x10000000); // still turning
        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 1.0); // moving

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0); // var_8c22861c[4] reset (skipHoldTimer path)

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjust_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // sigState 5 (turn signal held while stopped), now moving again with the
    // signal off: reverts to state 1.
    public function test_turn_signal_state_5_reverts_to_1(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x64, 5); // var_8c22861c[3] sig state = 5

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x64, 1); // sigState -> 1
        $this->shouldWriteLong($base + 0x68, 0); // var_8c22861c[4] hold-timer reset

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjust_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Hold-timer counter (var_8c22861c[4]) expires while the aliased signal
    // bits (var_8c1bbd1c) show a lane change in progress: flat penalty and
    // the counter reloads to 0x78.
    public function test_hold_timer_expiry_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x34c, 0x20000000); // var_8c1bbd1c alias
        $this->initUint32($base + 0x68, 0); // var_8c22861c[4] decrements to -1

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, -1);
        $this->shouldWriteLong($base + 0x68, 0x78);
        $this->shouldCall('_adjust_8c02b464')->with(0x15, 0xffffffce); // -50

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjust_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Right-trigger (accelerator) armed and held past the threshold: penalty
    // and the latch is disarmed.
    public function test_right_trigger_held_too_long_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x84, 1); // var_8c2285c4[33] latch already armed
        $this->initUint32($base + 0x8c, 0xe); // var_8c2285c4[35] repeat counter, one away from tripping
        $this->initUint16($this->addressOf('_var_padTriggerR_8c1ba374'), 0xff); // held at max

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);

        $this->shouldWriteLong($base + 0x8c, 0xf);
        $this->shouldWriteLong($base + 0x84, 0);
        $this->shouldCall('_adjust_8c02b464')->with(0x17, 0xfffffffb); // -5

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjust_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Right-trigger newly armed (var_firstUpshift_8c22864c set elsewhere) and already
    // held past the threshold: arms the latch immediately.
    public function test_right_trigger_newly_armed_and_already_held(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x88, 1); // var_8c2285c4[34] i.e. var_firstUpshift_8c22864c, armed
        $this->initUint16($this->addressOf('_var_padTriggerR_8c1ba374'), 0xff); // held at max

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);

        $this->shouldWriteLong($base + 0x84, 1); // var_8c2285c4[33] latch armed
        $this->shouldWriteLong($base + 0x8c, 0); // var_8c2285c4[35] repeat counter reset
        $this->shouldWriteLong($base + 0x88, 0); // var_firstUpshift_8c22864c cleared

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjust_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Float timer (var_8c2285c4[36]) past threshold and its countdown
    // (var_8c2285c4[37]) expired: penalty, timer reset, and vibration.
    public function test_float_timer_expiry_applies_penalty_and_vibrates(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initFloat($base + 0x90, 0.5); // var_8c2285c4[36] > 0.01
        $this->initUint32($base + 0x94, 0); // var_8c2285c4[37] already expired

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);

        $this->shouldCall('_adjust_8c02b464')->with(0x18, 0xfffffffb); // -5
        $this->shouldWriteFloat($base + 0x90, 0.0);
        $this->shouldWriteLong($base + 0x94, 0x3c);
        $this->shouldCall('_VibStart_8c010f7a')->with(3);

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjust_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Sharp/violent turn (speed * steering angle out of range) while driving:
    // penalty once the counter expires.
    public function test_sharp_turn_expiry_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 0);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x2b4, 1); // driving
        $this->initFloat($this->addressOf('_var_busState_8c1bb9d0') + 0x27c, 100.0); // speed
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x258, 100); // ang
        $this->initUint32($base + 0x98, 0); // var_8c2285c4[38] decrements to -1

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);

        $this->shouldWriteLong($base + 0x98, -1);
        $this->shouldCall('_adjust_8c02b464')->with(0x19, 0xfffffffb); // -5
        $this->shouldWriteLong($base + 0x98, 0x3c);

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjust_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Idle timeout: bus stopped (speed 0.0) for too many frames in a row.
    public function test_idle_timeout_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x5c, 0); // var_8c22861c[1] not yet armed
        $this->initUint32($base + 0x60, 0x709); // var_8c22861c[2] one away from tripping

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);
        $this->shouldWriteLong($base + 0x58, 1);

        $this->shouldWriteLong($base + 0x60, 0x70a); // var_8c22861c[2] incremented past threshold
        $this->shouldCall('_adjust_8c02b464')->with(0x13, 0xffffffb0); // -80
        $this->shouldWriteLong($base + 0x60, 0);

        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjust_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Idle timeout armed once the bus first moves (var_8c22861c[1] latches).
    public function test_idle_timeout_arms_on_first_movement(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x5c, 0); // var_8c22861c[1] not yet armed
        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 1.0); // moving

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);
        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldWriteLong($base + 0x5c, 1);

        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjust_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Message-box just closed (var_8c228640 is set): a small positive award.
    public function test_messagebox_closed_awards_points(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x7c, 1); // var_8c2285c4[31] i.e. var_8c228640
        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 0); // route into the if-branch

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);
        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');

        $this->shouldCall('_adjust_8c02b464')->with(0x1e, 0x14); // +20
        $this->shouldWriteLong($base + 0x7c, 0);

        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Bus stopped in the wrong substate while still moving: flat penalty.
    public function test_wrong_substate_while_moving_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 0); // route messagebox branch away
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x3c0, 2); // bus_substate
        $this->initFloat($this->addressOf('_var_busState_8c1bb9d0') + 0x27c, 1.0); // moving

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);
        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');

        $this->shouldCall('_adjust_8c02b464')->with(0x21, 0xfffffff1); // -15
        $this->shouldWriteLong($this->addressOf('_var_busState_8c1bb9d0') + 0x3c4, 1);

        $this->shouldWriteLong($base + 0x18, 1);
    }

    // playMode 1 (a course run) with the "ready" bit clear: the run-progress
    // countdown decrements and, once exhausted, applies a flat penalty and
    // returns without reaching the increment path.
    public function test_run_progress_countdown_expires_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0); // bit 0 clear -> messagebox branch too
        $this->initUint32($base + 0x18, 0); // var_runClock_8c2285dc decrements to -1

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);
        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');

        $this->shouldWriteLong($base + 0x18, -1);
        $this->shouldWriteLong($base + 0x18, 0);
        $this->shouldCall('_adjust_8c02b464')->with(0x1d, 0xffffff38); // -200
    }

    // playMode 1 with the "ready" bit set: the countdown is skipped (falls
    // to the increment path) but bit 0 also routes around the messagebox
    // award/penalty branch.
    public function test_run_progress_countdown_skipped_when_ready(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 3); // bits 0 and 1 both set

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);
        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');

        $this->shouldCall('_adjust_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);

        $this->shouldWriteLong($base + 0x18, 1); // var_runClock_8c2285dc incremented (bit 1 set -> else branch)
    }

    // Not a course run (playMode != 1): var_runClock_8c2285dc just increments, and
    // once past var_scheduleTime_8c2285d8 and a multiple of some divisor, a silent
    // (msgSet -1) adjustment fires via __modls.
    public function test_run_progress_probe_modls_divisor(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x18, 29); // var_runClock_8c2285dc -> increments to 30
        $this->initUint32($base + 0x14, 0); // var_scheduleTime_8c2285d8 (0 < 30)
        $this->initUint32($this->addressOf('_var_hudDriveMarkIcon_8c226450'), 1); // != -1

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);
        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_BusStopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjust_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);

        $this->shouldWriteLong($base + 0x18, 30);

        $this->shouldCall('__modls')
            ->with(30, 30)
            ->using(new \Lhsazevedo\Sh4ObjTest\Simulator\CallingConventions\RiroCallingConvention())
            ->andReturn(0);

        $this->shouldCall('_adjust_8c02b464')->with(-1, -1);
    }
};
