<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_draws_texlist_and_model(): void
    {
        $state = $this->alloc(0x7c);
        $dat = $this->alloc(0x20);
        $texlist = $this->alloc(4);
        $model = $this->alloc(4);
        $this->initUint32($dat + 0x18, $texlist); // dat->queuedItem_0x18
        $this->initUint32($dat + 0x1c, $model);    // dat->field_0x1c
        $this->initUint32($state + 0x48, $dat);    // state[0x12]

        $this->call('_drawDatModel_8c029f2a')->with($state);

        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($model);
    }
};
