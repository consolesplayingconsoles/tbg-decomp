<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

/*
 * _BusCameraSaveCameraState_8c024b4c(void): shifts 7 prev/current value pairs one frame forward
 * -- var_cameraMode_8c227d9c->da0 and var_cameraCueState_8c227da4->da8 (int copies), and 5 float pairs
 * dd8->ddc, de0->de4, de8->dec, df0->df4, df8->dfc.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
    }

    public function test(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 5);
        $this->initUint32($this->addressOf('_var_savedCameraMode_8c227da0'), 0);
        $this->initUint32($this->addressOf('_var_cameraCueState_8c227da4'), 7);
        $this->initUint32($this->addressOf('_var_savedCameraCueState_8c227da8'), 0);
        $this->initUint32($this->addressOf('_var_cameraHeightFrom_8c227dd8'), fdec(1.5));
        $this->initUint32($this->addressOf('_var_savedCameraHeightFrom_8c227ddc'), fdec(0.0));
        $this->initUint32($this->addressOf('_var_cameraHeightTo_8c227de0'), fdec(2.5));
        $this->initUint32($this->addressOf('_var_savedCameraHeightTo_8c227de4'), fdec(0.0));
        $this->initUint32($this->addressOf('_var_cameraHeightDelta_8c227de8'), fdec(-3.5));
        $this->initUint32($this->addressOf('_var_savedCameraHeightDelta_8c227dec'), fdec(0.0));
        $this->initUint32($this->addressOf('_var_cameraHeight_8c227df0'), fdec(4.5));
        $this->initUint32($this->addressOf('_var_savedCameraHeight_8c227df4'), fdec(0.0));
        $this->initUint32($this->addressOf('_var_cameraHeightPhase_8c227df8'), fdec(-5.5));
        $this->initUint32($this->addressOf('_var_savedCameraHeightPhase_8c227dfc'), fdec(0.0));

        $this->call('_BusCameraSaveCameraState_8c024b4c')->with();

        $this->shouldWriteLong($this->addressOf('_var_savedCameraMode_8c227da0'), 5);
        $this->shouldWriteLong($this->addressOf('_var_savedCameraCueState_8c227da8'), 7);
        $this->shouldWriteFloat($this->addressOf('_var_savedCameraHeightFrom_8c227ddc'), 1.5);
        $this->shouldWriteFloat($this->addressOf('_var_savedCameraHeightTo_8c227de4'), 2.5);
        $this->shouldWriteFloat($this->addressOf('_var_savedCameraHeightDelta_8c227dec'), -3.5);
        $this->shouldWriteFloat($this->addressOf('_var_savedCameraHeight_8c227df4'), 4.5);
        $this->shouldWriteFloat($this->addressOf('_var_savedCameraHeightPhase_8c227dfc'), -5.5);
    }
};
