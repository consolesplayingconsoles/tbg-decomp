<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

/*
 * TODO: revisit how &result is asserted. Capturing it from the first call only
 * proves both calls agree -- a consistently wrong pointer passes. The other
 * per-object stack addresses in this suite are hardcoded behind isAsmObject()
 * instead, which pins the real value (run once with a wrong literal and the
 * failure reports the actual one). Left as a capture because the absolute
 * address is a frame-layout artifact, not behaviour.
 */
return new class extends TestCase {
    public function test_primary_grid_hit_skips_fallback_query()
    {
        $this->setSize('_GroundQueryFindPolygon_8c020914', 4);
        $this->setSize('_GroundProbeInterpolateHeight_8c020f7e', 4);
        $this->setSize('_var_groundQueryPoint_8c1bc460', 0xc);
        $this->setSize('_var_currentCourse_8c1bb868', 0x50);

        $point = $this->addressOf('_var_groundQueryPoint_8c1bc460');
        $this->initUint32($point + 0x0, fdec(1.0)); // x
        $this->initUint32($point + 0x4, fdec(2.0)); // y
        $this->initUint32($point + 0x8, fdec(3.0)); // z

        $course = $this->addressOf('_var_currentCourse_8c1bb868');
        $this->initUint32($course + 0x28, 0x11111111); // atariHum_0x28
        $this->initUint32($course + 0x04, 0x22222222); // atariBus_0x04

        $this->call('_snapPointToGround_8c02840c');

        $resultPtr = null;
        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x11111111);
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')
            ->with(1.0, 2.0, 3.0)
            ->do(function () use (&$resultPtr) {
                $resultPtr = $this->registers[4]->value;
                $this->memory->writeUInt32($resultPtr + 0xc, U32::of(1));
            });
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')
            ->do(function () use (&$resultPtr, $point) {
                if ($this->registers[4]->value !== $resultPtr || $this->registers[5]->value !== $point) {
                    throw new RuntimeException(sprintf(
                        '_GroundProbeInterpolateHeight_8c020f7e: expected result=%08x point=%08x, got result=%08x point=%08x',
                        $resultPtr, $point, $this->registers[4]->value, $this->registers[5]->value,
                    ));
                }
            });
    }

    public function test_primary_grid_miss_falls_back_to_secondary_grid()
    {
        $this->setSize('_GroundQueryFindPolygon_8c020914', 4);
        $this->setSize('_GroundProbeInterpolateHeight_8c020f7e', 4);
        $this->setSize('_var_groundQueryPoint_8c1bc460', 0xc);
        $this->setSize('_var_currentCourse_8c1bb868', 0x50);

        $point = $this->addressOf('_var_groundQueryPoint_8c1bc460');
        $this->initUint32($point + 0x0, fdec(1.0)); // x
        $this->initUint32($point + 0x4, fdec(2.0)); // y
        $this->initUint32($point + 0x8, fdec(3.0)); // z

        $course = $this->addressOf('_var_currentCourse_8c1bb868');
        $this->initUint32($course + 0x28, 0x11111111); // atariHum_0x28
        $this->initUint32($course + 0x04, 0x22222222); // atariBus_0x04

        $this->call('_snapPointToGround_8c02840c');

        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x11111111);
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')
            ->with(1.0, 2.0, 3.0)
            ->do(function () {
                $this->memory->writeUInt32($this->registers[4]->value + 0xc, U32::of(0));
            });
        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x22222222);
        $resultPtr = null;
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')
            ->with(1.0, 2.0, 3.0)
            ->do(function () use (&$resultPtr) {
                $resultPtr = $this->registers[4]->value;
                $this->memory->writeUInt32($resultPtr + 0xc, U32::of(1));
            });
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')
            ->do(function () use (&$resultPtr, $point) {
                if ($this->registers[4]->value !== $resultPtr || $this->registers[5]->value !== $point) {
                    throw new RuntimeException(sprintf(
                        '_GroundProbeInterpolateHeight_8c020f7e: expected result=%08x point=%08x, got result=%08x point=%08x',
                        $resultPtr, $point, $this->registers[4]->value, $this->registers[5]->value,
                    ));
                }
            });
    }
};
