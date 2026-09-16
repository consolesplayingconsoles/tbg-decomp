<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /* Both buffers allocated -> free each and mark it freed with -1. */
    public function test_frees_both_buffers(): void
    {
        $this->setSize('_var_saveBuf_8c1ba2e0', 4);
        $this->setSize('_var_vmuIconFileBuf_8c1ba344', 4);
        $this->setSize('_syFree', 4);

        $this->initUint32($this->addressOf('_var_saveBuf_8c1ba2e0'), 0x1000);
        $this->initUint32($this->addressOf('_var_vmuIconFileBuf_8c1ba344'), 0x2000);

        $this->call('_FileMenuFreeBuffers_8c0187d0');

        $this->shouldCall('_syFree')->with(0x1000);
        $this->shouldWriteLongTo('_var_saveBuf_8c1ba2e0', 0xffffffff);
        $this->shouldCall('_syFree')->with(0x2000);
        $this->shouldWriteLongTo('_var_vmuIconFileBuf_8c1ba344', 0xffffffff);
    }

    /* Both already freed (-1) -> no syFree, no writes. */
    public function test_skips_when_already_freed(): void
    {
        $this->setSize('_var_saveBuf_8c1ba2e0', 4);
        $this->setSize('_var_vmuIconFileBuf_8c1ba344', 4);
        $this->setSize('_syFree', 4);

        $this->initUint32($this->addressOf('_var_saveBuf_8c1ba2e0'), 0xffffffff);
        $this->initUint32($this->addressOf('_var_vmuIconFileBuf_8c1ba344'), 0xffffffff);

        $this->call('_FileMenuFreeBuffers_8c0187d0');
    }
};
