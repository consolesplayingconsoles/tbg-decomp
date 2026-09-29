<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value)
    {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    private function setupCachedRows(): void
    {
        // var_nightLightIntensityStep_8c1bbda0[2], var_nightLightIntensityOff_8c1bbda8[2], var_nightLightIntensityOn_8c1bbdb0[2], var_nightLightColorStep_8c1bbdb8[3],
        // var_nightLightColorOff_8c1bbdc4[3], var_nightLightColorOn_8c1bbdd0[3].
        $this->setSize('_var_nightLightIntensityStep_8c1bbda0', 8);
        $this->setSize('_var_nightLightIntensityOff_8c1bbda8', 8);
        $this->setSize('_var_nightLightIntensityOn_8c1bbdb0', 8);
        $this->setSize('_var_nightLightColorStep_8c1bbdb8', 12);
        $this->setSize('_var_nightLightColorOff_8c1bbdc4', 12);
        $this->setSize('_var_nightLightColorOn_8c1bbdd0', 12);

        $da0 = $this->addressOf('_var_nightLightIntensityStep_8c1bbda0');
        $da8 = $this->addressOf('_var_nightLightIntensityOff_8c1bbda8');
        $db0 = $this->addressOf('_var_nightLightIntensityOn_8c1bbdb0');
        $db8 = $this->addressOf('_var_nightLightColorStep_8c1bbdb8');
        $dc4 = $this->addressOf('_var_nightLightColorOff_8c1bbdc4');
        $dd0 = $this->addressOf('_var_nightLightColorOn_8c1bbdd0');

        // Per-frame delta: (row2 - row1) / 20.
        $this->initUint32($da0 + 0x0, fdec(1.0));
        $this->initUint32($da0 + 0x4, fdec(2.0));
        $this->initUint32($db8 + 0x0, fdec(3.0));
        $this->initUint32($db8 + 0x4, fdec(4.0));
        $this->initUint32($db8 + 0x8, fdec(5.0));

        // "off" row (row1).
        $this->initUint32($da8 + 0x0, fdec(0.0));
        $this->initUint32($da8 + 0x4, fdec(0.0));
        $this->initUint32($dc4 + 0x0, fdec(0.0));
        $this->initUint32($dc4 + 0x4, fdec(0.0));
        $this->initUint32($dc4 + 0x8, fdec(0.0));

        // "on" row (row2).
        $this->initUint32($db0 + 0x0, fdec(20.0));
        $this->initUint32($db0 + 0x4, fdec(40.0));
        $this->initUint32($dd0 + 0x0, fdec(60.0));
        $this->initUint32($dd0 + 0x4, fdec(80.0));
        $this->initUint32($dd0 + 0x8, fdec(100.0));
    }

    private function makeBus(int $state, int $keepOn, float $c0): int
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $bus = $this->addressOf('_var_busState_8c1bb9d0');
        $this->doNotRandomizeMemory();

        $this->initUint32($bus + 0x80, 0); // blinker_0x080
        $this->initUint32($bus + 0x2d8, $state);
        $this->initUint32($bus + 0x2dc, $keepOn);
        $this->initUint32($bus + 0xc4, fdec($c0));
        $this->initUint32($bus + 0xc8, fdec(0.0));
        $this->initUint32($bus + 0xcc, fdec(0.0));
        $this->initUint32($bus + 0xd0, fdec(0.0));
        $this->initUint32($bus + 0xd4, fdec(0.0));

        return $bus;
    }

    public function test_state0_idle_stays_idle_when_not_requested(): void
    {
        $this->setupCachedRows();
        $bus = $this->makeBus(0, 0, 0.0);

        $this->call('_BusDrawFadeLights_8c028022')->with($bus);

        $this->shouldWriteLong($bus + 0x80, 0x18);
    }

    public function test_state0_starts_ramp_when_requested(): void
    {
        $this->setupCachedRows();
        $bus = $this->makeBus(0, 1, 0.0);

        $this->call('_BusDrawFadeLights_8c028022')->with($bus);

        $this->shouldWriteLong($bus + 0x80, 0x18);
        $this->shouldWriteLong($bus + 0x2d8, 1);
    }

    public function test_state1_ramps_up_without_reaching_target(): void
    {
        $this->setupCachedRows();
        $bus = $this->makeBus(1, 1, 0.0);

        $this->call('_BusDrawFadeLights_8c028022')->with($bus);

        $this->shouldWriteLong($bus + 0x80, 0x18);
        $this->shouldWriteLong($bus + 0xc4, fdec(1.0));
        $this->shouldWriteLong($bus + 0xc8, fdec(2.0));
        $this->shouldWriteLong($bus + 0xcc, fdec(3.0));
        $this->shouldWriteLong($bus + 0xd0, fdec(4.0));
        $this->shouldWriteLong($bus + 0xd4, fdec(5.0));
    }

    public function test_state1_reaches_target_and_snaps_to_on_row(): void
    {
        $this->setupCachedRows();
        $bus = $this->makeBus(1, 1, 19.5); // one step (+1.0) overshoots 20.0

        $this->call('_BusDrawFadeLights_8c028022')->with($bus);

        $this->shouldWriteLong($bus + 0x80, 0x18);
        // The unclamped sum is written first, then overwritten by the snap.
        $this->shouldWriteLong($bus + 0xc4, fdec(20.5));
        $this->shouldWriteLong($bus + 0xc4, fdec(20.0));
        $this->shouldWriteLong($bus + 0xc8, fdec(40.0));
        $this->shouldWriteLong($bus + 0xcc, fdec(60.0));
        $this->shouldWriteLong($bus + 0xd0, fdec(80.0));
        $this->shouldWriteLong($bus + 0xd4, fdec(100.0));
        $this->shouldWriteLong($bus + 0x2d8, 2);
    }

    public function test_state2_holds_while_still_requested(): void
    {
        $this->setupCachedRows();
        $bus = $this->makeBus(2, 1, 20.0);

        $this->call('_BusDrawFadeLights_8c028022')->with($bus);

        $this->shouldWriteLong($bus + 0x80, 0x18);
    }

    public function test_state2_starts_ramp_down_when_no_longer_requested(): void
    {
        $this->setupCachedRows();
        $bus = $this->makeBus(2, 0, 20.0);

        $this->call('_BusDrawFadeLights_8c028022')->with($bus);

        $this->shouldWriteLong($bus + 0x80, 0x18);
        $this->shouldWriteLong($bus + 0x2d8, 3);
    }

    public function test_state3_ramps_down_without_reaching_target(): void
    {
        $this->setupCachedRows();
        $bus = $this->makeBus(3, 0, 20.0);

        $this->call('_BusDrawFadeLights_8c028022')->with($bus);

        $this->shouldWriteLong($bus + 0x80, 0x18);
        $this->shouldWriteLong($bus + 0xc4, fdec(19.0));
        $this->shouldWriteLong($bus + 0xc8, fdec(-2.0));
        $this->shouldWriteLong($bus + 0xcc, fdec(-3.0));
        $this->shouldWriteLong($bus + 0xd0, fdec(-4.0));
        $this->shouldWriteLong($bus + 0xd4, fdec(-5.0));
    }

    public function test_state3_reaches_target_and_snaps_to_off_row(): void
    {
        $this->setupCachedRows();
        $bus = $this->makeBus(3, 0, 0.5); // one step (-1.0) undershoots 0.0

        $this->call('_BusDrawFadeLights_8c028022')->with($bus);

        $this->shouldWriteLong($bus + 0x80, 0x18);
        // The unclamped difference is written first, then overwritten by the
        // snap.
        $this->shouldWriteLong($bus + 0xc4, fdec(-0.5));
        $this->shouldWriteLong($bus + 0xc4, fdec(0.0));
        $this->shouldWriteLong($bus + 0xc8, fdec(0.0));
        $this->shouldWriteLong($bus + 0xcc, fdec(0.0));
        $this->shouldWriteLong($bus + 0xd0, fdec(0.0));
        $this->shouldWriteLong($bus + 0xd4, fdec(0.0));
        $this->shouldWriteLong($bus + 0x2d8, 0);
    }
};
