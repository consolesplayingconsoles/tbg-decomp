<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\CallingConventions\RiroCallingConvention;

return new class extends TestCase {
    public function test_single_digit_draws_once()
    {
        $this->resolveSymbols();

        $this->call('_drawDigits_8c01ead8')->with(7, 100.0);

        $resGrp = $this->addressOf('_var_menuState_8c1bc7a8') + 0x0c;

        $this->shouldCall('__modls')->with(7, 10)->using(new RiroCallingConvention())->andReturn(7);
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $resGrp, 7 + 0xc, 362.0, 106.0, -4.3
        );
        $this->shouldCall('__divls')->with(7, 10)->using(new RiroCallingConvention())->andReturn(0);
    }

    public function test_multi_digit_draws_least_significant_first(): void
    {
        $this->resolveSymbols();

        $this->call('_drawDigits_8c01ead8')->with(123, 0.0);

        $resGrp = $this->addressOf('_var_menuState_8c1bc7a8') + 0x0c;
        $remaining = [123, 12, 1];
        $x = 362.0;
        foreach ($remaining as $value) {
            $digit = $value % 10;
            $rest = intdiv($value, 10);
            $this->shouldCall('__modls')->with($value, 10)->using(new RiroCallingConvention())->andReturn($digit);
            $this->shouldCall('_SpriteDraw_8c014f54')->with($resGrp, $digit + 0xc, $x, 6.0, -4.3);
            $this->shouldCall('__divls')->with($value, 10)->using(new RiroCallingConvention())->andReturn($rest);
            $x -= 18.0;
        }
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x84);
        $this->setSize('_SpriteDraw_8c014f54', 4);
        $this->setSize('__modls', 4);
        $this->setSize('__divls', 4);
    }
};
