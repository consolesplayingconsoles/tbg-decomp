<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_returns_0_on_a_later_call()
    {
        $this->call('_InputCheckColdBoot_8c012984');

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($this->addressOf('_var_bootSentinel_8c157aec'), "FortyFive")
            ->andReturn(0);
        $this->shouldReturn(0);
    }

    public function test_returns_1_and_plants_the_sentinel_on_the_first_call()
    {
        $this->call('_InputCheckColdBoot_8c012984');

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($this->addressOf('_var_bootSentinel_8c157aec'), "FortyFive")
            ->andReturn(1);

        $strCpy = $this->isAsmObject() ? '_strcpy' : '__slow_strcpy';
        $this->shouldCall($strCpy)
            ->with($this->addressOf('_var_bootSentinel_8c157aec'), "FortyFive");
        $this->shouldReturn(1);
    }

    protected function isAsmObject(): bool
    {
        return str_contains($this->objectFile, '/asm/');
    }
};
