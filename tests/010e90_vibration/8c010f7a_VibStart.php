<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _VibStart_8c010f7a(int pattern): queue a rumble pattern.
 *
 * Nothing rumbles here -- it only picks which of init_vibPatterns_8c03be5c
 * VibUpdate_8c010fae will step next. The index doubles as the priority: a
 * request that arrives mid-pattern is dropped unless it outranks the one
 * playing.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
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

    public function test_ignores_an_index_past_the_pattern_table(): void
    {
        $this->resolveSymbols();
        // 8 is one past the last entry; the bound is the only thing between a
        // caller's argument and an out-of-table function pointer.
        $this->initState(2, 5, 1, 1);

        $this->call('_VibStart_8c010f7a')->with(8);
    }

    public function test_higher_index_displaces_the_playing_pattern(): void
    {
        $this->resolveSymbols();
        $this->initState(2, 5, 1, 1);

        $this->call('_VibStart_8c010f7a')->with(5);

        // Only pattern_0x00 moves: step_0x08 and frame_0x04 keep running, so
        // the new pattern is picked up from wherever the old one had got to.
        $this->shouldWriteLong($this->state() + 0x00, 5);
    }

    public function test_lower_index_does_not_displace_the_playing_pattern(): void
    {
        $this->resolveSymbols();
        $this->initState(5, 5, 1, 1);

        $this->call('_VibStart_8c010f7a')->with(2);
    }

    public function test_equal_index_does_not_restart_the_playing_pattern(): void
    {
        $this->resolveSymbols();
        $this->initState(3, 5, 1, 1);

        $this->call('_VibStart_8c010f7a')->with(3);
    }

    public function test_idle_state_takes_any_index_including_a_lower_one(): void
    {
        $this->resolveSymbols();
        // Parked on 7 by a previous VibClear_8c010fbe. The priority rule is
        // only about interrupting, so 3 wins here despite being lower.
        $this->initState(7, 0, 0, 0);

        $this->call('_VibStart_8c010f7a')->with(3);

        // Nothing here reads what VibClear_8c010fbe leaves behind -- the
        // pattern index is overwritten on the next line either way.
        $this->shouldCall('_VibClear_8c010fbe');
        $this->shouldWriteLong($this->state() + 0x00, 3);
    }
};
