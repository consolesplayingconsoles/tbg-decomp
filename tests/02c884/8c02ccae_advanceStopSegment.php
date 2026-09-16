<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _advanceStopSegment_8c02ccae(void): advances var_prevStopSegment_8c22870c
 * to the just-armed stop and reloads var_runState_8c2285c4.scheduleTime_0x14 (the EventPickForSegment
 * gate) from the course's per-segment gate table (courseConfig's ukn_0x0c).
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_prevStopSegment_8c22870c', 4);
        $this->setSize('_var_runState_8c2285c4', 0x9c);
        $this->setSize('_var_currentCourseConfig_8c18ad18', 4);
    }

    public function test_advances_segment_and_loads_gate_value(): void
    {
        $this->resolveSymbols();

        // Gate table with distinct values per index so a wrong index is
        // caught: [0]=111, [1]=222, [2]=333, [3]=444.
        $table = $this->alloc(4 * 4);
        $this->initUint32($table + 0 * 4, 111);
        $this->initUint32($table + 1 * 4, 222);
        $this->initUint32($table + 2 * 4, 333);
        $this->initUint32($table + 3 * 4, 444);

        // courseConfig is a struct; only ukn_0x0c (the gate-table pointer) is read.
        $config = $this->alloc(0x10);
        $this->initUint32($config + 0x0c, $table);
        $this->initUint32($this->addressOf('_var_currentCourseConfig_8c18ad18'), $config);

        $this->initUint32($this->addressOf('_var_prevStopSegment_8c22870c'), 1);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x14, 0xdeadbeef);

        $this->call('_advanceStopSegment_8c02ccae')->with();

        $this->shouldWriteLongTo('_var_prevStopSegment_8c22870c', 2);
        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x14, 333);
    }
};
