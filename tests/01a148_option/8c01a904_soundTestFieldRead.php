<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function arrangeDigits(array $digits): int
    {
        $ptr = $this->alloc(4 * count($digits));
        foreach ($digits as $i => $d) {
            $this->initUint32($ptr + 4 * $i, $d);
        }
        return $ptr;
    }

    public function test_two_digit_field()
    {
        // index 0 = ones, index 1 = tens
        $ptr = $this->arrangeDigits([2, 5]);
        $this->call('_soundTestFieldRead_8c01a904')->with($ptr, 2);
        $this->shouldReturn(52);
    }

    public function test_four_digit_field()
    {
        $ptr = $this->arrangeDigits([1, 2, 3, 4]);
        $this->call('_soundTestFieldRead_8c01a904')->with($ptr, 4);
        $this->shouldReturn(4321);
    }

    public function test_zero_count_returns_zero()
    {
        $ptr = $this->arrangeDigits([7]);
        $this->call('_soundTestFieldRead_8c01a904')->with($ptr, 0);
        $this->shouldReturn(0);
    }
};
