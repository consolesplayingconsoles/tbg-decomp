<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _drawStopMarker_8c02cd92(int arg0): draws the "fuu" stop-marker model at
 * the upcoming stop's position (var_nextStopPoint_8c228900),
 * facing its heading (var_nextStopHeading_8c228714) and animated by the frame counter
 * var_fuuFrame_8c1bc44c. Installed as a DrawCallback1; arg0 is unused.
 *
 * Ghidra's decompile folded all three var_nextStopPoint_8c228900 fields into
 * one repeated "DAT_8c228904" reference (and invented a used float
 * parameter); the actual x/y/z mapping here was disambiguated against the
 * real asm object.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_scratchMatrix_8c1bc46c', 0x40);
        $this->setSize('_var_fuuFrame_8c1bc44c', 4);
        $this->setSize('_var_fuuTexlist_8c1bc440', 4);
        $this->setSize('_var_fuuNj_8c1bc444', 4);
        $this->setSize('_var_fuuNjm_8c1bc448', 4);
    }

    public function test_1(): void
    {
        $this->resolveSymbols();

        $point = $this->addressOf('_var_nextStopPoint_8c228900');
        $this->initUint32($point + 0, unpack('L', pack('f', 11.0))[1]); // .x
        $this->initUint32($point + 4, unpack('L', pack('f', 22.0))[1]); // .y
        $this->initUint32($point + 8, unpack('L', pack('f', 33.0))[1]); // .z

        $this->initUint32($this->addressOf('_var_nextStopHeading_8c228714'), 12345);
        $this->initUint32($this->addressOf('_var_fuuFrame_8c1bc44c'), unpack('L', pack('f', 55.0))[1]);

        $texlist = 0xcafe0001;
        $model = 0xcafe0002;
        $motion = 0xcafe0003;
        $this->initUint32($this->addressOf('_var_fuuTexlist_8c1bc440'), $texlist);
        $this->initUint32($this->addressOf('_var_fuuNj_8c1bc444'), $model);
        $this->initUint32($this->addressOf('_var_fuuNjm_8c1bc448'), $motion);

        $mat = $this->addressOf('_var_scratchMatrix_8c1bc46c');

        $this->call('_drawStopMarker_8c02cd92')->with(0);

        $this->shouldCall('_njUnitMatrix')->with($mat);
        $this->shouldCall('_njTranslate')->with($mat, 11.0, 22.0, 33.0);
        $this->shouldCall('_njRotateY')->with($mat, 12345);
        $this->shouldCall('_njMultiMatrix')->with(0, $mat);
        $this->shouldCall('_njSetTexture')->with($texlist)->andReturn(0);
        $this->shouldCall('_njCnkSimpleDrawMotion')->with($model, $motion, 55.0);
    }
};
