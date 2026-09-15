<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function shouldResetAllToReady(): void
    {
        $this->shouldWriteLongTo('_var_cooldownCollision_8c228690', 1);
        $this->shouldWriteLongTo('_var_cooldownOffCourse_8c228694', 1);
        $this->shouldWriteLongTo('_var_cooldownSignal_8c228698', 1);
        $this->shouldWriteLongTo('_var_cooldownLane_8c22869c', 1);
        $this->shouldWriteLongTo('_var_cooldownIntersection_8c2286a0', 1);
    }

    public function test_type_1_arms_first_slot(): void
    {
        $this->call('_armCooldowns_8c02b578')->with(1);

        $this->shouldResetAllToReady();
        $this->shouldWriteLongTo('_var_cooldownCollision_8c228690', 0x96);
        $this->shouldWriteLongTo('_var_8c2285fc', 0);
    }

    public function test_type_2_arms_second_slot(): void
    {
        $this->call('_armCooldowns_8c02b578')->with(2);

        $this->shouldResetAllToReady();
        $this->shouldWriteLongTo('_var_cooldownOffCourse_8c228694', 0xd2);
        $this->shouldWriteLongTo('_var_8c2285fc', 0);
    }

    public function test_type_3_arms_third_slot(): void
    {
        $this->call('_armCooldowns_8c02b578')->with(3);

        $this->shouldResetAllToReady();
        $this->shouldWriteLongTo('_var_cooldownSignal_8c228698', 0x96);
        $this->shouldWriteLongTo('_var_8c2285fc', 0);
    }

    public function test_type_4_arms_fourth_slot_and_skips_reset(): void
    {
        $this->call('_armCooldowns_8c02b578')->with(4);

        $this->shouldResetAllToReady();
        $this->shouldWriteLongTo('_var_cooldownLane_8c22869c', 0xd2);
    }

    public function test_type_5_arms_fifth_slot_and_skips_reset(): void
    {
        $this->call('_armCooldowns_8c02b578')->with(5);

        $this->shouldResetAllToReady();
        $this->shouldWriteLongTo('_var_cooldownIntersection_8c2286a0', 0xd2);
    }

    public function test_other_type_resets_all_to_ready_and_skips_reset(): void
    {
        $this->call('_armCooldowns_8c02b578')->with(0);

        $this->shouldResetAllToReady();
    }
};
