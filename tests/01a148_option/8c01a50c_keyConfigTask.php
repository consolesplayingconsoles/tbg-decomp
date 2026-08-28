<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    const PRESS = 0x10;          // var_peripherals_8c1ba35c[0].press offset
    const R = 0x18;               // var_peripherals_8c1ba35c[0].r offset (ACCEL trigger)
    const L = 0x1a;               // var_peripherals_8c1ba35c[0].l offset (BRAKE trigger)
    const STATE = 0x18;           // var_menuState_8c1bc7a8.state_0x18 offset
    const SELECTED = 0x38;        // var_menuState_8c1bc7a8.selected_0x38 offset
    const TA = 1 << 2;            // confirm
    const TB = 1 << 1;            // cancel
    const KU = 1 << 4;            // prev row
    const KD = 1 << 5;            // next row
    const MIDI = 0xd1d1d1d1;

    const CONTROLLER = 0xf06fe;   // BT_CONTROLLER
    const RACING = 0x700fe;       // BT_RACING

    // PlayerProgress field offsets from var_progress_8c1ba1cc.
    const VARIANT = 0xc5;         // field_0xc5 -- A/B button-config variant
    const CHOICE_A_PAD = 0xcc;    // field_0xc7[5] -- variant A, BT_CONTROLLER
    const CHOICE_B_PAD = 0xcd;    // field_0xc7[6] -- variant B, BT_CONTROLLER
    const CHOICE_A_RACE = 0xce;   // field_0xc7[7] -- variant A, BT_RACING
    const CHOICE_B_RACE = 0xcf;   // field_0xc7[8] -- variant B, BT_RACING
    const ACCEL = 0xd0;           // field_0xd0
    const BRAKE = 0xd1;           // field_0xd1

    private int $task = 0;

    private function menu(int $off): int
    {
        return $this->addressOf('_var_menuState_8c1bc7a8') + $off;
    }

    private function progress(int $off): int
    {
        return $this->addressOf('_var_progress_8c1ba1cc') + $off;
    }

    private function arrange(int $press, int $state, int $selected,
                             int $ctrlType = self::CONTROLLER, int $variant = 0,
                             int $choiceAPad = 0, int $choiceBPad = 0,
                             int $choiceARace = 0, int $choiceBRace = 0,
                             int $accel = 0, int $brake = 0, int $r = 0, int $l = 0,
                             int $isFading = 0): void
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_var_progress_8c1ba1cc', 0xd2);
        $this->setSize('_TxtDrawSprite_8c014f54', 4);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_FileMenuResetViewDefaults_8c0188bc', 4);
        $this->setSize('_FadePushOut_8c022b60', 4);

        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + self::PRESS, $press);
        $this->initUint16($this->addressOf('_var_peripherals_8c1ba35c') + self::R, $r);
        $this->initUint16($this->addressOf('_var_peripherals_8c1ba35c') + self::L, $l);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), self::MIDI);
        $this->initUint32($this->addressOf('_var_activeCtrlType_8c157a70'), $ctrlType);
        $this->initUint32($this->menu(self::STATE), $state);
        $this->initUint32($this->menu(self::SELECTED), $selected);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), $isFading);
        $this->initUint8($this->progress(self::VARIANT), $variant);
        $this->initUint8($this->progress(self::CHOICE_A_PAD), $choiceAPad);
        $this->initUint8($this->progress(self::CHOICE_B_PAD), $choiceBPad);
        $this->initUint8($this->progress(self::CHOICE_A_RACE), $choiceARace);
        $this->initUint8($this->progress(self::CHOICE_B_RACE), $choiceBRace);
        $this->initUint8($this->progress(self::ACCEL), $accel);
        $this->initUint8($this->progress(self::BRAKE), $brake);

        $this->task = $this->alloc(0x20);
        $this->call('_keyConfigTask_8c01a50c')->with($this->task);
    }

    /* 3 row labels: highlighted row = i+0x2e, others i+0x2a. */
    private function shouldDrawRowLabels(int $selected): void
    {
        $gB = $this->menu(0x0c);
        for ($i = 0; $i < 3; $i++) {
            $idx = ($i === $selected) ? $i + 0x2e : $i + 0x2a;
            $this->shouldCall('_TxtDrawSprite_8c014f54')->with($gB, $idx, 0.0, 0.0, -5.0);
        }
    }

    /* State-2 middle: a single button-choice icon at the given sprite index. */
    private function shouldDrawButtonIcon(int $idx): void
    {
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->menu(0x0c), $idx, 0.0, 0.0, -5.0);
    }

    /*
     * Non-state-2 middle: the two ACCEL/BRAKE fill-bars (drawSensitivityBar has its
     * own test, so it's mocked) with their numeric-readout markers, then two static
     * sprites. The active bar (state 3 = accel, 4 = brake) draws marker 0x44, else 0x43.
     */
    private function shouldDrawSensitivityBars(int $state, int $accel, int $brake,
                                               float $accelX = 256.0, float $brakeX = 256.0): void
    {
        $gB = $this->menu(0x0c);
        $this->shouldCall('_drawSensitivityBar_8c01a42a')->with(144.0, $accel)->andReturn($accelX);
        $this->shouldCall('_TxtDrawSprite_8c014f54')
            ->with($gB, $state === 3 ? 0x44 : 0x43, $accelX - 10.0, 124.0, -5.0);
        $this->shouldCall('_drawSensitivityBar_8c01a42a')->with(224.0, $brake)->andReturn($brakeX);
        $this->shouldCall('_TxtDrawSprite_8c014f54')
            ->with($gB, $state === 4 ? 0x44 : 0x43, $brakeX - 10.0, 203.0, -5.0);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($gB, 0x40, 0.0, 0.0, -5.0);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($gB, 0x41, 0.0, 0.0, -5.0);
    }

    /* Row highlight (0x61 for a row < 3, 0x62 DEFAULT, 0x63 RETURN) + title frame. */
    private function shouldDrawChrome(int $selected): void
    {
        $gB = $this->menu(0x0c);
        if ($selected < 3) {
            $this->shouldCall('_TxtDrawSprite_8c014f54')->with($gB, 0x61, 0.0, 0.0, -5.0);
        } else {
            $this->shouldCall('_TxtDrawSprite_8c014f54')
                ->with($gB, $selected === 3 ? 0x62 : 0x63, 0.0, 0.0, -5.0);
        }
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->menu(0x00), 0, 0.0, 0.0, -7.0);
    }

    public function test_phase0_waits_while_fading()
    {
        $this->arrange(press: 0, state: 0, selected: 0, isFading: 1);
        $this->shouldDrawRowLabels(selected: 0);
        $this->shouldDrawSensitivityBars(state: 0, accel: 0, brake: 0);
        $this->shouldDrawChrome(selected: 0);
    }

    public function test_phase0_advances_when_fade_done()
    {
        $this->arrange(press: 0, state: 0, selected: 0, isFading: 0);
        $this->shouldWriteLong($this->menu(self::STATE), 1);
        $this->shouldDrawRowLabels(selected: 0);
        $this->shouldDrawSensitivityBars(state: 1, accel: 0, brake: 0);
        $this->shouldDrawChrome(selected: 0);
    }

    public function test_navigate_next_row()
    {
        $this->arrange(press: self::KD, state: 1, selected: 0);
        $this->shouldWriteLong($this->menu(self::SELECTED), 1);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldDrawRowLabels(selected: 1);
        $this->shouldDrawSensitivityBars(state: 1, accel: 0, brake: 0);
        $this->shouldDrawChrome(selected: 1);
    }

    public function test_navigate_prev_wraps_to_return()
    {
        $this->arrange(press: self::KU, state: 1, selected: 0);
        $this->shouldWriteLong($this->menu(self::SELECTED), -1);
        $this->shouldWriteLong($this->menu(self::SELECTED), 4);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldDrawRowLabels(selected: 4);
        $this->shouldDrawSensitivityBars(state: 1, accel: 0, brake: 0);
        $this->shouldDrawChrome(selected: 4);
    }

    public function test_confirm_row_enters_edit()
    {
        $this->arrange(press: self::TA, state: 1, selected: 0);
        $this->shouldWriteLong($this->menu(self::STATE), 2);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 0, 0);
        $this->shouldDrawRowLabels(selected: 0);
        $this->shouldDrawButtonIcon(idx: 0x35);   // controller, variant A, choice 0: 0x35 + 0
        $this->shouldDrawChrome(selected: 0);
    }

    public function test_confirm_default_row_resets_view()
    {
        $this->arrange(press: self::TA, state: 1, selected: 3);
        $this->shouldCall('_FileMenuResetViewDefaults_8c0188bc');
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 0, 0);
        $this->shouldDrawRowLabels(selected: 3);
        $this->shouldDrawSensitivityBars(state: 1, accel: 0, brake: 0);
        $this->shouldDrawChrome(selected: 3);
    }

    public function test_confirm_return_row_fades_out()
    {
        $this->arrange(press: self::TA, state: 1, selected: 4);
        $this->shouldWriteLong($this->menu(self::STATE), 6);
        $this->shouldCall('_FadePushOut_8c022b60')->with(10);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 0, 0);
        $this->shouldDrawRowLabels(selected: 4);
        $this->shouldDrawSensitivityBars(state: 6, accel: 0, brake: 0);
        $this->shouldDrawChrome(selected: 4);
    }

    public function test_cancel_fades_out()
    {
        $this->arrange(press: self::TB, state: 1, selected: 0);
        $this->shouldWriteLong($this->menu(self::STATE), 6);
        $this->shouldCall('_FadePushOut_8c022b60')->with(10);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 1, 0);
        $this->shouldDrawRowLabels(selected: 0);
        $this->shouldDrawSensitivityBars(state: 6, accel: 0, brake: 0);
        $this->shouldDrawChrome(selected: 0);
    }

    public function test_edit_button_variant_a_controller()
    {
        $this->arrange(press: 0, state: 2, selected: 0, ctrlType: self::CONTROLLER, variant: 0, choiceAPad: 1);
        $this->shouldCall('_keyConfigEditExit_8c01a4b4')->andReturn(0);
        $this->shouldCall('_cycleValue_8c01a3da')->with($this->progress(self::CHOICE_A_PAD), 3);
        $this->shouldDrawRowLabels(selected: 0);
        $this->shouldDrawButtonIcon(idx: 0x36);   // 0x35 + choice 1
        $this->shouldDrawChrome(selected: 0);
    }

    public function test_edit_button_variant_a_racing()
    {
        $this->arrange(press: 0, state: 2, selected: 0, ctrlType: self::RACING, variant: 0, choiceARace: 1);
        $this->shouldCall('_keyConfigEditExit_8c01a4b4')->andReturn(0);
        $this->shouldCall('_cycleValue_8c01a3da')->with($this->progress(self::CHOICE_A_RACE), 2);
        $this->shouldDrawRowLabels(selected: 0);
        $this->shouldDrawButtonIcon(idx: 0x3c);   // 0x3b + choice 1
        $this->shouldDrawChrome(selected: 0);
    }

    public function test_edit_button_variant_b_controller()
    {
        $this->arrange(press: 0, state: 2, selected: 0, ctrlType: self::CONTROLLER, variant: 1, choiceBPad: 2);
        $this->shouldCall('_keyConfigEditExit_8c01a4b4')->andReturn(0);
        $this->shouldCall('_cycleValue_8c01a3da')->with($this->progress(self::CHOICE_B_PAD), 3);
        $this->shouldDrawRowLabels(selected: 0);
        $this->shouldDrawButtonIcon(idx: 0x3a);   // 0x38 + choice 2
        $this->shouldDrawChrome(selected: 0);
    }

    public function test_edit_button_variant_b_racing()
    {
        $this->arrange(press: 0, state: 2, selected: 0, ctrlType: self::RACING, variant: 1, choiceBRace: 2);
        $this->shouldCall('_keyConfigEditExit_8c01a4b4')->andReturn(0);
        $this->shouldCall('_cycleValue_8c01a3da')->with($this->progress(self::CHOICE_B_RACE), 3);
        $this->shouldDrawRowLabels(selected: 0);
        $this->shouldDrawButtonIcon(idx: 0x3f);   // 0x3d + choice 2
        $this->shouldDrawChrome(selected: 0);
    }

    public function test_edit_button_unknown_controller_draws_no_icon()
    {
        // Neither BT_CONTROLLER nor BT_RACING: no cycleValue call, and the icon
        // draw is skipped entirely (idx stays -1) -- but we're still in state 2.
        $this->arrange(press: 0, state: 2, selected: 0, ctrlType: 0x12345, variant: 0);
        $this->shouldCall('_keyConfigEditExit_8c01a4b4')->andReturn(0);
        $this->shouldDrawRowLabels(selected: 0);
        $this->shouldDrawChrome(selected: 0);
    }

    public function test_edit_button_exit_skips_cycle_this_frame()
    {
        // keyConfigEditExit signals the edit ended this frame (its state/midi side
        // effects are its own contract); we only assert OptionKeyConfigTask skips
        // cycleValue and still draws the (stale) state-2 icon this same frame.
        $this->arrange(press: 0, state: 2, selected: 0, ctrlType: self::CONTROLLER, variant: 0, choiceAPad: 1);
        $this->shouldCall('_keyConfigEditExit_8c01a4b4')->andReturn(1);
        $this->shouldDrawRowLabels(selected: 0);
        $this->shouldDrawButtonIcon(idx: 0x36);
        $this->shouldDrawChrome(selected: 0);
    }

    public function test_edit_accel_reads_right_trigger()
    {
        $this->arrange(press: 0, state: 3, selected: 0, r: 0x50);
        $this->shouldCall('_keyConfigEditExit_8c01a4b4')->andReturn(0);
        $this->shouldWriteByte($this->progress(self::ACCEL), 0x50);
        $this->shouldDrawRowLabels(selected: 0);
        $this->shouldDrawSensitivityBars(state: 3, accel: 0x50, brake: 0);
        $this->shouldDrawChrome(selected: 0);
    }

    public function test_edit_accel_clamps_to_0x80()
    {
        $this->arrange(press: 0, state: 3, selected: 0, r: 0x95);
        $this->shouldCall('_keyConfigEditExit_8c01a4b4')->andReturn(0);
        $this->shouldWriteByte($this->progress(self::ACCEL), 0x95);
        $this->shouldWriteByte($this->progress(self::ACCEL), 0x80);
        // 0x80 in the signed char field sign-extends to -128 when passed on to
        // drawSensitivityBar_8c01a42a; the callee reads only the low byte, so this
        // is functionally identical to unsigned 0x80.
        $this->shouldDrawRowLabels(selected: 0);
        $this->shouldDrawSensitivityBars(state: 3, accel: -128, brake: 0);
        $this->shouldDrawChrome(selected: 0);
    }

    public function test_edit_accel_exit_skips_read()
    {
        $this->arrange(press: 0, state: 3, selected: 0, accel: 0x10);
        $this->shouldCall('_keyConfigEditExit_8c01a4b4')->andReturn(1);
        $this->shouldDrawRowLabels(selected: 0);
        $this->shouldDrawSensitivityBars(state: 3, accel: 0x10, brake: 0);
        $this->shouldDrawChrome(selected: 0);
    }

    public function test_edit_brake_reads_left_trigger()
    {
        $this->arrange(press: 0, state: 4, selected: 0, l: 0x60);
        $this->shouldCall('_keyConfigEditExit_8c01a4b4')->andReturn(0);
        $this->shouldWriteByte($this->progress(self::BRAKE), 0x60);
        $this->shouldDrawRowLabels(selected: 0);
        $this->shouldDrawSensitivityBars(state: 4, accel: 0, brake: 0x60);
        $this->shouldDrawChrome(selected: 0);
    }

    public function test_edit_brake_clamps_to_0x80()
    {
        $this->arrange(press: 0, state: 4, selected: 0, l: 0xa0);
        $this->shouldCall('_keyConfigEditExit_8c01a4b4')->andReturn(0);
        $this->shouldWriteByte($this->progress(self::BRAKE), 0xa0);
        $this->shouldWriteByte($this->progress(self::BRAKE), 0x80);
        $this->shouldDrawRowLabels(selected: 0);
        $this->shouldDrawSensitivityBars(state: 4, accel: 0, brake: -128);
        $this->shouldDrawChrome(selected: 0);
    }

    public function test_phase6_hands_back_to_top_menu_when_fade_done()
    {
        $this->arrange(press: 0, state: 6, selected: 0, isFading: 0);
        $this->shouldCall('_OptionSwitchToTopMenu_8c01b122')->with($this->task, 1);
    }

    public function test_phase6_keeps_fading()
    {
        $this->arrange(press: 0, state: 6, selected: 4, isFading: 1);
        $this->shouldDrawRowLabels(selected: 4);
        $this->shouldDrawSensitivityBars(state: 6, accel: 0, brake: 0);
        $this->shouldDrawChrome(selected: 4);
    }

    public function test_default_row_highlight()
    {
        $this->arrange(press: 0, state: 1, selected: 3);
        $this->shouldDrawRowLabels(selected: 3);
        $this->shouldDrawSensitivityBars(state: 1, accel: 0, brake: 0);
        $this->shouldDrawChrome(selected: 3);
    }

    public function test_return_row_highlight()
    {
        $this->arrange(press: 0, state: 1, selected: 4);
        $this->shouldDrawRowLabels(selected: 4);
        $this->shouldDrawSensitivityBars(state: 1, accel: 0, brake: 0);
        $this->shouldDrawChrome(selected: 4);
    }
};
