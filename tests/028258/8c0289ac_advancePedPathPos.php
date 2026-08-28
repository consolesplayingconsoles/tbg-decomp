<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function f(float $value): int
    {
        return unpack('L', pack('f', $value))[1];
    }

    private function initFloat(int $address, float $value): void
    {
        $this->initUint32($address, $this->f($value));
    }

    public function test_forward_no_advance()
    {
        // Segment: {length, offX, offZ, dirX, dirZ, heading}
        $seg = $this->alloc(6 * 4);
        $this->initFloat($seg + 0x00, 100.0);
        $this->initFloat($seg + 0x04, 10.0);
        $this->initFloat($seg + 0x08, 20.0);
        $this->initFloat($seg + 0x0c, 1.0);
        $this->initFloat($seg + 0x10, 0.0);
        $this->initFloat($seg + 0x14, 5.0);

        $obj = $this->alloc(0x19 * 4);
        $this->initFloat($obj + 0x08 * 4, 1000.0); // param_1[8]
        $this->initFloat($obj + 0x09 * 4, 2000.0); // param_1[9]
        $this->initFloat($obj + 0x10 * 4, 0.0);    // direction flag: forward
        $this->initUint32($obj + 0x11 * 4, 0);     // path start ptr (unused this path)
        $this->initUint32($obj + 0x12 * 4, 0);     // path loop-back ptr (unused this path)
        $this->initUint32($obj + 0x13 * 4, $seg);  // current segment ptr
        $this->initFloat($obj + 0x15 * 4, 30.0);   // current progress along segment
        $this->initFloat($obj + 0x18 * 4, 0.0);    // segment heading cache

        $this->call('_advancePedPathPos_8c0289ac')->with($obj);

        $this->shouldWriteFloat($obj + 0x15 * 4, 30.0);
        $this->shouldWriteFloat($obj + 0x00 * 4, 30.0 * 1.0 + 10.0 + 1000.0);
        $this->shouldWriteFloat($obj + 0x02 * 4, 30.0 * 0.0 + 20.0 + 2000.0);

        $this->shouldReturn(0);
    }

    public function test_forward_advances_one_segment_no_wrap()
    {
        $seg1 = $this->alloc(6 * 4);
        $this->initFloat($seg1 + 0x00, 20.0);
        $this->initFloat($seg1 + 0x04, 10.0);
        $this->initFloat($seg1 + 0x08, 20.0);
        $this->initFloat($seg1 + 0x0c, 1.0);
        $this->initFloat($seg1 + 0x10, 0.0);
        $this->initFloat($seg1 + 0x14, 5.0);

        $seg2 = $this->alloc(6 * 4);
        $this->initFloat($seg2 + 0x00, 50.0);
        $this->initFloat($seg2 + 0x04, 30.0);
        $this->initFloat($seg2 + 0x08, 40.0);
        $this->initFloat($seg2 + 0x0c, 0.0);
        $this->initFloat($seg2 + 0x10, 1.0);
        $this->initFloat($seg2 + 0x14, 15.0);

        $obj = $this->alloc(0x19 * 4);
        $this->initFloat($obj + 0x08 * 4, 1000.0); // param_1[8]
        $this->initFloat($obj + 0x09 * 4, 2000.0); // param_1[9]
        $this->initFloat($obj + 0x10 * 4, 0.0);    // direction flag: forward
        $this->initUint32($obj + 0x11 * 4, 0);     // path start ptr (unused this path)
        $this->initUint32($obj + 0x12 * 4, 0);     // path loop-back ptr (unused this path)
        $this->initUint32($obj + 0x13 * 4, $seg1); // current segment ptr
        $this->initFloat($obj + 0x15 * 4, 25.0);   // current progress along segment
        $this->initFloat($obj + 0x18 * 4, 0.0);    // segment heading cache

        $this->call('_advancePedPathPos_8c0289ac')->with($obj);

        $this->shouldWriteLong($obj + 0x13 * 4, $seg2);
        $this->shouldWriteFloat($obj + 0x18 * 4, 15.0);
        $this->shouldWriteFloat($obj + 0x15 * 4, 5.0);
        $this->shouldWriteFloat($obj + 0x00 * 4, 5.0 * 0.0 + 30.0 + 1000.0);
        $this->shouldWriteFloat($obj + 0x02 * 4, 5.0 * 1.0 + 40.0 + 2000.0);

        $this->shouldReturn(1);
    }

    public function test_forward_wraps_to_path_start()
    {
        $seg1 = $this->alloc(6 * 4);
        $this->initFloat($seg1 + 0x00, 20.0);
        $this->initFloat($seg1 + 0x04, 10.0);
        $this->initFloat($seg1 + 0x08, 20.0);
        $this->initFloat($seg1 + 0x0c, 1.0);
        $this->initFloat($seg1 + 0x10, 0.0);
        $this->initFloat($seg1 + 0x14, 5.0);

        // Sentinel end-of-path segment: length == 0.0
        $segEnd = $this->alloc(6 * 4);
        $this->initFloat($segEnd + 0x00, 0.0);
        $this->initFloat($segEnd + 0x04, 0.0);
        $this->initFloat($segEnd + 0x08, 0.0);
        $this->initFloat($segEnd + 0x0c, 0.0);
        $this->initFloat($segEnd + 0x10, 0.0);
        $this->initFloat($segEnd + 0x14, 0.0);

        $segStart = $this->alloc(6 * 4);
        $this->initFloat($segStart + 0x00, 50.0);
        $this->initFloat($segStart + 0x04, 30.0);
        $this->initFloat($segStart + 0x08, 40.0);
        $this->initFloat($segStart + 0x0c, 0.0);
        $this->initFloat($segStart + 0x10, 1.0);
        $this->initFloat($segStart + 0x14, 15.0);

        $obj = $this->alloc(0x19 * 4);
        $this->initFloat($obj + 0x08 * 4, 1000.0); // param_1[8]
        $this->initFloat($obj + 0x09 * 4, 2000.0); // param_1[9]
        $this->initFloat($obj + 0x10 * 4, 0.0);    // direction flag: forward
        $this->initUint32($obj + 0x11 * 4, $segStart); // path start ptr
        $this->initUint32($obj + 0x12 * 4, 0);     // path loop-back ptr (unused this path)
        $this->initUint32($obj + 0x13 * 4, $seg1); // current segment ptr
        $this->initFloat($obj + 0x15 * 4, 25.0);   // current progress along segment
        $this->initFloat($obj + 0x18 * 4, 0.0);    // segment heading cache

        $this->call('_advancePedPathPos_8c0289ac')->with($obj);

        $this->shouldWriteLong($obj + 0x13 * 4, $segStart);
        $this->shouldWriteFloat($obj + 0x18 * 4, 15.0);
        $this->shouldWriteFloat($obj + 0x15 * 4, 5.0);
        $this->shouldWriteFloat($obj + 0x00 * 4, 5.0 * 0.0 + 30.0 + 1000.0);
        $this->shouldWriteFloat($obj + 0x02 * 4, 5.0 * 1.0 + 40.0 + 2000.0);

        $this->shouldReturn(1);
    }

    public function test_backward_no_retreat()
    {
        $seg = $this->alloc(6 * 4);
        $this->initFloat($seg + 0x00, 100.0);
        $this->initFloat($seg + 0x04, 10.0);
        $this->initFloat($seg + 0x08, 20.0);
        $this->initFloat($seg + 0x0c, 1.0);
        $this->initFloat($seg + 0x10, 0.0);
        $this->initFloat($seg + 0x14, 5.0);

        $obj = $this->alloc(0x19 * 4);
        $this->initFloat($obj + 0x08 * 4, 1000.0); // param_1[8]
        $this->initFloat($obj + 0x09 * 4, 2000.0); // param_1[9]
        $this->initFloat($obj + 0x10 * 4, 1.0);    // direction flag: backward
        $this->initUint32($obj + 0x11 * 4, 0);     // path start ptr (unused this path)
        $this->initUint32($obj + 0x12 * 4, 0);     // path loop-back ptr (unused this path)
        $this->initUint32($obj + 0x13 * 4, $seg);  // current segment ptr
        $this->initFloat($obj + 0x15 * 4, 30.0);   // current progress along segment
        $this->initFloat($obj + 0x18 * 4, 0.0);    // segment heading cache

        $this->call('_advancePedPathPos_8c0289ac')->with($obj);

        $this->shouldWriteFloat($obj + 0x15 * 4, 30.0);
        $this->shouldWriteFloat($obj + 0x00 * 4, 30.0 * 1.0 + 10.0 + 1000.0);
        $this->shouldWriteFloat($obj + 0x02 * 4, 30.0 * 0.0 + 20.0 + 2000.0);

        $this->shouldReturn(0);
    }

    public function test_backward_retreats_one_segment_no_wrap()
    {
        $seg1 = $this->alloc(6 * 4);
        $this->initFloat($seg1 + 0x00, 20.0);
        $this->initFloat($seg1 + 0x04, 10.0);
        $this->initFloat($seg1 + 0x08, 20.0);
        $this->initFloat($seg1 + 0x0c, 1.0);
        $this->initFloat($seg1 + 0x10, 0.0);
        $this->initFloat($seg1 + 0x14, 5.0);

        $seg2 = $this->alloc(6 * 4);
        $this->initFloat($seg2 + 0x00, 50.0);
        $this->initFloat($seg2 + 0x04, 30.0);
        $this->initFloat($seg2 + 0x08, 40.0);
        $this->initFloat($seg2 + 0x0c, 0.0);
        $this->initFloat($seg2 + 0x10, 1.0);
        $this->initFloat($seg2 + 0x14, 15.0);

        $obj = $this->alloc(0x19 * 4);
        $this->initFloat($obj + 0x08 * 4, 1000.0); // param_1[8]
        $this->initFloat($obj + 0x09 * 4, 2000.0); // param_1[9]
        $this->initFloat($obj + 0x10 * 4, 1.0);    // direction flag: backward
        $this->initUint32($obj + 0x11 * 4, 0);     // path start ptr (not seg2, so no wrap)
        $this->initUint32($obj + 0x12 * 4, 0);     // path loop-back ptr (unused this path)
        $this->initUint32($obj + 0x13 * 4, $seg2); // current segment ptr
        $this->initFloat($obj + 0x15 * 4, -5.0);   // current progress along segment
        $this->initFloat($obj + 0x18 * 4, 0.0);    // segment heading cache

        $this->call('_advancePedPathPos_8c0289ac')->with($obj);

        $this->shouldWriteLong($obj + 0x13 * 4, $seg1);
        $this->shouldWriteFloat($obj + 0x18 * 4, 5.0);
        $this->shouldWriteFloat($obj + 0x15 * 4, 15.0);
        $this->shouldWriteFloat($obj + 0x00 * 4, 15.0 * 1.0 + 10.0 + 1000.0);
        $this->shouldWriteFloat($obj + 0x02 * 4, 15.0 * 0.0 + 20.0 + 2000.0);

        $this->shouldReturn(1);
    }

    public function test_backward_wraps_to_loop_back()
    {
        $segLoopBack = $this->alloc(6 * 4);
        $this->initFloat($segLoopBack + 0x00, 50.0);
        $this->initFloat($segLoopBack + 0x04, 30.0);
        $this->initFloat($segLoopBack + 0x08, 40.0);
        $this->initFloat($segLoopBack + 0x0c, 0.0);
        $this->initFloat($segLoopBack + 0x10, 1.0);
        $this->initFloat($segLoopBack + 0x14, 15.0);

        $segStart = $this->alloc(6 * 4);
        $this->initFloat($segStart + 0x00, 20.0);
        $this->initFloat($segStart + 0x04, 10.0);
        $this->initFloat($segStart + 0x08, 20.0);
        $this->initFloat($segStart + 0x0c, 1.0);
        $this->initFloat($segStart + 0x10, 0.0);
        $this->initFloat($segStart + 0x14, 5.0);

        $obj = $this->alloc(0x19 * 4);
        $this->initFloat($obj + 0x08 * 4, 1000.0); // param_1[8]
        $this->initFloat($obj + 0x09 * 4, 2000.0); // param_1[9]
        $this->initFloat($obj + 0x10 * 4, 1.0);    // direction flag: backward
        $this->initUint32($obj + 0x11 * 4, $segStart); // path start ptr
        $this->initUint32($obj + 0x12 * 4, $segLoopBack); // path loop-back ptr
        $this->initUint32($obj + 0x13 * 4, $segStart); // current segment ptr
        $this->initFloat($obj + 0x15 * 4, -5.0);   // current progress along segment
        $this->initFloat($obj + 0x18 * 4, 0.0);    // segment heading cache

        $this->call('_advancePedPathPos_8c0289ac')->with($obj);

        $this->shouldWriteLong($obj + 0x13 * 4, $segLoopBack);
        $this->shouldWriteFloat($obj + 0x18 * 4, 15.0);
        $this->shouldWriteFloat($obj + 0x15 * 4, 45.0);
        $this->shouldWriteFloat($obj + 0x00 * 4, 45.0 * 0.0 + 30.0 + 1000.0);
        $this->shouldWriteFloat($obj + 0x02 * 4, 45.0 * 1.0 + 40.0 + 2000.0);

        $this->shouldReturn(1);
    }

    public function test_forward_advances_when_progress_equals_segment_length()
    {
        // FCMP/GT + BT exits the loop only when length > progress, so
        // progress == length still satisfies "length <= progress" and advances.
        $seg1 = $this->alloc(6 * 4);
        $this->initFloat($seg1 + 0x00, 20.0);
        $this->initFloat($seg1 + 0x04, 10.0);
        $this->initFloat($seg1 + 0x08, 20.0);
        $this->initFloat($seg1 + 0x0c, 1.0);
        $this->initFloat($seg1 + 0x10, 0.0);
        $this->initFloat($seg1 + 0x14, 5.0);

        $seg2 = $this->alloc(6 * 4);
        $this->initFloat($seg2 + 0x00, 50.0);
        $this->initFloat($seg2 + 0x04, 30.0);
        $this->initFloat($seg2 + 0x08, 40.0);
        $this->initFloat($seg2 + 0x0c, 0.0);
        $this->initFloat($seg2 + 0x10, 1.0);
        $this->initFloat($seg2 + 0x14, 15.0);

        $obj = $this->alloc(0x19 * 4);
        $this->initFloat($obj + 0x08 * 4, 1000.0); // param_1[8]
        $this->initFloat($obj + 0x09 * 4, 2000.0); // param_1[9]
        $this->initFloat($obj + 0x10 * 4, 0.0);    // direction flag: forward
        $this->initUint32($obj + 0x11 * 4, 0);     // path start ptr (unused this path)
        $this->initUint32($obj + 0x12 * 4, 0);     // path loop-back ptr (unused this path)
        $this->initUint32($obj + 0x13 * 4, $seg1); // current segment ptr
        $this->initFloat($obj + 0x15 * 4, 20.0);   // progress == seg1 length exactly
        $this->initFloat($obj + 0x18 * 4, 0.0);    // segment heading cache

        $this->call('_advancePedPathPos_8c0289ac')->with($obj);

        $this->shouldWriteLong($obj + 0x13 * 4, $seg2);
        $this->shouldWriteFloat($obj + 0x18 * 4, 15.0);
        $this->shouldWriteFloat($obj + 0x15 * 4, 0.0);
        $this->shouldWriteFloat($obj + 0x00 * 4, 0.0 * 0.0 + 30.0 + 1000.0);
        $this->shouldWriteFloat($obj + 0x02 * 4, 0.0 * 1.0 + 40.0 + 2000.0);

        $this->shouldReturn(1);
    }

    public function test_backward_retreats_when_progress_equals_zero()
    {
        // FCMP/GT + BT exits the loop only when progress > 0.0, so
        // progress == 0.0 still satisfies "progress <= 0.0" and retreats.
        $seg1 = $this->alloc(6 * 4);
        $this->initFloat($seg1 + 0x00, 20.0);
        $this->initFloat($seg1 + 0x04, 10.0);
        $this->initFloat($seg1 + 0x08, 20.0);
        $this->initFloat($seg1 + 0x0c, 1.0);
        $this->initFloat($seg1 + 0x10, 0.0);
        $this->initFloat($seg1 + 0x14, 5.0);

        $seg2 = $this->alloc(6 * 4);
        $this->initFloat($seg2 + 0x00, 50.0);
        $this->initFloat($seg2 + 0x04, 30.0);
        $this->initFloat($seg2 + 0x08, 40.0);
        $this->initFloat($seg2 + 0x0c, 0.0);
        $this->initFloat($seg2 + 0x10, 1.0);
        $this->initFloat($seg2 + 0x14, 15.0);

        $obj = $this->alloc(0x19 * 4);
        $this->initFloat($obj + 0x08 * 4, 1000.0); // param_1[8]
        $this->initFloat($obj + 0x09 * 4, 2000.0); // param_1[9]
        $this->initFloat($obj + 0x10 * 4, 1.0);    // direction flag: backward
        $this->initUint32($obj + 0x11 * 4, 0);     // path start ptr (not seg2, so no wrap)
        $this->initUint32($obj + 0x12 * 4, 0);     // path loop-back ptr (unused this path)
        $this->initUint32($obj + 0x13 * 4, $seg2); // current segment ptr
        $this->initFloat($obj + 0x15 * 4, 0.0);    // progress == 0.0 exactly
        $this->initFloat($obj + 0x18 * 4, 0.0);    // segment heading cache

        $this->call('_advancePedPathPos_8c0289ac')->with($obj);

        $this->shouldWriteLong($obj + 0x13 * 4, $seg1);
        $this->shouldWriteFloat($obj + 0x18 * 4, 5.0);
        $this->shouldWriteFloat($obj + 0x15 * 4, 20.0);
        $this->shouldWriteFloat($obj + 0x00 * 4, 20.0 * 1.0 + 10.0 + 1000.0);
        $this->shouldWriteFloat($obj + 0x02 * 4, 20.0 * 0.0 + 20.0 + 2000.0);

        $this->shouldReturn(1);
    }

    public function test_backward_direction_flag_is_the_plain_int_one()
    {
        // pedGroupTask_8c029078 stores nReverse as an int copied from the spec's
        // byte, so the live value is 1, not 1.0. The other backward cases here
        // encode it as a float and would still pass if the flag were read as one.
        //
        // Note this case does not yet discriminate either: reading 1 as a float
        // gives a denormal, which real SH4 hardware flushes to zero (FPSCR.DN)
        // but the simulator compares as a nonzero PHP double, so a float compare
        // still takes the backward branch here while the console takes forward.
        $seg1 = $this->alloc(6 * 4);
        $this->initFloat($seg1 + 0x00, 20.0);
        $this->initFloat($seg1 + 0x04, 10.0);
        $this->initFloat($seg1 + 0x08, 20.0);
        $this->initFloat($seg1 + 0x0c, 1.0);
        $this->initFloat($seg1 + 0x10, 0.0);
        $this->initFloat($seg1 + 0x14, 5.0);

        $seg2 = $this->alloc(6 * 4);
        $this->initFloat($seg2 + 0x00, 50.0);
        $this->initFloat($seg2 + 0x04, 30.0);
        $this->initFloat($seg2 + 0x08, 40.0);
        $this->initFloat($seg2 + 0x0c, 0.0);
        $this->initFloat($seg2 + 0x10, 1.0);
        $this->initFloat($seg2 + 0x14, 15.0);

        $obj = $this->alloc(0x19 * 4);
        $this->initFloat($obj + 0x08 * 4, 1000.0); // param_1[8]
        $this->initFloat($obj + 0x09 * 4, 2000.0); // param_1[9]
        $this->initUint32($obj + 0x10 * 4, 1);     // direction flag: backward, as an int
        $this->initUint32($obj + 0x11 * 4, 0);     // path start ptr (not seg2, so no wrap)
        $this->initUint32($obj + 0x12 * 4, 0);     // path loop-back ptr (unused this path)
        $this->initUint32($obj + 0x13 * 4, $seg2); // current segment ptr
        $this->initFloat($obj + 0x15 * 4, -5.0);   // current progress along segment
        $this->initFloat($obj + 0x18 * 4, 0.0);    // segment heading cache

        $this->call('_advancePedPathPos_8c0289ac')->with($obj);

        $this->shouldWriteLong($obj + 0x13 * 4, $seg1);
        $this->shouldWriteFloat($obj + 0x18 * 4, 5.0);
        $this->shouldWriteFloat($obj + 0x15 * 4, 15.0);
        $this->shouldWriteFloat($obj + 0x00 * 4, 15.0 * 1.0 + 10.0 + 1000.0);
        $this->shouldWriteFloat($obj + 0x02 * 4, 15.0 * 0.0 + 20.0 + 2000.0);

        $this->shouldReturn(1);
    }

    public function test_forward_advances_two_segments_in_one_call()
    {
        // Progress overshoots seg1 and seg2's lengths both, so the while
        // loop body must run twice before landing inside seg3.
        $seg1 = $this->alloc(6 * 4);
        $this->initFloat($seg1 + 0x00, 10.0);
        $this->initFloat($seg1 + 0x04, 1.0);
        $this->initFloat($seg1 + 0x08, 2.0);
        $this->initFloat($seg1 + 0x0c, 1.0);
        $this->initFloat($seg1 + 0x10, 0.0);
        $this->initFloat($seg1 + 0x14, 100.0);

        $seg2 = $this->alloc(6 * 4);
        $this->initFloat($seg2 + 0x00, 10.0);
        $this->initFloat($seg2 + 0x04, 3.0);
        $this->initFloat($seg2 + 0x08, 4.0);
        $this->initFloat($seg2 + 0x0c, 0.0);
        $this->initFloat($seg2 + 0x10, 1.0);
        $this->initFloat($seg2 + 0x14, 200.0);

        $seg3 = $this->alloc(6 * 4);
        $this->initFloat($seg3 + 0x00, 50.0);
        $this->initFloat($seg3 + 0x04, 5.0);
        $this->initFloat($seg3 + 0x08, 6.0);
        $this->initFloat($seg3 + 0x0c, 1.0);
        $this->initFloat($seg3 + 0x10, 1.0);
        $this->initFloat($seg3 + 0x14, 300.0);

        $obj = $this->alloc(0x19 * 4);
        $this->initFloat($obj + 0x08 * 4, 1000.0); // param_1[8]
        $this->initFloat($obj + 0x09 * 4, 2000.0); // param_1[9]
        $this->initFloat($obj + 0x10 * 4, 0.0);    // direction flag: forward
        $this->initUint32($obj + 0x11 * 4, 0);     // path start ptr (unused this path)
        $this->initUint32($obj + 0x12 * 4, 0);     // path loop-back ptr (unused this path)
        $this->initUint32($obj + 0x13 * 4, $seg1); // current segment ptr
        $this->initFloat($obj + 0x15 * 4, 25.0);   // progress: past seg1 (10) and seg2 (10)
        $this->initFloat($obj + 0x18 * 4, 0.0);    // segment heading cache

        $this->call('_advancePedPathPos_8c0289ac')->with($obj);

        $this->shouldWriteLong($obj + 0x13 * 4, $seg3);
        $this->shouldWriteFloat($obj + 0x18 * 4, 300.0);
        $this->shouldWriteFloat($obj + 0x15 * 4, 5.0);
        $this->shouldWriteFloat($obj + 0x00 * 4, 5.0 * 1.0 + 5.0 + 1000.0);
        $this->shouldWriteFloat($obj + 0x02 * 4, 5.0 * 1.0 + 6.0 + 2000.0);

        $this->shouldReturn(1);
    }

    public function test_backward_retreats_two_segments_in_one_call()
    {
        // Progress undershoots by more than seg2 and seg1's lengths both, so
        // the for-loop body must run twice before landing inside seg0.
        $seg0 = $this->alloc(6 * 4);
        $this->initFloat($seg0 + 0x00, 20.0);
        $this->initFloat($seg0 + 0x04, 1.0);
        $this->initFloat($seg0 + 0x08, 2.0);
        $this->initFloat($seg0 + 0x0c, 1.0);
        $this->initFloat($seg0 + 0x10, 0.0);
        $this->initFloat($seg0 + 0x14, 100.0);

        $seg1 = $this->alloc(6 * 4);
        $this->initFloat($seg1 + 0x00, 10.0);
        $this->initFloat($seg1 + 0x04, 3.0);
        $this->initFloat($seg1 + 0x08, 4.0);
        $this->initFloat($seg1 + 0x0c, 0.0);
        $this->initFloat($seg1 + 0x10, 1.0);
        $this->initFloat($seg1 + 0x14, 200.0);

        $seg2 = $this->alloc(6 * 4);
        $this->initFloat($seg2 + 0x00, 10.0);
        $this->initFloat($seg2 + 0x04, 5.0);
        $this->initFloat($seg2 + 0x08, 6.0);
        $this->initFloat($seg2 + 0x0c, 1.0);
        $this->initFloat($seg2 + 0x10, 1.0);
        $this->initFloat($seg2 + 0x14, 300.0);

        $obj = $this->alloc(0x19 * 4);
        $this->initFloat($obj + 0x08 * 4, 1000.0); // param_1[8]
        $this->initFloat($obj + 0x09 * 4, 2000.0); // param_1[9]
        $this->initFloat($obj + 0x10 * 4, 1.0);    // direction flag: backward
        $this->initUint32($obj + 0x11 * 4, 0);     // path start ptr (not seg1/seg2, so no wrap)
        $this->initUint32($obj + 0x12 * 4, 0);     // path loop-back ptr (unused this path)
        $this->initUint32($obj + 0x13 * 4, $seg2); // current segment ptr
        $this->initFloat($obj + 0x15 * 4, -15.0);  // undershoots seg1 (10) then seg0 (20)
        $this->initFloat($obj + 0x18 * 4, 0.0);    // segment heading cache

        $this->call('_advancePedPathPos_8c0289ac')->with($obj);

        $this->shouldWriteLong($obj + 0x13 * 4, $seg0);
        $this->shouldWriteFloat($obj + 0x18 * 4, 100.0);
        $this->shouldWriteFloat($obj + 0x15 * 4, 15.0);
        $this->shouldWriteFloat($obj + 0x00 * 4, 15.0 * 1.0 + 1.0 + 1000.0);
        $this->shouldWriteFloat($obj + 0x02 * 4, 15.0 * 0.0 + 2.0 + 2000.0);

        $this->shouldReturn(1);
    }
};
