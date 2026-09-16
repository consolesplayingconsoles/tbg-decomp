<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_practiceRules_8c226410', 4);
        $this->setSize('_var_practiceLesson_8c22640c', 4);
        $this->setSize('_var_route_8c18ad1c', 4);
        $this->setSize('_var_nextStopSegment_8c228710', 4);
    }

    public function test_practice_bit_clear_returns_true(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0); // bit 2 clear

        $this->call('_DrivePointsRunComplete_8c02c586');
        $this->shouldReturn(1);
    }

    public function test_practice_lesson_7_threshold_2_met(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 4);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 7);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 2);

        $this->call('_DrivePointsRunComplete_8c02c586');
        $this->shouldReturn(1); // 2 <= 2
    }

    public function test_practice_lesson_7_threshold_2_not_met(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 4);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 7);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 1);

        $this->call('_DrivePointsRunComplete_8c02c586');
        $this->shouldReturn(0); // 2 <= 1 is false
    }

    public function test_practice_lesson_9_or_10_threshold_4(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 4);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 10);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 4);

        $this->call('_DrivePointsRunComplete_8c02c586');
        $this->shouldReturn(1);
    }

    public function test_route_wangan_threshold_0xf_met(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 1);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 0xf);

        $this->call('_DrivePointsRunComplete_8c02c586');
        $this->shouldReturn(1);
    }

    public function test_route_wangan_threshold_0xf_not_met(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 1);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 0xe);

        $this->call('_DrivePointsRunComplete_8c02c586');
        $this->shouldReturn(0);
    }

    public function test_route_shinjuku_threshold_0x16(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 0);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 0x16);

        $this->call('_DrivePointsRunComplete_8c02c586');
        $this->shouldReturn(1);
    }

    public function test_route_ome_threshold_0x16(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 2);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 0x16);

        $this->call('_DrivePointsRunComplete_8c02c586');
        $this->shouldReturn(1);
    }
};
