<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// applyBrakingSfx_8c024606: STATIC, called from BusInputUpdate_8c0246b2 each frame.
// While the .l trigger (var_padTriggerL_8c1ba376) sits further past its saved deadzone
// (brakeSensitivity_0xd1) than var_brakePressPeak_8c227d8c's current value maps to, ratchets
// var_brakePressPeak_8c227d8c up to delta*255/(255-deadzone) (never down). Once the
// trigger stops advancing (<= the deadzone) and var_brakePressPeak_8c227d8c is nonzero,
// plays one of two midi notes on var_midiHandles_8c0fcd28[0] depending on
// whether var_brakePressPeak_8c227d8c reached the 0x80 halfway point -- counterintuitively
// the LOW note (0x27) plays once it DID reach 0x80, the HIGH note (0x28)
// otherwise -- then resets it to 0.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_padTriggerL_8c1ba376', 2);
        $this->setSize('_var_progress_8c1ba1cc', 0xe8);
        $this->setSize('_var_brakePressPeak_8c227d8c', 4);
        $this->setSize('_var_midiHandles_8c0fcd28', 4 * 8);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('__divls', 4);
        $this->onCall('__divls', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });
    }

    private function setup(int $trigger, int $prevDeadzone, int $counter): int {
        $this->resolveSymbols();
        $this->initUint16($this->addressOf('_var_padTriggerL_8c1ba376'), $trigger);
        $this->initUint8($this->addressOf('_var_progress_8c1ba1cc') + 0xd1, $prevDeadzone);
        $this->initUint32($this->addressOf('_var_brakePressPeak_8c227d8c'), $counter);

        $midiHandle = 0xcafe0900;
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), $midiHandle);
        return $midiHandle;
    }

    // Trigger pressed past the deadzone from a fresh (0) counter: ratchets
    // up to the scaled travel distance. delta=90, target=(90*255)/245=93.
    public function test_triggerAdvancing_ratchetsCounterUp(): void {
        $this->setup(100, 10, 0);

        $this->call('_applyBrakingSfx_8c024606');

        $this->shouldCall('__divls');
        $this->shouldWriteLong($this->addressOf('_var_brakePressPeak_8c227d8c'), 93);
    }

    // Trigger past the deadzone, but the scaled travel distance (93) is not
    // past the counter's current value (200): no write happens.
    public function test_triggerAdvancing_belowCurrentCounter_noWrite(): void {
        $this->setup(100, 10, 200);

        $this->call('_applyBrakingSfx_8c024606');

        $this->shouldCall('__divls');
    }

    // Trigger at or under the deadzone with a zero counter: nothing to do.
    public function test_triggerReleased_zeroCounter_doesNothing(): void {
        $this->setup(5, 10, 0);

        $this->call('_applyBrakingSfx_8c024606');
    }

    // Trigger released with a nonzero counter under the 0x80 halfway point:
    // plays the high note (0x28) and resets the counter.
    public function test_triggerReleased_lowCounter_playsHighNote(): void {
        $midiHandle = $this->setup(5, 10, 0x7f);

        $this->call('_applyBrakingSfx_8c024606');

        $this->shouldCall('_sdMidiPlay')->with(
            $midiHandle,
            1,
            0x28,
            0
        );
        $this->shouldWriteLong($this->addressOf('_var_brakePressPeak_8c227d8c'), 0);
    }

    // Trigger released with a counter at/past the 0x80 halfway point: plays
    // the low note (0x27) and resets the counter.
    public function test_triggerReleased_highCounter_playsLowNote(): void {
        $midiHandle = $this->setup(5, 10, 0x80);

        $this->call('_applyBrakingSfx_8c024606');

        $this->shouldCall('_sdMidiPlay')->with(
            $midiHandle,
            1,
            0x27,
            0
        );
        $this->shouldWriteLong($this->addressOf('_var_brakePressPeak_8c227d8c'), 0);
    }

    // Trigger exactly at the deadzone (not past it) with a nonzero counter
    // behaves like "released".
    public function test_triggerAtDeadzone_isTreatedAsReleased(): void {
        $midiHandle = $this->setup(10, 10, 0x7f);

        $this->call('_applyBrakingSfx_8c024606');

        $this->shouldCall('_sdMidiPlay')->with(
            $midiHandle,
            1,
            0x28,
            0
        );
        $this->shouldWriteLong($this->addressOf('_var_brakePressPeak_8c227d8c'), 0);
    }
};
