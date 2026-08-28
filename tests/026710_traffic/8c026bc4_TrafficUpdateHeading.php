<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

// TrafficUpdateHeading_8c026bc4 re-derives heading and the vehicle's 4
// body-corner points after the entry reaches a new waypoint (entry+0x100/
// 0x108): direction is (target - current), normalized by its own length
// (njSqrt(dx*dx+dy*dy) -- confirmed via probing that the incoming float
// argument is never read at all). entry+0x274/0x278 hold the normalized
// dx/dy (the sin/cos heading basis from initEntryState_8c026748).
// entry+0x23c/0x240 place front/rear reference points along that heading;
// entry+0x248 (half-width) offsets those into the 4 body corners at
// entry+0x118/0x120/0x124/0x12c. The heading angle (entry+0x250) is
// acosf(dy) scaled to the 16-bit angle format, sign-flipped when dx < 0 --
// confirmed via probing that acosf's argument is the original normalized
// dy, not the half-width-scaled value that overwrites the same variable
// just before the call in Ghidra's output.
return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_njSqrt', 4);
        $this->setSize('_acosf', 4);
    }

    private function allocEntry(float $curX, float $curY, float $tgtX, float $tgtY): int {
        $entry = $this->alloc(0x140);
        $this->initUint32($entry + 0xf4, fdec($curX));
        $this->initUint32($entry + 0xfc, fdec($curY));
        $this->initUint32($entry + 0x100, fdec($tgtX));
        $this->initUint32($entry + 0x108, fdec($tgtY));
        $this->initUint32($entry + 0x23c, fdec(2.0));  // front reference distance
        $this->initUint32($entry + 0x240, fdec(4.0));  // rear reference distance
        $this->initUint32($entry + 0x248, fdec(1.5));  // half-width
        return $entry;
    }

    // dx = 3, dy = 4 (3-4-5 triangle): dx >= 0, no angle sign flip.
    public function test_positiveDx(): void {
        $this->resolveSymbols();
        $entry = $this->allocEntry(100.0, 200.0, 103.0, 204.0);

        // The incoming float argument (25.0) is never read by the real asm.
        $this->call('_TrafficUpdateHeading_8c026bc4')->with(25.0, $entry);

        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);

        // Normalized direction: (0.6, 0.8).
        $this->shouldWriteFloat($entry + 0x274, 0.6);
        $this->shouldWriteFloat($entry + 0x278, 0.8);

        $this->shouldWriteFloat($entry + 0x100, 101.2);   // 0.6*2 + 100
        $this->shouldWriteFloat($entry + 0x108, 201.6);   // 0.8*2 + 200
        $this->shouldWriteFloat($entry + 0x10c, 102.4);   // 0.6*4 + 100
        $this->shouldWriteFloat($entry + 0x114, 203.2);   // 0.8*4 + 200

        // Half-width (1.5) offsets: halfDy = 1.2, halfDx = 0.9.
        $this->shouldWriteFloat($entry + 0x118, 98.8);    // 100 - 1.2
        $this->shouldWriteFloat($entry + 0x120, 200.9);   // 200 + 0.9
        $this->shouldWriteFloat($entry + 0x124, 101.2);   // 100 + 1.2
        $this->shouldWriteFloat($entry + 300, 199.1);     // 200 - 0.9

        // acosf receives the original normalized dy (0.8), not the
        // half-width-scaled halfDy (1.2) that overwrote the same Ghidra
        // SSA variable just before this call.
        $this->shouldCall('_acosf')->with(0.8)->andReturn(0.6435011);
        $this->shouldWriteLong($entry + 0x250, (int)((0.6435011 * 65536.0) / 6.283184));
    }

    // dx = -3, dy = 4: dx < 0, so the angle must be negated -- the branch
    // the first case can't exercise.
    public function test_negativeDxFlipsAngleSign(): void {
        $this->resolveSymbols();
        $entry = $this->allocEntry(100.0, 200.0, 97.0, 204.0);

        $this->call('_TrafficUpdateHeading_8c026bc4')->with(25.0, $entry);

        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);

        $this->shouldWriteFloat($entry + 0x274, -0.6);
        $this->shouldWriteFloat($entry + 0x278, 0.8);

        $this->shouldWriteFloat($entry + 0x100, 98.8);    // -0.6*2 + 100
        $this->shouldWriteFloat($entry + 0x108, 201.6);   // 0.8*2 + 200
        $this->shouldWriteFloat($entry + 0x10c, 97.6);    // -0.6*4 + 100
        $this->shouldWriteFloat($entry + 0x114, 203.2);   // 0.8*4 + 200

        $this->shouldWriteFloat($entry + 0x118, 98.8);    // 100 - 1.2
        $this->shouldWriteFloat($entry + 0x120, 199.1);   // 200 + (-0.9)
        $this->shouldWriteFloat($entry + 0x124, 101.2);   // 100 + 1.2
        $this->shouldWriteFloat($entry + 300, 200.9);     // 200 - (-0.9)

        $this->shouldCall('_acosf')->with(0.8)->andReturn(0.6435011);

        // Same acosf result as the positive-dx case, but negated.
        $this->shouldWriteLong($entry + 0x250, -(int)((0.6435011 * 65536.0) / 6.283184));
    }
};
