<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\CallingConventions\RiroCallingConvention;

return new class extends TestCase {
    public function test_draws_single_digit_for_zero_value(): void
    {
        $this->call('_drawScoreDigits_8c01d7fc')->with(0, 12.0);

        $this->shouldCall('__modls')->with(0, 10)->using(new RiroCallingConvention())->andReturn(0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $this->addressOf('_var_menuState_8c1bc7a8'),
            0x1f,
            464.0,
            12.0,
            -3.0,
        );
        $this->shouldCall('__divls')->with(0, 10)->using(new RiroCallingConvention())->andReturn(0);
    }

    public function test_draws_one_sprite_per_digit_scrolling_left(): void
    {
        $this->call('_drawScoreDigits_8c01d7fc')->with(123, 12.0);

        $this->shouldCall('__modls')->with(123, 10)->using(new RiroCallingConvention())->andReturn(3);
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $this->addressOf('_var_menuState_8c1bc7a8'),
            0x22,
            464.0,
            12.0,
            -3.0,
        );
        $this->shouldCall('__divls')->with(123, 10)->using(new RiroCallingConvention())->andReturn(12);

        $this->shouldCall('__modls')->with(12, 10)->using(new RiroCallingConvention())->andReturn(2);
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $this->addressOf('_var_menuState_8c1bc7a8'),
            0x21,
            446.0,
            12.0,
            -3.0,
        );
        $this->shouldCall('__divls')->with(12, 10)->using(new RiroCallingConvention())->andReturn(1);

        $this->shouldCall('__modls')->with(1, 10)->using(new RiroCallingConvention())->andReturn(1);
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $this->addressOf('_var_menuState_8c1bc7a8'),
            0x20,
            428.0,
            12.0,
            -3.0,
        );
        $this->shouldCall('__divls')->with(1, 10)->using(new RiroCallingConvention())->andReturn(0);
    }
};
