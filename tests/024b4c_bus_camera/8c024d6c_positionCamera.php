<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

if (!function_exists('fdec')) {
    function fdec(float $value): int
    {
        return unpack('L', pack('f', $value))[1];
    }
}

/*
 * _positionCamera_8c024d6c(float dist, float dyOffset, float interestDyOffset):
 * moves busState's posX_0x2fc/posY_0x300/posZ_0x304 by dist along the unit
 * vector from the current position (posX_0x0f4/posZ_0x0fc) toward it,
 * dyOffset in Y; when the resulting bearing change from the bus's stored
 * heading (headingX_0x230/headingZ_0x238) exceeds ~5 degrees (angleInt > 910),
 * clamps the turn-angle step and rotates the move by it instead (sign from
 * a cross-product test) before applying it. Then positions the camera at
 * the new location and points its interest at the unmoved position offset
 * by interestDyOffset in Y.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_var_camera_8c1bb904', 0x40);
        $this->setSize('_var_scratchMatrix_8c1bc46c', 0x40);
        $this->setSize('_var_groundQueryPoint_8c1bc460', 0xc);
        $this->setSize('_njSqrt', 4);
        $this->setSize('_acosf', 4);
        $this->setSize('__divls', 4);
        $this->onCall('__divls', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });
        $this->setSize('_njUnitMatrix', 4);
        $this->setSize('_njRotateY', 4);
        $this->setSize('_njCalcPoint', 4);
        $this->setSize('_njTranslateCameraPosition', 4);
        $this->setSize('_njPointCameraInterest', 4);
    }

    private function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }

    private function initFloat(int $address, float $value): void
    {
        $this->initUint32($address, unpack('L', pack('f', $value))[1]);
    }

    public function test_small_bearing_change_moves_straight(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $camera = $this->addressOf('_var_camera_8c1bb904');

        $this->initFloat($base + 0x0f4, 10.0); // posX_0x0f4
        $this->initFloat($base + 0x0f8, 100.0); // posY_0x0f8
        $this->initFloat($base + 0x0fc, 20.0); // posZ_0x0fc
        $this->initFloat($base + 0x2fc, 13.0); // posX_0x2fc (prior target)
        $this->initFloat($base + 0x304, 24.0); // posZ_0x304 (prior target)
        $this->initFloat($base + 0x230, 1.0); // heading x
        $this->initFloat($base + 0x238, 0.0); // heading z

        $this->call('_positionCamera_8c024d6c')->with(10.0, 2.0, 7.0);

        $this->shouldWriteFloat($base + 0x314, 10.0);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);

        $newPosX = $this->f32(10.0 + $this->f32(10.0 * $this->f32(3.0 / 5.0)));
        $newPosY = $this->f32(100.0 + 2.0);
        $newPosZ = $this->f32(20.0 + $this->f32(10.0 * $this->f32(4.0 / 5.0)));
        $this->shouldWriteFloat($base + 0x2fc, $newPosX);
        $this->shouldWriteFloat($base + 0x300, $newPosY);
        $this->shouldWriteFloat($base + 0x304, $newPosZ);

        $moveX = $this->f32($newPosX - 10.0);
        $moveZ = $this->f32($newPosZ - 20.0);
        $dot = $this->f32($this->f32($moveX * 1.0) + $this->f32($moveZ * 0.0));
        $denom = $this->f32(10.0 * 4.9);
        $acosfArg = $this->f32($dot / $denom);
        $this->shouldCall('_acosf')->with($acosfArg)->andReturn(0.0);

        // angleInt = 0 <= 910: straight assignment, no rotation.
        $this->shouldWriteFloat($base + 0x308, $moveX);
        $this->shouldWriteFloat($base + 0x310, $moveZ);

        $this->shouldCall('_njTranslateCameraPosition')->with($camera, $newPosX, $newPosY, $newPosZ);
        $this->shouldCall('_njPointCameraInterest')->with($camera, 10.0, 107.0, 20.0);
    }

    public function test_large_bearing_change_clamps_to_max_and_rotates_positive(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $camera = $this->addressOf('_var_camera_8c1bb904');
        $matrix = $this->addressOf('_var_scratchMatrix_8c1bc46c');
        $groundPt = $this->addressOf('_var_groundQueryPoint_8c1bc460');

        $this->initFloat($base + 0x0f4, 0.0);
        $this->initFloat($base + 0x0f8, 0.0);
        $this->initFloat($base + 0x0fc, 0.0);
        $this->initFloat($base + 0x2fc, 1.0);
        $this->initFloat($base + 0x304, 0.0);
        $this->initFloat($base + 0x230, 0.0); // heading x
        $this->initFloat($base + 0x238, -1.0); // heading z

        $this->call('_positionCamera_8c024d6c')->with(1.0, 0.0, 0.0);

        $this->shouldWriteFloat($base + 0x314, 1.0);
        $this->shouldCall('_njSqrt')->with(1.0)->andReturn(1.0);

        $newPosX = 1.0;
        $newPosY = 0.0;
        $newPosZ = 0.0;
        $this->shouldWriteFloat($base + 0x2fc, $newPosX);
        $this->shouldWriteFloat($base + 0x300, $newPosY);
        $this->shouldWriteFloat($base + 0x304, $newPosZ);

        // moveX=1.0, moveZ=0.0; dot = 1.0*0.0 + 0.0*-1.0 = 0.0
        $denom = $this->f32(1.0 * 4.9);
        $acosfArg = $this->f32(0.0 / $denom);
        // acosf(0.0) mocked to 3.0 rad -> angleInt well above 5461, clamped.
        $this->shouldCall('_acosf')->with($acosfArg)->andReturn(3.0);

        $angleInt = (int) $this->f32($this->f32(3.0 * 65536.0) / 6.283184);
        $turnStep = $angleInt - 5461;
        // cross = moveZ*headingX - moveX*headingZ = 0*0 - 1*-1 = 1.0 >= 0: no sign flip.

        $this->shouldWriteFloat($groundPt + 0x0, 1.0); // groundPt.x = moveX
        $this->shouldWriteFloat($groundPt + 0x8, 0.0); // groundPt.z = moveZ
        $this->shouldCall('_njUnitMatrix')->with($matrix);
        $this->shouldCall('_njRotateY')->with($matrix, $turnStep);
        $this->shouldCall('_njCalcPoint')
            ->with($matrix, $groundPt, $base + 0x308)
            ->do(function () {
                $this->memory->writeUInt32($this->registers[6]->value + 0x0, U32::of(fdec(2.0)));
                $this->memory->writeUInt32($this->registers[6]->value + 0x8, U32::of(fdec(3.0)));
            });

        $this->shouldWriteFloat($base + 0x2fc, $this->f32(0.0 + 2.0));
        $this->shouldWriteFloat($base + 0x304, $this->f32(0.0 + 3.0));

        $this->shouldCall('_njTranslateCameraPosition')->with($camera, 2.0, 0.0, 3.0);
        $this->shouldCall('_njPointCameraInterest')->with($camera, 0.0, 0.0, 0.0);
    }

    public function test_medium_bearing_change_rotates_negative(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $camera = $this->addressOf('_var_camera_8c1bb904');
        $matrix = $this->addressOf('_var_scratchMatrix_8c1bc46c');
        $groundPt = $this->addressOf('_var_groundQueryPoint_8c1bc460');

        $this->initFloat($base + 0x0f4, 0.0);
        $this->initFloat($base + 0x0f8, 0.0);
        $this->initFloat($base + 0x0fc, 0.0);
        $this->initFloat($base + 0x2fc, 1.0);
        $this->initFloat($base + 0x304, 0.0);
        $this->initFloat($base + 0x230, 0.0); // heading x
        $this->initFloat($base + 0x238, 1.0); // heading z (flipped vs prior test)

        $this->call('_positionCamera_8c024d6c')->with(1.0, 0.0, 0.0);

        $this->shouldWriteFloat($base + 0x314, 1.0);
        $this->shouldCall('_njSqrt')->with(1.0)->andReturn(1.0);

        $this->shouldWriteFloat($base + 0x2fc, 1.0);
        $this->shouldWriteFloat($base + 0x300, 0.0);
        $this->shouldWriteFloat($base + 0x304, 0.0);

        $denom = $this->f32(1.0 * 4.9);
        $acosfArg = $this->f32(0.0 / $denom);
        // acosf(0.0) mocked to 0.3 rad -> angleInt between 910 and 5461 (no
        // 5461 clamp; turnLimit computed via /30/2 needs no min-45 clamp).
        $this->shouldCall('_acosf')->with($acosfArg)->andReturn(0.3);

        $this->shouldCall('__divls');

        $angleInt = (int) $this->f32($this->f32(0.3 * 65536.0) / 6.283184);
        $turnLimit = intdiv(intdiv($angleInt, 30), 2);
        if ($turnLimit < 45) {
            $turnLimit = 45;
        }
        $angleAfterTurn = $angleInt - $turnLimit;
        $turnStep = $angleInt - $angleAfterTurn;
        // cross = moveZ*headingX - moveX*headingZ = 0*0 - 1*1 = -1.0 < 0: sign flip.
        $turnStep = -$turnStep;

        $this->shouldWriteFloat($groundPt + 0x0, 1.0);
        $this->shouldWriteFloat($groundPt + 0x8, 0.0);
        $this->shouldCall('_njUnitMatrix')->with($matrix);
        $this->shouldCall('_njRotateY')->with($matrix, $turnStep);
        $this->shouldCall('_njCalcPoint')
            ->with($matrix, $groundPt, $base + 0x308)
            ->do(function () {
                $this->memory->writeUInt32($this->registers[6]->value + 0x0, U32::of(fdec(-1.0)));
                $this->memory->writeUInt32($this->registers[6]->value + 0x8, U32::of(fdec(4.0)));
            });

        $this->shouldWriteFloat($base + 0x2fc, $this->f32(0.0 + -1.0));
        $this->shouldWriteFloat($base + 0x304, $this->f32(0.0 + 4.0));

        $this->shouldCall('_njTranslateCameraPosition')->with($camera, -1.0, 0.0, 4.0);
        $this->shouldCall('_njPointCameraInterest')->with($camera, 0.0, 0.0, 0.0);
    }

    public function test_small_turn_limit_clamps_to_minimum(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $camera = $this->addressOf('_var_camera_8c1bb904');
        $matrix = $this->addressOf('_var_scratchMatrix_8c1bc46c');
        $groundPt = $this->addressOf('_var_groundQueryPoint_8c1bc460');

        $this->initFloat($base + 0x0f4, 0.0);
        $this->initFloat($base + 0x0f8, 0.0);
        $this->initFloat($base + 0x0fc, 0.0);
        $this->initFloat($base + 0x2fc, 1.0);
        $this->initFloat($base + 0x304, 0.0);
        $this->initFloat($base + 0x230, 0.0); // heading x
        $this->initFloat($base + 0x238, 1.0); // heading z

        $this->call('_positionCamera_8c024d6c')->with(1.0, 0.0, 0.0);

        $this->shouldWriteFloat($base + 0x314, 1.0);
        $this->shouldCall('_njSqrt')->with(1.0)->andReturn(1.0);

        $this->shouldWriteFloat($base + 0x2fc, 1.0);
        $this->shouldWriteFloat($base + 0x300, 0.0);
        $this->shouldWriteFloat($base + 0x304, 0.0);

        $denom = $this->f32(1.0 * 4.9);
        $acosfArg = $this->f32(0.0 / $denom);
        // acosf(0.0) mocked to 0.115 rad -> angleInt=1199: /30/2 gives 19,
        // below the 45 minimum, so the clamp applies.
        $this->shouldCall('_acosf')->with($acosfArg)->andReturn(0.115);

        $this->shouldCall('__divls');

        $angleInt = (int) $this->f32($this->f32(0.115 * 65536.0) / 6.283184);
        $turnLimit = intdiv(intdiv($angleInt, 30), 2);
        if ($turnLimit < 45) {
            $turnLimit = 45;
        }
        $angleAfterTurn = $angleInt - $turnLimit;
        $turnStep = $angleInt - $angleAfterTurn;
        // cross = moveZ*headingX - moveX*headingZ = 0*0 - 1*1 = -1.0 < 0: sign flip.
        $turnStep = -$turnStep;

        $this->shouldWriteFloat($groundPt + 0x0, 1.0);
        $this->shouldWriteFloat($groundPt + 0x8, 0.0);
        $this->shouldCall('_njUnitMatrix')->with($matrix);
        $this->shouldCall('_njRotateY')->with($matrix, $turnStep);
        $this->shouldCall('_njCalcPoint')
            ->with($matrix, $groundPt, $base + 0x308)
            ->do(function () {
                $this->memory->writeUInt32($this->registers[6]->value + 0x0, U32::of(fdec(0.5)));
                $this->memory->writeUInt32($this->registers[6]->value + 0x8, U32::of(fdec(0.25)));
            });

        $this->shouldWriteFloat($base + 0x2fc, $this->f32(0.0 + 0.5));
        $this->shouldWriteFloat($base + 0x304, $this->f32(0.0 + 0.25));

        $this->shouldCall('_njTranslateCameraPosition')->with($camera, 0.5, 0.0, 0.25);
        $this->shouldCall('_njPointCameraInterest')->with($camera, 0.0, 0.0, 0.0);
    }
};
