<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function emulateDivMod(): void
    {
        $this->setSize('__modls', 4);
        $this->setSize('__divls', 4);
        // Riro convention: dividend R1, divisor R0.
        $this->onCall('__modls', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });
        $this->onCall('__divls', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });
    }

    public function test_single_digit(): void
    {
        $this->emulateDivMod();
        $dst = $this->alloc(4);

        $this->call('_writeDecimalDigits_8c01b1c0')->with($dst, 5);

        $this->shouldCall('__modls');
        $this->shouldCall('__divls');
        $this->shouldWriteByte($dst + 0, ord('5'));
    }

    public function test_three_digits(): void
    {
        $this->emulateDivMod();
        $dst = $this->alloc(4);

        $this->call('_writeDecimalDigits_8c01b1c0')->with($dst, 123);

        $this->shouldCall('__modls');
        $this->shouldCall('__divls');
        $this->shouldCall('__modls');
        $this->shouldCall('__divls');
        $this->shouldCall('__modls');
        $this->shouldCall('__divls');
        $this->shouldWriteByte($dst + 0, ord('1'));
        $this->shouldWriteByte($dst + 1, ord('2'));
        $this->shouldWriteByte($dst + 2, ord('3'));
    }
};
