<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    private function resolveSymbols(): int
    {
        $this->setSize('_var_8c2285c4', 0xa0);
        $base = $this->addressOf('_var_8c2285c4');
        $this->rellocate('_var_8c22861c', $base + 0x58);
        $this->rellocate('_var_8c228634', $base + 0x70);

        $this->setSize('_var_driveMsgQueue_8c228564', 0x60);
        $this->setSize('_var_busState_8c1bb9d0', 0x400);
        $this->setSize('_var_driverPoints_8c2285d0', 4);
        $this->setSize('_var_messageBoxActive_8c22847c', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_fadeCompleteCallback_8c22656c', 4);
        $this->setSize('_var_8c228660', 4);
        $this->setSize('_var_8c228680', 4);
        $this->setSize('_var_8c22868c', 4);
        $this->setSize('_var_8c228688', 4);
        $this->setSize('_var_8c228674', 4);
        $this->setSize('_var_8c228678', 4);
        $this->setSize('_var_8c22867c', 4);
        $this->setSize('_var_8c228684', 4);
        $this->setSize('_var_8c228690', 4);
        $this->setSize('_var_8c228694', 4);
        $this->setSize('_var_8c228698', 4);
        $this->setSize('_var_8c22869c', 4);
        $this->setSize('_var_8c2286a0', 4);
        $this->setSize('_init_8c03bd80', 4);
        $this->setSize('_var_vibport_8c1ba354', 4);
        $this->setSize('_var_fadeRequest_8c226564', 4);
        $this->setSize('_DriveMsgDraw_8c02b388', 4);
        $this->setSize('_var_8c22866c', 4);

        return $base;
    }

    private function initFloat(int $address, float $value): void
    {
        $raw = unpack('L', pack('f', $value))[1];
        $this->initUint32($address, $raw);
    }

    private function baselineMsgQueue(): void
    {
        $q = $this->addressOf('_var_driveMsgQueue_8c228564');
        for ($i = 0; $i < 4; $i++) {
            $this->initUint32($q + $i * 0x18 + 0x14, 0); // holdFrames = 0 -> ticker no-op
        }
    }

    public function test_phase_0_is_idle_and_only_ticks_message_queue(): void
    {
        $base = $this->resolveSymbols();
        $this->baselineMsgQueue();
        $this->initUint32($base + 0x00, 0); // phase 0

        $this->call('_taskCallback_8c02c072');

        $this->shouldCall('_FadeCmdPushCall1_8c0223ea')->with(0, new WildcardArgument(), 0);
    }

    public function test_phase_2_active_driving_ticks_offense_graders(): void
    {
        $base = $this->resolveSymbols();
        $this->baselineMsgQueue();
        $this->initUint32($base + 0x00, 2); // phase 2

        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initFloat($busState + 0x27c, 0.0); // speed
        $this->initUint32($busState + 0x368, 0);
        $this->initUint32($busState + 0x34c, 0);
        $this->initUint32($busState + 0x36c, 0x80); // skip the steering-angle offense-code branch
        $this->initUint32($busState + 0x358, 0);
        $this->initUint32($busState + 0x374, 1); // != field_0x358 alias -> skip the lane-alias re-latch
        $this->initUint32($busState + 0x390, 0);

        $this->initUint32($base + 0x6c, 0); // var_8c22861c[5]
        $this->initUint32($base + 0x70, 0); // var_8c228634[0]

        $this->initUint32($this->addressOf('_var_8c228660'), 0); // no bump flags -> no vibration
        $this->initUint32($this->addressOf('_var_8c228690'), 5); // cooldowns all still active -> no graders fire
        $this->initUint32($this->addressOf('_var_8c228694'), 5);
        $this->initUint32($this->addressOf('_var_8c228698'), 5);
        $this->initUint32($this->addressOf('_var_8c22869c'), 5);
        $this->initUint32($this->addressOf('_var_8c2286a0'), 5);

        $this->initUint32($this->addressOf('_var_driverPoints_8c2285d0'), 50); // still > 0 -> stays in phase 2

        $this->call('_taskCallback_8c02c072');

        $this->shouldWriteFloat($this->addressOf('_var_8c22866c'), 0.0);
        $this->shouldWriteLong($this->addressOf('_var_8c228680'), 0);
        $this->shouldWriteLong($this->addressOf('_var_8c22868c'), 0); // field_0x36c high bit set -> skip grading
        $this->shouldWriteLong($this->addressOf('_var_8c228684'), 0); // var_8c22861c[5]
        $this->shouldWriteLong($this->addressOf('_var_8c228688'), 0); // var_8c228634[0]
        $this->shouldWriteLong($this->addressOf('_var_8c228674'), 0);
        $this->shouldWriteLong($this->addressOf('_var_8c228678'), 1);
        $this->shouldWriteLong($this->addressOf('_var_8c22867c'), 0);

        // Same-object callees: mock rather than let them really execute
        // (their own dependencies aren't set up in this test).
        $this->shouldCall('_handleBump_8c02b6d4')->andReturn(0);
        $this->shouldCall('_FUN_8c02b864')->andReturn(0);
        $this->shouldWriteLong($this->addressOf('_var_8c228690'), 4);
        $this->shouldCall('_FUN_8c02bcd8')->andReturn(0);

        $this->shouldWriteLong($base + 0x74, 0); // var_8c228634[1]
        $this->shouldWriteLong($base + 0x78, 0); // var_8c228634[2]

        // var_driverPoints_8c2285d0 stays > 0 here, so the phase-4 handoff
        // (and its SndStartAdxFadeOut pair) does NOT fire -- goes straight
        // to the tail's message-queue tick and fade-command push.
        $this->shouldCall('_FadeCmdPushCall1_8c0223ea')->with(0, new WildcardArgument(), 0);
    }
};
