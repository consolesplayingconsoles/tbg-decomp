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
 * _BusRenderUpdateMirrorCamera_8c025604(void): rear-view-mirror camera,
 * called last each frame by BusTask_8c022bdc. No-op unless
 * busState.mirror_0x268 is nonzero. Otherwise picks a local mirror-camera
 * offset/interest by mirror_0x268 (1/2/3), rotates BOTH the offset and the
 * interest point into world space by the bus's world matrix (worldMatrix_0x084,
 * two separate njCalcPoint calls), positions the mirror camera (var_mirrorCamera_8c1bb944)
 * at the rotated offset, points its interest at the rotated interest point,
 * rolls it by recent Y waypoint history, activates it, and queues
 * BusRenderDrawBusModel_8c024bb8 on fade layer 1 with the alt light direction.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_var_mirrorCamera_8c1bb944', 0x40);
        $this->setSize('_var_mirrorLightDir_8c227dc4', 0xc);
        $this->setSize('_var_sceneParams_8c18ad24', 4);

        $this->setSize('_njInitCamera', 4);
        $this->setSize('_njSetCameraAngle', 4);
        $this->setSize('_njSetCameraDepth', 4);
        $this->setSize('_njCalcPoint', 4);
        $this->setSize('_njTranslateCameraPosition', 4);
        $this->setSize('_njPointCameraInterest', 4);
        $this->setSize('_atan2f', 4);
        $this->setSize('_njRollCameraInterest', 4);
        $this->setSize('_njSqrt', 4);
        $this->setSize('_njSetCamera', 4);
        $this->setSize('_njCalcVector', 4);
        $this->setSize('_FadePushCall1_8c0223ea', 4);
    }

    private function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }

    private function initFloat(int $address, float $value): void
    {
        $this->initUint32($address, fdec($value));
    }

    private function allocSceneParams(float $dx, float $dy, float $dz): int
    {
        $scene = $this->alloc(0x88);
        $this->initUint32($scene + 0x0, fdec($dx));
        $this->initUint32($scene + 0x4, fdec($dy));
        $this->initUint32($scene + 0x8, fdec($dz));
        return $scene;
    }

    public function test_mirror_disabled_is_noop(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($base + 0x268, 0);

        $this->call('_BusRenderUpdateMirrorCamera_8c025604')->with();
    }

    private function runMode(
        int $mode,
        float $expectOffsetX, float $expectOffsetY, float $expectOffsetZ,
        float $rotatedOffsetX, float $rotatedOffsetY, float $rotatedOffsetZ,
        float $rotatedInterestX, float $rotatedInterestY, float $rotatedInterestZ,
    ): void {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $camera = $this->addressOf('_var_mirrorCamera_8c1bb944');

        $this->initUint32($base + 0x268, $mode);
        $this->initFloat($base + 0x11c, 1.5); // posHistory[2].y
        $this->initFloat($base + 0x128, 4.5); // posHistory[3].y

        $scene = $this->allocSceneParams(0.4, 0.5, 0.6);
        $this->initUint32($this->addressOf('_var_sceneParams_8c18ad24'), $scene);

        $this->call('_BusRenderUpdateMirrorCamera_8c025604')->with();

        $this->shouldCall('_njInitCamera')->with($camera);
        $this->shouldCall('_njSetCameraAngle')->with($camera, 9102);

        $this->shouldWriteFloat($base + 0x318, $expectOffsetX);
        $this->shouldWriteFloat($base + 0x31c, $expectOffsetY);
        $this->shouldWriteFloat($base + 0x320, $expectOffsetZ);

        $this->shouldCall('_njSetCameraDepth')->with($camera, -1.0, -50.0);

        // First njCalcPoint rotates the offset (in place at mirrorWorldOffsetX_0x318).
        $this->shouldCall('_njCalcPoint')
            ->with($base + 0x084, $base + 0x318, $base + 0x318)
            ->do(function () use ($base, $rotatedOffsetX, $rotatedOffsetY, $rotatedOffsetZ) {
                $this->memory->writeUInt32($base + 0x318, U32::of(fdec($rotatedOffsetX)));
                $this->memory->writeUInt32($base + 0x31c, U32::of(fdec($rotatedOffsetY)));
                $this->memory->writeUInt32($base + 0x320, U32::of(fdec($rotatedOffsetZ)));
            });

        // Second njCalcPoint rotates the interest point in place, on the
        // caller's stack -- its src/dst address is a callee-local stack
        // address unknowable ahead of time, so verify it manually (matrix
        // arg, and src==dst) instead of via with().
        $this->shouldCall('_njCalcPoint')->do(function () use ($base, $rotatedInterestX, $rotatedInterestY, $rotatedInterestZ) {
            $matrix = $this->registers[4]->value;
            $src = $this->registers[5]->value;
            $dst = $this->registers[6]->value;
            if ($matrix !== $base + 0x084 || $src !== $dst) {
                throw new \Exception(sprintf(
                    'Unexpected second njCalcPoint args: matrix=0x%x src=0x%x dst=0x%x',
                    $matrix, $src, $dst
                ));
            }
            $this->memory->writeUInt32($dst + 0x0, U32::of(fdec($rotatedInterestX)));
            $this->memory->writeUInt32($dst + 0x4, U32::of(fdec($rotatedInterestY)));
            $this->memory->writeUInt32($dst + 0x8, U32::of(fdec($rotatedInterestZ)));
        });

        $this->shouldCall('_njTranslateCameraPosition')->with($camera, $rotatedOffsetX, $rotatedOffsetY, $rotatedOffsetZ);
        $this->shouldCall('_njPointCameraInterest')->with($camera, $rotatedInterestX, $rotatedInterestY, $rotatedInterestZ);

        $angle = 20.0;
        $this->shouldCall('_atan2f')->with($this->f32(4.5 - 1.5), 2.33)->andReturn($angle);
        $rollAngle = (int) $this->f32($this->f32($angle * 65536.0) / 6.283184);
        $this->shouldCall('_njRollCameraInterest')->with($camera, $rollAngle);

        $dx = $this->f32($rotatedInterestX - $rotatedOffsetX);
        $dz = $this->f32($rotatedInterestZ - $rotatedOffsetZ);
        $this->shouldWriteFloat($base + 0x324, $dx);
        $this->shouldWriteFloat($base + 0x32c, $dz);
        $this->shouldCall('_njSqrt')->with($this->f32($this->f32($dx * $dx) + $this->f32($dz * $dz)))->andReturn(3.7);
        $this->shouldWriteFloat($base + 0x330, 3.7);

        $this->shouldCall('_njSetCamera')->with($camera);

        $this->shouldWriteFloat($this->addressOf('_var_mirrorLightDir_8c227dc4'), 0.4);
        $this->shouldWriteFloat($this->addressOf('_var_mirrorLightDir_8c227dc4') + 4, 0.5);
        $this->shouldWriteFloat($this->addressOf('_var_mirrorLightDir_8c227dc4') + 8, 0.6);
        $this->shouldCall('_njCalcVector')->with(0, $this->addressOf('_var_mirrorLightDir_8c227dc4'), $this->addressOf('_var_mirrorLightDir_8c227dc4'));

        $this->shouldCall('_FadePushCall1_8c0223ea')->with(1, $this->addressOf('_BusRenderDrawBusModel_8c024bb8'), 1);
    }

    public function test_mirror_mode_1(): void
    {
        $this->runMode(1, -1.5, 4.2, -14.2, 10.0, 11.0, 12.0, 20.0, 21.0, 22.0);
    }

    public function test_mirror_mode_2(): void
    {
        $this->runMode(2, 1.5, 4.2, -14.2, -5.0, -6.0, -7.0, -15.0, -16.0, -17.0);
    }

    public function test_mirror_mode_3(): void
    {
        $this->runMode(3, -2.0, 2.2, -5.2, 0.5, 0.25, 0.125, 1.5, 1.25, 1.125);
    }
};
