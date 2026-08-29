<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_njCnkSetSimpleLight', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njCnkSimpleDrawObject', 4);
        $this->setSize('_var_busSimpleLightDir_8c227db8', 0xc);
        $this->setSize('_var_interiorTexlist_8c1bc438', 4);
        $this->setSize('_var_interiorNj_8c1bc43c', 4);
    }

    private function f(float $v): int
    {
        return unpack('L', pack('f', $v))[1];
    }

    public function test_sets_light_texture_and_draws_interior(): void
    {
        $this->resolveSymbols();

        $dir = $this->addressOf('_var_busSimpleLightDir_8c227db8');
        $this->initUint32($dir + 0x0, $this->f(0.1));
        $this->initUint32($dir + 0x4, $this->f(0.2));
        $this->initUint32($dir + 0x8, $this->f(0.3));

        $tlist = 0xcafef00d;
        $this->initUint32($this->addressOf('_var_interiorTexlist_8c1bc438'), $tlist);

        $nj = 0xdeadbeef;
        $this->initUint32($this->addressOf('_var_interiorNj_8c1bc43c'), $nj);

        $this->call('_FUN_8c02d1f4')->with(0);

        $this->shouldCall('_njCnkSetSimpleLight')->with(0.1, 0.2, 0.3);
        $this->shouldCall('_njSetTexture')->with($tlist);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($nj);
    }
};
