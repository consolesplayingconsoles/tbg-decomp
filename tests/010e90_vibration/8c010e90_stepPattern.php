<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U8;
use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _stepPattern_8c010e90(int port): one frame of the queued rumble pattern.
 *
 * A pattern is a VibStep list terminated by a zero frames_0x00. Each step is
 * handed to the pack once and then just held: frame_0x04 counts up until it
 * passes the step's frames_0x00, at which point the pack is stopped and the
 * next step started.
 *
 * Pattern 0 (init_vibEngineStart_8c03bdac) drives most of these:
 *   [0] 20 frames, flag 1, power -1 (0xff), freq 15
 *   [1] 100 frames, flag 1, power -2 (0xfe), freq 30
 *   [2] terminator
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

    /**
     * pdVibMxStart's PDS_VIBPARAM is a stack local, so check the struct it
     * points at instead of its address. unit is always 1 and inc always 0;
     * reserved[3] is never written. $result is what the pack answers --
     * PDD_VIBERR_OK is 0, PDD_VIBERR_BUSY is -1.
     */
    private function shouldStartVib(int $port, int $flag, int $power, int $freq, int $result): void
    {
        $this->shouldCall('_pdVibMxStart')
            ->with($port)
            ->do(function () use ($flag, $power, $freq) {
                $p = $this->registers[5]->value;
                $got = [];
                foreach ([0, 1, 2, 3, 4] as $offset) {
                    $got[] = $this->memory->readUInt8($p + $offset)->value;
                }
                $want = [1, $flag, $power, $freq, 0];
                if ($got !== $want) {
                    throw new \Exception(sprintf(
                        'Unexpected PDS_VIBPARAM: got [%s], want [%s]',
                        implode(', ', $got),
                        implode(', ', $want)
                    ));
                }
            })
            ->andReturn($result);
    }

    public function test_first_step_starts_the_pack_and_marks_the_state_playing(): void
    {
        $this->resolveSymbols();
        $this->initState(0, 0, 0, 0);

        $this->call('_stepPattern_8c010e90')->with(0);

        // This one start is the only one whose result is never checked; the
        // step is counted as begun whatever the pack answers.
        $this->shouldStartVib(0, 1, 0xff, 15, 0);
        $this->shouldWriteLong($this->state() + 0x08, 1);
        $this->shouldWriteLong($this->state() + 0x0c, 1);
        $this->shouldWriteLong($this->state() + 0x04, 1);
    }

    public function test_pattern_index_selects_which_step_list_is_played(): void
    {
        $this->resolveSymbols();
        // Pattern 3, init_vibHardBrake_8c03bdf4: 12 frames, power -7 (0xf9).
        $this->initState(3, 0, 0, 0);

        $this->call('_stepPattern_8c010e90')->with(0);

        $this->shouldStartVib(0, 1, 0xf9, 15, 0);
        $this->shouldWriteLong($this->state() + 0x08, 1);
        $this->shouldWriteLong($this->state() + 0x0c, 1);
        $this->shouldWriteLong($this->state() + 0x04, 1);
    }

    public function test_step_is_held_while_its_frame_count_has_not_been_passed(): void
    {
        $this->resolveSymbols();
        // frame_0x04 has reached step 0's 20 but not passed it, so the step
        // actually lasts frames_0x00 + 1 frames.
        $this->initState(0, 20, 1, 1);

        $this->call('_stepPattern_8c010e90')->with(0);

        $this->shouldWriteLong($this->state() + 0x04, 21);
    }

    public function test_step_advances_once_its_frame_count_is_passed(): void
    {
        $this->resolveSymbols();
        $this->initState(0, 21, 1, 1);

        $this->call('_stepPattern_8c010e90')->with(0);

        $this->shouldCall('_pdVibMxStop')->with(0);
        $this->shouldWriteLong($this->state() + 0x04, 0);
        $this->shouldStartVib(0, 1, 0xfe, 30, 0);
        $this->shouldWriteLong($this->state() + 0x08, 2);
        // The frame the switch happened on still counts.
        $this->shouldWriteLong($this->state() + 0x04, 1);
    }

    public function test_start_is_retried_until_the_pack_stops_answering_busy(): void
    {
        $this->resolveSymbols();
        $this->initState(0, 21, 1, 1);

        $this->call('_stepPattern_8c010e90')->with(0);

        $this->shouldCall('_pdVibMxStop')->with(0);
        $this->shouldWriteLong($this->state() + 0x04, 0);
        // The stop above is still in flight, so the first two starts bounce.
        $this->shouldStartVib(0, 1, 0xfe, 30, -1); // PDD_VIBERR_BUSY
        $this->shouldStartVib(0, 1, 0xfe, 30, -1);
        $this->shouldStartVib(0, 1, 0xfe, 30, 0);  // PDD_VIBERR_OK
        $this->shouldWriteLong($this->state() + 0x08, 2);
        $this->shouldWriteLong($this->state() + 0x04, 1);
    }

    /**
     * The loop only ends on PDD_VIBERR_OK, so a pack that has gone away
     * (PDD_VIBERR_NO_VIBRATOR) would spin forever -- original-game
     * behaviour, kept off the table by DriveCueTask_8c020214, which ticks
     * only while var_vibport_8c1ba354 names a live pack.
     */
    public function test_a_non_busy_error_is_retried_just_the_same(): void
    {
        $this->resolveSymbols();
        $this->initState(0, 21, 1, 1);

        $this->call('_stepPattern_8c010e90')->with(0);

        $this->shouldCall('_pdVibMxStop')->with(0);
        $this->shouldWriteLong($this->state() + 0x04, 0);
        $this->shouldStartVib(0, 1, 0xfe, 30, -2); // PDD_VIBERR_NO_VIBRATOR
        $this->shouldStartVib(0, 1, 0xfe, 30, 0);
        $this->shouldWriteLong($this->state() + 0x08, 2);
        $this->shouldWriteLong($this->state() + 0x04, 1);
    }

    public function test_terminator_stops_the_pack_and_parks_the_state(): void
    {
        $this->resolveSymbols();
        // Step 1 (100 frames) has just run out; step 2 is the terminator.
        $this->initState(0, 101, 2, 1);
        $state = $this->state();

        $this->call('_stepPattern_8c010e90')->with(0);

        $this->shouldCall('_pdVibMxStop')->with(0);
        $this->shouldWriteLong($state + 0x04, 0);
        // VibClear_8c010fbe is mocked out, so stand in for it: the tail of
        // this function reads the playing_0x0c it zeroes, and that is what
        // keeps frame_0x04 from being bumped on the way out.
        $this->shouldCall('_VibClear_8c010fbe')->do(function () use ($state) {
            for ($i = 0; $i < 0x10; $i++) {
                $this->memory->writeUInt8($state + $i, U8::of(0));
            }
            $this->memory->writeUInt32($state + 0x00, U32::of(7));
        });
    }
};
