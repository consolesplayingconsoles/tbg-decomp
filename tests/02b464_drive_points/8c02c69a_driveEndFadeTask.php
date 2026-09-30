<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_gameMode_8c1bb8fc', 4);
        $this->setSize('_var_shouldShowFreeRunIntro_8c1bb8c0', 4);
        $this->setSize('_var_runWasPractice_8c1bb8bc', 4);
        $this->setSize('_var_markTexlist_8c1bc418', 4);
    }

    public function test_phase_0_arms_once_not_fading(): void
    {
        $this->resolveSymbols();
        $task = $this->alloc(0x1c);
        $this->initUint32($task + 8, 0); // phase 0
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $this->call('_driveEndFadeTask_8c02c69a')->with($task, 0);

        $this->shouldWriteLong($task + 0xc, 0);
        $this->shouldWriteLong($task + 8, 1);

        $this->shouldCall('_TxtDrawSprite_8c014f54')
            ->with($this->addressOf('_var_markTexlist_8c1bc418'), 0x79, 0.0, 0.0, -3.0);
    }

    public function test_phase_1_counts_up_to_fade_out(): void
    {
        $this->resolveSymbols();
        $this->setSize('_RenderPushFadeOut_8c022b60', 4);
        $task = $this->alloc(0x1c);
        $this->initUint32($task + 8, 1); // phase 1
        $this->initUint32($task + 0xc, 0x1e); // counter, one away from tripping

        $this->call('_driveEndFadeTask_8c02c69a')->with($task, 0);

        $this->shouldWriteLong($task + 0xc, 0x1f);
        $this->shouldWriteLong($task + 8, 2);
        $this->shouldCall('_RenderPushFadeOut_8c022b60')->with(10);

        $this->shouldCall('_TxtDrawSprite_8c014f54')
            ->with($this->addressOf('_var_markTexlist_8c1bc418'), 0x79, 0.0, 0.0, -3.0);
    }

    public function test_phase_2_not_fading_free_run_shows_results(): void
    {
        $this->resolveSymbols();
        $task = $this->alloc(0x1c);
        $this->initUint32($task + 8, 2); // phase 2
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);

        $this->call('_driveEndFadeTask_8c02c69a')->with($task, 0);

        $this->shouldCall('_ReplayMenuFreeSessionAssets_8c016182');
        $this->shouldCall('_ResultShowPassedRun_8c01e0b4');
    }

    public function test_phase_2_not_fading_course_mode_returns_to_menu(): void
    {
        $this->resolveSymbols();
        $task = $this->alloc(0x1c);
        $this->initUint32($task + 8, 2); // phase 2
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 1);

        $this->call('_driveEndFadeTask_8c02c69a')->with($task, 0);

        $this->shouldCall('_ReplayMenuFreeSessionAssets_8c016182');
        $this->shouldWriteLong($this->addressOf('_var_shouldShowFreeRunIntro_8c1bb8c0'), 0);
        $this->shouldWriteLong($this->addressOf('_var_runWasPractice_8c1bb8bc'), 0);
        $this->shouldCall('_CourseMenuReturn_8c017ef2');
    }
};
