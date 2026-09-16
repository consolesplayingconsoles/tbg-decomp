<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _BusStopGetStopArea_8c02cd7a(int segmentIndex): looks up the segment
 * record (BusStopGetSegment_8c02cd6a), then returns the StopAreaRecord*
 * stored at the course's lineBus_0x08 table[stopAreaId_0x02] (8-byte stride).
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_currentCourse_8c1bb868', 0x50);

        // _BusStopGetSegment_8c02cd6a is same-object -- mock with
        // shouldCall() directly, no setSize().
    }

    public function test_returns_table_entry_selected_by_stop_area_id(): void
    {
        $this->resolveSymbols();

        // Segment record: only stopAreaId_0x02 (offset 2) is read.
        $seg = $this->alloc(0x2c);
        $this->initUint16($seg + 2, 3);

        // Table of 8-byte slots {StopAreaRecord *, unknown}; slot 3 points
        // at a stop-area record. Slots 0-2 are initialized so a wrong index
        // would read a defined-but-different pointer, not garbage.
        $table = $this->alloc(4 * 8);
        for ($i = 0; $i < 4; $i++) {
            $this->initUint32($table + $i * 8, 0);
            $this->initUint32($table + $i * 8 + 4, 0);
        }
        $record = $this->alloc(0x14);
        $this->initUint32($table + 3 * 8, $record);

        $this->initUint32($this->addressOf('_var_currentCourse_8c1bb868') + 0x08, $table); // lineBus_0x08

        $this->call('_BusStopGetStopArea_8c02cd7a')->with(5);

        $this->shouldCall('_BusStopGetSegment_8c02cd6a')->with(5)->andReturn($seg);
        $this->shouldReturn($record);
    }
};
