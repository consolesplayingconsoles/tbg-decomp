<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // NJS_POLYGON_VTX[4] field offsets: vtx = 16 bytes, x=+0, y=+4.
    const V0_X = 0x00;
    const V0_Y = 0x04;
    const V1_X = 0x10;
    const V1_Y = 0x14;
    const V2_Y = 0x24;
    const V3_Y = 0x34;

    private function arrange(): int
    {
        $this->setSize('_njDrawPolygon', 4);
        return $this->addressOf('_init_8c044de8');
    }

    // x = value * 320 / 256 + 256; a quad spanning [x, 576] x [y, y+20].
    // The left-edge x is written twice (redundant stores kept from the original).
    private function expect(int $base, float $y, float $x): void
    {
        $yTop = $y + 20.0;
        $this->shouldWriteFloat($base + self::V1_X, $x);
        $this->shouldWriteFloat($base + self::V0_X, $x);
        $this->shouldWriteFloat($base + self::V2_Y, $y);
        $this->shouldWriteFloat($base + self::V0_Y, $y);
        $this->shouldWriteFloat($base + self::V1_X, $x);
        $this->shouldWriteFloat($base + self::V0_X, $x);
        $this->shouldWriteFloat($base + self::V3_Y, $yTop);
        $this->shouldWriteFloat($base + self::V1_Y, $yTop);
        $this->shouldCall('_njDrawPolygon')->with($base, 4, 1);
    }

    public function test_zero_sits_at_left()
    {
        $base = $this->arrange();
        $this->call('_drawSensitivityBar_8c01a42a')->with(100.0, 0);
        $this->expect($base, 100.0, 256.0);
    }

    public function test_max_0x80()
    {
        $base = $this->arrange();
        $this->call('_drawSensitivityBar_8c01a42a')->with(100.0, 0x80);
        $this->expect($base, 100.0, 416.0);
    }

    public function test_mid_value()
    {
        $base = $this->arrange();
        $this->call('_drawSensitivityBar_8c01a42a')->with(40.0, 0x40);
        $this->expect($base, 40.0, 336.0);
    }
};
