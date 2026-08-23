<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_lesson_mode_writes_choose_only_and_returns()
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 1);

        $this->call('_buildDialogQueue_8c01e992');

        $queue = $this->addressOf('_var_8c226414');
        $this->shouldWriteLong($queue + 0, 0x18);
        $this->shouldWriteLong($queue + 4, -1);
    }

    public function test_already_queued_and_pending_skips_state0()
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);
        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_8c1bb8bc'), 1);
        $this->initUint32($this->addressOf('_var_8c1bb8f4'), 0);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);

        $this->call('_buildDialogQueue_8c01e992');

        $queue = $this->addressOf('_var_8c226414');
        // state1 (PERFECT, since 8c1bb8f4 == 0), no award, terminal CHOOSE
        $this->shouldWriteLong($queue + 0, 0x1b);
        $this->shouldWriteLong($queue + 4, 0x18);
        $this->shouldWriteLong($queue + 8, -1);
    }

    public function test_progress_below_threshold_queues_tips_and_choose()
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);
        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 0);
        $this->initUint32($this->addressOf('_var_8c1bb8bc'), 1);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 0x10);

        $this->call('_buildDialogQueue_8c01e992');

        $queue = $this->addressOf('_var_8c226414');
        // gameMode == 0, so WARNING is also queued between TIPS and CHOOSE
        $this->shouldWriteLong($queue + 0, 0x16);
        $this->shouldWriteLong($queue + 4, 0x17);
        $this->shouldWriteLong($queue + 8, 0x18);
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8b8'), 1);
        $this->shouldWriteLong($queue + 12, -1);
    }

    public function test_progress_at_threshold_queues_final_day_and_choose()
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);
        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 0);
        $this->initUint32($this->addressOf('_var_8c1bb8bc'), 1);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 0x1e);

        $this->call('_buildDialogQueue_8c01e992');

        $queue = $this->addressOf('_var_8c226414');
        // days_0x00 >= 30, so FINAL_DAY replaces TIPS/WARNING
        $this->shouldWriteLong($queue + 0, 0x1a);
        $this->shouldWriteLong($queue + 4, 0x18);
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8b8'), 1);
        $this->shouldWriteLong($queue + 8, -1);
    }

    public function test_award_appends_score_record_before_terminal_choose()
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);
        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_8c1bb8bc'), 1);
        $this->initUint32($this->addressOf('_var_8c1bb8f4'), 0);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 1);

        $this->call('_buildDialogQueue_8c01e992');

        $queue = $this->addressOf('_var_8c226414');
        $this->shouldWriteLong($queue + 0, 0x1b);
        $this->shouldWriteLong($queue + 4, 0x19);
        $this->shouldWriteLong($queue + 8, 0x18);
        $this->shouldWriteLong($queue + 12, -1);
    }

    public function test_result_variant_looked_up_by_table_index()
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);
        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_8c1bb8bc'), 1);
        $this->initUint32($this->addressOf('_var_8c1bb8f4'), 1);
        $this->initUint32($this->addressOf('_var_8c1bb8ec'), 5);
        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 1);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);

        $this->call('_buildDialogQueue_8c01e992');

        $queue = $this->addressOf('_var_8c226414');
        // init_8c045208[5] == 0x26 (private table, no direct addressOf needed)
        $this->shouldWriteLong($queue + 0, 0x26);
        $this->shouldWriteLong($queue + 4, 0x1c);
        $this->shouldWriteLong($queue + 8, 0x18);
        $this->shouldWriteLong($queue + 12, -1);
    }

    public function test_fail_variant_looked_up_by_table_index()
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);
        $this->initUint32($this->addressOf('_var_8c1bb8b8'), 1);
        $this->initUint32($this->addressOf('_var_8c1bb8bc'), 1);
        $this->initUint32($this->addressOf('_var_8c1bb8f4'), 1);
        $this->initUint32($this->addressOf('_var_8c1bb8ec'), 5);
        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 0);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);

        $this->call('_buildDialogQueue_8c01e992');

        $queue = $this->addressOf('_var_8c226414');
        // init_8c045208[5] == 0x26 (private table, no direct addressOf needed)
        $this->shouldWriteLong($queue + 0, 0x26);
        $this->shouldWriteLong($queue + 4, 0x1e);
        $this->shouldWriteLong($queue + 8, 0x18);
        $this->shouldWriteLong($queue + 12, -1);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_gameMode_8c1bb8fc', 0x4);
        $this->setSize('_var_8c1bb8b8', 0x4);
        $this->setSize('_var_8c1bb8bc', 0x4);
        $this->setSize('_var_runSucceeded_8c1bb8dc', 0x4);
        $this->setSize('_var_8c1bb8ec', 0x4);
        $this->setSize('_var_8c1bb8f4', 0x4);
        $this->setSize('_var_award_8c1bb8f8', 0x4);
        $this->setSize('_var_progress_8c1ba1cc', 0x4);
        $this->setSize('_var_8c226414', 0x18);
    }
};
