<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    const PRESS = 0x10;           // var_peripherals_8c1ba35c[0].press offset
    const STATE = 0x18;          // var_menuState_8c1bc7a8.state_0x18 offset
    const SELECTED = 0x38;       // var_menuState_8c1bc7a8.selected_0x38 offset
    const LOGO = 0x68;           // var_menuState_8c1bc7a8.timer_0x68 offset
    const TA = 1 << 2;           // confirm
    const TB = 1 << 1;           // cancel
    const KU = 1 << 4;           // prev row
    const KD = 1 << 5;           // next row
    const KL = 1 << 6;           // decrement value
    const KR = 1 << 7;           // increment value
    const MIDI = 0xd1d1d1d1;

    private int $settings = 0;

    private function menu(int $off): int
    {
        return $this->addressOf('_var_menuState_8c1bc7a8') + $off;
    }

    /* Seed globals; point var_settingValues_8c226074 at a fresh 5-byte settings array. */
    private function arrange(int $press, int $state, int $selected,
                             array $values, int $isFading = 0, int $logo = 1): void
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_SpriteDraw_8c014f54', 4);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_FileSelectResetSettingDefaults_8c018862', 4);
        $this->setSize('_RenderStartFadeOut_8c022b60', 4);

        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + self::PRESS, $press);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), self::MIDI);
        $this->initUint32($this->menu(self::STATE), $state);
        $this->initUint32($this->menu(self::SELECTED), $selected);
        $this->initUint32($this->menu(self::LOGO), $logo);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), $isFading);

        $this->settings = $this->alloc(5);
        foreach ($values as $i => $v) {
            $this->initUint8($this->settings + $i, $v & 0xff);
        }
        $this->initUint32($this->addressOf('_var_settingValues_8c226074'), $this->settings);
    }

    /*
     * Queue the per-frame draw. Each of the 5 toggle rows draws a label sprite,
     * then a value marker (positioned at 337 + 64*value, 77 + 56*row); the marker
     * blinks off when its row is being edited on an even timer_0x68 frame -- and
     * the edited row always bumps timer_0x68. Then the DEFAULT-area sprite, the
     * RETURN sprite, and the title frame.
     */
    private function expectDraw(int $state, int $selected, array $values, int $logo): void
    {
        $gB = $this->menu(0x0c);
        $gA = $this->menu(0x00);
        for ($i = 0; $i < 5; $i++) {
            $label = ($i === $selected) ? $i + 0x23 : $i + 0x1e;
            $this->shouldCall('_SpriteDraw_8c014f54')->with($gB, $label, 0.0, 0.0, -5.0);

            $editing = $state === 2 && $i === $selected;
            $draw = true;
            if ($editing) {
                $draw = ($logo & 1) !== 0;
                $this->shouldWriteLong($this->menu(self::LOGO), $logo + 1);
            }
            if ($draw) {
                $x = 337.0 + 64.0 * $values[$i];
                $y = 77.0 + 56.0 * $i;
                $this->shouldCall('_SpriteDraw_8c014f54')->with($gB, 0x29, $x, $y, -4.0);
            }
        }
        if ($selected < 5) {
            $this->shouldCall('_SpriteDraw_8c014f54')->with($gB, 0x61, 0.0, 0.0, -4.0);
        } else {
            $idx = ($selected === 5) ? 0x62 : 0x63;
            $this->shouldCall('_SpriteDraw_8c014f54')->with($gB, $idx, 0.0, 0.0, -4.0);
        }
        $this->shouldCall('_SpriteDraw_8c014f54')->with($gB, 0x28, 0.0, 0.0, -5.0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($gA, 0, 0.0, 0.0, -7.0);
    }

    public function test_phase0_waits_while_fading()
    {
        $this->arrange(0, 0, 0, [0, 0, 0, 0, 0], 1);
        $this->call('_settingTask_8c01a148');
        $this->expectDraw(0, 0, [0, 0, 0, 0, 0], 1);
    }

    public function test_phase0_advances_when_fade_done()
    {
        $this->arrange(0, 0, 0, [0, 0, 0, 0, 0], 0);
        $this->call('_settingTask_8c01a148');
        $this->shouldWriteLong($this->menu(self::STATE), 1);
        $this->expectDraw(1, 0, [0, 0, 0, 0, 0], 1);
    }

    public function test_navigate_next_row()
    {
        $this->arrange(self::KD, 1, 0, [0, 0, 0, 0, 0]);
        $this->call('_settingTask_8c01a148');
        $this->shouldWriteLong($this->menu(self::SELECTED), 1);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->expectDraw(1, 1, [0, 0, 0, 0, 0], 1);
    }

    public function test_navigate_prev_wraps_to_return()
    {
        $this->arrange(self::KU, 1, 0, [0, 0, 0, 0, 0]);
        $this->call('_settingTask_8c01a148');
        $this->shouldWriteLong($this->menu(self::SELECTED), -1);
        $this->shouldWriteLong($this->menu(self::SELECTED), 6);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->expectDraw(1, 6, [0, 0, 0, 0, 0], 1);
    }

    public function test_confirm_toggle_row_enters_edit()
    {
        $this->arrange(self::TA, 1, 0, [0, 0, 0, 0, 0]);
        $this->call('_settingTask_8c01a148');
        $this->shouldWriteLong($this->menu(self::STATE), 2);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 0, 0);
        $this->expectDraw(2, 0, [0, 0, 0, 0, 0], 1);
    }

    public function test_confirm_default_row_resets_settings()
    {
        $this->arrange(self::TA, 1, 5, [0, 0, 0, 0, 0]);
        $this->call('_settingTask_8c01a148');
        $this->shouldCall('_FileSelectResetSettingDefaults_8c018862');
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 0, 0);
        $this->expectDraw(1, 5, [0, 0, 0, 0, 0], 1);
    }

    public function test_confirm_return_row_fades_out()
    {
        $this->arrange(self::TA, 1, 6, [0, 0, 0, 0, 0]);
        $this->call('_settingTask_8c01a148');
        $this->shouldWriteLong($this->menu(self::STATE), 3);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 0, 0);
        $this->expectDraw(3, 6, [0, 0, 0, 0, 0], 1);
    }

    public function test_cancel_fades_out()
    {
        $this->arrange(self::TB, 1, 0, [0, 0, 0, 0, 0]);
        $this->call('_settingTask_8c01a148');
        $this->shouldWriteLong($this->menu(self::STATE), 3);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 1, 0);
        $this->expectDraw(3, 0, [0, 0, 0, 0, 0], 1);
    }

    public function test_edit_increment_value()
    {
        $this->arrange(self::KR, 2, 0, [0, 0, 0, 0, 0]);
        $this->call('_settingTask_8c01a148');
        $this->shouldWriteByte($this->settings, 1);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->expectDraw(2, 0, [1, 0, 0, 0, 0], 1);
    }

    public function test_edit_increment_wraps_at_count()
    {
        // DIFFICULTY (row 0) has 3 options; 2 -> 3 -> wraps to 0.
        $this->arrange(self::KR, 2, 0, [2, 0, 0, 0, 0]);
        $this->call('_settingTask_8c01a148');
        $this->shouldWriteByte($this->settings, 3);
        $this->shouldWriteByte($this->settings, 0);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->expectDraw(2, 0, [0, 0, 0, 0, 0], 1);
    }

    public function test_edit_decrement_wraps_to_top()
    {
        // DIFFICULTY (row 0) has 3 options; 0 -> -1 -> wraps to 2.
        $this->arrange(self::KL, 2, 0, [0, 0, 0, 0, 0]);
        $this->call('_settingTask_8c01a148');
        $this->shouldWriteByte($this->settings, 0xff);
        $this->shouldWriteByte($this->settings, 2);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->expectDraw(2, 0, [2, 0, 0, 0, 0], 1);
    }

    public function test_edit_confirm_returns_to_navigate()
    {
        $this->arrange(self::TA, 2, 0, [0, 0, 0, 0, 0]);
        $this->call('_settingTask_8c01a148');
        $this->shouldWriteLong($this->menu(self::STATE), 1);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 0, 0);
        $this->expectDraw(1, 0, [0, 0, 0, 0, 0], 1);
    }

    public function test_edit_marker_blinks_hidden_on_even_frame()
    {
        // Editing row 0 with an even timer_0x68: that row's marker is skipped.
        $this->arrange(0, 2, 0, [0, 0, 0, 0, 0], 0, 0);
        $this->call('_settingTask_8c01a148');
        $this->expectDraw(2, 0, [0, 0, 0, 0, 0], 0);
    }

    public function test_phase3_hands_back_to_top_menu_when_fade_done()
    {
        $this->arrange(0, 3, 0, [0, 0, 0, 0, 0], 0);
        $task = $this->alloc(0x20);
        $this->call('_settingTask_8c01a148')->with($task);
        $this->shouldCall('_OptionEnter_8c01b122')->with($task, 0);
    }

    public function test_phase3_keeps_fading()
    {
        $this->arrange(0, 3, 0, [0, 0, 0, 0, 0], 1);
        $this->call('_settingTask_8c01a148');
        $this->expectDraw(3, 0, [0, 0, 0, 0, 0], 1);
    }
};
