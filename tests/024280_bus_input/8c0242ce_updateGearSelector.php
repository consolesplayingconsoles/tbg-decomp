<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Called from BusInputUpdate_8c0246b2 with the engine off, cranking, or
// running at a standstill. Manual transmission only (var_driveMode_8c1bb8c8
// == 0): D-pad up (PDD_DGT_KU) selects gear 0, down (PDD_DGT_KD) reverse (5).

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_driveMode_8c1bb8c8', 4);
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_busState_8c1bb9d0', 0x3c8);
    }

    private function setup(int $driveMode, int $press): int {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), $driveMode);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, $press);
        $gearAddr = $this->addressOf('_var_busState_8c1bb9d0') + 0x2f4;
        $this->initUint32($gearAddr, 3); // pre-existing gear, distinct from 0 and 5
        return $gearAddr;
    }

    public function test_autoMode_isIgnored(): void {
        $gearAddr = $this->setup(1, 0x10); // KU pressed, but the transmission is automatic

        $this->call('_updateGearSelector_8c0242ce');
    }

    public function test_manualMode_noButtons_leavesGear(): void {
        $gearAddr = $this->setup(0, 0);

        $this->call('_updateGearSelector_8c0242ce');
    }

    public function test_manualMode_dpadUp_setsGearZero(): void {
        $gearAddr = $this->setup(0, 0x10); // PDD_DGT_KU

        $this->call('_updateGearSelector_8c0242ce');

        $this->shouldWriteLong($gearAddr, 0);
    }

    public function test_manualMode_dpadDown_setsGearReverse(): void {
        $gearAddr = $this->setup(0, 0x20); // PDD_DGT_KD

        $this->call('_updateGearSelector_8c0242ce');

        $this->shouldWriteLong($gearAddr, 5);
    }

    // Both bits set: asm checks KU first, so KU wins.
    public function test_manualMode_bothPressed_prefersUp(): void {
        $gearAddr = $this->setup(0, 0x30);

        $this->call('_updateGearSelector_8c0242ce');

        $this->shouldWriteLong($gearAddr, 0);
    }
};
