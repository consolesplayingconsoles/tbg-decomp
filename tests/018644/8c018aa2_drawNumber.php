<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /* 100 -> digits drawn right-to-left: 0,0,1 at x=42,32,22 (glyph 15+digit). */
    public function test_draws_digits_right_to_left(): void
    {
        $this->resolveSymbols();

        $ms = $this->addressOf('_var_menuState_8c1bc7a8');

        $this->call('_drawNumber_8c018aa2')->with(100, 42.0, 69.0);

        $this->shouldCall('__modls');
        $this->shouldCall('_SpriteDraw_8c014f54')->with($ms + 0, 15, 42.0, 69.0, -4.0);
        $this->shouldCall('__divls');
        $this->shouldCall('__modls');
        $this->shouldCall('_SpriteDraw_8c014f54')->with($ms + 0, 15, 32.0, 69.0, -4.0);
        $this->shouldCall('__divls');
        $this->shouldCall('__modls');
        $this->shouldCall('_SpriteDraw_8c014f54')->with($ms + 0, 16, 22.0, 69.0, -4.0);
        $this->shouldCall('__divls');
    }

    /* Single digit: exactly one sprite, no further division loop. */
    public function test_single_digit(): void
    {
        $this->resolveSymbols();

        $ms = $this->addressOf('_var_menuState_8c1bc7a8');

        $this->call('_drawNumber_8c018aa2')->with(7, 10.0, 20.0);

        $this->shouldCall('__modls');
        $this->shouldCall('_SpriteDraw_8c014f54')->with($ms + 0, 22, 10.0, 20.0, -4.0);
        $this->shouldCall('__divls');
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x6c);
        $this->setSize('__modls', 4);
        $this->setSize('__divls', 4);

        $this->onCall('__modls', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });
        $this->onCall('__divls', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });
    }
};
