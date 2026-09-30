<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _VibUpdate_8c010fae(int port): the per-frame rumble tick.
 *
 * Just the gate in front of stepPattern_8c010e90 -- pattern 7 is
 * init_vibIdle_8c03be4c, where VibClear_8c010fbe parks the state, and it is
 * never stepped. driveCueTask_8c020214 is the only caller and gates this
 * again on the player's vibration setting.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_pdVibMxStart', 4);
        $this->setSize('_pdVibMxStop', 4);
        $this->setSize('_memset', 4);
    }

    private function state(): int
    {
        return $this->addressOf('_var_vibState_8c157a48');
    }

    private function initState(int $pattern, int $frame, int $step, int $playing): void
    {
        $s = $this->state();
        $this->initUint32($s + 0x00, $pattern);
        $this->initUint32($s + 0x04, $frame);
        $this->initUint32($s + 0x08, $step);
        $this->initUint32($s + 0x0c, $playing);
    }

    public function test_parked_idle_pattern_is_never_stepped(): void
    {
        $this->resolveSymbols();
        // Only the index is checked, not the (all-zero) steps behind it, so
        // frame_0x04 does not tick either while parked.
        $this->initState(7, 0, 0, 0);

        $this->call('_VibUpdate_8c010fae')->with(0);
    }

    public function test_queued_pattern_is_stepped_on_the_given_port(): void
    {
        $this->resolveSymbols();
        // Any index but 7 will do; the rest of the state is stepPattern's
        // business, and it is mocked out here.
        $this->initState(2, 0, 0, 0);

        $this->call('_VibUpdate_8c010fae')->with(1);

        $this->shouldCall('_stepPattern_8c010e90')->with(1);
    }
};
