<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_drawCommandCount_8c226570', 12);
        $this->setSize('_var_drawCommands_8c22657c', 6144);
        $this->setSize('_njSetCamera', 4);
        $this->setSize('_njCnkModDrawObject', 4);
        $this->setSize('_njMultiMatrix', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njDrawObject', 4);
        $this->setSize('_njCnkDrawObject', 4);
        $this->setSize('_njCnkEasyDrawObject', 4);
        $this->setSize('_njCnkSimpleDrawObject', 4);
    }

    private function seedEntry(int $queueBase, int $type, int $arg1, int $arg2, int $arg3): void {
        $this->initUint32($queueBase + 0x0, $type);
        $this->initUint32($queueBase + 0x4, $arg1);
        $this->initUint32($queueBase + 0x8, $arg2);
        $this->initUint32($queueBase + 0xc, $arg3);
    }

    public function test_type5_calls_the_function_pointer(): void {
        $this->resolveSymbols();
        $this->setSize('_callback', 4);

        $countBase = $this->addressOf('_var_drawCommandCount_8c226570');
        $this->initUint32($countBase, 1);

        $camera = $this->addressOf('_var_drawCamera_8c226558');
        $this->initUint32($camera, 0x11111111);

        $queueBase = $this->addressOf('_var_drawCommands_8c22657c');
        $callback = $this->addressOf('_callback');
        $this->seedEntry($queueBase, 5, $callback, 0x22222222, 0);

        $this->call('_drawLayer_8c022464')->with(0);

        $this->shouldCall('_njSetCamera')->with(0x11111111);
        $this->shouldCall('_callback')->with(0x22222222);
    }

    // Type 6 differs from type 5 only in passing arg3 as a second argument.
    public function test_type6_calls_the_function_pointer_with_two_args(): void {
        $this->resolveSymbols();
        $this->setSize('_callback', 4);

        $countBase = $this->addressOf('_var_drawCommandCount_8c226570');
        $this->initUint32($countBase, 1);

        $camera = $this->addressOf('_var_drawCamera_8c226558');
        $this->initUint32($camera, 0x11111111);

        $queueBase = $this->addressOf('_var_drawCommands_8c22657c');
        $callback = $this->addressOf('_callback');
        $this->seedEntry($queueBase, 6, $callback, 0x33333333, 0x44444444);

        $this->call('_drawLayer_8c022464')->with(0);

        $this->shouldCall('_njSetCamera')->with(0x11111111);
        $this->shouldCall('_callback')->with(0x33333333, 0x44444444);
    }

    public function test_unknown_type_is_skipped(): void {
        $this->resolveSymbols();

        $countBase = $this->addressOf('_var_drawCommandCount_8c226570');
        $this->initUint32($countBase, 1);

        $camera = $this->addressOf('_var_drawCamera_8c226558');
        $this->initUint32($camera, 0x11111111);

        $queueBase = $this->addressOf('_var_drawCommands_8c22657c');
        $this->seedEntry($queueBase, 7, 0, 0, 0);

        $this->call('_drawLayer_8c022464')->with(0);

        $this->shouldCall('_njSetCamera')->with(0x11111111);
    }
};
