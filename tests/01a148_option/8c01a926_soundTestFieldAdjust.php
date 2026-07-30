<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    const PRESS = 16;              // var_peripherals_8c1ba35c[0].press offset
    const STATE = 0x18;            // var_menuState_8c1bc7a8.state_0x18 offset
    const FIELD = 0x3c;            // var_menuState_8c1bc7a8.field_0x3c (edit digit)
    const KU = 1 << 4;             // up    -> increment digit
    const KD = 1 << 5;             // down  -> decrement digit
    const KL = 1 << 6;             // left  -> move edit digit up
    const KR = 1 << 7;             // right -> move edit digit down
    const TB = 1 << 1;             // cancel -> leave field
    const MIDI = 0xd1d1d1d1;

    private function menu(int $off): int
    {
        return $this->addressOf('_var_menuState_8c1bc7a8') + $off;
    }

    private function arrange(int $press, int $field = 0, int $state = 0): int
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + self::PRESS, $press);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), self::MIDI);
        $this->initUint32($this->menu(self::FIELD), $field);
        $this->initUint32($this->menu(self::STATE), $state);

        // Field-reload-from-max path divides by 10 via the signed runtime helpers.
        $this->setSize('__modls', 4);
        $this->setSize('__divls', 4);
        $this->onCall('__modls', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });
        $this->onCall('__divls', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });
        return 0;
    }

    private function digits(array $vals): int
    {
        $ptr = $this->alloc(4 * count($vals));
        foreach ($vals as $i => $v) {
            $this->initUint32($ptr + 4 * $i, $v);
        }
        return $ptr;
    }

    public function test_left_moves_edit_digit_up()
    {
        $this->arrange(self::KL, 0);
        $d = $this->digits([0, 0]);
        $this->call('_soundTestFieldAdjust_8c01a926')->with($d, 2, 99);
        $this->shouldWriteLong($this->menu(self::FIELD), 1);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
    }

    public function test_left_clamped_at_top_digit()
    {
        $this->arrange(self::KL, 1);   // already at count-1
        $d = $this->digits([0, 0]);
        $this->call('_soundTestFieldAdjust_8c01a926')->with($d, 2, 99);
    }

    public function test_right_moves_edit_digit_down()
    {
        $this->arrange(self::KR, 1);
        $d = $this->digits([0, 0]);
        $this->call('_soundTestFieldAdjust_8c01a926')->with($d, 2, 99);
        $this->shouldWriteLong($this->menu(self::FIELD), 0);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
    }

    public function test_up_increments_digit()
    {
        $this->arrange(self::KU, 0);
        $d = $this->digits([5, 0]);
        $this->call('_soundTestFieldAdjust_8c01a926')->with($d, 2, 99);
        $this->shouldWriteLong($d, 6);
        $this->shouldCall('_soundTestFieldRead_8c01a904')->with($d, 2)->andReturn(6);
    }

    public function test_up_carries_across_digits()
    {
        $this->arrange(self::KU, 0);
        $d = $this->digits([9, 0]);
        $this->call('_soundTestFieldAdjust_8c01a926')->with($d, 2, 99);
        $this->shouldWriteLong($d, 10);
        $this->shouldWriteLong($d, 0);
        $this->shouldWriteLong($d + 4, 1);
        $this->shouldCall('_soundTestFieldRead_8c01a904')->with($d, 2)->andReturn(10);
    }

    public function test_up_past_max_wraps_to_zero()
    {
        $this->arrange(self::KU, 0);
        $d = $this->digits([9, 9]);
        $this->call('_soundTestFieldAdjust_8c01a926')->with($d, 2, 99);
        $this->shouldWriteLong($d, 10);
        $this->shouldWriteLong($d, 0);
        $this->shouldWriteLong($d + 4, 10);
        $this->shouldWriteLong($d + 4, 0);
        $this->shouldCall('_soundTestFieldRead_8c01a904')->with($d, 2)->andReturn(100);
        $this->shouldWriteLong($d, 0);
        $this->shouldWriteLong($d + 4, 0);
    }

    public function test_down_decrements_digit()
    {
        $this->arrange(self::KD, 0);
        $d = $this->digits([5, 0]);
        $this->call('_soundTestFieldAdjust_8c01a926')->with($d, 2, 99);
        $this->shouldWriteLong($d, 4);
    }

    public function test_down_borrow_wraps_to_max()
    {
        $this->arrange(self::KD, 0);
        $d = $this->digits([0, 0]);
        $this->call('_soundTestFieldAdjust_8c01a926')->with($d, 2, 42);
        $this->shouldWriteLong($d, -1);
        $this->shouldWriteLong($d, 9);
        $this->shouldWriteLong($d + 4, -1);
        $this->shouldWriteLong($d + 4, 9);
        $this->shouldCall('__modls');
        $this->shouldWriteLong($d, 2);        // 42 % 10
        $this->shouldCall('__divls');         // max = 42 / 10 = 4
        $this->shouldCall('__modls');
        $this->shouldWriteLong($d + 4, 4);    // 4 % 10
        $this->shouldCall('__divls');         // max = 4 / 10 = 0
    }

    public function test_cancel_music_test_stops_via_010ca6_0()
    {
        $this->arrange(self::TB, 0, 6);
        $d = $this->digits([0, 0]);
        $this->call('_soundTestFieldAdjust_8c01a926')->with($d, 2, 99);
        $this->shouldCall('_FUN_8c010ca6')->with(0);
        $this->shouldWriteLong($this->menu(self::STATE), 1);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 1, 0);
    }

    public function test_cancel_sfx_test_stops_playback()
    {
        $this->arrange(self::TB, 0, 7);
        $d = $this->digits([0, 0]);
        $this->call('_soundTestFieldAdjust_8c01a926')->with($d, 2, 99);
        $this->shouldCall('_sdMidiStop')->with(self::MIDI);
        $this->shouldWriteLong($this->menu(self::STATE), 1);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 1, 0);
    }

    public function test_cancel_voice_test_stops_via_010ca6_1()
    {
        $this->arrange(self::TB, 0, 8);
        $d = $this->digits([0, 0]);
        $this->call('_soundTestFieldAdjust_8c01a926')->with($d, 2, 99);
        $this->shouldCall('_FUN_8c010ca6')->with(1);
        $this->shouldWriteLong($this->menu(self::STATE), 1);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 1, 0);
    }

    public function test_cancel_other_phase_just_returns_to_navigate()
    {
        $this->arrange(self::TB, 0, 2);
        $d = $this->digits([0, 0]);
        $this->call('_soundTestFieldAdjust_8c01a926')->with($d, 2, 99);
        $this->shouldWriteLong($this->menu(self::STATE), 1);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 1, 0);
    }

    public function test_no_input_does_nothing()
    {
        $this->arrange(0, 0, 1);
        $d = $this->digits([0, 0]);
        $this->call('_soundTestFieldAdjust_8c01a926')->with($d, 2, 99);
    }
};
