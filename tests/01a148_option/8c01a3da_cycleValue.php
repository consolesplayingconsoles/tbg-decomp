<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    const PRESS = 16;              // var_peripherals_8c1ba35c[0].press offset
    const KL = 1 << 6;             // PDD_DGT_KL (left)  -> decrement
    const KR = 1 << 7;             // PDD_DGT_KR (right) -> increment
    const MIDI = 0xd1d1d1d1;

    private function arrange(int $press, int $value): int
    {
        $this->resolveSymbols();

        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + self::PRESS, $press);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), self::MIDI);

        $ptr = $this->alloc(1);
        $this->initUint8($ptr, $value);
        return $ptr;
    }

    public function test_decrement()
    {
        $ptr = $this->arrange(self::KL, 2);
        $this->call('_cycleValue_8c01a3da')->with($ptr, 5);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldWriteByte($ptr, 1);
    }

    public function test_decrement_wraps_to_count_minus_one()
    {
        $ptr = $this->arrange(self::KL, 0);
        $this->call('_cycleValue_8c01a3da')->with($ptr, 5);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldWriteByte($ptr, 4);
    }

    public function test_increment()
    {
        $ptr = $this->arrange(self::KR, 2);
        $this->call('_cycleValue_8c01a3da')->with($ptr, 5);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldWriteByte($ptr, 3);
    }

    public function test_increment_wraps_to_zero()
    {
        $ptr = $this->arrange(self::KR, 4);
        $this->call('_cycleValue_8c01a3da')->with($ptr, 5);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldWriteByte($ptr, 0);
    }

    public function test_no_button_leaves_value_and_plays_nothing()
    {
        $ptr = $this->arrange(0, 2);
        $this->call('_cycleValue_8c01a3da')->with($ptr, 5);
        $this->shouldWriteByte($ptr, 2);
    }

    public function resolveSymbols()
    {
    }
};
