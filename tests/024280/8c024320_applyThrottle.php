<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// applyThrottle_8c024320: STATIC, called from BusInputUpdate_8c0246b2 each frame while driving.
// While the .r trigger (var_8c1ba374) clears its saved deadzone
// (var_8c1ba29c) by at least var_8c1bbcb4's minimum scaled step, ramps
// BusState.needleCurrentValue_0x2e4 toward that step by init_8c045638[gear].throttleRampRate_0x00,
// feeds it through njSin to derive target_0x2e8/speed_0x27c, and upshifts
// (with a shift-cue MIDI note) once speed clears the next gear's top speed
// -- or, at the top gear, just clamps speed to its max. Otherwise coasts:
// decays speed_0x27c by 0.00025 (clamped to 0), downshifting (same cue) if
// speed drops under the current gear's own top speed, then -- for every
// coast and every upshift -- recomputes target_0x2e8/needleCurrentValue_0x2e4 from the
// (possibly new) gear via asinf, same formula as applyBraking_8c024530.
//
// asinf's return is mocked to 0.0 throughout: sh4objtest has no FPU
// register accessor, so the real argument (target_0x2e8/6000) is asserted
// via with(), but the angle it produces cannot be chained from a live libm
// call -- only from the mocked return.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_busState_8c1bb9d0', 0x3c8);
        $this->setSize('_var_8c1ba374', 2);
        $this->setSize('_var_8c1ba29c', 1);
        $this->setSize('_var_8c1bbcb4', 4);
        $this->setSize('_var_cameraMode_8c227d9c', 4);
        $this->setSize('_var_8c22864c', 4);
        $this->setSize('_var_midiHandles_8c0fcd28', 4 * 8);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_njSin', 4);
        $this->setSize('_asinf', 4);
    }

    private function initFloat(int $addr, float $value): void {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    private function setup(
        int $trigger,
        int $deadzone,
        int $minStep,
        int $gear,
        float $speed,
        int $field2e4,
        int $mirrorLevel = 0
    ): int {
        $this->resolveSymbols();
        $this->initUint16($this->addressOf('_var_8c1ba374'), $trigger);
        $this->initUint8($this->addressOf('_var_8c1ba29c'), $deadzone);
        $this->initUint32($this->addressOf('_var_8c1bbcb4'), $minStep);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), $mirrorLevel);
        $this->initUint32($this->addressOf('_var_8c22864c'), 0xdeadbeef);

        $bus = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($bus + 0x2f4, $gear);
        $this->initFloat($bus + 0x27c, $speed);
        $this->initUint32($bus + 0x2e4, $field2e4);

        $midiHandle = 0xcafe0900;
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), $midiHandle);
        return $bus;
    }

    // Trigger within the deadzone: coasts. No downshift since gear 1's
    // decayed speed (0.19975) stays above gear 0's top speed (0.0926).
    public function test_coasting_noDownshift(): void {
        $bus = $this->setup(0, 0, 0, 1, 0.2, 999);

        $this->call('_applyThrottle_8c024320');

        $this->shouldWriteFloat($bus + 0x27c, 0.19975000619888306);
        $this->shouldWriteFloat($bus + 0x2e8, 4622.7861328125);
        $this->shouldCall('_asinf')->with(4622.7861328125 / 6000.0)->andReturn(0.0);
        $this->shouldWriteLong($bus + 0x2e4, 0);
    }

    // Trigger past deadzone but below var_8c1bbcb4's minimum step: treated
    // as coasting even though the trigger is technically pressed.
    public function test_belowMinStep_treatedAsCoasting(): void {
        $bus = $this->setup(200, 50, 99999, 1, 0.2, 999);

        $this->call('_applyThrottle_8c024320');

        $this->shouldWriteFloat($bus + 0x27c, 0.19975000619888306);
        $this->shouldWriteFloat($bus + 0x2e8, 4622.7861328125);
        $this->shouldCall('_asinf')->with(4622.7861328125 / 6000.0)->andReturn(0.0);
        $this->shouldWriteLong($bus + 0x2e4, 0);
    }

    // Coasting with a low enough speed to drop below gear 1's top speed:
    // downshifts to gear 0, plays the shift cue (mirror level 0 -> note
    // 0x26), and derives the needle from the new gear.
    public function test_coasting_downshifts(): void {
        $bus = $this->setup(0, 0, 0, 2, 0.05, 999, 0);

        $this->call('_applyThrottle_8c024320');

        $this->shouldWriteFloat($bus + 0x27c, 0.04975000023841858);
        $this->shouldCall('_sdMidiPlay')->with(0xcafe0900, 1, 0x26, 0);
        $this->shouldWriteLong($bus + 0x2f4, 1);
        $this->shouldWriteFloat($bus + 0x2e8, 1151.357177734375);
        $this->shouldCall('_asinf')->with(1151.357177734375 / 6000.0)->andReturn(0.0);
        $this->shouldWriteLong($bus + 0x2e4, 0);
    }

    // Same downshift, but mirror level >= 2 selects the other shift-cue note.
    public function test_coasting_downshift_altNoteWhenMirrorLevelHigh(): void {
        $bus = $this->setup(0, 0, 0, 2, 0.05, 999, 2);

        $this->call('_applyThrottle_8c024320');

        $this->shouldWriteFloat($bus + 0x27c, 0.04975000023841858);
        $this->shouldCall('_sdMidiPlay')->with(0xcafe0900, 1, 0x25, 0);
        $this->shouldWriteLong($bus + 0x2f4, 1);
        $this->shouldWriteFloat($bus + 0x2e8, 1151.357177734375);
        $this->shouldCall('_asinf')->with(1151.357177734375 / 6000.0)->andReturn(0.0);
        $this->shouldWriteLong($bus + 0x2e4, 0);
    }

    // Accelerating in gear 1, but the resulting speed stays under gear 1's
    // top speed: no upshift, no shared-tail needle recompute.
    public function test_accelerating_noUpshift(): void {
        $bus = $this->setup(200, 50, 0, 1, 0.0, 100);

        $this->call('_applyThrottle_8c024320');

        $this->shouldWriteLong($bus + 0x2e4, 236);
        $this->shouldCall('_njSin')->with(236)->andReturn(0.5);
        $this->shouldWriteFloat($bus + 0x2e8, 3000.0);
        $this->shouldWriteFloat($bus + 0x27c, 0.12962962687015533);
    }

    // Accelerating in gear 1 with enough speed to clear gear 1's top speed:
    // upshifts to gear 2, plays the shift cue, and the shared tail
    // recomputes the needle from the new gear.
    public function test_accelerating_upshifts(): void {
        $bus = $this->setup(200, 50, 0, 1, 0.0, 100);

        $this->call('_applyThrottle_8c024320');

        $this->shouldWriteLong($bus + 0x2e4, 236);
        $this->shouldCall('_njSin')->with(236)->andReturn(0.9);
        $this->shouldWriteFloat($bus + 0x2e8, 5400.0);
        $this->shouldWriteFloat($bus + 0x27c, 0.23333331942558289);
        $this->shouldCall('_sdMidiPlay')->with(0xcafe0900, 1, 0x26, 0);
        $this->shouldWriteLong($bus + 0x2f4, 2);
        $this->shouldWriteFloat($bus + 0x2e8, 3600.0);
        $this->shouldCall('_asinf')->with(0.6000000238418579)->andReturn(0.0);
        $this->shouldWriteLong($bus + 0x2e4, 0);
    }

    // Accelerating out of gear 0 specifically: var_8c22864c is set to 1 on
    // the first upshift out of gear 0.
    public function test_accelerating_upshiftFromGearZero_setsFirstShiftFlag(): void {
        $bus = $this->setup(200, 50, 0, 0, 0.0, 0);

        $this->call('_applyThrottle_8c024320');

        $this->shouldWriteLong($bus + 0x2e4, 546);
        $this->shouldCall('_njSin')->with(546)->andReturn(0.9);
        $this->shouldWriteFloat($bus + 0x2e8, 5400.0);
        $this->shouldWriteFloat($bus + 0x27c, 0.11666665971279144);
        $this->shouldCall('_sdMidiPlay')->with(0xcafe0900, 1, 0x26, 0);
        $this->shouldWriteLong($this->addressOf('_var_8c22864c'), 1);
        $this->shouldWriteLong($bus + 0x2f4, 1);
        $this->shouldWriteFloat($bus + 0x2e8, 2700.0);
        $this->shouldCall('_asinf')->with(2700.0 / 6000.0)->andReturn(0.0);
        $this->shouldWriteLong($bus + 0x2e4, 0);
    }

    // Accelerating at the top gear (4): speed exceeding the top gear's max
    // just gets clamped to it -- no shift, no needle recompute, no gear
    // write at all.
    public function test_accelerating_topGear_clampsSpeed(): void {
        $bus = $this->setup(255, 0, 0, 4, 0.0, 5000);

        $this->call('_applyThrottle_8c024320');

        $this->shouldWriteLong($bus + 0x2e4, 5034);
        $this->shouldCall('_njSin')->with(5034)->andReturn(1.05);
        $this->shouldWriteFloat($bus + 0x2e8, 6300.0);
        $this->shouldWriteFloat($bus + 0x27c, 0.680555522441864);
        $this->shouldWriteFloat($bus + 0x27c, 0.6481481194496155);
    }
};
