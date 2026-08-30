<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _BusStopUpdateArrival_8c02ce48(): per-frame bus-stop arrival state machine
 * (var_stopPhase_8c2285e4, states 0-4); takes no meaningful argument.
 * See 02c884_bus_stop.c's doc comment for the full state breakdown.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3b4 + 4);
        $this->setSize('_var_nextStopSegment_8c228710', 4);
        $this->setSize('_var_prevStopSegment_8c22870c', 4);
        $this->setSize('_var_8c2264c4', 4);
        $this->setSize('_var_8c1bc44c', 4);
        $this->setSize('_var_8c1bc450', 4);
        $this->setSize('_var_fadeCompleteCallback_8c22656c', 4);
        $this->setSize('_var_8c1bbc4c', 4);
        $this->setSize('_njSqrt', 4);

        // Referenced only inside drawStopMarker_8c02cd92's body, which is
        // never actually invoked by this function (only its address is
        // taken), but the object's own relocations still need resolving.
        $this->setSize('_var_8c228900', 8);
        $this->setSize('_var_8c228908', 4);
        $this->setSize('_var_8c228714', 4);
        $this->setSize('_var_8c1bc46c', 0x40);
        $this->setSize('_var_8c1bc440', 4);
        $this->setSize('_var_8c1bc444', 4);
        $this->setSize('_var_loadedFooNjm_8c1bc448', 4);

        // var_8c226450/8c226454 are accessed via displacement off the
        // already-imported var_8c22643c (0x14/0x18 further in), not by their
        // own relocation -- see src/asm/sectionB.src for the real layout.
        // rellocate() only takes effect the *first* time a name is resolved,
        // so a symbol already registered by setSize() can't be moved
        // afterwards; instead, size each symbol (and any gap between them)
        // to its real width so plain sequential allocation reproduces the
        // real relative layout on its own.
        $this->setSize('_var_8c22643c', 4);
        $this->alloc(0x10); // gap: 8c226440..8c22644f (unused by this function)
        $this->setSize('_var_8c226450', 4);
        $this->setSize('_var_8c226454', 4);

        // var_8c2285c4 through var_8c228640 are ALL one contiguous scratch
        // region in the real binary (see src/asm/sectionB.src -- there is no
        // gap between any of these labels), and this function reaches most
        // of them via displacement off var_8c2285c4 rather than by their own
        // relocation, so they must sit at their real relative offsets here
        // too (see the note above about rellocate() and 8c02caba_BusStopSetup.php
        // for the same pattern on a smaller slice of this same region).
        $this->setSize('_var_8c2285c4', 4);
        $this->setSize('_var_8c2285c8', 4);
        $this->setSize('_var_8c2285cc', 4);
        $this->setSize('_var_driverPoints_8c2285d0', 4);
        $this->setSize('_var_8c2285d4', 4);
        $this->setSize('_var_8c2285d8', 4);
        $this->setSize('_var_8c2285dc', 8); // covers 8c2285dc and the unnamed 8c2285e0
        $this->setSize('_var_stopPhase_8c2285e4', 4);
        $this->setSize('_var_8c2285e8', 4);
        $this->setSize('_var_stopMinDistance_8c2285ec', 4);
        $this->alloc(0x44); // gap: 8c2285f0..8c228633 (unused by this function)
        $this->setSize('_var_8c228634', 4);
        $this->alloc(0x08); // gap: 8c228638..8c22863f (unused by this function)
        $this->setSize('_var_8c228640', 4);

        // Same-object statics -- mock with shouldCall(), no setSize().
        //   _pickWaitingPassengers_8c02c8ae
        //   _advanceStopSegment_8c02ccae
        //   _drawStopMarker_8c02cd92 (address only, not called directly)

        // Cross-unit, not yet decompiled (02b464) -- mock with shouldCall().
        //   _FUN_8c02c586
        //   _FUN_8c02c784 (address only, not called directly)
    }

    private function initBusState(int $field0x3b0, int $field0x3b4, float $speed): int
    {
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($base + 0x2b4, 0xdeadbeef); // bus_state_0x2b4, overwritten on every tested path
        $this->initUint32($base + 0x3b0, $field0x3b0);
        $this->initUint32($base + 0x3b4, $field0x3b4);
        // Checked by state 2, not busState.speed_0x27c (Ghidra folded this
        // read into a bogus "flSpeed_0x27c" reference).
        $this->initUint32($this->addressOf('_var_8c1bbc4c'), unpack('L', pack('f', $speed))[1]);
        return $base;
    }

    // State 2 recomputes the distance to the stop itself from the bus's
    // position (busState posX_0x0f4/posZ_0x0fc) and the stop's ground
    // position (var_8c228900.x/var_8c228908); seed both, keeping the bus at
    // the origin so the caller can pick a convenient (dx, dz).
    private function seedStopOffset(float $dx, float $dz): void
    {
        $point = $this->addressOf('_var_8c228900');
        $this->initUint32($point + 0x00, unpack('L', pack('f', $dx))[1]); // .x
        $this->initUint32($point + 0x04, 0); // .y, unused here
        $this->initUint32($this->addressOf('_var_8c228908'), unpack('L', pack('f', $dz))[1]); // .z

        $busBase = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($busBase + 0xf4, 0); // posX_0x0f4
        $this->initUint32($busBase + 0xfc, 0); // posZ_0x0fc
    }

    // State 0, no crossed-segment flag set at all -- no-op.
    public function test_state0_idle(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 0);
        $this->initBusState(0, 0, 0.0);

        $this->call('_BusStopUpdateArrival_8c02ce48')->with();
    }

    // State 0, high byte of field_0x3b4 matches the upcoming stop segment --
    // arms the approach (state 2).
    public function test_state0_arms_approach(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 0);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 7);
        $this->initUint32($this->addressOf('_var_prevStopSegment_8c22870c'), 6);
        $this->initBusState(0, 7 << 8, 0.0);

        $this->call('_BusStopUpdateArrival_8c02ce48')->with();

        $this->shouldWriteLongTo('_var_stopPhase_8c2285e4', 2);
        $this->shouldWriteFloat($this->addressOf('_var_stopMinDistance_8c2285ec'), 9999.0);
        $this->shouldWriteLongTo('_var_8c226454', 0);
        $this->shouldCall('_pickWaitingPassengers_8c02c8ae')->with();
        $this->shouldWriteFloat($this->addressOf('_var_8c1bc44c'), 0.0);
    }

    // State 0, high byte of field_0x3b4 matches the previous stop segment --
    // arms the post-departure wait (state 1).
    public function test_state0_arms_wait(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 0);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 7);
        $this->initUint32($this->addressOf('_var_prevStopSegment_8c22870c'), 6);
        $this->initBusState(0, 6 << 8, 0.0);

        $this->call('_BusStopUpdateArrival_8c02ce48')->with();

        $this->shouldWriteLongTo('_var_stopPhase_8c2285e4', 1);
        $this->shouldWriteLongTo('_var_8c226454', 0);
    }

    // State 0, high byte set but matches neither segment -- no-op.
    public function test_state0_no_match(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 0);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 7);
        $this->initUint32($this->addressOf('_var_prevStopSegment_8c22870c'), 6);
        $this->initBusState(0, 9 << 8, 0.0);

        $this->call('_BusStopUpdateArrival_8c02ce48')->with();
    }

    // State 1, low byte still clear -- no-op.
    public function test_state1_waiting(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 1);
        $this->initBusState(0, 0, 0.0);

        $this->call('_BusStopUpdateArrival_8c02ce48')->with();
    }

    // State 1, low byte set, var_8c226450 armed (!= -1) -- flags var_8c228640.
    public function test_state1_departs_armed(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 1);
        $this->initUint32($this->addressOf('_var_8c226450'), 3);
        $this->initBusState(0, 1, 0.0);

        $this->call('_BusStopUpdateArrival_8c02ce48')->with();

        $this->shouldWriteLongTo('_var_stopPhase_8c2285e4', 0);
        $this->shouldWriteLongTo('_var_8c228640', 1);
        $this->shouldWriteLongTo('_var_8c2264c4', 0);
        $this->shouldCall('_advanceStopSegment_8c02ccae')->with();
    }

    // State 1, low byte set, var_8c226450 unarmed (-1) -- var_8c228640 untouched.
    public function test_state1_departs_unarmed(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 1);
        $this->initUint32($this->addressOf('_var_8c226450'), -1);
        $this->initBusState(0, 1, 0.0);

        $this->call('_BusStopUpdateArrival_8c02ce48')->with();

        $this->shouldWriteLongTo('_var_stopPhase_8c2285e4', 0);
        $this->shouldWriteLongTo('_var_8c2264c4', 0);
        $this->shouldCall('_advanceStopSegment_8c02ccae')->with();
    }

    // State 2, still far -- ticks the anim frame, pushes the draw callback,
    // updates the running minimum, no state transition.
    public function test_state2_approaching(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 2);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 7);
        $this->initUint32($this->addressOf('_var_8c1bc44c'), unpack('L', pack('f', 2.0))[1]);
        $this->initUint32($this->addressOf('_var_8c1bc450'), unpack('L', pack('f', 10.0))[1]);
        $this->initUint32($this->addressOf('_var_stopMinDistance_8c2285ec'), unpack('L', pack('f', 50.0))[1]);
        $this->initBusState(0, 0, 1.0); // moving -- distance conditions can't fire

        $this->seedStopOffset(20.0, 0.0);
        $this->call('_BusStopUpdateArrival_8c02ce48')->with();

        $this->shouldCall('_njSqrt')->with(400.0)->andReturn(20.0);
        $this->shouldWriteFloat($this->addressOf('_var_8c1bc44c'), 3.0);
        $this->shouldCall('_FadeCmdPushCall1_8c0223ea')->with(0, $this->addressOf('_drawStopMarker_8c02cd92'), 0);
        $this->shouldWriteFloat($this->addressOf('_var_stopMinDistance_8c2285ec'), 20.0);
    }

    // State 2, anim frame wraps back to 0 once it reaches the loaded
    // motion's frame limit (var_8c1bc450).
    public function test_state2_anim_wraps(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 2);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 7);
        $this->initUint32($this->addressOf('_var_8c1bc44c'), unpack('L', pack('f', 9.0))[1]);
        $this->initUint32($this->addressOf('_var_8c1bc450'), unpack('L', pack('f', 10.0))[1]);
        $this->initUint32($this->addressOf('_var_stopMinDistance_8c2285ec'), unpack('L', pack('f', 50.0))[1]);
        $this->initBusState(0, 0, 1.0);

        $this->seedStopOffset(20.0, 0.0);
        $this->call('_BusStopUpdateArrival_8c02ce48')->with();

        $this->shouldCall('_njSqrt')->with(400.0)->andReturn(20.0);
        $this->shouldWriteFloat($this->addressOf('_var_8c1bc44c'), 10.0);
        $this->shouldWriteFloat($this->addressOf('_var_8c1bc44c'), 0.0);
        $this->shouldCall('_FadeCmdPushCall1_8c0223ea')->with(0, $this->addressOf('_drawStopMarker_8c02cd92'), 0);
        $this->shouldWriteFloat($this->addressOf('_var_stopMinDistance_8c2285ec'), 20.0);
    }

    // State 2, close enough and stopped -- transitions to "stopped" (bus
    // state 3), via the direct-distance side of the OR.
    public function test_state2_arrives_stopped_close(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 2);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 7);
        $this->initUint32($this->addressOf('_var_8c1bc44c'), unpack('L', pack('f', 0.0))[1]);
        $this->initUint32($this->addressOf('_var_8c1bc450'), unpack('L', pack('f', 10.0))[1]);
        $this->initUint32($this->addressOf('_var_stopMinDistance_8c2285ec'), unpack('L', pack('f', 50.0))[1]);
        $busBase = $this->initBusState(0, 0, 0.0); // stopped

        $this->seedStopOffset(2.0, 0.0);
        $this->call('_BusStopUpdateArrival_8c02ce48')->with();

        $this->shouldCall('_njSqrt')->with(4.0)->andReturn(2.0); // < 3.0
        $this->shouldWriteFloat($this->addressOf('_var_8c1bc44c'), 1.0);
        $this->shouldCall('_FadeCmdPushCall1_8c0223ea')->with(0, $this->addressOf('_drawStopMarker_8c02cd92'), 0);
        $this->shouldWriteFloat($this->addressOf('_var_stopMinDistance_8c2285ec'), 2.0);
        $this->shouldWriteLong($busBase + 0x2b4, 3); // bus_state_0x2b4
        $this->shouldWriteLongTo('_var_stopPhase_8c2285e4', 3);
        $this->shouldWriteLongTo('_var_8c2285c4', 3);
        $this->shouldWriteLongTo('_var_8c2285e8', 0);
    }

    // State 2, still far by direct distance but the running minimum has
    // already dropped below the threshold -- other side of the OR.
    public function test_state2_arrives_stopped_via_minimum(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 2);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 7);
        $this->initUint32($this->addressOf('_var_8c1bc44c'), unpack('L', pack('f', 0.0))[1]);
        $this->initUint32($this->addressOf('_var_8c1bc450'), unpack('L', pack('f', 10.0))[1]);
        $this->initUint32($this->addressOf('_var_stopMinDistance_8c2285ec'), unpack('L', pack('f', 2.0))[1]);
        $busBase = $this->initBusState(0, 0, 0.0); // stopped

        $this->seedStopOffset(10.0, 0.0);
        $this->call('_BusStopUpdateArrival_8c02ce48')->with();

        $this->shouldCall('_njSqrt')->with(100.0)->andReturn(10.0); // not < 3.0
        $this->shouldWriteFloat($this->addressOf('_var_8c1bc44c'), 1.0);
        $this->shouldCall('_FadeCmdPushCall1_8c0223ea')->with(0, $this->addressOf('_drawStopMarker_8c02cd92'), 0);
        // 10.0 is not < running minimum 2.0, so no minimum update.
        $this->shouldWriteLong($busBase + 0x2b4, 3);
        $this->shouldWriteLongTo('_var_stopPhase_8c2285e4', 3);
        $this->shouldWriteLongTo('_var_8c2285c4', 3);
        $this->shouldWriteLongTo('_var_8c2285e8', 0);
    }

    // State 2, the bus reaches the stop segment outright while still moving
    // -- transitions via bus state 4 instead.
    public function test_state2_reaches_segment_while_moving(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 2);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 7);
        $this->initUint32($this->addressOf('_var_8c1bc44c'), unpack('L', pack('f', 0.0))[1]);
        $this->initUint32($this->addressOf('_var_8c1bc450'), unpack('L', pack('f', 10.0))[1]);
        $this->initUint32($this->addressOf('_var_stopMinDistance_8c2285ec'), unpack('L', pack('f', 50.0))[1]);
        $busBase = $this->initBusState(0, 7, 5.0); // moving, field_0x3b4 low byte == nextStopSegment

        $this->seedStopOffset(20.0, 0.0);
        $this->call('_BusStopUpdateArrival_8c02ce48')->with();

        $this->shouldCall('_njSqrt')->with(400.0)->andReturn(20.0);
        $this->shouldWriteFloat($this->addressOf('_var_8c1bc44c'), 1.0);
        $this->shouldCall('_FadeCmdPushCall1_8c0223ea')->with(0, $this->addressOf('_drawStopMarker_8c02cd92'), 0);
        $this->shouldWriteFloat($this->addressOf('_var_stopMinDistance_8c2285ec'), 20.0);
        $this->shouldWriteLong($busBase + 0x2b4, 4); // bus_state_0x2b4
        $this->shouldWriteLongTo('_var_stopPhase_8c2285e4', 3);
        $this->shouldWriteLongTo('_var_8c2285c4', 3);
        $this->shouldWriteLongTo('_var_8c2285e8', 2);
    }

    // State 3 -- fully idle, waits for something external.
    public function test_state3_idle(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 3);
        $this->initBusState(0xff000000, 0xff, 0.0);

        $this->call('_BusStopUpdateArrival_8c02ce48')->with();
    }

    // State 4, finish bit not yet set -- no-op.
    public function test_state4_waiting(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 4);
        $this->initBusState(0, 0, 0.0);

        $this->call('_BusStopUpdateArrival_8c02ce48')->with();
    }

    // State 4, finish bit set, not enough driver points -- skips the extra
    // FUN_8c02c586 side effect.
    public function test_state4_finishes_no_bonus(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 4);
        $this->initUint32($this->addressOf('_var_driverPoints_8c2285d0'), 0);
        $busBase = $this->initBusState(0xff000000, 0, 0.0);

        $this->call('_BusStopUpdateArrival_8c02ce48')->with();

        $this->shouldWriteLong($busBase + 0x2b4, 4); // bus_state_0x2b4
        $this->shouldWriteLongTo('_var_stopPhase_8c2285e4', 3);
        $this->shouldWriteLongTo('_var_8c2285e8', 0);
        $this->shouldWriteLongTo('_var_8c2285c4', 4);
        $this->shouldWriteLongTo('_var_8c2285cc', 0x1e);
        $this->shouldWriteLongTo('_var_fadeCompleteCallback_8c22656c', $this->addressOf('_FUN_8c02c784'));
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(0);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(1);
    }

    // State 4, finish bit set, enough driver points and FUN_8c02c586 signals
    // -- also sets var_8c2285c8.
    public function test_state4_finishes_with_bonus(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_stopPhase_8c2285e4'), 4);
        $this->initUint32($this->addressOf('_var_driverPoints_8c2285d0'), 1);
        $busBase = $this->initBusState(0xff000000, 0, 0.0);

        $this->call('_BusStopUpdateArrival_8c02ce48')->with();

        $this->shouldWriteLong($busBase + 0x2b4, 4);
        $this->shouldWriteLongTo('_var_stopPhase_8c2285e4', 3);
        $this->shouldWriteLongTo('_var_8c2285e8', 0);
        $this->shouldWriteLongTo('_var_8c2285c4', 4);
        $this->shouldWriteLongTo('_var_8c2285cc', 0x1e);
        $this->shouldWriteLongTo('_var_fadeCompleteCallback_8c22656c', $this->addressOf('_FUN_8c02c784'));
        $this->shouldCall('_FUN_8c02c586')->with()->andReturn(1);
        $this->shouldWriteLongTo('_var_8c2285c8', 1);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(0);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(1);
    }
};
