<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

/*
 * _BusCameraRestoreCameraState_8c024b86(void): reverse-direction mirror of _BusCameraSaveCameraState_8c024b4c -- shifts
 * 6 of the 7 prev/current pairs backward (var_cameraHeightPhase_8c227df8/dfc untouched), then
 * tail-calls _BusCameraApplyCameraMode_8c024f32.
 *
 * _BusCameraApplyCameraMode_8c024f32 is defined in this same unit. The asm object still traps
 * the BSR and requires an explicit shouldCall(); the C object's direct call
 * to a locally-defined function is not interceptable, so it genuinely runs
 * -- var_savedCameraMode_8c227da0 is seeded with 1 so that after the shift var_cameraMode_8c227d9c ==
 * 1, _BusCameraApplyCameraMode_8c024f32's no-op state (its own behavior is covered by
 * 8c024f32_FUN.php).
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
    }

    protected function isAsmObject(): bool
    {
        return str_contains($this->objectFile, '/asm/');
    }

    public function test(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 0);
        $this->initUint32($this->addressOf('_var_savedCameraMode_8c227da0'), 1);
        $this->initUint32($this->addressOf('_var_cameraCueState_8c227da4'), 0);
        $this->initUint32($this->addressOf('_var_savedCameraCueState_8c227da8'), 7);
        $this->initUint32($this->addressOf('_var_cameraHeightFrom_8c227dd8'), fdec(0.0));
        $this->initUint32($this->addressOf('_var_savedCameraHeightFrom_8c227ddc'), fdec(1.5));
        $this->initUint32($this->addressOf('_var_cameraHeightTo_8c227de0'), fdec(0.0));
        $this->initUint32($this->addressOf('_var_savedCameraHeightTo_8c227de4'), fdec(2.5));
        $this->initUint32($this->addressOf('_var_cameraHeightDelta_8c227de8'), fdec(0.0));
        $this->initUint32($this->addressOf('_var_savedCameraHeightDelta_8c227dec'), fdec(-3.5));
        $this->initUint32($this->addressOf('_var_cameraHeight_8c227df0'), fdec(0.0));
        $this->initUint32($this->addressOf('_var_savedCameraHeight_8c227df4'), fdec(4.5));
        $this->initUint32($this->addressOf('_var_cameraHeightPhase_8c227df8'), fdec(-5.5));
        $this->initUint32($this->addressOf('_var_savedCameraHeightPhase_8c227dfc'), fdec(0.0));

        $this->call('_BusCameraRestoreCameraState_8c024b86')->with();

        $this->shouldWriteLong($this->addressOf('_var_cameraMode_8c227d9c'), 1);
        $this->shouldWriteLong($this->addressOf('_var_cameraCueState_8c227da4'), 7);
        $this->shouldWriteFloat($this->addressOf('_var_cameraHeightFrom_8c227dd8'), 1.5);
        $this->shouldWriteFloat($this->addressOf('_var_cameraHeightTo_8c227de0'), 2.5);
        $this->shouldWriteFloat($this->addressOf('_var_cameraHeightDelta_8c227de8'), -3.5);
        $this->shouldWriteFloat($this->addressOf('_var_cameraHeight_8c227df0'), 4.5);

        if ($this->isAsmObject()) {
            $this->shouldCall('_BusCameraApplyCameraMode_8c024f32');
        }
    }
};
