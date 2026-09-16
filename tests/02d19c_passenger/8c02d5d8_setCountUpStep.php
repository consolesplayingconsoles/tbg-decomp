<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        // The original reaches the clock globals by displacement off
        // var_runPhase_8c2285c4, so they need their real relative offsets here.
        $runState = $this->alloc(8 * 4);
        $this->rellocate('_var_runPhase_8c2285c4', $runState + 0x00);
        $this->rellocate('_var_scheduleTime_8c2285d8', $runState + 0x14);
        $this->rellocate('_var_runClock_8c2285dc', $runState + 0x18);
        $this->rellocate('_var_clockCatchUpStep_8c2285e0', $runState + 0x1c);

        $this->setSize('__divls', 4);
    }

    public function test_large_remaining_divides_by_5(): void
    {
        $this->resolveSymbols();
        $this->onCall('__divls', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });

        $base = $this->addressOf('_var_runPhase_8c2285c4');
        $this->initUint32($base + 5 * 4, 100); // var_scheduleTime_8c2285d8
        $this->initUint32($base + 6 * 4, 0);   // var_runClock_8c2285dc, remaining = 100

        $this->call('_setCountUpStep_8c02d5d8');

        $this->shouldCall('__divls');
        $this->shouldWriteLong($base + 7 * 4, 20);
    }

    public function test_small_remaining_clamps_to_10(): void
    {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_runPhase_8c2285c4');
        $this->initUint32($base + 5 * 4, 30); // var_scheduleTime_8c2285d8
        $this->initUint32($base + 6 * 4, 5);  // var_runClock_8c2285dc, remaining = 25

        $this->call('_setCountUpStep_8c02d5d8');

        $this->shouldWriteLong($base + 7 * 4, 0xa);
    }

    public function test_boundary_at_50_divides_by_5(): void
    {
        $this->resolveSymbols();
        $this->onCall('__divls', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });

        $base = $this->addressOf('_var_runPhase_8c2285c4');
        $this->initUint32($base + 5 * 4, 50); // var_scheduleTime_8c2285d8
        $this->initUint32($base + 6 * 4, 0);  // var_runClock_8c2285dc, remaining = 50

        $this->call('_setCountUpStep_8c02d5d8');

        $this->shouldCall('__divls');
        $this->shouldWriteLong($base + 7 * 4, 10);
    }
};
