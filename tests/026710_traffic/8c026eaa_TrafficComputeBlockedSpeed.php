<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

// TrafficComputeBlockedSpeed_8c026eaa scans the global list of traffic
// entries starting at "other" (advanced via TrafficPathScanNext_8c02f212, a no-argument
// accessor for some external iteration cursor), looking for one that
// constrains "entry"'s speed. var_8c1bbd9c is a global pointer variable
// whose stored value is a sentinel standing in for the player's bus in this
// list; when "other" equals that stored value, the scan always ends there
// (no further TrafficPathScanNext_8c02f212() call), reading the bus's own fields directly
// instead of a normal entry struct's. Otherwise, an "other" entry at or
// behind "entry" on the path (own 0x2c0 <= entry's 0x2c0) only refreshes its
// own 0x418 field -- computed raw, then clamped to 0.0 with a second store
// if negative -- and does not affect the return value; one strictly ahead
// (own 0x2c0 > entry's 0x2c0) sets the returned speed limit itself (also
// clamped to 0.0, but kept only in a register -- no second store), and the
// scan continues to the next entry. Default when nothing constrains it is
// 9999.0 (no limit).
return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_TrafficPathScanNext_8c02f212', 4);
        // Pin the bus sentinel variable to its own address so a test's
        // allocated "other" entry can never coincidentally match its
        // (uninitialized, randomized) stored value.
        $this->setSize('_var_8c1bbd9c', 4);
        $this->initUint32($this->addressOf('_var_8c1bbd9c'), 0);
        // The compiler schedules the bus-state loads used by the sentinel
        // branch alongside the rest of the loop body's reads, ahead of the
        // branch itself that actually gates using them -- so every test that
        // enters the loop body at all needs this resolved, not just the ones
        // that take the sentinel branch.
        $this->setSize('_var_busState_8c1bb9d0', 0x3c8);
    }

    private function allocEntry(float $projDist, float $speedMargin): int {
        $entry = $this->alloc(0x420);
        $this->initUint32($entry + 0x2c0, fdec($projDist));
        $this->initUint32($entry + 0x27c, fdec($speedMargin));
        $this->initUint32($entry + 0x418, fdec(9999.0)); // only meaningful when this entry is "other" and behind/equal
        return $entry;
    }

    // "other" is NULL from the start: nothing to scan, default returned, no calls.
    public function test_emptyListReturnsDefault(): void {
        $this->resolveSymbols();
        $entry = $this->allocEntry(0.0, 0.0);

        $this->call('_TrafficComputeBlockedSpeed_8c026eaa')->with($entry, 0);

        $this->shouldReturn(9999.0);
    }

    // "other" is at or behind "entry" on the path (own 0x2c0 <= entry's
    // 0x2c0): only other's own 0x418 field is refreshed. The returned speed
    // limit must stay at the default -- this is the case a broken
    // implementation that folds this branch's value into the return would
    // get wrong.
    public function test_otherBehindOrEqualUpdatesFieldNotReturnValue(): void {
        $this->resolveSymbols();
        $entry = $this->allocEntry(10.0, 2.0);
        $other = $this->allocEntry(10.0, 0.0); // other->0x2c0 (10.0) <= entry->0x2c0 (10.0)

        $this->call('_TrafficComputeBlockedSpeed_8c026eaa')->with($entry, $other);

        // other->0x418 = entry->0x27c (2.0) + margin(-0.18518518) = 1.81481482, not negative.
        $this->shouldWriteFloat($other + 0x418, 1.81481482);

        $this->shouldCall('_TrafficPathScanNext_8c02f212')->andReturn(0);

        $this->shouldReturn(9999.0);
    }

    // Same branch, but the margin-adjusted value goes negative: it is first
    // stored raw, then immediately overwritten with 0.0 -- two writes to the
    // same address, in order.
    public function test_otherBehindOrEqualClampsFieldToZero(): void {
        $this->resolveSymbols();
        $entry = $this->allocEntry(10.0, 0.1); // 0.1 + margin < 0
        $other = $this->allocEntry(10.0, 0.0);

        $this->call('_TrafficComputeBlockedSpeed_8c026eaa')->with($entry, $other);

        $this->shouldWriteFloat($other + 0x418, 0.1 - 0.18518518);
        $this->shouldWriteFloat($other + 0x418, 0.0);

        $this->shouldCall('_TrafficPathScanNext_8c02f212')->andReturn(0);

        $this->shouldReturn(9999.0);
    }

    // "other" is strictly ahead of "entry" on the path (own 0x2c0 > entry's
    // 0x2c0): the returned speed limit is set from other's own speed
    // margin, kept in a register only (no memory write).
    public function test_otherStrictlyAheadLowersReturnValue(): void {
        $this->resolveSymbols();
        $entry = $this->allocEntry(10.0, 0.0);
        $other = $this->allocEntry(20.0, 1.0); // other->0x2c0 (20.0) > entry->0x2c0 (10.0)

        $this->call('_TrafficComputeBlockedSpeed_8c026eaa')->with($entry, $other);

        $this->shouldCall('_TrafficPathScanNext_8c02f212')->andReturn(0);

        // candidate = 1.0 + margin = 0.81481482, not negative.
        $this->shouldReturn(0.81481482);
    }

    // Same branch, negative candidate: clamped to 0.0 in the register-held
    // return value (no memory write either way).
    public function test_otherStrictlyAheadClampsReturnValueToZero(): void {
        $this->resolveSymbols();
        $entry = $this->allocEntry(10.0, 0.0);
        $other = $this->allocEntry(20.0, 0.1); // candidate = 0.1 + margin < 0

        $this->call('_TrafficComputeBlockedSpeed_8c026eaa')->with($entry, $other);

        $this->shouldCall('_TrafficPathScanNext_8c02f212')->andReturn(0);

        $this->shouldReturn(0.0);
    }

    // Two entries in the chain: the first is at/behind (field-only effect),
    // the second is strictly ahead and sets the final returned speed. Proves
    // the scan actually advances past the first entry via TrafficPathScanNext_8c02f212().
    public function test_scanContinuesPastBehindEntryToAheadEntry(): void {
        $this->resolveSymbols();
        $entry = $this->allocEntry(10.0, 2.0);
        $first = $this->allocEntry(10.0, 0.0);  // at/behind: 10.0 <= entry's 10.0
        $second = $this->allocEntry(20.0, 0.5); // ahead: 20.0 > entry's 10.0

        $this->call('_TrafficComputeBlockedSpeed_8c026eaa')->with($entry, $first);

        $this->shouldWriteFloat($first + 0x418, 1.81481482); // entry->0x27c (2.0) + margin

        $this->shouldCall('_TrafficPathScanNext_8c02f212')->andReturn($second);
        $this->shouldCall('_TrafficPathScanNext_8c02f212')->andReturn(0);

        // second->0x27c (0.5) + margin = 0.31481482
        $this->shouldReturn(0.31481482);
    }

    // Allocates a bus-shaped blob (own 0xf4/0xfc current, 0x100/0x108
    // target) and points the global var_8c1bbd9c pointer variable at it, so
    // the scan recognizes it as the bus sentinel.
    private function allocBus(float $curX, float $curY, float $tgtX, float $tgtY): int {
        $bus = $this->alloc(0x140);
        $this->initUint32($bus + 0xf4, fdec($curX));
        $this->initUint32($bus + 0xfc, fdec($curY));
        $this->initUint32($bus + 0x100, fdec($tgtX));
        $this->initUint32($bus + 0x108, fdec($tgtY));
        $this->initUint32($this->addressOf('_var_8c1bbd9c'), $bus);
        return $bus;
    }

    // Hitting the bus sentinel always ends the scan immediately (no further
    // TrafficPathScanNext_8c02f212() call). When the computed dot product is >= 0.0, the
    // default speed limit is kept untouched.
    public function test_busSentinelDotNonNegativeKeepsDefault(): void {
        $this->resolveSymbols();
        $entry = $this->allocEntry(10.0, 2.0);
        $bus = $this->allocBus(0.0, 0.0, 10.0, 0.0);

        $busStateAddr = $this->addressOf('_var_busState_8c1bb9d0');
        // dot = (target_x - posX)*(target_x - cur_x) + (target_y - cur_y)*(target_y - posZ)
        //     = (10 - 5)*(10 - 0) + (0 - 0)*(0 - posZ) = 50 >= 0.
        $this->initUint32($busStateAddr + 0xf4, fdec(5.0));  // posX_0x0f4
        $this->initUint32($busStateAddr + 0xfc, fdec(0.0));  // posZ_0x0fc
        $this->initUint32($busStateAddr + 0x27c, fdec(1.0)); // speed_0x27c (unread on this path)

        $this->call('_TrafficComputeBlockedSpeed_8c026eaa')->with($entry, $bus);

        $this->shouldReturn(9999.0);
    }

    // dot < 0.0: the bus's own speed margin becomes the returned speed
    // limit, replacing the default.
    public function test_busSentinelDotNegativeLowersReturnValue(): void {
        $this->resolveSymbols();
        $entry = $this->allocEntry(10.0, 2.0);
        $bus = $this->allocBus(0.0, 0.0, 10.0, 0.0);

        $busStateAddr = $this->addressOf('_var_busState_8c1bb9d0');
        // dot = (10-20)*(10-0) + (0-0)*(0-0) = -100 < 0.
        $this->initUint32($busStateAddr + 0xf4, fdec(20.0)); // posX_0x0f4
        $this->initUint32($busStateAddr + 0xfc, fdec(0.0));  // posZ_0x0fc
        $this->initUint32($busStateAddr + 0x27c, fdec(1.0)); // candidate = 1.0 + margin = 0.81481482

        $this->call('_TrafficComputeBlockedSpeed_8c026eaa')->with($entry, $bus);

        $this->shouldReturn(0.81481482);
    }

    // Same branch, negative candidate: clamped to 0.0, same as the other
    // two branches.
    public function test_busSentinelDotNegativeClampsReturnValueToZero(): void {
        $this->resolveSymbols();
        $entry = $this->allocEntry(10.0, 2.0);
        $bus = $this->allocBus(0.0, 0.0, 10.0, 0.0);

        $busStateAddr = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($busStateAddr + 0xf4, fdec(20.0)); // posX_0x0f4, dot < 0 as above
        $this->initUint32($busStateAddr + 0xfc, fdec(0.0));  // posZ_0x0fc
        $this->initUint32($busStateAddr + 0x27c, fdec(0.1)); // candidate = 0.1 + margin < 0

        $this->call('_TrafficComputeBlockedSpeed_8c026eaa')->with($entry, $bus);

        $this->shouldReturn(0.0);
    }
};
