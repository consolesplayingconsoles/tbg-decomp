<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Builds a bus/traffic-entity placement matrix: a rotation basis from the
// bus's width_0x23c width and the height delta between posY_0x0f8 and
// posHistory_0x100[0].y (also stashing the cos/sin as pitchCos_0x270/0x26c
// for the camera bob elsewhere), a yaw from atan2f of two more history
// points via njRotateZ, then the translation row from the bus's current
// position.
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_njSqrt', 4);
        $this->setSize('_atan2f', 4);
        $this->setSize('_njRotateZ', 4);
    }

    // Rounds to float32 precision, so expected products/quotients can be
    // computed the same way the SH4 FPU does (round each operand to
    // float32 first) rather than PHP's double precision.
    private function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }

    private function initFloat(int $address, float $value): void
    {
        $raw = unpack('L', pack('f', $value))[1];
        $this->initUint32($address, $raw);
    }

    public function test_placesModel(): void
    {
        $this->resolveSymbols();

        $bus = $this->alloc(0x3cc);
        $matrix = $this->alloc(0x40);

        $this->initFloat($bus + 0xf8, 10.0);  // posY_0x0f8
        $this->initFloat($bus + 0x104, 14.0); // posHistory_0x100[0].y
        $this->initFloat($bus + 0x23c, 3.0);  // width_0x23c (width)
        $this->initFloat($bus + 0x278, 2.0);  // headingDirZ_0x278
        $this->initFloat($bus + 0x274, 1.0);  // headingDirX_0x274
        $this->initFloat($bus + 0x11c, 1.0);  // posHistory_0x100[2].y
        $this->initFloat($bus + 0x128, 5.0);  // posHistory_0x100[3].y
        $this->initFloat($bus + 0x244, 2.0);  // height_0x244
        $this->initFloat($bus + 0xf4, 7.0);   // posX_0x0f4
        $this->initFloat($bus + 0xfc, 9.0);   // posZ_0x0fc

        $this->call('_VehicleModelPlace_8c020594')->with($matrix, $bus);

        // dy = 14 - 10 = 4; dist = njSqrt(4^2 + 3^2) = njSqrt(25) = 5
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);

        $cosT = $this->f32(3.0 / 5.0); // width_0x23c / dist
        $sinT = $this->f32(4.0 / 5.0); // dy / dist

        $this->shouldWriteFloat($bus + 0x270, $cosT);
        $this->shouldWriteFloat($bus + 0x26c, $sinT);

        $this->shouldWriteFloat($matrix + 0x00, 2.0);                    // headingDirZ_0x278
        $this->shouldWriteFloat($matrix + 0x04, 0.0);
        $this->shouldWriteFloat($matrix + 0x08, -1.0);                   // -headingDirX_0x274
        $this->shouldWriteFloat($matrix + 0x0c, 0.0);

        $this->shouldWriteFloat($matrix + 0x10, -$this->f32(1.0 * $sinT)); // -(headingDirX_0x274*sinT)
        $this->shouldWriteFloat($matrix + 0x14, $cosT);
        $this->shouldWriteFloat($matrix + 0x18, -$this->f32(2.0 * $sinT)); // -(headingDirZ_0x278*sinT)
        $this->shouldWriteFloat($matrix + 0x1c, 0.0);

        $this->shouldWriteFloat($matrix + 0x20, $this->f32(1.0 * $cosT)); // headingDirX_0x274*cosT
        $this->shouldWriteFloat($matrix + 0x24, $sinT);
        $this->shouldWriteFloat($matrix + 0x28, $this->f32(2.0 * $cosT)); // headingDirZ_0x278*cosT
        $this->shouldWriteFloat($matrix + 0x2c, 0.0);

        // atan2f(posHistory[3].y - posHistory[2].y, height_0x244) = atan2f(4.0, 2.0)
        $this->shouldCall('_atan2f')->with(4.0, 2.0)->andReturn(1.2);

        $step1 = $this->f32(1.2 * 65536.0);
        $step2 = $this->f32($step1 / $this->f32(6.283184));
        $angle = (int) $step2; // truncated toward zero

        $this->shouldCall('_njRotateZ')->with($matrix, $angle);

        $this->shouldWriteFloat($matrix + 0x30, 7.0); // posX_0x0f4
        $this->shouldWriteFloat($matrix + 0x34, 10.0); // posY_0x0f8
        $this->shouldWriteFloat($matrix + 0x38, 9.0); // posZ_0x0fc
        $this->shouldWriteFloat($matrix + 0x3c, 1.0);
    }
};
