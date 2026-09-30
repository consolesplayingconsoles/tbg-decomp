<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_fadeDrawCommandCount_8c226570', 12);
        $this->setSize('_var_fadeDrawCommands_8c22657c', 6144);
        $this->setSize('_var_drawCamera_8c226558', 4);
        $this->setSize('_njSetCamera', 4);
        $this->setSize('_njCnkModDrawObject', 4);
        $this->setSize('_njMultiMatrix', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njDrawObject', 4);
        $this->setSize('_njCnkDrawObject', 4);
        $this->setSize('_njCnkEasyDrawObject', 4);
        $this->setSize('_njCnkSimpleDrawObject', 4);
    }

    private function seedEntry(int $queueBase, int $type, int $matrix, int $texlist, int $object): void {
        $this->initUint32($queueBase + 0x0, $type);
        $this->initUint32($queueBase + 0x4, $matrix);
        $this->initUint32($queueBase + 0x8, $texlist);
        $this->initUint32($queueBase + 0xc, $object);
    }

    public function test_type1_uses_cnk_draw_object(): void {
        $this->resolveSymbols();

        $countBase = $this->addressOf('_var_fadeDrawCommandCount_8c226570');
        $this->initUint32($countBase, 1);

        $camera = $this->addressOf('_var_drawCamera_8c226558');
        $this->initUint32($camera, 0x11111111);

        $queueBase = $this->addressOf('_var_fadeDrawCommands_8c22657c');
        $matrix = $this->alloc(4);
        $texlist = $this->alloc(4);
        $object = $this->alloc(4);
        $this->seedEntry($queueBase, 1, $matrix, $texlist, $object);

        $this->call('_fadeDraw_8c022464')->with(0);

        $this->shouldCall('_njSetCamera')->with(0x11111111);
        $this->shouldCall('_njMultiMatrix')->with(0, $matrix);
        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njCnkDrawObject')->with($object);
    }

    public function test_type2_uses_cnk_easy_draw_object(): void {
        $this->resolveSymbols();

        $countBase = $this->addressOf('_var_fadeDrawCommandCount_8c226570');
        $this->initUint32($countBase, 1);

        $camera = $this->addressOf('_var_drawCamera_8c226558');
        $this->initUint32($camera, 0x11111111);

        $queueBase = $this->addressOf('_var_fadeDrawCommands_8c22657c');
        $matrix = $this->alloc(4);
        $texlist = $this->alloc(4);
        $object = $this->alloc(4);
        $this->seedEntry($queueBase, 2, $matrix, $texlist, $object);

        $this->call('_fadeDraw_8c022464')->with(0);

        $this->shouldCall('_njSetCamera')->with(0x11111111);
        $this->shouldCall('_njMultiMatrix')->with(0, $matrix);
        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njCnkEasyDrawObject')->with($object);
    }

    public function test_type3_uses_cnk_simple_draw_object(): void {
        $this->resolveSymbols();

        $countBase = $this->addressOf('_var_fadeDrawCommandCount_8c226570');
        $this->initUint32($countBase, 1);

        $camera = $this->addressOf('_var_drawCamera_8c226558');
        $this->initUint32($camera, 0x11111111);

        $queueBase = $this->addressOf('_var_fadeDrawCommands_8c22657c');
        $matrix = $this->alloc(4);
        $texlist = $this->alloc(4);
        $object = $this->alloc(4);
        $this->seedEntry($queueBase, 3, $matrix, $texlist, $object);

        $this->call('_fadeDraw_8c022464')->with(0);

        $this->shouldCall('_njSetCamera')->with(0x11111111);
        $this->shouldCall('_njMultiMatrix')->with(0, $matrix);
        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($object);
    }

    public function test_type4_uses_cnk_mod_draw_object_and_skips_set_texture(): void {
        $this->resolveSymbols();

        $countBase = $this->addressOf('_var_fadeDrawCommandCount_8c226570');
        $this->initUint32($countBase, 1);

        $camera = $this->addressOf('_var_drawCamera_8c226558');
        $this->initUint32($camera, 0x11111111);

        $queueBase = $this->addressOf('_var_fadeDrawCommands_8c22657c');
        $matrix = $this->alloc(4);
        $texlist = $this->alloc(4);
        $object = $this->alloc(4);
        $this->seedEntry($queueBase, 4, $matrix, $texlist, $object);

        $this->call('_fadeDraw_8c022464')->with(0);

        $this->shouldCall('_njSetCamera')->with(0x11111111);
        $this->shouldCall('_njMultiMatrix')->with(0, $matrix);
        $this->shouldCall('_njCnkModDrawObject')->with($object);
    }
};
