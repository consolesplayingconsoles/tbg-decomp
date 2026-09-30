<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_resets_both_vars(): void
    {
        $this->initUint32($this->addressOf('_var_messageAssetCount_8c228514'), 0x12345678);
        $this->initUint32($this->addressOf('_var_messageTextDat_8c228518'), 0x12345678);

        $this->call('_ObjectsClearMessageAssets_8c02aa28');

        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 0);
        $this->shouldWriteLongTo('_var_messageTextDat_8c228518', -1);
    }
};
