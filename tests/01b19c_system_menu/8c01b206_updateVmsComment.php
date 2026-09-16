<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\CallingConventions\RoriCallingConvention;

return new class extends TestCase {
    protected function isAsmObject(): bool
    {
        return str_contains($this->objectFile, '/asm/');
    }

    /* SHC lowers strcpy to __slow_strcpy (dst/src in R0/R1, Rori). */
    private function expectStrcpy(int $dst, $src): void
    {
        if ($this->isAsmObject()) {
            $this->shouldCall('_strcpy')->with($dst, $src);
        } else {
            $this->shouldCall('__slow_strcpy')->with($dst, $src)
                ->using(new RoriCallingConvention());
        }
    }

    public function test_builds_vms_comment(): void
    {
        $this->setSize('_var_vmsComment_8c226098', 0x10);
        $text = $this->addressOf('_var_vmsComment_8c226098');
        $progress = $this->addressOf('_var_progress_8c1ba1cc');
        $exp = $this->addressOf('_var_exp_8c1ba25c');

        $this->initUint32($progress + 0x00, 9);   // days_0x00
        $this->initUint32($exp, 250);

        $this->call('_updateVmsComment_8c01b206');

        // Fill the 16-byte comment with spaces.
        for ($i = 0; $i < 0x10; $i++) {
            $this->shouldWriteByte($text + $i, 0x20);
        }
        $this->expectStrcpy($text, "9/   EXP ");
        $this->shouldCall('_writeDecimalDigits_8c01b1c0')->with($text + 2, 9);
        $this->shouldCall('_writeDecimalDigits_8c01b1c0')->with($text + 9, 250);
    }
};
