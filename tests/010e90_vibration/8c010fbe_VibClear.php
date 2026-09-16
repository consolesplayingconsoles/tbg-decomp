<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\Simulator\Types\U8;
use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _VibClear_8c010fbe(): park the rumble machine.
 *
 * Zeroes var_vibState_8c157a48, then puts pattern_0x00 on 7 --
 * init_vibIdle_8c03be4c, the all-zero pattern VibUpdate_8c010fae refuses to
 * step. 7 is not the bss default, so the state has to be driven there.
 */
return new class extends TestCase {
    public function test_parks_on_the_all_zero_idle_pattern(): void
    {
        $this->setSize('_memset', 4);

        $s = $this->addressOf('_var_vibState_8c157a48');
        $this->initUint32($s + 0x00, 3); // pattern_0x00
        $this->initUint32($s + 0x04, 9); // frame_0x04
        $this->initUint32($s + 0x08, 1); // step_0x08
        $this->initUint32($s + 0x0c, 1); // playing_0x0c

        $this->call('_VibClear_8c010fbe');

        $this->shouldCall('_memset')->with($s, 0, 0x10)->do(function () {
            $dst = $this->registers[4]->value;
            $len = $this->registers[6]->value;
            for ($i = 0; $i < $len; $i++) {
                $this->memory->writeUInt8($dst + $i, U8::of(0));
            }
        });
        $this->shouldWriteLong($s + 0x00, 7);
    }
};
