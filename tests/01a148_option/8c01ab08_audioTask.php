<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    const PRESS = 0x10;      // var_peripherals_8c1ba35c[0].press offset
    const STATE = 0x18;      // var_menuState_8c1bc7a8.state_0x18 offset
    const SELECTED = 0x38;   // .selected_0x38
    const FIELD3C = 0x3c;    // .cursorCol_0x3c (sound-test edit digit index)
    const LOGO = 0x68;       // .timer_0x68 (marker blink)
    const TA = 1 << 2;       // confirm
    const TB = 1 << 1;       // cancel
    const KU = 1 << 4;       // prev row
    const KD = 1 << 5;       // next row
    const MIDI = 0xd1d1d1d1;

    // PlayerProgress volume field offsets from var_progress_8c1ba1cc.
    const D4 = 0xd4;         // MUSIC volume
    const D5 = 0xd5;         // SFX volume
    const D6 = 0xd6;         // VOICE volume

    // Marker Y positions.
    const MUSIC_Y = 110.0;
    const SFX_Y = 146.0;
    const VOICE_Y = 182.0;

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
                             int $soundMode = 0, int $d4 = 0, int $d5 = 0, int $d6 = 0,
                             int $field3c = 0, int $logo = 1, int $isFading = 0): void
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_var_progress_8c1ba1cc', 0xd7);
        $this->setSize('_var_musicTestDigits_8c226078', 8);
        $this->setSize('_var_sfxTestDigits_8c226080', 8);
        $this->setSize('_var_voiceTestDigits_8c226088', 16);
        $this->setSize('_TxtDrawSprite_8c014f54', 4);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_SndSetSoundMode_8c0108c0', 4);
        $this->setSize('_SndSetAdxVol_8c010972', 4);
        $this->setSize('_SndSetMidiVolAndInitStruct_8c0109f4', 4);
        $this->setSize('_FileMenuResetSoundDefaults_8c0188dc', 4);
        $this->setSize('_FadePushOut_8c022b60', 4);
        $this->setSize('_FUN_8c0107ac', 4);
        $this->setSize('_FUN_8c0106d2', 4);
        $this->setSize('_FUN_8c010720', 4);

        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + self::PRESS, $press);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), self::MIDI);
        $this->initUint8($this->addressOf('_var_soundMode_8c226070'), $soundMode);
        $this->initUint32($this->menu(self::STATE), $state);
        $this->initUint32($this->menu(self::SELECTED), $selected);
        $this->initUint32($this->menu(self::FIELD3C), $field3c);
        $this->initUint32($this->menu(self::LOGO), $logo);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), $isFading);
        $this->initUint8($this->progress(self::D4), $d4);
        $this->initUint8($this->progress(self::D5), $d5);
        $this->initUint8($this->progress(self::D6), $d6);

        $this->task = $this->alloc(0x20);
        $this->call('_audioTask_8c01ab08')->with($this->task);
    }

    /* 7 row labels: highlighted row = i+0x4c, others i+0x45. */
    private function shouldDrawLabels(int $selected): void
    {
        $gB = $this->menu(0x0c);
        for ($i = 0; $i < 7; $i++) {
            $idx = ($i === $selected) ? $i + 0x4c : $i + 0x45;
            $this->shouldCall('_TxtDrawSprite_8c014f54')->with($gB, $idx, 0.0, 0.0, -5.0);
        }
    }

    /* SOUND-mode selector marker (0x29), x = 401 + 64*mode. */
    private function shouldDrawSoundModeMarker(int $soundMode): void
    {
        $this->shouldCall('_TxtDrawSprite_8c014f54')
            ->with($this->menu(0x0c), 0x29, 64.0 * $soundMode + 401.0, 69.0, -4.0);
    }

    /* A volume slider marker (0x5e) at row height $y, x = 400 + 20*value. */
    private function shouldDrawVolMarker(float $y, int $value): void
    {
        $this->shouldCall('_TxtDrawSprite_8c014f54')
            ->with($this->menu(0x0c), 0x5e, 20.0 * $value + 400.0, $y, -4.0);
    }

    /* All 4 markers unconditionally (no edit-blink) -- for non-2..5 states. */
    private function shouldDrawMarkersPlain(int $soundMode, int $d4, int $d5, int $d6): void
    {
        $this->shouldDrawSoundModeMarker($soundMode);
        $this->shouldDrawVolMarker(self::MUSIC_Y, $d4);
        $this->shouldDrawVolMarker(self::SFX_Y, $d5);
        $this->shouldDrawVolMarker(self::VOICE_Y, $d6);
    }

    /* The 3 sound-test digit fields (soundTestFieldDraw has its own test -> mocked). */
    private function shouldDrawTestFields(): void
    {
        $this->shouldCall('_soundTestFieldDraw_8c01aaaa')->with(441.0, 225.0, $this->addressOf('_var_musicTestDigits_8c226078'), 2);
        $this->shouldCall('_soundTestFieldDraw_8c01aaaa')->with(441.0, 261.0, $this->addressOf('_var_sfxTestDigits_8c226080'), 2);
        $this->shouldCall('_soundTestFieldDraw_8c01aaaa')->with(441.0, 298.0, $this->addressOf('_var_voiceTestDigits_8c226088'), 4);
    }

    /* Sound-test digit cursor (0x70) at row height $y, x = 436 - 26*off. */
    private function shouldDrawTestCursor(float $y, int $off): void
    {
        $this->shouldCall('_TxtDrawSprite_8c014f54')
            ->with($this->menu(0x0c), 0x70, 436.0 - 26.0 * $off, $y, -3.0);
    }

    /* Row highlight (0x61 row<7, 0x62 DEFAULT, 0x63 RETURN) + title + frame. */
    private function shouldDrawChrome(int $selected): void
    {
        $gB = $this->menu(0x0c);
        if ($selected < 7) {
            $this->shouldCall('_TxtDrawSprite_8c014f54')->with($gB, 0x61, 0.0, 0.0, -4.0);
        } else {
            $this->shouldCall('_TxtDrawSprite_8c014f54')
                ->with($gB, $selected === 7 ? 0x62 : 0x63, 0.0, 0.0, -4.0);
        }
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($gB, 0x53, 0.0, 0.0, -5.0);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->menu(0x00), 0, 0.0, 0.0, -7.0);
    }

    public function test_phase0_waits_while_fading()
    {
        $this->arrange(press: 0, state: 0, selected: 0, isFading: 1);
        $this->shouldDrawLabels(0);
        $this->shouldDrawMarkersPlain(0, 0, 0, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(0);
    }

    public function test_phase0_advances_when_fade_done()
    {
        $this->arrange(press: 0, state: 0, selected: 0, isFading: 0);
        $this->shouldWriteLong($this->menu(self::STATE), 1);
        $this->shouldDrawLabels(0);
        $this->shouldDrawMarkersPlain(0, 0, 0, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(0);
    }

    public function test_navigate_next_row()
    {
        $this->arrange(press: self::KD, state: 1, selected: 0);
        $this->shouldWriteLong($this->menu(self::SELECTED), 1);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldDrawLabels(1);
        $this->shouldDrawMarkersPlain(0, 0, 0, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(1);
    }

    public function test_navigate_prev_wraps_to_return()
    {
        $this->arrange(press: self::KU, state: 1, selected: 0);
        $this->shouldWriteLong($this->menu(self::SELECTED), -1);
        $this->shouldWriteLong($this->menu(self::SELECTED), 8);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldDrawLabels(8);
        $this->shouldDrawMarkersPlain(0, 0, 0, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(8);
    }

    public function test_confirm_row_enters_edit()
    {
        // Row 0 -> state 2 (SOUND mode edit); marker then blinks (seed odd frame).
        $this->arrange(press: self::TA, state: 1, selected: 0, logo: 1);
        $this->shouldWriteLong($this->menu(self::STATE), 2);
        $this->shouldWriteLong($this->menu(self::FIELD3C), 0);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 0, 0);
        $this->shouldDrawLabels(0);
        $this->shouldWriteLong($this->menu(self::LOGO), 2);   // sound-mode blink
        $this->shouldDrawSoundModeMarker(0);
        $this->shouldDrawVolMarker(self::MUSIC_Y, 0);
        $this->shouldDrawVolMarker(self::SFX_Y, 0);
        $this->shouldDrawVolMarker(self::VOICE_Y, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(0);
    }

    public function test_confirm_default_row_resets_sound()
    {
        $this->arrange(press: self::TA, state: 1, selected: 7);
        $this->shouldCall('_FileMenuResetSoundDefaults_8c0188dc');
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 0, 0);
        $this->shouldDrawLabels(7);
        $this->shouldDrawMarkersPlain(0, 0, 0, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(7);
    }

    public function test_confirm_return_row_fades_out()
    {
        $this->arrange(press: self::TA, state: 1, selected: 8, soundMode: 1);
        $this->shouldWriteLong($this->menu(self::STATE), 9);
        $this->shouldCall('_SndSetSoundMode_8c0108c0')->with(1);
        $this->shouldCall('_FadePushOut_8c022b60')->with(10);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 0, 0);
        $this->shouldDrawLabels(8);
        $this->shouldDrawMarkersPlain(1, 0, 0, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(8);
    }

    public function test_cancel_fades_out()
    {
        $this->arrange(press: self::TB, state: 1, selected: 0, soundMode: 1);
        $this->shouldWriteLong($this->menu(self::STATE), 9);
        $this->shouldCall('_SndSetSoundMode_8c0108c0')->with(1);
        $this->shouldCall('_FadePushOut_8c022b60')->with(10);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 1, 0);
        $this->shouldDrawLabels(0);
        $this->shouldDrawMarkersPlain(1, 0, 0, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(0);
    }

    public function test_edit_sound_mode()
    {
        // audioEditValue drives the cycle/exit (its own contract) -> mocked.
        $this->arrange(press: 0, state: 2, selected: 0, soundMode: 1, logo: 1);
        $this->shouldCall('_audioEditValue_8c01a8b6')->with($this->addressOf('_var_soundMode_8c226070'), 2);
        $this->shouldDrawLabels(0);
        $this->shouldWriteLong($this->menu(self::LOGO), 2);   // sound-mode blink
        $this->shouldDrawSoundModeMarker(1);
        $this->shouldDrawVolMarker(self::MUSIC_Y, 0);
        $this->shouldDrawVolMarker(self::SFX_Y, 0);
        $this->shouldDrawVolMarker(self::VOICE_Y, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(0);
    }

    public function test_edit_music_volume_holds()
    {
        // No confirm: cycle only, no SndSet this frame.
        $this->arrange(press: 0, state: 3, selected: 1, d4: 5, logo: 1);
        $this->shouldCall('_audioEditValue_8c01a8b6')->with($this->progress(self::D4), 10);
        $this->shouldDrawLabels(1);
        $this->shouldDrawSoundModeMarker(0);
        $this->shouldWriteLong($this->menu(self::LOGO), 2);   // MUSIC blink
        $this->shouldDrawVolMarker(self::MUSIC_Y, 5);
        $this->shouldDrawVolMarker(self::SFX_Y, 0);
        $this->shouldDrawVolMarker(self::VOICE_Y, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(1);
    }

    public function test_edit_music_volume_confirm_applies()
    {
        $this->arrange(press: self::TA, state: 3, selected: 1, d4: 5, logo: 1);
        $this->shouldCall('_audioEditValue_8c01a8b6')->with($this->progress(self::D4), 10);
        $this->shouldCall('_SndSetAdxVol_8c010972')->with(5, 0);
        $this->shouldDrawLabels(1);
        $this->shouldDrawSoundModeMarker(0);
        $this->shouldWriteLong($this->menu(self::LOGO), 2);
        $this->shouldDrawVolMarker(self::MUSIC_Y, 5);
        $this->shouldDrawVolMarker(self::SFX_Y, 0);
        $this->shouldDrawVolMarker(self::VOICE_Y, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(1);
    }

    public function test_edit_sfx_volume_confirm_applies()
    {
        $this->arrange(press: self::TA, state: 4, selected: 2, d5: 7, logo: 1);
        $this->shouldCall('_audioEditValue_8c01a8b6')->with($this->progress(self::D5), 10);
        $this->shouldCall('_SndSetMidiVolAndInitStruct_8c0109f4')->with(7);
        $this->shouldDrawLabels(2);
        $this->shouldDrawSoundModeMarker(0);
        $this->shouldDrawVolMarker(self::MUSIC_Y, 0);
        $this->shouldWriteLong($this->menu(self::LOGO), 2);
        $this->shouldDrawVolMarker(self::SFX_Y, 7);
        $this->shouldDrawVolMarker(self::VOICE_Y, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(2);
    }

    public function test_edit_voice_volume_confirm_applies()
    {
        $this->arrange(press: self::TA, state: 5, selected: 3, d6: 9, logo: 1);
        $this->shouldCall('_audioEditValue_8c01a8b6')->with($this->progress(self::D6), 10);
        $this->shouldCall('_SndSetAdxVol_8c010972')->with(9, 1);
        $this->shouldDrawLabels(3);
        $this->shouldDrawSoundModeMarker(0);
        $this->shouldDrawVolMarker(self::MUSIC_Y, 0);
        $this->shouldDrawVolMarker(self::SFX_Y, 0);
        $this->shouldWriteLong($this->menu(self::LOGO), 2);
        $this->shouldDrawVolMarker(self::VOICE_Y, 9);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(3);
    }

    public function test_edit_marker_hidden_on_even_frame()
    {
        // Editing MUSIC vol with an even timer_0x68: the marker is skipped, but
        // timer_0x68 still bumps.
        $this->arrange(press: 0, state: 3, selected: 1, d4: 5, logo: 2);
        $this->shouldCall('_audioEditValue_8c01a8b6')->with($this->progress(self::D4), 10);
        $this->shouldDrawLabels(1);
        $this->shouldDrawSoundModeMarker(0);
        $this->shouldWriteLong($this->menu(self::LOGO), 3);   // bump, marker skipped
        $this->shouldDrawVolMarker(self::SFX_Y, 0);
        $this->shouldDrawVolMarker(self::VOICE_Y, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(1);
    }

    public function test_music_test_adjusts_field()
    {
        // No confirm: soundTestFieldAdjust drives the digits (its own contract).
        $this->arrange(press: 0, state: 6, selected: 4, field3c: 1);
        $this->shouldCall('_soundTestFieldAdjust_8c01a926')->with($this->addressOf('_var_musicTestDigits_8c226078'), 2, 0x10);
        $this->shouldDrawTestCursor(220.0, 1);
        $this->shouldDrawLabels(4);
        $this->shouldDrawMarkersPlain(0, 0, 0, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(4);
    }

    public function test_music_test_confirm_plays()
    {
        $this->arrange(press: self::TA, state: 6, selected: 4, field3c: 0);
        $this->shouldCall('_soundTestFieldRead_8c01a904')->with($this->addressOf('_var_musicTestDigits_8c226078'), 2)->andReturn(0x2a);
        $this->shouldCall('_FUN_8c0107ac')->with(0x2a);
        $this->shouldDrawTestCursor(220.0, 0);
        $this->shouldDrawLabels(4);
        $this->shouldDrawMarkersPlain(0, 0, 0, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(4);
    }

    public function test_sfx_test_confirm_plays()
    {
        $this->arrange(press: self::TA, state: 7, selected: 5, field3c: 0);
        $this->shouldCall('_soundTestFieldRead_8c01a904')->with($this->addressOf('_var_sfxTestDigits_8c226080'), 2)->andReturn(0x11);
        $this->shouldCall('_FUN_8c0106d2')->with(0x11);
        $this->shouldDrawTestCursor(256.0, 0);
        $this->shouldDrawLabels(5);
        $this->shouldDrawMarkersPlain(0, 0, 0, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(5);
    }

    public function test_voice_test_confirm_plays()
    {
        $this->arrange(press: self::TA, state: 8, selected: 6, field3c: 2);
        $this->shouldCall('_soundTestFieldRead_8c01a904')->with($this->addressOf('_var_voiceTestDigits_8c226088'), 4)->andReturn(0x100);
        $this->shouldCall('_FUN_8c010720')->with(0x100);
        $this->shouldDrawTestCursor(292.0, 2);
        $this->shouldDrawLabels(6);
        $this->shouldDrawMarkersPlain(0, 0, 0, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(6);
    }

    public function test_phase9_hands_back_to_top_menu_when_fade_done()
    {
        $this->arrange(press: 0, state: 9, selected: 8, isFading: 0);
        $this->shouldCall('_OptionSwitchToTopMenu_8c01b122')->with($this->task, 2);
    }

    public function test_phase9_keeps_fading()
    {
        $this->arrange(press: 0, state: 9, selected: 8, isFading: 1);
        $this->shouldDrawLabels(8);
        $this->shouldDrawMarkersPlain(0, 0, 0, 0);
        $this->shouldDrawTestFields();
        $this->shouldDrawChrome(8);
    }
};
