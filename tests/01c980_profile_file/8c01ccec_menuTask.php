<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * ProfileFileMenuTask_8c01ccec: per-frame PROFILE FILE task. Incremental
 * coverage -- states still marked TODO in the C aren't tested here yet.
 */
return new class extends TestCase {
    const STATE = 0x18;
    const MIDI = 0xd1d1d1d1;

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_isFading_8c226568', 4);

        $this->setSize('_RouteLoadIsPvmReady_8c01432a', 4);
        $this->setSize('_AsqFreeQueues_8c011f7e', 4);
        $this->setSize('_push_fadein_8c022a9c', 4);
        $this->setSize('_CourseMenuInterpolateCursor_8c016d2c', 4);
        $this->setSize('_TxtDrawSprite_8c014f54', 4);
        $this->setSize('_njSetBackColor', 4);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_var_midiHandles_8c0fcd28', 4);
        $this->setSize('_push_fadeout_8c022b60', 4);
        $this->setSize('_PromptHandleMultiple_8c016c58', 4);
        $this->setSize('_var_currentSysResGroupInfo_8c225fb0', 4);
        $this->setSize('_var_resourceGroup_8c2263a8', 0x0c);
        $this->setSize('_var_tex_8c157af8', 4);
        $this->setSize('_CourseMenuFreeResourceGroup_8c0185c4', 4);
        $this->setSize('_njGarbageTexture', 4);
        $this->setSize('_AsqInitQueues_8c011f36', 4);
        $this->setSize('_AsqResetQueues_8c011f6c', 4);
        $this->setSize('_CourseMenuRequestSysResgrp_8c018568', 4);
        $this->setSize('_RouteLoadSetPvmReady_8c014330', 4);
        $this->setSize('_AsqProcessQueues_8c011fe0', 4);
        $this->setSize('_var_profileUnlockedCount_8c2263a4', 4);
        $this->setSize('_var_profileUnlocked_8c2263b4', 56);
        $this->setSize('_EventHasProgressFlagAlt_8c02aff0', 4);
        $this->setSize('__divls', 4);
        $this->setSize('__modls', 4);
        $this->setSize('_CourseMenuReturn_8c017ef2', 4);
        $this->setSize('_FUN_8c016182', 4);

        // Uninitialized, but read on every path.
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), self::MIDI);
    }

    private function menu(int $off): int
    {
        return $this->addressOf('_var_menuState_8c1bc7a8') + $off;
    }

    private function expectDrawTail(): void
    {
        $resGroup = $this->menu(0x0c);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with(
            $resGroup, 0, 0.0, 0.0, -5.0
        );
        $this->shouldCall('_njSetBackColor')->with(0xff000000, 0xff000000, 0xff000000);
    }

    private function expectDrawChecklistTail(): void
    {
        $this->shouldCall('_drawEpisodeChecklist_8c01cac8');
        $this->expectDrawTail();
    }

    public function test_init_waits_for_pvm_ready(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 0);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldCall('_RouteLoadIsPvmReady_8c01432a')->andReturn(0);
        $this->shouldCall('_AsqFreeQueues_8c011f7e');
        $this->shouldWriteLong($this->menu(self::STATE), 1);
        $this->shouldCall('_push_fadein_8c022a9c')->with(10);
    }

    public function test_init_returns_early_when_pvm_not_ready(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 0);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldCall('_RouteLoadIsPvmReady_8c01432a')->andReturn(1);
    }

    public function test_grid_fade_in_waits_for_fade(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 1);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldCall('_drawUnlockGrid_8c01c9f2');
        $this->expectDrawTail();

    }

    public function test_grid_fade_in_completes(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 1);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(self::STATE), 2);
        $this->shouldCall('_drawUnlockGrid_8c01c9f2');
        $this->expectDrawTail();

    }

    public function test_grid_animating_waits_for_interpolation(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 3);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldCall('_CourseMenuInterpolateCursor_8c016d2c')->andReturn(0);
        $this->shouldCall('_drawUnlockGrid_8c01c9f2');
        $this->expectDrawTail();

    }

    public function test_grid_animating_completes(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 3);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldCall('_CourseMenuInterpolateCursor_8c016d2c')->andReturn(1);
        $this->shouldWriteLong($this->menu(self::STATE), 2);
        $this->shouldCall('_drawUnlockGrid_8c01c9f2');
        $this->expectDrawTail();

    }

    private function setPress(int $press): void
    {
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, $press);
    }

    private function setCursorPos(int $row, int $col): void
    {
        $this->initUint32($this->menu(0x40), $row);
        $this->initUint32($this->menu(0x3c), $col);
    }

    private function setCursorScreenPos(float $x, float $y): void
    {
        $this->initUint32($this->menu(0x20), $this->floatToUint32($x));
        $this->initUint32($this->menu(0x24), $this->floatToUint32($y));
    }

    public function test_grid_idle_no_input_just_draws(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 2);
        $this->setCursorPos(1, 1);
        $this->setPress(0);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldCall('_drawUnlockGrid_8c01c9f2');
        $this->expectDrawTail();
    }

    public function test_grid_idle_up_moves_and_animates(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 2);
        $this->setCursorPos(3, 2); // row 3, col 2
        $this->setCursorScreenPos(50.0, 60.0);
        $this->setPress(0x10); // KU

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x40), 2); // row--
        $this->shouldWriteLong($this->menu(self::STATE), 3);
        $this->shouldWriteLong($this->menu(0x28), $this->floatToUint32(2 * 45.0 + 96.0));
        $this->shouldWriteLong($this->menu(0x2c), $this->floatToUint32(2 * 48.0 + 128.0));
        $this->shouldWriteLong($this->menu(0x30), $this->floatToUint32(((2 * 45.0 + 96.0) - 50.0) / 6.0));
        $this->shouldWriteLong($this->menu(0x34), $this->floatToUint32(((2 * 48.0 + 128.0) - 60.0) / 6.0));
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldCall('_drawUnlockGrid_8c01c9f2');
        $this->expectDrawTail();
    }

    public function test_grid_idle_up_wraps_at_row_zero_col_under_five(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 2);
        $this->setCursorPos(0, 4); // row 0, col 4 (< 5)
        $this->setCursorScreenPos(0.0, 0.0);
        $this->setPress(0x10); // KU

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x40), -1);
        $this->shouldWriteLong($this->menu(0x40), 5);
        $this->shouldWriteLong($this->menu(self::STATE), 3);
        $this->shouldWriteLong($this->menu(0x28), $this->floatToUint32(4 * 45.0 + 96.0));
        $this->shouldWriteLong($this->menu(0x2c), $this->floatToUint32(5 * 48.0 + 128.0));
        $this->shouldWriteLong($this->menu(0x30), $this->floatToUint32((4 * 45.0 + 96.0) / 6.0));
        $this->shouldWriteLong($this->menu(0x34), $this->floatToUint32((5 * 48.0 + 128.0) / 6.0));
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldCall('_drawUnlockGrid_8c01c9f2');
        $this->expectDrawTail();
    }

    public function test_grid_idle_up_wraps_at_row_zero_col_five_or_more(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 2);
        $this->setCursorPos(0, 7); // row 0, col 7 (>= 5)
        $this->setCursorScreenPos(0.0, 0.0);
        $this->setPress(0x10); // KU

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x40), -1);
        $this->shouldWriteLong($this->menu(0x40), 4);
        $this->shouldWriteLong($this->menu(self::STATE), 3);
        $this->shouldWriteLong($this->menu(0x28), $this->floatToUint32(7 * 45.0 + 96.0));
        $this->shouldWriteLong($this->menu(0x2c), $this->floatToUint32(4 * 48.0 + 128.0));
        $this->shouldWriteLong($this->menu(0x30), $this->floatToUint32((7 * 45.0 + 96.0) / 6.0));
        $this->shouldWriteLong($this->menu(0x34), $this->floatToUint32((4 * 48.0 + 128.0) / 6.0));
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldCall('_drawUnlockGrid_8c01c9f2');
        $this->expectDrawTail();
    }

    public function test_grid_idle_down_wraps_to_zero_col_under_five(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 2);
        $this->setCursorPos(5, 3); // row 5, col 3 (< 5), max row is 5
        $this->setCursorScreenPos(0.0, 0.0);
        $this->setPress(0x20); // KD

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x40), 6);
        $this->shouldWriteLong($this->menu(0x40), 0);
        $this->shouldWriteLong($this->menu(self::STATE), 3);
        $this->shouldWriteLong($this->menu(0x28), $this->floatToUint32(3 * 45.0 + 96.0));
        $this->shouldWriteLong($this->menu(0x2c), $this->floatToUint32(0 * 48.0 + 128.0));
        $this->shouldWriteLong($this->menu(0x30), $this->floatToUint32((3 * 45.0 + 96.0) / 6.0));
        $this->shouldWriteLong($this->menu(0x34), $this->floatToUint32((128.0) / 6.0));
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldCall('_drawUnlockGrid_8c01c9f2');
        $this->expectDrawTail();
    }

    public function test_grid_idle_down_wraps_to_zero_col_five_or_more(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 2);
        $this->setCursorPos(4, 7); // row 4, col 7 (>= 5), max row is 4
        $this->setCursorScreenPos(0.0, 0.0);
        $this->setPress(0x20); // KD

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x40), 5);
        $this->shouldWriteLong($this->menu(0x40), 0);
        $this->shouldWriteLong($this->menu(self::STATE), 3);
        $this->shouldWriteLong($this->menu(0x28), $this->floatToUint32(7 * 45.0 + 96.0));
        $this->shouldWriteLong($this->menu(0x2c), $this->floatToUint32(0 * 48.0 + 128.0));
        $this->shouldWriteLong($this->menu(0x30), $this->floatToUint32((7 * 45.0 + 96.0) / 6.0));
        $this->shouldWriteLong($this->menu(0x34), $this->floatToUint32((128.0) / 6.0));
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldCall('_drawUnlockGrid_8c01c9f2');
        $this->expectDrawTail();
    }

    public function test_grid_idle_left_wraps_col_row_under_five(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 2);
        $this->setCursorPos(3, 0); // row 3 (< 5), col 0
        $this->setCursorScreenPos(0.0, 0.0);
        $this->setPress(0x40); // KL

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x3c), -1);
        $this->shouldWriteLong($this->menu(0x3c), 9);
        $this->shouldWriteLong($this->menu(self::STATE), 3);
        $this->shouldWriteLong($this->menu(0x28), $this->floatToUint32(9 * 45.0 + 96.0));
        $this->shouldWriteLong($this->menu(0x2c), $this->floatToUint32(3 * 48.0 + 128.0));
        $this->shouldWriteLong($this->menu(0x30), $this->floatToUint32((9 * 45.0 + 96.0) / 6.0));
        $this->shouldWriteLong($this->menu(0x34), $this->floatToUint32((3 * 48.0 + 128.0) / 6.0));
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldCall('_drawUnlockGrid_8c01c9f2');
        $this->expectDrawTail();
    }

    public function test_grid_idle_left_wraps_col_row_five_or_more(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 2);
        $this->setCursorPos(5, 0); // row 5 (>= 5), col 0
        $this->setCursorScreenPos(0.0, 0.0);
        $this->setPress(0x40); // KL

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x3c), -1);
        $this->shouldWriteLong($this->menu(0x3c), 4);
        $this->shouldWriteLong($this->menu(self::STATE), 3);
        $this->shouldWriteLong($this->menu(0x28), $this->floatToUint32(4 * 45.0 + 96.0));
        $this->shouldWriteLong($this->menu(0x2c), $this->floatToUint32(5 * 48.0 + 128.0));
        $this->shouldWriteLong($this->menu(0x30), $this->floatToUint32((4 * 45.0 + 96.0) / 6.0));
        $this->shouldWriteLong($this->menu(0x34), $this->floatToUint32((5 * 48.0 + 128.0) / 6.0));
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldCall('_drawUnlockGrid_8c01c9f2');
        $this->expectDrawTail();
    }

    public function test_grid_idle_right_wraps_to_zero_row_under_five(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 2);
        $this->setCursorPos(3, 9); // row 3 (< 5), col 9 (max)
        $this->setCursorScreenPos(0.0, 0.0);
        $this->setPress(0x80); // KR

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x3c), 10);
        $this->shouldWriteLong($this->menu(0x3c), 0);
        $this->shouldWriteLong($this->menu(self::STATE), 3);
        $this->shouldWriteLong($this->menu(0x28), $this->floatToUint32(0 * 45.0 + 96.0));
        $this->shouldWriteLong($this->menu(0x2c), $this->floatToUint32(3 * 48.0 + 128.0));
        $this->shouldWriteLong($this->menu(0x30), $this->floatToUint32((96.0) / 6.0));
        $this->shouldWriteLong($this->menu(0x34), $this->floatToUint32((3 * 48.0 + 128.0) / 6.0));
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldCall('_drawUnlockGrid_8c01c9f2');
        $this->expectDrawTail();
    }

    public function test_grid_idle_right_wraps_to_zero_row_five_or_more(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 2);
        $this->setCursorPos(5, 4); // row 5 (>= 5), col 4 (max)
        $this->setCursorScreenPos(0.0, 0.0);
        $this->setPress(0x80); // KR

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x3c), 5);
        $this->shouldWriteLong($this->menu(0x3c), 0);
        $this->shouldWriteLong($this->menu(self::STATE), 3);
        $this->shouldWriteLong($this->menu(0x28), $this->floatToUint32(0 * 45.0 + 96.0));
        $this->shouldWriteLong($this->menu(0x2c), $this->floatToUint32(5 * 48.0 + 128.0));
        $this->shouldWriteLong($this->menu(0x30), $this->floatToUint32((96.0) / 6.0));
        $this->shouldWriteLong($this->menu(0x34), $this->floatToUint32((5 * 48.0 + 128.0) / 6.0));
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldCall('_drawUnlockGrid_8c01c9f2');
        $this->expectDrawTail();
    }

    public function test_grid_idle_confirm_starts_confirm_fade_out(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 2);
        $this->setCursorPos(1, 1);
        $this->setPress(4); // TA

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(self::STATE), 4);
        $this->shouldWriteLong($this->menu(0x38), 2);
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 7, 0);
        $this->shouldCall('_drawUnlockGrid_8c01c9f2');
        $this->expectDrawTail();
    }

    public function test_grid_idle_cancel_exits_to_course_menu(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 2);
        $this->setCursorPos(1, 1);
        $this->setPress(2); // TB

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(self::STATE), 11);
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 1, 0);
        $this->shouldCall('_drawUnlockGrid_8c01c9f2');
        $this->expectDrawTail();
    }

    public function test_confirm_fade_out_still_fading_draws_grid(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 4);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldCall('_drawUnlockGrid_8c01c9f2');
        $this->expectDrawTail();
    }

    public function test_confirm_fade_out_completes_updates_page_load(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 4);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldCall('_updatePageLoad_8c01cbec');
    }

    public function test_page_load_waits_for_pvm_ready(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 5);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldCall('_RouteLoadIsPvmReady_8c01432a')->andReturn(1);
    }

    public function test_page_load_completes_selected_zero_plays_low_jingle(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 5);
        $this->initUint32($this->menu(0x38), 0);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldCall('_RouteLoadIsPvmReady_8c01432a')->andReturn(0);
        $this->shouldCall('_AsqFreeQueues_8c011f7e');
        $this->shouldCall('_sdMidiPlay')->with(
            self::MIDI, 1, 8, 0
        );
        $this->shouldWriteLong($this->menu(self::STATE), 6);
        $this->shouldCall('_push_fadein_8c022a9c')->with(10);
    }

    public function test_page_load_completes_selected_one_plays_other_jingle(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 5);
        $this->initUint32($this->menu(0x38), 1);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldCall('_RouteLoadIsPvmReady_8c01432a')->andReturn(0);
        $this->shouldCall('_AsqFreeQueues_8c011f7e');
        $this->shouldCall('_sdMidiPlay')->with(
            self::MIDI, 1, 7, 0
        );
        $this->shouldWriteLong($this->menu(self::STATE), 6);
        $this->shouldCall('_push_fadein_8c022a9c')->with(10);
    }

    public function test_page_fade_in_waits_for_fade(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 6);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->expectDrawChecklistTail();
    }

    public function test_page_fade_in_completes(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 6);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(self::STATE), 7);
        $this->expectDrawChecklistTail();
    }

    public function test_page_view_no_input_just_draws_checklist(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 7);
        $this->initUint32($this->menu(0x38), 1);
        $this->setPress(0);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($this->menu(0x38), 5);
        $this->expectDrawChecklistTail();
    }

    public function test_page_view_confirm_selected_under_four_exits_fade_out(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 7);
        $this->initUint32($this->menu(0x38), 2);
        $this->setPress(4); // TA

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($this->menu(0x38), 5);
        $this->shouldWriteLong($this->menu(self::STATE), 8);
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);
        $this->expectDrawChecklistTail();
    }

    public function test_page_view_confirm_selected_four_or_more_returns_to_grid(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 7);
        $this->initUint32($this->menu(0x38), 4);
        $this->setPress(4); // TA

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($this->menu(0x38), 5);
        $this->shouldWriteLong($this->menu(self::STATE), 10);
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);
        $this->expectDrawChecklistTail();
    }

    public function test_page_view_cancel_returns_to_grid(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 7);
        $this->initUint32($this->menu(0x38), 1);
        $this->setPress(2); // TB

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($this->menu(0x38), 5);
        $this->shouldWriteLong($this->menu(self::STATE), 10);
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 1, 0);
        $this->expectDrawChecklistTail();
    }

    public function test_page_exit_fade_out_still_fading_draws_checklist(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 8);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->expectDrawChecklistTail();
    }

    public function test_page_exit_fade_out_completes(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 8);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(self::STATE), 9);
        $this->shouldWriteLong($this->menu(0x68), 0);
    }

    public function test_grid_fade_out_still_fading_draws_checklist(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 10);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->expectDrawChecklistTail();
    }

    public function test_grid_fade_out_completes_snaps_cursor(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 10);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->menu(0x3c), 2); // col
        $this->initUint32($this->menu(0x40), 3); // row

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(self::STATE), 1);
        $this->shouldWriteLong($this->menu(0x20), $this->floatToUint32(2 * 45.0 + 96.0));
        $this->shouldWriteLong($this->menu(0x24), $this->floatToUint32(3 * 48.0 + 128.0));
        $this->shouldCall('_push_fadein_8c022a9c')->with(10);
    }

    private function floatToUint32(float $value): int
    {
        return unpack('L', pack('f', $value))[1];
    }

    private function useDivMod(): void
    {
        $this->onCall('__divls', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });
        $this->onCall('__modls', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });
    }

    private function initUnlocked(array $unlocked): void
    {
        $base = $this->addressOf('_var_profileUnlocked_8c2263b4');
        foreach ($unlocked as $i => $value) {
            $this->initUint8($base + $i, $value ? 1 : 0);
        }
    }

    public function test_page_advance_delay_waits_four_frames(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 9);
        $this->initUint32($this->menu(0x68), 2); // logo_timer, below threshold

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x68), 3);
    }

    public function test_page_advance_delay_prev_no_unlocked_just_updates_page(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 9);
        $this->initUint32($this->menu(0x68), 3);
        $this->initUint32($this->menu(0x38), 0); // selected: prev
        $this->initUint32($this->addressOf('_var_profileUnlockedCount_8c2263a4'), 0);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x68), 4);
        $this->shouldCall('_updatePageLoad_8c01cbec');
    }

    public function test_page_advance_delay_prev_finds_immediately_unlocked_slot(): void
    {
        $this->resolveSymbols();
        $this->useDivMod();
        $this->initUint32($this->menu(self::STATE), 9);
        $this->initUint32($this->menu(0x68), 3);
        $this->initUint32($this->menu(0x38), 0); // selected: prev
        $this->setCursorPos(2, 5); // slot 25
        $this->initUint32($this->addressOf('_var_profileUnlockedCount_8c2263a4'), 1);
        $unlocked = array_fill(0, 55, 0);
        $unlocked[24] = 1;
        $this->initUnlocked($unlocked);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x68), 4);
        $this->shouldCall('__modls');
        $this->shouldWriteLong($this->menu(0x3c), 4);
        $this->shouldCall('__divls');
        $this->shouldWriteLong($this->menu(0x40), 2);
        $this->shouldCall('_updatePageLoad_8c01cbec');
    }

    public function test_page_advance_delay_prev_wraps_around_to_last_slot(): void
    {
        $this->resolveSymbols();
        $this->useDivMod();
        $this->initUint32($this->menu(self::STATE), 9);
        $this->initUint32($this->menu(0x68), 3);
        $this->initUint32($this->menu(0x38), 0); // selected: prev
        $this->setCursorPos(0, 0); // slot 0
        $this->initUint32($this->addressOf('_var_profileUnlockedCount_8c2263a4'), 1);
        $unlocked = array_fill(0, 55, 0);
        $unlocked[54] = 1; // last slot
        $this->initUnlocked($unlocked);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x68), 4);
        $this->shouldCall('__modls');
        $this->shouldWriteLong($this->menu(0x3c), 4);
        $this->shouldCall('__divls');
        $this->shouldWriteLong($this->menu(0x40), 5);
        $this->shouldCall('_updatePageLoad_8c01cbec');
    }

    public function test_page_advance_delay_selected_one_moves_col_back(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 9);
        $this->initUint32($this->menu(0x68), 3);
        $this->initUint32($this->menu(0x38), 1);
        $this->setCursorPos(2, 5);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x68), 4);
        $this->shouldWriteLong($this->menu(0x3c), 4);
        $this->shouldCall('_updatePageLoad_8c01cbec');
    }

    public function test_page_advance_delay_selected_one_wraps_col_and_row(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 9);
        $this->initUint32($this->menu(0x68), 3);
        $this->initUint32($this->menu(0x38), 1);
        $this->setCursorPos(2, 0);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x68), 4);
        $this->shouldWriteLong($this->menu(0x3c), -1);
        $this->shouldWriteLong($this->menu(0x3c), 9);
        $this->shouldWriteLong($this->menu(0x40), 1);
        $this->shouldCall('_updatePageLoad_8c01cbec');
    }

    public function test_page_advance_delay_selected_one_wraps_past_first_row(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 9);
        $this->initUint32($this->menu(0x68), 3);
        $this->initUint32($this->menu(0x38), 1);
        $this->setCursorPos(0, 0);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x68), 4);
        $this->shouldWriteLong($this->menu(0x3c), -1);
        $this->shouldWriteLong($this->menu(0x3c), 9);
        $this->shouldWriteLong($this->menu(0x40), -1);
        $this->shouldWriteLong($this->menu(0x3c), 4);
        $this->shouldWriteLong($this->menu(0x40), 5);
        $this->shouldCall('_updatePageLoad_8c01cbec');
    }

    public function test_page_advance_delay_selected_two_moves_col_forward(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 9);
        $this->initUint32($this->menu(0x68), 3);
        $this->initUint32($this->menu(0x38), 2);
        $this->setCursorPos(2, 5); // row < 5

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x68), 4);
        $this->shouldWriteLong($this->menu(0x3c), 6);
        $this->shouldCall('_updatePageLoad_8c01cbec');
    }

    public function test_page_advance_delay_selected_two_wraps_row_under_five(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 9);
        $this->initUint32($this->menu(0x68), 3);
        $this->initUint32($this->menu(0x38), 2);
        $this->setCursorPos(2, 9); // row < 5, col at max

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x68), 4);
        $this->shouldWriteLong($this->menu(0x3c), 10);
        $this->shouldWriteLong($this->menu(0x3c), 0);
        $this->shouldWriteLong($this->menu(0x40), 3);
        $this->shouldCall('_updatePageLoad_8c01cbec');
    }

    public function test_page_advance_delay_selected_two_wraps_row_five_or_more(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 9);
        $this->initUint32($this->menu(0x68), 3);
        $this->initUint32($this->menu(0x38), 2);
        $this->setCursorPos(5, 4); // row >= 5, col at max

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x68), 4);
        $this->shouldWriteLong($this->menu(0x3c), 5);
        $this->shouldWriteLong($this->menu(0x3c), 0);
        $this->shouldWriteLong($this->menu(0x40), 0);
        $this->shouldCall('_updatePageLoad_8c01cbec');
    }

    public function test_page_advance_delay_next_no_unlocked_just_updates_page(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 9);
        $this->initUint32($this->menu(0x68), 3);
        $this->initUint32($this->menu(0x38), 3); // selected: next
        $this->initUint32($this->addressOf('_var_profileUnlockedCount_8c2263a4'), 0);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x68), 4);
        $this->shouldCall('_updatePageLoad_8c01cbec');
    }

    public function test_page_advance_delay_next_finds_immediately_unlocked_slot(): void
    {
        $this->resolveSymbols();
        $this->useDivMod();
        $this->initUint32($this->menu(self::STATE), 9);
        $this->initUint32($this->menu(0x68), 3);
        $this->initUint32($this->menu(0x38), 3); // selected: next
        $this->setCursorPos(2, 5); // slot 25
        $this->initUint32($this->addressOf('_var_profileUnlockedCount_8c2263a4'), 1);
        $unlocked = array_fill(0, 55, 0);
        $unlocked[26] = 1;
        $this->initUnlocked($unlocked);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x68), 4);
        $this->shouldCall('__modls');
        $this->shouldWriteLong($this->menu(0x3c), 6);
        $this->shouldCall('__divls');
        $this->shouldWriteLong($this->menu(0x40), 2);
        $this->shouldCall('_updatePageLoad_8c01cbec');
    }

    public function test_page_advance_delay_next_wraps_around_to_first_slot(): void
    {
        $this->resolveSymbols();
        $this->useDivMod();
        $this->initUint32($this->menu(self::STATE), 9);
        $this->initUint32($this->menu(0x68), 3);
        $this->initUint32($this->menu(0x38), 3); // selected: next
        $this->setCursorPos(5, 4); // slot 54 (last)
        $this->initUint32($this->addressOf('_var_profileUnlockedCount_8c2263a4'), 1);
        $unlocked = array_fill(0, 55, 0);
        $unlocked[0] = 1;
        $this->initUnlocked($unlocked);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x68), 4);
        $this->shouldCall('__modls');
        $this->shouldWriteLong($this->menu(0x3c), 0);
        $this->shouldCall('__divls');
        $this->shouldWriteLong($this->menu(0x40), 0);
        $this->shouldCall('_updatePageLoad_8c01cbec');
    }

    public function test_exit_to_course_menu_still_fading_draws_nothing(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 11);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->expectDrawTail();
    }

    public function test_exit_to_course_menu_completes(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 11);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->shouldWriteLong($this->menu(0x3c), 0);
        $this->shouldWriteLong($this->menu(0x40), 1);
        $this->shouldWriteLong($this->menu(0x20), $this->floatToUint32(0.0));
        $this->shouldCall('_FUN_8c016182');
        $this->shouldCall('_CourseMenuReturn_8c017ef2');
    }

    public function test_default_state_draws_nothing(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->menu(self::STATE), 0xff);

        $this->call('_menuTask_8c01ccec')->with(0, 0);
        $this->expectDrawTail();

    }
};
