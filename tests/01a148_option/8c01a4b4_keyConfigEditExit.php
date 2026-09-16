<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    const PRESS = 16;              // var_peripherals_8c1ba35c[0].press offset
    const STATE = 0x18;           // var_menuState_8c1bc7a8.state_0x18 offset
    const TA = 1 << 2;             // PDD_DGT_TA (confirm)
    const TB = 1 << 1;             // PDD_DGT_TB (cancel)
    const CONTROLLER = 0xf06fe;    // BT_CONTROLLER
    const RACING = 0x700fe;        // BT_RACING
    const MIDI = 0xd1d1d1d1;

    private function arrange(int $ctrlType, int $press)
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->initUint32($this->addressOf('_var_activeCtrlType_8c157a70'), $ctrlType);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + self::PRESS, $press);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), self::MIDI);
    }

    public function test_confirm_plays_sound_0_and_exits()
    {
        $this->arrange(self::CONTROLLER, self::TA);
        $this->call('_keyConfigEditExit_8c01a4b4');
        $this->shouldWriteLong($this->addressOf('_var_menuState_8c1bc7a8') + self::STATE, 1);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 0, 0);
        $this->shouldReturn(1);
    }

    public function test_cancel_plays_sound_1_and_exits()
    {
        $this->arrange(self::RACING, self::TB);
        $this->call('_keyConfigEditExit_8c01a4b4');
        $this->shouldWriteLong($this->addressOf('_var_menuState_8c1bc7a8') + self::STATE, 1);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 1, 0);
        $this->shouldReturn(1);
    }

    public function test_no_button_keeps_editing()
    {
        $this->arrange(self::CONTROLLER, 0);
        $this->call('_keyConfigEditExit_8c01a4b4');
        $this->shouldReturn(0);
    }

    public function test_unknown_controller_bails_out()
    {
        $this->arrange(0x12345, 0);
        $this->call('_keyConfigEditExit_8c01a4b4');
        $this->shouldWriteLong($this->addressOf('_var_menuState_8c1bc7a8') + self::STATE, 1);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 1, 0);
        $this->shouldReturn(1);
    }
};
