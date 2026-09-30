<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// TrafficLookaheadScan_8c02dfca(self, entry, lookahead): tops
// entry->lookaheadPoints_0x49c back up to 20.0 units ahead if it fell short
// (entry->lookaheadCacheLen_0x4ec < 20.0 -- same path-walk as TrafficLookaheadInit_8c02df3c,
// but first shifting the still-valid tail of the old cache down to slot 0),
// then scans the cache for a candidate within lookahead+5.0 units: the
// player bus's own recent-history/current position (var_busState_8c1bb9d0),
// or another live traffic entry's front-reference (rearPointX_0x10c/rearPointZ_0x114) or
// current (posX_0xf4/posZ_0xfc) position, via the shared
// var_collisionScanCursor_8c228974/var_tasks_8c1bac28 scan (self excluded).
// Returns var_playerBus_8c1bbd9c (player), the matched entry, or NULL.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_busState_8c1bb9d0', 0x400);
        $this->setSize('_var_playerBus_8c1bbd9c', 4);
        $this->setSize('_var_tasks_8c1bac28', 4 * 0x20);
    }

    private function initFloat(int $addr, float $value): void {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    private function makeEntry(): int {
        return $this->alloc(0x514);
    }

    private function makeSeg(float $length, float $x, float $z, float $dirX, float $dirZ): int {
        $seg = $this->alloc(0x14 * 2);
        $this->initFloat($seg + 0x00, $length);
        $this->initFloat($seg + 0x04, $x);
        $this->initFloat($seg + 0x08, $z);
        $this->initFloat($seg + 0x0c, $dirX);
        $this->initFloat($seg + 0x10, $dirZ);
        $this->initFloat($seg + 0x14, 0.0); // terminator
        return $seg;
    }

    private function makeTask(int $index, int $action, int $state): int {
        $task = $this->addressOf('_var_tasks_8c1bac28') + $index * 0x20;
        $this->initUint32($task + 0x00, $action);
        $this->initUint32($task + 0x04, $state);
        return $task;
    }

    // Sends every busState field far away so no accidental match occurs.
    private function farBusState(): void {
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        foreach ([0x0f4, 0x0fc, 0x100, 0x108, 0x160, 0x168, 0x16c, 0x174, 0x178, 0x180, 0x184, 0x18c] as $off) {
            $this->initFloat($base + $off, -9999.0);
        }
    }

    private function setCachePoint(int $entry, int $slot, float $x, float $z): void {
        $this->initFloat($entry + 0x49c + $slot * 8, $x);
        $this->initFloat($entry + 0x49c + $slot * 8 + 4, $z);
    }

    // Cache already covers >= 20.0 units (no refill) and its first slot is
    // already the 9999.0 terminator: the scan reads it, finds the sentinel,
    // and returns NULL without touching the player or the task list at all.
    public function test_refillSkipped_emptyCache_returnsNull(): void {
        $this->resolveSymbols();

        $entry = $this->makeEntry();
        $this->initFloat($entry + 0x4ec, 20.0);
        $this->setCachePoint($entry, 0, 9999.0, 0.0);

        $this->call('_TrafficLookaheadScan_8c02dfca')->with(
            $this->alloc(4), $entry, 10.0
        );

        $this->shouldReturn(0);
    }

    // First cache point lands within 2.0 of the player bus's
    // posHistory_0x100[10] (a recent-history sample, not the live position):
    // returns var_playerBus_8c1bbd9c straight away.
    public function test_refillSkipped_matchesBusHistoryPoint_returnsBusSentinel(): void {
        $this->resolveSymbols();

        $entry = $this->makeEntry();
        $this->initFloat($entry + 0x4ec, 20.0);
        $this->farBusState();
        $this->initFloat($this->addressOf('_var_busState_8c1bb9d0') + 0x178, 100.0);
        $this->initFloat($this->addressOf('_var_busState_8c1bb9d0') + 0x180, 200.0);
        $this->setCachePoint($entry, 0, 100.5, 200.5);
        $this->makeTask(0, 0, 0); // terminator, unused on this path

        $bus = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), $bus);

        $this->call('_TrafficLookaheadScan_8c02dfca')->with(
            $this->alloc(4), $entry, 10.0
        );

        $this->shouldReturn($bus);
    }

    // First cache point lands within 2.5 of the player bus's live position
    // (posX_0x0f4/posZ_0x0fc) -- the "break" branch, distinct in the asm/C
    // from the direct-return history-point checks above but observably the
    // same result: var_playerBus_8c1bbd9c.
    public function test_refillSkipped_matchesBusLivePosition_returnsBusSentinel(): void {
        $this->resolveSymbols();

        $entry = $this->makeEntry();
        $this->initFloat($entry + 0x4ec, 20.0);
        $this->farBusState();
        $this->initFloat($this->addressOf('_var_busState_8c1bb9d0') + 0x0f4, 100.0);
        $this->initFloat($this->addressOf('_var_busState_8c1bb9d0') + 0x0fc, 200.0);
        $this->setCachePoint($entry, 0, 100.5, 200.5);
        $this->makeTask(0, 0, 0);

        $bus = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), $bus);

        $this->call('_TrafficLookaheadScan_8c02dfca')->with(
            $this->alloc(4), $entry, 10.0
        );

        $this->shouldReturn($bus);
    }

    // No player match; scans the traffic task list instead. The list holds
    // self (skipped by identity) and the -1 sentinel (skipped by action)
    // before a live candidate whose posX_0xf4/posZ_0xfc matches the point.
    public function test_refillSkipped_matchesTrafficEntry_returnsItsState(): void {
        $this->resolveSymbols();

        $entry = $this->makeEntry();
        $this->initFloat($entry + 0x4ec, 20.0);
        $this->farBusState();
        $this->setCachePoint($entry, 0, 100.0, 200.0);

        $tasksBase = $this->addressOf('_var_tasks_8c1bac28');
        $self = $this->makeTask(0, 1, 0xdeadbeef);
        $this->makeTask(1, -1, 0xdeadbeef); // sentinel, skipped
        $candidateEntry = $this->alloc(0x120);
        $this->initFloat($candidateEntry + 0xf4, 100.0);
        $this->initFloat($candidateEntry + 0xfc, 200.0);
        $this->initFloat($candidateEntry + 0x10c, -9999.0);
        $this->initFloat($candidateEntry + 0x114, -9999.0);
        $this->makeTask(2, 1, $candidateEntry);
        $this->makeTask(3, 0, 0);

        $this->call('_TrafficLookaheadScan_8c02dfca')->with(
            $self, $entry, 10.0
        );

        $this->shouldWriteLong($this->addressOf('_var_collisionScanCursor_8c228974'), $tasksBase);
        $this->shouldWriteLong($this->addressOf('_var_collisionScanCursor_8c228974'), $tasksBase + 0x20);
        $this->shouldWriteLong($this->addressOf('_var_collisionScanCursor_8c228974'), $tasksBase + 0x40);
        $this->shouldReturn($candidateEntry);
    }

    // Refill needed (lookaheadCacheLen_0x4ec == 0.0) and the path cursor is already the
    // -1 "exhausted" sentinel: the shift loop is a no-op (nothing cached
    // yet), the refill's own inner loop ends immediately without writing
    // anything (only a register-local bump, no memory store -- matches
    // TrafficLookaheadInit_8c02df3c's non-writing sentinel path), and since
    // no point was ever appended the function returns NULL immediately,
    // before even reaching the player/task scan.
    public function test_refillNeeded_sentinelCursor_emptyCache_returnsNull(): void {
        $this->resolveSymbols();

        $entry = $this->makeEntry();
        $this->initFloat($entry + 0x4ec, 0.0);
        $this->initUint32($entry + 0x4f4, 0xffffffff);
        $this->initFloat($entry + 0x4fc, 3.0);

        $this->call('_TrafficLookaheadScan_8c02dfca')->with(
            $this->alloc(4), $entry, 10.0
        );

        $this->shouldReturn(0);
    }

    // Refill needed: lookaheadCacheLen_0x4ec == 15.0 means 3 cached pairs already exist
    // (slots 0-2) plus a 9999.0 terminator at slot 3. The shift loop drops
    // slot 0 and copies slots 1-3 down to 0-2 (consuming the old
    // terminator into slot 2), then the walk appends exactly one fresh
    // point (15.0 -> 20.0) at slot 3 and writes a new terminator at slot 4.
    // The scan afterwards reads slots 0 and 1 (no match, empty task list
    // each time), then hits the shifted-in terminator at slot 2 and
    // returns NULL without ever reaching the freshly appended point.
    public function test_refillNeeded_shiftsAndAppendsOnePoint(): void {
        $this->resolveSymbols();

        $seg = $this->makeSeg(length: 1000.0, x: 0.0, z: 0.0, dirX: 1.0, dirZ: 0.0);
        $entry = $this->makeEntry();
        $this->initFloat($entry + 0x4ec, 15.0);
        $this->initUint32($entry + 0x4f4, $seg);
        $this->initFloat($entry + 0x4fc, 0.0);
        $this->setCachePoint($entry, 0, 1.0, 1.0); // dropped
        $this->setCachePoint($entry, 1, 2.0, 2.0); // -> slot 0
        $this->setCachePoint($entry, 2, 3.0, 3.0); // -> slot 1
        $this->setCachePoint($entry, 3, 9999.0, 5.0); // old terminator -> slot 2
        $this->farBusState();

        $tasksBase = $this->addressOf('_var_tasks_8c1bac28');
        $this->makeTask(0, 0, 0); // terminator

        $out = $entry + 0x49c;

        $this->call('_TrafficLookaheadScan_8c02dfca')->with(
            $this->alloc(4), $entry, 1.0
        );

        // shift loop
        $this->shouldWriteFloat($out + 0x00, 2.0);
        $this->shouldWriteFloat($out + 0x04, 2.0);
        $this->shouldWriteFloat($out + 0x08, 3.0);
        $this->shouldWriteFloat($out + 0x0c, 3.0);
        $this->shouldWriteFloat($out + 0x10, 9999.0);
        $this->shouldWriteFloat($out + 0x14, 5.0);

        // append one fresh point (dist == 0.0 on the never-exhausted segment).
        // lookaheadCacheLen_0x4ec's running total stays a register the whole walk (unlike
        // TrafficLookaheadInit_8c02df3c's per-iteration store) -- it's only
        // written once, below, alongside the terminator/cursor/distance.
        $this->shouldWriteFloat($out + 0x18, 0.0);
        $this->shouldWriteFloat($out + 0x1c, 0.0);

        // new terminator + cursor/distance/lookaheadCacheLen_0x4ec
        $this->shouldWriteFloat($out + 0x20, 9999.0);
        $this->shouldWriteLong($entry + 0x4f4, $seg);
        $this->shouldWriteFloat($entry + 0x4fc, 5.0);
        $this->shouldWriteFloat($entry + 0x4ec, 20.0);

        // scan: slot 0 (2.0, 2.0) -- no player match, empty task list
        $this->shouldWriteLong($this->addressOf('_var_collisionScanCursor_8c228974'), $tasksBase);
        // scan: slot 1 (3.0, 3.0) -- no player match, empty task list
        $this->shouldWriteLong($this->addressOf('_var_collisionScanCursor_8c228974'), $tasksBase);

        $this->shouldReturn(0);
    }

    // lookahead is negative enough that lookahead + 5.0 <= 0.0 from the very
    // first check: the terminal branch reads slot 0 in place (no advance,
    // no task-list scan) and matches the player's posHistory_0x100[0]
    // (its "index 0" history point).
    public function test_terminalCheck_matchesBusHistoryIndexZero_returnsBusSentinel(): void {
        $this->resolveSymbols();

        $entry = $this->makeEntry();
        $this->initFloat($entry + 0x4ec, 20.0);
        $this->farBusState();
        $this->initFloat($this->addressOf('_var_busState_8c1bb9d0') + 0x100, 100.0);
        $this->initFloat($this->addressOf('_var_busState_8c1bb9d0') + 0x108, 200.0);
        $this->setCachePoint($entry, 0, 100.5, 200.5);

        $bus = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), $bus);

        $this->call('_TrafficLookaheadScan_8c02dfca')->with(
            $this->alloc(4), $entry, -10.0
        );

        $this->shouldReturn($bus);
    }
};
