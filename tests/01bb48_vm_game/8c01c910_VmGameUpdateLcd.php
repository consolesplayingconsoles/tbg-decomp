<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_draws_active_slot()
    {
        $this->setSize('_var_lcdSlot_8c2263a0', 4);
        $this->setSize('_var_lcdAnimBus_8c2260ac', 12);
        $this->setSize('_var_lcdAnimDanger_8c2260b8', 12);
        $this->setSize('_var_lcdAnimLoading_8c2260c4', 12);

        $slot = 1;
        $anim = $this->addressOf('_var_lcdAnimDanger_8c2260b8');
        $this->initUint32($this->addressOf('_var_lcdSlot_8c2263a0'), $slot);

        $this->call('_VmGameUpdateLcd_8c01c910');
        $this->shouldCall('_advanceLcdAnim_8c01bb48')->with($anim, 0);
    }
};
