<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// applyBraking_8c024530: STATIC, called from BusInputUpdate_8c0246b2 each frame while
// driving. Decelerates BusState.speed_0x27c by 0.002 plus a term that grows
// with speed and with how far the .l trigger (var_peripherals_8c1ba35c[0].l) has moved past
// its saved deadzone (brakeSensitivity_0xd1), clamped to 0. If the resulting speed
// drops under the next-lower gear's top speed (init_gears_8c045638[gear-1].
// upshiftSpeed_0x08), downshifts one gear. Then derives targetRpm_0x2e8/rpmRampAngle_0x2e4
// from the (possibly new) gear's table entry via asinf, and updates
// var_runState_8c2285c4.brakeAverage_0x90's running average with the brake amount.
//
// asinf's return is mocked to 0.0 throughout: sh4objtest has no FPU
// register accessor, so the real argument (ratio/6000) is asserted via
// with(), but the angle it produces cannot be chained from a live libm
// call -- only from the mocked return.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_busState_8c1bb9d0', 0x3c8);
        $this->setSize('_var_peripherals_8c1ba35c', 0x1c);
        $this->setSize('_var_progress_8c1ba1cc', 0xe8);
        $this->setSize('_var_runState_8c2285c4', 0x9c);
        $this->setSize('_asinf', 4);
    }

    private function initFloat(int $addr, float $value): void {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    private function setup(float $speed, int $gear, int $trigger, int $prevDeadzone, float $smoothedBrake): int {
        $this->resolveSymbols();
        $bus = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initFloat($bus + 0x27c, $speed);
        $this->initUint32($bus + 0x2f4, $gear);
        $this->initUint16($this->addressOf('_var_peripherals_8c1ba35c') + 0x1a, $trigger);
        $this->initUint8($this->addressOf('_var_progress_8c1ba1cc') + 0xd1, $prevDeadzone);
        $this->initFloat($this->addressOf('_var_runState_8c2285c4') + 0x90, $smoothedBrake);
        return $bus;
    }

    // No trigger delta (trigger == deadzone): brake amount is the constant
    // 0.002 baseline. Gear 0 never checks for a downshift.
    public function test_gearZero_noDelta_baselineBraking(): void {
        $bus = $this->setup(0.05, 0, 0, 0, 0.0);

        $this->call('_applyBraking_8c024530');

        $this->shouldWriteFloat($bus + 0x27c, 0.04800000041723251);
        $this->shouldWriteFloat($bus + 0x2e8, 2221.71435546875);
        $this->shouldCall('_asinf')->with(2221.71435546875 / 6000.0)->andReturn(0.0);
        $this->shouldWriteLong($bus + 0x2e4, 0);
        $this->shouldWriteFloat($this->addressOf('_var_runState_8c2285c4') + 0x90, 0.0020000000949949026);
        $this->shouldWriteFloat($this->addressOf('_var_runState_8c2285c4') + 0x90, 0.0010000000474974513);
    }

    // Gear 2 with baseline braking: new speed (0.048) drops below gear 1's
    // top speed (init_gears_8c045638[1].upshiftSpeed_0x08 ~= 0.185), so it downshifts to
    // gear 1, and the needle values are derived from gear 1's row.
    public function test_downshift_whenBelowLowerGearTop(): void {
        $bus = $this->setup(0.05, 2, 0, 0, 0.0);

        $this->call('_applyBraking_8c024530');

        $this->shouldWriteFloat($bus + 0x27c, 0.04800000041723251);
        $this->shouldWriteLong($bus + 0x2f4, 1);
        $this->shouldWriteFloat($bus + 0x2e8, 1110.857177734375);
        $this->shouldCall('_asinf')->with(1110.857177734375 / 6000.0)->andReturn(0.0);
        $this->shouldWriteLong($bus + 0x2e4, 0);
        $this->shouldWriteFloat($this->addressOf('_var_runState_8c2285c4') + 0x90, 0.0020000000949949026);
        $this->shouldWriteFloat($this->addressOf('_var_runState_8c2285c4') + 0x90, 0.0010000000474974513);
    }

    // Gear 1 with a high enough speed after braking: stays above gear 0's
    // top speed, so no downshift happens (no write to gear_0x2f4).
    public function test_noDownshift_whenAboveLowerGearTop(): void {
        $bus = $this->setup(0.15, 1, 0, 0, 0.0);

        $this->call('_applyBraking_8c024530');

        $this->shouldWriteFloat($bus + 0x27c, 0.14800000190734863);
        $this->shouldWriteFloat($bus + 0x2e8, 3425.14306640625);
        $this->shouldCall('_asinf')->with(3425.14306640625 / 6000.0)->andReturn(0.0);
        $this->shouldWriteLong($bus + 0x2e4, 0);
        $this->shouldWriteFloat($this->addressOf('_var_runState_8c2285c4') + 0x90, 0.0020000000949949026);
        $this->shouldWriteFloat($this->addressOf('_var_runState_8c2285c4') + 0x90, 0.0010000000474974513);
    }

    // Trigger held all the way down (full delta) and a very low starting
    // speed: the quadratic term dominates enough to clamp speed to 0.
    public function test_heavyBraking_clampsSpeedToZero(): void {
        $bus = $this->setup(0.001, 0, 255, 0, 0.0);

        $this->call('_applyBraking_8c024530');

        // Speed is stored unclamped first, then clamped to 0 in a separate
        // store when it went negative.
        $this->shouldWriteFloat($bus + 0x27c, -0.001020833384245634);
        $this->shouldWriteFloat($bus + 0x27c, 0.0);
        $this->shouldWriteFloat($bus + 0x2e8, 0.0);
        $this->shouldCall('_asinf')->with(0.0)->andReturn(0.0);
        $this->shouldWriteLong($bus + 0x2e4, 0);
        // The running average is also a raw store followed by the halved one.
        $this->shouldWriteFloat($this->addressOf('_var_runState_8c2285c4') + 0x90, 0.0020208333153277636);
        $this->shouldWriteFloat($this->addressOf('_var_runState_8c2285c4') + 0x90, 0.0010104166576638818);
    }
};
