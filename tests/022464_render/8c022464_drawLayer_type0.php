<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_njSetCamera', 4);
        $this->setSize('_njCnkModDrawObject', 4);
        $this->setSize('_njMultiMatrix', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njDrawObject', 4);
        $this->setSize('_njCnkDrawObject', 4);
        $this->setSize('_njCnkEasyDrawObject', 4);
        $this->setSize('_njCnkSimpleDrawObject', 4);
    }

    public function test_draws_a_single_type0_entry(): void {
        $this->resolveSymbols();

        $countBase = $this->addressOf('_var_drawCommandCount_8c226570');
        $this->initUint32($countBase, 1);

        $camera = $this->addressOf('_var_drawCamera_8c226558');
        $this->initUint32($camera, 0x11111111);

        $queueBase = $this->addressOf('_var_drawCommands_8c22657c');
        $matrix = $this->alloc(4);
        $texlist = $this->alloc(4);
        $object = $this->alloc(4);
        $this->initUint32($queueBase + 0x0, 0);
        $this->initUint32($queueBase + 0x4, $matrix);
        $this->initUint32($queueBase + 0x8, $texlist);
        $this->initUint32($queueBase + 0xc, $object);

        $this->call('_drawLayer_8c022464')->with(0);

        $this->shouldCall('_njSetCamera')->with(0x11111111);
        $this->shouldCall('_njMultiMatrix')->with(0, $matrix);
        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njDrawObject')->with($object);
    }
};
