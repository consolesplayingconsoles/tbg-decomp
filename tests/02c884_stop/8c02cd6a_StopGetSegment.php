<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _StopGetSegment_8c02cd6a(int segmentIndex): returns a pointer to the
 * course's segments_0x08[segmentIndex] record (0x2c bytes each).
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_currentCourseConfig_8c18ad18', 4);
    }

    public function test_returns_indexed_segment_pointer(): void
    {
        $this->resolveSymbols();

        // courseConfig is a struct; only segments_0x08 (offset 8) is read.
        $segments = $this->alloc(0x2c * 4);
        $config = $this->alloc(0x10);
        $this->initUint32($config + 0x08, $segments);
        $this->initUint32($this->addressOf('_var_currentCourseConfig_8c18ad18'), $config);

        $this->call('_StopGetSegment_8c02cd6a')->with(3);

        $this->shouldReturn($segments + 3 * 0x2c);
    }
};
