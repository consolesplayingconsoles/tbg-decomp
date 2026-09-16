<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

// BusInputUpdate_8c0246b2: PUBLIC, called by BusTask_8c022bdc (022bdc) once per frame
// while driving. See 024280.c for the full breakdown of the three-state
// engineState_0x2e0 dispatch (relax/settle/ramp) and the mirror-button +
// steering-wheel-ramp tail that follows it.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_busState_8c1bb9d0', 0x3c8);
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_progress_8c1ba1cc', 0xd8);
        $this->setSize('_var_vibport_8c1ba354', 4);
        $this->setSize('_var_driveMode_8c1bb8c8', 4);
        $this->setSize('_var_runState_8c2285c4', 0x9c);
        $this->setSize('_VibStart_8c010f7a', 4);
        $this->setSize('_pdVibMxStop', 4);
        $this->setSize('__divls', 4);
        $this->onCall('__divls', function () {
            $dividend = $this->getRegister(1)->signedValue();
            $divisor = $this->getRegister(0)->signedValue();
            $this->setRegister(0, U32::of(intdiv($dividend, $divisor) & 0xffffffff));
        });

    }

    private function initFloat(int $addr, float $value): void {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    private function setup(
        int $mode,
        int $brakeTrigger,
        int $throttleTrigger,
        int $brakeDeadzone = 0,
        int $throttleDeadzone = 0,
        int $driveMode = 0
    ): array {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), $driveMode);
        $this->initUint32($this->addressOf('_var_vibport_8c1ba354'), 0xdeadbeef);

        $pad = $this->addressOf('_var_peripherals_8c1ba35c');
        $this->initUint16($pad + 0x1a, $brakeTrigger); // .l
        $this->initUint16($pad + 0x18, $throttleTrigger); // .r
        $this->initUint16($pad + 0x1c, 0); // .x1, centered
        $this->initUint32($pad + 0x10, 0); // .press, nothing held

        $progress = $this->addressOf('_var_progress_8c1ba1cc');
        $this->initUint8($progress + 0xd0, $throttleDeadzone); // accelSensitivity_0xd0
        $this->initUint8($progress + 0xd1, $brakeDeadzone);    // brakeSensitivity_0xd1

        $bus = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($bus + 0x2e0, $mode);
        $this->initUint32($bus + 0x2ec, 0);
        $this->initUint32($bus + 0x2f0, 0xdeadbeef);
        $this->initUint32($bus + 0x2f4, 1); // gear, not reverse
        $this->initUint32($bus + 0x080, 0); // blinker
        $this->initUint32($bus + 0x334, 0);
        $this->initUint32($bus + 0x338, 0xdeadbeef);
        $this->initUint32($bus + 0x25c, 0xdeadbeef); // unused unless press bits set
        $this->initFloat($bus + 0x27c, 0.5); // speed, nonzero so the mode-2
                                              // tail doesn't call debugGearOverride
        $this->initFloat($bus + 0x2e8, 0.0);
        $this->initUint32($bus + 0x258, 0); // ang_0x258, centered
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x80, 0); // var_runState_8c2285c4.field_0x80

        return ['bus' => $bus, 'pad' => $pad];
    }

    // --- mode 0 (relax) ---

    public function test_relax_brakePressed_setsBlinker(): void {
        ['bus' => $bus] = $this->setup(0, brakeTrigger: 100, throttleTrigger: 0, brakeDeadzone: 10);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');
        $this->shouldWriteLong($bus + 0x080, 1);
        $this->shouldCall('_updateGearSelector_8c0242ce');

        $this->forceStop();
    }

    public function test_relax_throttlePastHalfDeadzone_switchesToSettle(): void {
        ['bus' => $bus] = $this->setup(0, brakeTrigger: 0, throttleTrigger: 100, throttleDeadzone: 10);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');
        $this->shouldWriteLong($bus + 0x2ec, 0);
        $this->shouldWriteLong($bus + 0x2e0, 1);
        $this->shouldCall('_VibStart_8c010f7a')->with(0);
        $this->shouldCall('_updateGearSelector_8c0242ce');

        $this->forceStop();
    }

    public function test_relax_throttleAtHalfDeadzone_staysRelaxed(): void {
        $this->setup(0, brakeTrigger: 0, throttleTrigger: 5, throttleDeadzone: 10);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');
        $this->shouldCall('_updateGearSelector_8c0242ce');

        $this->forceStop();
    }

    // --- mode 1 (settle) ---

    public function test_settle_brakePressed_setsBlinkerAndIncrementsCounter(): void {
        ['bus' => $bus] = $this->setup(1, brakeTrigger: 100, throttleTrigger: 0, brakeDeadzone: 10);
        $this->initUint32($bus + 0x2ec, 5);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');
        $this->shouldWriteLong($bus + 0x080, 1);
        $this->shouldWriteLong($bus + 0x2ec, 6);
        $this->shouldCall('_updateGearSelector_8c0242ce');

        $this->forceStop();
    }

    public function test_settle_underThreshold_justIncrements(): void {
        ['bus' => $bus] = $this->setup(1, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint32($bus + 0x2ec, 10);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($bus + 0x2ec, 11);
        $this->shouldCall('_updateGearSelector_8c0242ce');

        $this->forceStop();
    }

    public function test_settle_overThreshold_pedalsPressed_abortsToRelax(): void {
        ['bus' => $bus] = $this->setup(1, brakeTrigger: 100, throttleTrigger: 0, brakeDeadzone: 10);
        $this->initUint32($bus + 0x2ec, 31);
        $this->initUint32($this->addressOf('_var_vibport_8c1ba354'), 5);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($bus + 0x080, 1);
        $this->shouldWriteLong($bus + 0x2ec, 32);
        $this->shouldWriteLong($bus + 0x2ec, 0);
        $this->shouldWriteLong($bus + 0x2e0, 0);
        $this->shouldCall('_pdVibMxStop')->with(5);
        $this->shouldCall('_updateGearSelector_8c0242ce');

        $this->forceStop();
    }

    public function test_settle_overThreshold_pedalsPressed_noVibportStop_whenUnset(): void {
        ['bus' => $bus] = $this->setup(1, brakeTrigger: 100, throttleTrigger: 0, brakeDeadzone: 10);
        $this->initUint32($bus + 0x2ec, 31);
        $this->initUint32($this->addressOf('_var_vibport_8c1ba354'), 0xffffffff);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($bus + 0x080, 1);
        $this->shouldWriteLong($bus + 0x2ec, 32);
        $this->shouldWriteLong($bus + 0x2ec, 0);
        $this->shouldWriteLong($bus + 0x2e0, 0);
        $this->shouldCall('_updateGearSelector_8c0242ce');

        $this->forceStop();
    }

    // Advancing to the ramp requires the throttle to STILL be held past its
    // deadzone (the press that got here in the first place) with the brake
    // not pressed -- not simply "neither pedal pressed".
    public function test_settle_overThreshold_throttleStillHeld_advancesToRamp(): void {
        ['bus' => $bus] = $this->setup(1, brakeTrigger: 0, throttleTrigger: 100, throttleDeadzone: 10);
        $this->initUint32($bus + 0x2ec, 31);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($bus + 0x2ec, 32);
        $this->shouldWriteLong($bus + 0x2e0, 2);
        $this->shouldWriteLong($bus + 0x2ec, 0);
        $this->shouldWriteLong($bus + 0x2f0, 0);
        $this->shouldCall('_updateGearSelector_8c0242ce');

        $this->forceStop();
    }

    // Throttle released (even with the brake also not pressed): aborts back
    // to relax, same as an explicit brake press.
    public function test_settle_overThreshold_throttleReleased_abortsToRelax(): void {
        ['bus' => $bus] = $this->setup(1, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint32($bus + 0x2ec, 31);
        $this->initUint32($this->addressOf('_var_vibport_8c1ba354'), 0xffffffff);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($bus + 0x2ec, 32);
        $this->shouldWriteLong($bus + 0x2ec, 0);
        $this->shouldWriteLong($bus + 0x2e0, 0);
        $this->shouldCall('_updateGearSelector_8c0242ce');

        $this->forceStop();
    }

    // --- mode 2 (ramp), forward gears ---

    public function test_ramp_forward_braking_callsApplyBraking(): void {
        ['bus' => $bus] = $this->setup(2, brakeTrigger: 100, throttleTrigger: 0, brakeDeadzone: 10);
        $this->initUint32($bus + 0x2f4, 2); // forward gear

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x80, 1); // var_runState_8c2285c4.field_0x80 += 1 (speed != 0)
        $this->shouldCall('_applyBraking_8c024530');
        $this->shouldWriteLong($bus + 0x080, 1);
        // speed_0x27c != 0.0 (untouched by the mocked call): resets idleFrameCounter_0x2ec.
        $this->shouldWriteLong($bus + 0x2ec, 0);

        $this->forceStop();
    }

    public function test_ramp_forward_notBraking_callsThrottleHandler(): void {
        ['bus' => $bus] = $this->setup(2, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint32($bus + 0x2f4, 2);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x80, 1);
        $this->shouldCall('_applyThrottle_8c024320');
        $this->shouldWriteFloat($this->addressOf('_var_runState_8c2285c4') + 0x90, 0.0);
        $this->shouldWriteLong($bus + 0x2ec, 0);

        $this->forceStop();
    }

    public function test_ramp_forward_atRest_resetsIdleCounter(): void {
        ['bus' => $bus] = $this->setup(2, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint32($bus + 0x2f4, 2);
        $this->initFloat($bus + 0x27c, 0.0);
        $this->initUint32($bus + 0x2ec, 10);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x80, 0);
        $this->shouldCall('_applyThrottle_8c024320');
        $this->shouldWriteFloat($this->addressOf('_var_runState_8c2285c4') + 0x90, 0.0);
        // speed_0x27c == 0.0 (the mocked call doesn't touch it): the mode-2
        // tail runs debugGearOverride and, since idleFrameCounter_0x2ec (10) < 30, just
        // increments it.
        $this->shouldCall('_updateGearSelector_8c0242ce');
        $this->shouldWriteLong($bus + 0x2ec, 11);

        $this->forceStop();
    }

    public function test_ramp_forward_atRestLongEnough_dropsToSettle(): void {
        ['bus' => $bus] = $this->setup(2, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint32($bus + 0x2f4, 2);
        $this->initFloat($bus + 0x27c, 0.0);
        $this->initUint32($bus + 0x2ec, 30);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x80, 0);
        $this->shouldCall('_applyThrottle_8c024320');
        $this->shouldWriteFloat($this->addressOf('_var_runState_8c2285c4') + 0x90, 0.0);
        $this->shouldCall('_updateGearSelector_8c0242ce');
        $this->shouldWriteLong($bus + 0x2e0, 1);
        $this->shouldWriteLong($bus + 0x2ec, 0);
        $this->shouldCall('_VibStart_8c010f7a')->with(0);

        $this->forceStop();
    }

    // --- mode 2 (ramp), reverse gear (5) ---

    public function test_ramp_reverse_braking_decelsTowardZero(): void {
        ['bus' => $bus] = $this->setup(2, brakeTrigger: 150, throttleTrigger: 0, brakeDeadzone: 50);
        $this->initUint32($bus + 0x2f4, 5);
        $this->initFloat($bus + 0x27c, -0.3);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteFloat($bus + 0x27c, -0.295121967792511);
        $this->shouldWriteLong($bus + 0x080, 1);
        $this->shouldWriteFloat($bus + 0x2e8, 4835.2783203125);

        $this->forceStop();
    }

    public function test_ramp_reverse_throttle_acceleratesInReverse(): void {
        ['bus' => $bus] = $this->setup(2, brakeTrigger: 0, throttleTrigger: 150, throttleDeadzone: 50);
        $this->initUint32($bus + 0x2f4, 5);
        $this->initFloat($bus + 0x27c, 0.0);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteFloat($bus + 0x27c, -0.0015432097716256976);
        $this->shouldWriteFloat($bus + 0x2e8, 25.28394889831543);

        $this->forceStop();
    }

    public function test_ramp_reverse_throttle_clampsAtTargetOnOvershoot(): void {
        ['bus' => $bus] = $this->setup(2, brakeTrigger: 0, throttleTrigger: 150, throttleDeadzone: 50);
        $this->initUint32($bus + 0x2f4, 5);
        $this->initFloat($bus + 0x27c, -0.09083423763513565);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteFloat($bus + 0x27c, -0.0892910286784172);
        $this->shouldWriteFloat($bus + 0x27c, -0.09033423662185669); // clamped to the target
        $this->shouldWriteFloat($bus + 0x2e8, 1480.0361328125);

        $this->forceStop();
    }

    public function test_ramp_reverse_neitherPedal_decaysTowardZero(): void {
        ['bus' => $bus] = $this->setup(2, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint32($bus + 0x2f4, 5);
        $this->initFloat($bus + 0x27c, -0.299);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteFloat($bus + 0x27c, -0.2980000078678131);
        $this->shouldWriteFloat($bus + 0x2e8, 4882.43212890625);

        $this->forceStop();
    }

    public function test_ramp_reverse_neitherPedal_clampsToZeroOnOvershoot(): void {
        ['bus' => $bus] = $this->setup(2, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint32($bus + 0x2f4, 5);
        $this->initFloat($bus + 0x27c, -0.0005);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteFloat($bus + 0x27c, 0.0005000000237487257);
        $this->shouldWriteFloat($bus + 0x27c, 0.0);
        // targetRpm_0x2e8 = -(0.0 * 16384.0) = -0.0 (distinct bit pattern from
        // 0.0, and this asserts the exact bits).
        $this->shouldWriteFloat($bus + 0x2e8, -0.0);

        $this->forceStop();
    }

    // --- default mode: proceeds straight to the mirror/steering tail ---

    public function test_invalidMode_skipsDispatch_stillRunsTail(): void {
        ['bus' => $bus] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');
        // No debugGearOverride, no gear/needle handler calls -- straight to
        // the steering ramp (centered stick, no press bits): ang_0x258 goes
        // from its random initial value toward 0.

        $this->forceStop();
    }

    // --- mirror buttons, manual drive mode (driveMode == 0) ---

    public function test_directMode_mirrorButtonA_pressed_entersMirror(): void {
        ['bus' => $bus, 'pad' => $pad] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint32($pad + 0x10, 0x400);
        $this->initUint32($bus + 0x25c, 0);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x6c, 0); // var_runState_8c2285c4.field_0x58[5], != the 0x10000000 sentinel

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($bus + 0x25c, 1);
        $this->shouldWriteLong($bus + 0x268, 1);

        $this->forceStop();
    }

    public function test_directMode_mirrorButtonA_pressed_sentinelSuppressesMirror(): void {
        ['bus' => $bus, 'pad' => $pad] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint32($pad + 0x10, 0x400);
        $this->initUint32($bus + 0x25c, 0);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x6c, 0x10000000);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($bus + 0x25c, 1);

        $this->forceStop();
    }

    public function test_directMode_mirrorButtonA_held_releasesMirror(): void {
        ['bus' => $bus, 'pad' => $pad] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint32($pad + 0x10, 0x400);
        $this->initUint32($bus + 0x25c, 1);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($bus + 0x25c, 0);
        $this->shouldWriteLong($bus + 0x268, 0);

        $this->forceStop();
    }

    public function test_directMode_mirrorButtonB_pressed_entersAltMirror(): void {
        ['bus' => $bus, 'pad' => $pad] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint32($pad + 0x10, 0x2);
        $this->initUint32($bus + 0x25c, 0);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x6c, 4); // var_runState_8c2285c4.field_0x58[5], != var_runState_8c2285c4.field_0x70[0] << 4
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x70, 0);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($bus + 0x25c, 2);
        $this->shouldWriteLong($bus + 0x268, 2);

        $this->forceStop();
    }

    public function test_directMode_mirrorButtonB_held_releasesMirror(): void {
        ['bus' => $bus, 'pad' => $pad] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint32($pad + 0x10, 0x2);
        $this->initUint32($bus + 0x25c, 2);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($bus + 0x25c, 0);
        $this->shouldWriteLong($bus + 0x268, 0);

        $this->forceStop();
    }

    // --- mirror buttons, auto drive mode (driveMode == 1) ---

    public function test_mappedMode_field334Set_bailsImmediately(): void {
        ['bus' => $bus, 'pad' => $pad] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0, driveMode: 1);
        $this->initUint32($pad + 0x10, 0x400);
        $this->initUint32($bus + 0x334, 1);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($bus + 0x338, 2);

        $this->forceStop();
    }

    public function test_mappedMode_mirrorButtonA_pressed_entersMirror(): void {
        ['bus' => $bus, 'pad' => $pad] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0, driveMode: 1);
        $this->initUint32($pad + 0x10, 0x400);
        $this->initUint32($bus + 0x25c, 0);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x6c, 0);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($bus + 0x338, 2);
        $this->shouldWriteLong($bus + 0x25c, 1);
        $this->shouldWriteLong($bus + 0x268, 1);

        $this->forceStop();
    }

    public function test_mappedMode_mirrorButtonA_held_clearsField338(): void {
        ['bus' => $bus, 'pad' => $pad] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0, driveMode: 1);
        $this->initUint32($pad + 0x10, 0x400);
        $this->initUint32($bus + 0x25c, 1);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($bus + 0x338, 2);
        $this->shouldWriteLong($bus + 0x338, 0);

        $this->forceStop();
    }

    public function test_mappedMode_mirrorButtonB_held_releasesMirror(): void {
        ['bus' => $bus, 'pad' => $pad] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0, driveMode: 1);
        $this->initUint32($pad + 0x10, 0x2);
        $this->initUint32($bus + 0x25c, 1);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($bus + 0x338, 2);
        $this->shouldWriteLong($bus + 0x25c, 0);
        $this->shouldWriteLong($bus + 0x268, 0);

        $this->forceStop();
    }

    public function test_mappedMode_mirrorButtonB_atAltState_setsField338(): void {
        ['bus' => $bus, 'pad' => $pad] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0, driveMode: 1);
        $this->initUint32($pad + 0x10, 0x2);
        $this->initUint32($bus + 0x25c, 2);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($bus + 0x338, 2);
        $this->shouldWriteLong($bus + 0x338, 1);

        $this->forceStop();
    }

    public function test_mappedMode_noMirrorButton_onlySetsField338(): void {
        ['bus' => $bus] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0, driveMode: 1);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldWriteLong($bus + 0x338, 2);

        $this->forceStop();
    }

    // --- steering force-feedback ramp (direct mode only) ---

    public function test_steering_left_increasesFromPositiveBelowTarget(): void {
        ['bus' => $bus, 'pad' => $pad] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint16($pad + 0x1c, -50 & 0xffff); // x1
        $this->initUint32($bus + 0x258, 1000); // ang_0x258

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldCall('__divls');
        $this->shouldWriteLong($bus + 0x258, 1080);

        $this->forceStop();
    }

    // Covers the cur>=target "decrease" branch's product-sign multiply
    // (targetAngle * cur > 0) with a genuine steering-right (negative
    // target) input: sh4objtest's MUL.L simulates the multiplicand as
    // unsigned and overflows before truncating whenever either operand is
    // negative, so that multiply can only be exercised safely here with
    // both operands positive -- a left-turn target below a larger current
    // angle, which reaches the identical branch and step (-182).
    public function test_steering_decreasesFromPositiveAboveTarget(): void {
        ['bus' => $bus, 'pad' => $pad] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint16($pad + 0x1c, -50 & 0xffff); // x1, target 3458
        $this->initUint32($bus + 0x258, 4000);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldCall('__divls');
        // target 3458: current (4000) >= target, both nonzero -> step -182.
        $this->shouldWriteLong($bus + 0x258, 3818);

        $this->forceStop();
    }

    // x1=200 (steering right) lands past the -127 clamp bound, so the
    // target comes from the 0x2aaa clamp constant rather than a real
    // division; ang_0x258 starts at 0 (>= target trivially, since target is
    // very negative), landing back in the same product-sign-safe case as
    // the centered-deadzone test below (product == 0 whenever cur == 0).
    public function test_steering_right_clampedTarget_fromRest(): void {
        ['bus' => $bus, 'pad' => $pad] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint16($pad + 0x1c, 200); // x1
        $this->initUint32($bus + 0x258, 0);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        // target -0x2aaa (-10922); cur(0) > 0 is false, cur(0) <= target?
        // no (0 > -10922) -- so this is cur<=0's "decrease magnitude"
        // branch: a flat -80, no product multiply involved.
        $this->shouldWriteLong($bus + 0x258, -80 & 0xffffffff);

        $this->forceStop();
    }

    public function test_steering_centeredDeadzone_decaysToward0_fastStep(): void {
        ['bus' => $bus, 'pad' => $pad] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint16($pad + 0x1c, 3); // within the +-8 deadzone -> target 0
        $this->initUint32($bus + 0x258, 1000);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        // target 0: product == 0 -> the faster 364 step.
        $this->shouldWriteLong($bus + 0x258, 636);

        $this->forceStop();
    }

    // x1=200 clamps to the -0x2aaa target (see the note above on the
    // right-turn division path being unreachable through this harness).
    public function test_steering_negativeCurrent_aboveMoreNegativeTarget_decreasesMagnitude(): void {
        ['bus' => $bus, 'pad' => $pad] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint16($pad + 0x1c, 200); // x1, target -0x2aaa (-10922)
        $this->initUint32($bus + 0x258, -3000 & 0xffffffff);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        // cur(-3000) <= target(-10922)? No -- -3000 > -10922, so this takes
        // the cur<=0 "decrease magnitude" branch instead: -80.
        $this->shouldWriteLong($bus + 0x258, -3080 & 0xffffffff);

        $this->forceStop();
    }

    public function test_steering_negativeCurrent_atOrBelowTarget_increasesMagnitude(): void {
        ['bus' => $bus, 'pad' => $pad] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint16($pad + 0x1c, 3); // target 0
        $this->initUint32($bus + 0x258, -3000 & 0xffffffff);

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        // cur(-3000) <= target(0): product == 0 -> +364.
        $this->shouldWriteLong($bus + 0x258, -2636 & 0xffffffff);

        $this->forceStop();
    }

    public function test_steering_clampsOnOvershoot_positiveIncreasing(): void {
        ['bus' => $bus, 'pad' => $pad] = $this->setup(3, brakeTrigger: 0, throttleTrigger: 0);
        $this->initUint16($pad + 0x1c, -50 & 0xffff); // target 3458
        $this->initUint32($bus + 0x258, 3400); // close enough to overshoot by one step

        $this->call('_BusInputUpdate_8c0246b2');

        $this->shouldCall('_applyBrakingSfx_8c024606');

        $this->shouldCall('__divls');
        $this->shouldWriteLong($bus + 0x258, 3480); // 3400 + 80 (truncated)
        $this->shouldWriteLong($bus + 0x258, 3458); // clamped to target

        $this->forceStop();
    }
};
