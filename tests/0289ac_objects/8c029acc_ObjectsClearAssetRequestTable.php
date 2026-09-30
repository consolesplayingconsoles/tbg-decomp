<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_sets_var_to_minus_one(): void {
        $var = $this->addressOf('_var_assetRequestTable_8c228408');
        $this->initUint32($var, 0);

        $this->call('_ObjectsClearAssetRequestTable_8c029acc')->with();

        $this->shouldWriteLong($var, 0xffffffff);
    }
};
