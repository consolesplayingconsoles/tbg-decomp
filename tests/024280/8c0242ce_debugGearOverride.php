<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// debugGearOverride_8c0242ce: STATIC, called from FUN_8c0246b2 each frame.
// Only acts in direct (non-mapped) steering mode (var_inputMapSel_8c1bb8c8
// == 0): the first peripheral's D-pad up (PDD_DGT_KU, 0x10) forces
// BusState.gear_0x2f4 to 0, D-pad down (PDD_DGT_KD, 0x20) forces it to 5
// (reverse). Neither pressed, or mapped-route mode active, leaves the gear
// untouched.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_inputMapSel_8c1bb8c8', 4);
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_busState_8c1bb9d0', 0x3c8);
    }

    private function setup(int $inputMapSel, int $press): int {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_inputMapSel_8c1bb8c8'), $inputMapSel);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, $press);
        $gearAddr = $this->addressOf('_var_busState_8c1bb9d0') + 0x2f4;
        $this->initUint32($gearAddr, 3); // pre-existing gear, distinct from 0 and 5
        return $gearAddr;
    }

    public function test_mappedMode_isIgnored(): void {
        $gearAddr = $this->setup(1, 0x10); // KU pressed but mapped mode active

        $this->call('_debugGearOverride_8c0242ce');
    }

    public function test_directMode_noButtons_leavesGear(): void {
        $gearAddr = $this->setup(0, 0);

        $this->call('_debugGearOverride_8c0242ce');
    }

    public function test_directMode_dpadUp_setsGearZero(): void {
        $gearAddr = $this->setup(0, 0x10); // PDD_DGT_KU

        $this->call('_debugGearOverride_8c0242ce');

        $this->shouldWriteLong($gearAddr, 0);
    }

    public function test_directMode_dpadDown_setsGearReverse(): void {
        $gearAddr = $this->setup(0, 0x20); // PDD_DGT_KD

        $this->call('_debugGearOverride_8c0242ce');

        $this->shouldWriteLong($gearAddr, 5);
    }

    // Both bits set: asm checks KU first, so KU wins.
    public function test_directMode_bothPressed_prefersUp(): void {
        $gearAddr = $this->setup(0, 0x30);

        $this->call('_debugGearOverride_8c0242ce');

        $this->shouldWriteLong($gearAddr, 0);
    }
};
