<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_fadeDrawCommandCount_8c226570', 12);
        $this->setSize('_var_fadeDrawCommands_8c22657c', 6144);
        $this->setSize('_njSetCamera', 4);
        $this->setSize('_njCnkModDrawObject', 4);
        $this->setSize('_njMultiMatrix', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njDrawObject', 4);
        $this->setSize('_njCnkDrawObject', 4);
        $this->setSize('_njCnkEasyDrawObject', 4);
        $this->setSize('_njCnkSimpleDrawObject', 4);
    }

    private function seedEntry(int $queueBase, int $index, int $type, int $arg1, int $arg2, int $arg3): void {
        $entry = $queueBase + $index * 0x10;
        $this->initUint32($entry + 0x0, $type);
        $this->initUint32($entry + 0x4, $arg1);
        $this->initUint32($entry + 0x8, $arg2);
        $this->initUint32($entry + 0xc, $arg3);
    }

    public function test_draws_layer_2_entries_in_order(): void {
        $this->resolveSymbols();

        $countBase = $this->addressOf('_var_fadeDrawCommandCount_8c226570');
        $this->initUint32($countBase + 0, 0);
        $this->initUint32($countBase + 4, 0);
        $this->initUint32($countBase + 8, 2);

        $camera = $this->addressOf('_var_drawCamera_8c226558');
        $this->initUint32($camera, 0x11111111);

        $queueBase = $this->addressOf('_var_fadeDrawCommands_8c22657c') + 2 * 0x800;
        $object0 = $this->alloc(4);
        $object1 = $this->alloc(4);
        $this->seedEntry($queueBase, 0, 0, 0, 0, $object0);
        $this->seedEntry($queueBase, 1, 4, 0, 0, $object1);

        $this->call('_fadeDraw_8c022464')->with(2);

        $this->shouldCall('_njSetCamera')->with(0x11111111);
        $this->shouldCall('_njMultiMatrix')->with(0, 0);
        $this->shouldCall('_njSetTexture')->with(0);
        $this->shouldCall('_njDrawObject')->with($object0);
        $this->shouldCall('_njSetCamera')->with(0x11111111);
        $this->shouldCall('_njMultiMatrix')->with(0, 0);
        $this->shouldCall('_njCnkModDrawObject')->with($object1);
    }

    public function test_draws_nothing_for_an_empty_layer(): void {
        $this->resolveSymbols();

        $countBase = $this->addressOf('_var_fadeDrawCommandCount_8c226570');
        $this->initUint32($countBase + 0, 0);
        $this->initUint32($countBase + 4, 0);
        $this->initUint32($countBase + 8, 0);

        $this->addressOf('_var_drawCamera_8c226558');
        $this->addressOf('_var_fadeDrawCommands_8c22657c');

        $this->call('_fadeDraw_8c022464')->with(2);
    }
};
