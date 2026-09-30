<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    const GROUP = 0x0c;            // var_menuState_8c1bc7a8.resourceGroupB_0x0c offset

    private function group(): int
    {
        return $this->addressOf('_var_menuState_8c1bc7a8') + self::GROUP;
    }

    private function digits(array $vals): int
    {
        $ptr = $this->alloc(4 * count($vals));
        foreach ($vals as $i => $v) {
            $this->initUint32($ptr + 4 * $i, $v);
        }
        return $ptr;
    }

    public function test_draws_digits_right_to_left()
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_SpriteDraw_8c014f54', 4);
        $d = $this->digits([5, 3]);   // index 0 = ones, drawn at the right (largest x)
        $this->call('_soundTestFieldDraw_8c01aaaa')->with(100.0, 50.0, $d, 2);
        // sprite index = digit + 0x54 ('0'); x steps left by 26.0 each digit
        $this->shouldCall('_SpriteDraw_8c014f54')->with($this->group(), 0x59, 100.0, 50.0, -4.0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($this->group(), 0x57, 74.0, 50.0, -4.0);
    }

    public function test_zero_count_draws_nothing()
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_SpriteDraw_8c014f54', 4);
        $d = $this->digits([7]);
        $this->call('_soundTestFieldDraw_8c01aaaa')->with(100.0, 50.0, $d, 0);
    }
};
