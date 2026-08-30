<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\CallingConventions\RiroCallingConvention;

return new class extends TestCase {
    public function test_draws_six_digit_hms_readout(): void
    {
        // value = 12h * 108000 + 34m * 1800 + 56s * 30 (30fps-based frame count)
        $value = 12 * 108000 + 34 * 1800 + 56 * 30;

        $this->call('_drawTimeDigits_8c01fa80')->with($value, 12.0, 0x64);

        $this->shouldCall('__divls')->with($value, 108000)->using(new RiroCallingConvention())->andReturn(12);
        $this->shouldCall('__modls')->with($value, 108000)->using(new RiroCallingConvention())->andReturn(1080);
        $this->shouldCall('__divls')->with(1080, 1800)->using(new RiroCallingConvention())->andReturn(34);
        $this->shouldCall('__modls')->with($value, 1800)->using(new RiroCallingConvention())->andReturn(1680);
        $this->shouldCall('__divls')->with(1680, 30)->using(new RiroCallingConvention())->andReturn(56);

        $this->shouldCall('__modls')->with(12, 10)->using(new RiroCallingConvention())->andReturn(2);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with(
            $this->addressOf('_var_busStopTexlist_8c1bc424'),
            0x64 + 2,
            448.0,
            12.0,
            -1.21,
        );
        $this->shouldCall('__divls')->with(12, 10)->using(new RiroCallingConvention())->andReturn(1);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with(
            $this->addressOf('_var_busStopTexlist_8c1bc424'),
            0x64 + 1,
            436.0,
            12.0,
            -1.21,
        );

        $this->shouldCall('__modls')->with(34, 10)->using(new RiroCallingConvention())->andReturn(4);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with(
            $this->addressOf('_var_busStopTexlist_8c1bc424'),
            0x64 + 4,
            477.0,
            12.0,
            -1.21,
        );
        $this->shouldCall('__divls')->with(34, 10)->using(new RiroCallingConvention())->andReturn(3);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with(
            $this->addressOf('_var_busStopTexlist_8c1bc424'),
            0x64 + 3,
            465.0,
            12.0,
            -1.21,
        );

        $this->shouldCall('__modls')->with(56, 10)->using(new RiroCallingConvention())->andReturn(6);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with(
            $this->addressOf('_var_busStopTexlist_8c1bc424'),
            0x64 + 6,
            506.0,
            12.0,
            -1.21,
        );
        $this->shouldCall('__divls')->with(56, 10)->using(new RiroCallingConvention())->andReturn(5);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with(
            $this->addressOf('_var_busStopTexlist_8c1bc424'),
            0x64 + 5,
            494.0,
            12.0,
            -1.21,
        );
    }
};
