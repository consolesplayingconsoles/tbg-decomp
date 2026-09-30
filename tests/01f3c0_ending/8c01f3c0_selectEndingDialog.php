<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _selectEndingDialog_8c01f3c0(void): tallies each of the 9 courses'
 * storyAward_0x03 state (0-3, 3 = perfect) in var_progress_8c1ba1cc and
 * picks the ending's dialog tier from the result: all 9 perfect -> tier 1
 * (init_endingVoicesPerfect_8c04522c), all 9 attempted (state 1-3) -> tier 2
 * (init_endingVoicesHigh_8c045244), >=5 attempted -> tier 3
 * (init_endingVoicesNormal_8c04525c), otherwise tier 4
 * (init_endingVoicesFailure_8c045278).
 * The tier is written to var_dialogQueue_8c225fbc[0]; the matching voice id
 * list pointer is written to var_endingVoiceList_8c226430.
 */
return new class extends TestCase {
    const PLAYER_PROGRESS_SIZE = 0xe8;
    const COURSES_0X44 = 0x44;
    const COURSE_SIZE = 0x08;
    const STORY_SPRITE_NO_0X03 = 0x03;

    private function resolveSymbols(): void
    {
        $this->setSize('_var_progress_8c1ba1cc', self::PLAYER_PROGRESS_SIZE);
        $this->setSize('_var_dialogQueue_8c225fbc', 0x10);
    }

    private function setCourseState(int $index, int $state): void
    {
        $base = $this->addressOf('_var_progress_8c1ba1cc')
            + self::COURSES_0X44 + $index * self::COURSE_SIZE;
        $this->initUint8($base + self::STORY_SPRITE_NO_0X03, $state);
    }

    private function setAllCourseStates(array $states): void
    {
        foreach ($states as $i => $state) {
            $this->setCourseState($i, $state);
        }
    }

    public function test_all_nine_perfect_selects_tier_1(): void
    {
        $this->resolveSymbols();
        $this->setAllCourseStates(array_fill(0, 9, 3));

        $this->call('_selectEndingDialog_8c01f3c0');

        $this->shouldWriteLongTo('_var_dialogQueue_8c225fbc', 1);
        $this->shouldWriteLongTo('_var_endingVoiceList_8c226430', $this->addressOf('_init_endingVoicesPerfect_8c04522c'));
    }

    public function test_nine_attempted_but_not_all_perfect_selects_tier_2(): void
    {
        $this->resolveSymbols();
        // 8 perfect, 1 merely attempted (state 2): all 9 attempted, not all perfect.
        $this->setAllCourseStates([3, 3, 3, 3, 3, 3, 3, 3, 2]);

        $this->call('_selectEndingDialog_8c01f3c0');

        $this->shouldWriteLongTo('_var_dialogQueue_8c225fbc', 2);
        $this->shouldWriteLongTo('_var_endingVoiceList_8c226430', $this->addressOf('_init_endingVoicesHigh_8c045244'));
    }

    public function test_five_attempted_selects_tier_3(): void
    {
        $this->resolveSymbols();
        // Exactly 5 attempted (mix of states 1 and 2), rest untouched (0).
        $this->setAllCourseStates([1, 2, 1, 2, 1, 0, 0, 0, 0]);

        $this->call('_selectEndingDialog_8c01f3c0');

        $this->shouldWriteLongTo('_var_dialogQueue_8c225fbc', 3);
        $this->shouldWriteLongTo('_var_endingVoiceList_8c226430', $this->addressOf('_init_endingVoicesNormal_8c04525c'));
    }

    public function test_fewer_than_five_attempted_selects_tier_4(): void
    {
        $this->resolveSymbols();
        $this->setAllCourseStates(array_fill(0, 9, 0));

        $this->call('_selectEndingDialog_8c01f3c0');

        $this->shouldWriteLongTo('_var_dialogQueue_8c225fbc', 4);
        $this->shouldWriteLongTo('_var_endingVoiceList_8c226430', $this->addressOf('_init_endingVoicesFailure_8c045278'));
    }
};
