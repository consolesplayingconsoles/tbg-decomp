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
 * _BusCameraUpdateCamera_8c025078(void): drives the gameplay camera each
 * frame. First eases var_cameraHeight_8c227df0 between var_cameraHeightFrom_8c227dd8 and a target via a
 * quarter-sine ramp over var_cameraHeightPhase_8c227df8, driven by a state machine on
 * var_cameraCueState_8c227da4 (0..3) gated by a scripted cue nibble in
 * busState.markAudioCue_0x3b8's bits 24-27. The Y button cycles the camera mode
 * var_cameraMode_8c227d9c (0..3) when allowed. Then positions/aims the camera per
 * var_cameraMode_8c227d9c (0=fixed follow, 1=bump/sway follow, 2/3=smooth chase via
 * positionCamera_8c024d6c, 4=fixed on var_fixedCameraTarget_8c227d90; modes 0/1 also
 * roll by recent Y history), activates the camera, recomputes the simple
 * light direction, and queues the bus draw callback for modes 0/2/3.
 */
return new class extends TestCase {
    // PDD_DGT_TY (1 << 9), from shinobi/include/sg_pad.h.
    private const PDD_DGT_TY = 0x200;

    private function resolveSymbols(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_var_camera_8c1bb904', 0x40);
        $this->setSize('_var_scratchMatrix_8c1bc46c', 0x40);
        $this->setSize('_var_groundQueryPoint_8c1bc460', 0xc);
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_progress_8c1ba1cc', 0xd8);
        $this->setSize('_var_sceneParams_8c18ad24', 4);

        $this->setSize('_njInitCamera', 4);
        $this->setSize('_njSetCameraAngle', 4);
        $this->setSize('_njSetCameraDepth', 4);
        $this->setSize('_njSin', 4);
        $this->setSize('_njSetMatrix', 4);
        $this->setSize('_njRotateY', 4);
        $this->setSize('_njCalcPoint', 4);
        $this->setSize('_njTranslateCameraPosition', 4);
        $this->setSize('_njPointCameraInterest', 4);
        $this->setSize('_atan2f', 4);
        $this->setSize('_njRollCameraInterest', 4);
        $this->setSize('_njSetCamera', 4);
        $this->setSize('_njCalcVector', 4);
        $this->setSize('_RenderPushCall1_8c0223ea', 4);
        $this->setSize('__divls', 4);
        $this->onCall('__divls', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });

        // Same bytes as $base + 0x2b4 (BusState.driveState_0x2b4); the Y-button
        // gate reads the field through its own section B symbol.
    }

    private function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }

    private function initFloat(int $address, float $value): void
    {
        $this->initUint32($address, fdec($value));
    }

    /** Sets busState.markAudioCue_0x3b8's cue nibble to a raw masked value (e.g. 0x09000000 or 0x05000000). */
    private function setCue(int $base, int $maskedCue): void
    {
        // Only bits 24-27 are read (masked with 0x0F000000); other bits are
        // don't-care to the function, so use the masked value directly.
        $this->initUint32($base + 0x3b8, $maskedCue);
    }

    private function allocSceneParams(float $dx, float $dy, float $dz): int
    {
        $scene = $this->alloc(0x88);
        $this->initUint32($scene + 0x0, fdec($dx));
        $this->initUint32($scene + 0x4, fdec($dy));
        $this->initUint32($scene + 0x8, fdec($dz));
        return $scene;
    }

    /** Seeds the state common to every test: idle mode-transition, no button press. */
    private function seedCommon(int $base): void
    {
        $this->setCue($base, 0);
        $this->initUint32($base + 0x2b4, 0); // driveState_0x2b4: not driving

        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0); // press
        $this->initUint32($this->addressOf('_var_cameraCueBusy_8c227dac'), 0);

        $this->initFloat($this->addressOf('_var_farClipDepth_8c227dd0'), 12.5);
        $this->initUint32($base + 0x078, 0); // pitchAngle_0x078
        $this->initUint32($base + 0x07c, 777); // rollAngle_0x07c
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xc7 + 1, 0); // progress flag clear
        $this->initFloat($this->addressOf('_var_cameraHeight_8c227df0'), 0.0);

        $scene = $this->allocSceneParams(0.1, 0.2, 0.3);
        $this->initUint32($this->addressOf('_var_sceneParams_8c18ad24'), $scene);
    }

    /** Asserts the always-executed camera init + tail (light dir, no draw dispatch). */
    private function expectAlwaysAndTail(bool $expectDraw = false, ?string $drawFn = null): void
    {
        $this->shouldCall('_njInitCamera')->with($this->addressOf('_var_camera_8c1bb904'));
        $this->shouldCall('_njSetCameraAngle')->with($this->addressOf('_var_camera_8c1bb904'), 10194);
        $this->shouldCall('_njSetCameraDepth')->with($this->addressOf('_var_camera_8c1bb904'), -1.0, 12.5);
    }

    private function expectTail(?string $drawFn = null): void
    {
        $this->shouldCall('_njSetCamera')->with($this->addressOf('_var_camera_8c1bb904'));
        $this->shouldWriteFloat($this->addressOf('_var_busSimpleLightDir_8c227db8'), 0.1);
        $this->shouldWriteFloat($this->addressOf('_var_busSimpleLightDir_8c227db8') + 4, 0.2);
        $this->shouldWriteFloat($this->addressOf('_var_busSimpleLightDir_8c227db8') + 8, 0.3);
        $this->shouldCall('_njCalcVector')->with(0, $this->addressOf('_var_busSimpleLightDir_8c227db8'), $this->addressOf('_var_busSimpleLightDir_8c227db8'));
        if ($drawFn !== null) {
            $this->shouldCall('_RenderPushCall1_8c0223ea')->with(0, $this->addressOf($drawFn), 0);
        }
    }

    public function test_idle_transition_and_unknown_mode_skips_positioning(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->seedCommon($base);
        $this->initUint32($this->addressOf('_var_cameraCueState_8c227da4'), 0);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 99); // sentinel: no valid positioning mode

        $this->call('_BusCameraUpdateCamera_8c025078')->with();

        $this->expectAlwaysAndTail();
        $this->expectTail(null);
    }

    public function test_transition_da4_0_cue_starts_ramp_and_positions_mode3(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->seedCommon($base);
        $this->initUint32($this->addressOf('_var_cameraCueState_8c227da4'), 0);
        $this->setCue($base, 0x05000000); // nibble 5
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 3);
        $this->initFloat($this->addressOf('_var_cameraHeight_8c227df0'), 12.0);

        $this->call('_BusCameraUpdateCamera_8c025078')->with();

        $this->shouldWriteFloat($this->addressOf('_var_cameraHeightTo_8c227de0'), 5.0);
        $this->shouldWriteFloat($this->addressOf('_var_cameraHeightFrom_8c227dd8'), 12.0);
        $this->shouldWriteFloat($this->addressOf('_var_cameraHeightDelta_8c227de8'), $this->f32(12.0 - 5.0));
        $this->shouldWriteLong($this->addressOf('_var_cameraHeightPhase_8c227df8'), 0);
        $this->shouldWriteLong($this->addressOf('_var_cameraCueBusy_8c227dac'), 1);
        $this->shouldWriteLong($this->addressOf('_var_cameraCueState_8c227da4'), 1);

        $this->expectAlwaysAndTail();

        $this->shouldCall('_positionCamera_8c024d6c')->with(30.0, 12.0, 2.0);

        $this->expectTail('_BusCameraDrawBusModel_8c024bb8');
    }

    public function test_transition_da4_1_continues_ramp(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->seedCommon($base);
        $this->initUint32($this->addressOf('_var_cameraCueState_8c227da4'), 1);
        $this->initUint32($this->addressOf('_var_cameraHeightPhase_8c227df8'), 0x1000);
        $this->initFloat($this->addressOf('_var_cameraHeightFrom_8c227dd8'), 10.0);
        $this->initFloat($this->addressOf('_var_cameraHeightDelta_8c227de8'), 4.0);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 99);

        $this->call('_BusCameraUpdateCamera_8c025078')->with();

        $this->shouldWriteLong($this->addressOf('_var_cameraHeightPhase_8c227df8'), 0x1000 + 0x222);
        $this->shouldCall('_njSin')->with(0x1000 + 0x222)->andReturn(0.5);
        $this->shouldWriteFloat($this->addressOf('_var_cameraHeight_8c227df0'), $this->f32(10.0 - $this->f32(0.5 * 4.0)));

        $this->expectAlwaysAndTail();
        $this->expectTail(null);
    }

    public function test_transition_da4_1_finishes_ramp(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->seedCommon($base);
        $this->initUint32($this->addressOf('_var_cameraCueState_8c227da4'), 1);
        $this->initUint32($this->addressOf('_var_cameraHeightPhase_8c227df8'), 0x3f00);
        $this->initFloat($this->addressOf('_var_cameraHeightTo_8c227de0'), 20.0);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 99);

        $this->call('_BusCameraUpdateCamera_8c025078')->with();

        $this->shouldWriteLong($this->addressOf('_var_cameraHeightPhase_8c227df8'), 0x3f00 + 0x222);
        $this->shouldWriteFloat($this->addressOf('_var_cameraHeight_8c227df0'), 20.0);
        $this->shouldWriteLong($this->addressOf('_var_cameraCueBusy_8c227dac'), 0);
        $this->shouldWriteLong($this->addressOf('_var_cameraCueState_8c227da4'), 2);

        $this->expectAlwaysAndTail();
        $this->expectTail(null);
    }

    public function test_transition_da4_2_ignores_non_release_cue(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->seedCommon($base);
        $this->initUint32($this->addressOf('_var_cameraCueState_8c227da4'), 2);
        $this->setCue($base, 0); // not exactly 0x09000000
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 99);

        $this->call('_BusCameraUpdateCamera_8c025078')->with();

        $this->expectAlwaysAndTail();
        $this->expectTail(null);
    }

    public function test_transition_da4_2_release_starts_reverse_ramp_and_positions_mode2(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->seedCommon($base);
        $this->initUint32($this->addressOf('_var_cameraCueState_8c227da4'), 2);
        $this->setCue($base, 0x09000000);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 2);
        $this->initFloat($this->addressOf('_var_cameraHeight_8c227df0'), 3.0); // < 5.0

        $this->call('_BusCameraUpdateCamera_8c025078')->with();

        $this->shouldWriteFloat($this->addressOf('_var_cameraHeightTo_8c227de0'), 5.0);
        $this->shouldWriteFloat($this->addressOf('_var_cameraHeightFrom_8c227dd8'), 3.0);
        $this->shouldWriteFloat($this->addressOf('_var_cameraHeightDelta_8c227de8'), $this->f32(5.0 - 3.0));
        $this->shouldWriteLong($this->addressOf('_var_cameraHeightPhase_8c227df8'), 0);
        $this->shouldWriteLong($this->addressOf('_var_cameraCueBusy_8c227dac'), 1);
        $this->shouldWriteLong($this->addressOf('_var_cameraCueState_8c227da4'), 3);

        $this->expectAlwaysAndTail();

        $this->shouldCall('_positionCamera_8c024d6c')->with(18.0, 3.0, 0.5);

        $this->expectTail('_BusCameraDrawBusModel_8c024bb8');
    }

    public function test_transition_da4_3_continues_ramp(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->seedCommon($base);
        $this->initUint32($this->addressOf('_var_cameraCueState_8c227da4'), 3);
        $this->initUint32($this->addressOf('_var_cameraHeightPhase_8c227df8'), 0x2000);
        $this->initFloat($this->addressOf('_var_cameraHeightFrom_8c227dd8'), 6.0);
        $this->initFloat($this->addressOf('_var_cameraHeightDelta_8c227de8'), 2.0);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 99);

        $this->call('_BusCameraUpdateCamera_8c025078')->with();

        $this->shouldWriteLong($this->addressOf('_var_cameraHeightPhase_8c227df8'), 0x2000 + 0x222);
        $this->shouldCall('_njSin')->with(0x2000 + 0x222)->andReturn(0.25);
        $this->shouldWriteFloat($this->addressOf('_var_cameraHeight_8c227df0'), $this->f32(6.0 + $this->f32(0.25 * 2.0)));

        $this->expectAlwaysAndTail();
        $this->expectTail(null);
    }

    public function test_transition_da4_3_finishes_ramp(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->seedCommon($base);
        $this->initUint32($this->addressOf('_var_cameraCueState_8c227da4'), 3);
        $this->initUint32($this->addressOf('_var_cameraHeightPhase_8c227df8'), 0x3f00);
        $this->initFloat($this->addressOf('_var_cameraHeightTo_8c227de0'), 8.0);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 99);

        $this->call('_BusCameraUpdateCamera_8c025078')->with();

        $this->shouldWriteLong($this->addressOf('_var_cameraHeightPhase_8c227df8'), 0x3f00 + 0x222);
        $this->shouldWriteFloat($this->addressOf('_var_cameraHeight_8c227df0'), 8.0);
        $this->shouldWriteLong($this->addressOf('_var_cameraCueBusy_8c227dac'), 0);
        $this->shouldWriteLong($this->addressOf('_var_cameraCueState_8c227da4'), 0);

        $this->expectAlwaysAndTail();
        $this->expectTail(null);
    }

    public function test_y_button_advances_mode_and_calls_blink_update(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->seedCommon($base);
        $this->initUint32($this->addressOf('_var_cameraCueState_8c227da4'), 0);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, self::PDD_DGT_TY);
        $this->initUint32($base + 0x2b4, 1); // driveState_0x2b4: driving
        $this->initUint32($this->addressOf('_var_cameraCueBusy_8c227dac'), 0);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 1);

        $this->call('_BusCameraUpdateCamera_8c025078')->with();

        $this->shouldWriteLong($this->addressOf('_var_cameraMode_8c227d9c'), 2);
        $this->shouldCall('_BusCameraApplyCameraMode_8c024f32');

        $this->expectAlwaysAndTail();

        // Positioning for the new mode (2) follows.
        $this->shouldCall('_positionCamera_8c024d6c')->with(18.0, 0.0, 0.5);

        $this->expectTail('_BusCameraDrawBusModel_8c024bb8');
    }

    public function test_y_button_wraps_mode_to_zero(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->seedCommon($base);
        $this->initUint32($this->addressOf('_var_cameraCueState_8c227da4'), 0);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, self::PDD_DGT_TY);
        $this->initUint32($base + 0x2b4, 1); // driveState_0x2b4: driving
        $this->initUint32($this->addressOf('_var_cameraCueBusy_8c227dac'), 0);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 3);

        $this->initFloat($base + 0x0f4, 5.0);
        $this->initFloat($base + 0x0f8, 6.0);
        $this->initFloat($base + 0x0fc, 7.0);
        $this->initUint32($base + 0x258, 0); // target = 0
        $this->initUint32($base + 0x3c8, 0); // cameraYawEase_0x3c8 = 0, in-range target: no clamp step
        $this->initFloat($base + 0x11c, 0.0); // posHistory[2].y
        $this->initFloat($base + 0x128, 0.0); // posHistory[3].y

        $this->call('_BusCameraUpdateCamera_8c025078')->with();

        $this->shouldWriteLong($this->addressOf('_var_cameraMode_8c227d9c'), 4);
        $this->shouldWriteLong($this->addressOf('_var_cameraMode_8c227d9c'), 0);
        $this->shouldCall('_BusCameraApplyCameraMode_8c024f32');

        $this->expectAlwaysAndTail();

        $this->shouldWriteFloat($base + 0x2fc, 5.0);
        $this->shouldWriteFloat($base + 0x304, 7.0);
        $this->shouldWriteFloat($base + 0x300, $this->f32(6.0 + 0.0 + 2.0));

        $this->shouldCall('__divls');
        $this->shouldCall('_njSetMatrix')->with($this->addressOf('_var_scratchMatrix_8c1bc46c'), $base + 0x084);
        $this->shouldCall('_njRotateY')->with($this->addressOf('_var_scratchMatrix_8c1bc46c'), 0);
        $this->shouldWriteFloat($this->addressOf('_var_groundQueryPoint_8c1bc460'), 0.0);
        $this->shouldWriteFloat($this->addressOf('_var_groundQueryPoint_8c1bc460') + 4, 2.0);
        $this->shouldWriteFloat($this->addressOf('_var_groundQueryPoint_8c1bc460') + 8, -1.0);
        $groundPt = $this->addressOf('_var_groundQueryPoint_8c1bc460');
        $this->shouldCall('_njCalcPoint')
            ->with($this->addressOf('_var_scratchMatrix_8c1bc46c'), $groundPt, $groundPt)
            ->do(function () use ($groundPt) {
                $this->memory->writeUInt32($groundPt + 0x0, U32::of(fdec(1.0)));
                $this->memory->writeUInt32($groundPt + 0x4, U32::of(fdec(2.0)));
                $this->memory->writeUInt32($groundPt + 0x8, U32::of(fdec(3.0)));
            });
        $this->shouldWriteFloat($base + 0x308, $this->f32(5.0 - 1.0));
        $this->shouldWriteFloat($base + 0x30c, $this->f32(6.0 - 2.0));
        $this->shouldWriteFloat($base + 0x310, $this->f32(7.0 - 3.0));
        $this->shouldWriteFloat($base + 0x314, 1.0);

        $this->shouldCall('_njTranslateCameraPosition')->with($this->addressOf('_var_camera_8c1bb904'), 5.0, $this->f32(6.0 + 2.0), 7.0);
        $this->shouldCall('_njPointCameraInterest')->with($this->addressOf('_var_camera_8c1bb904'), 1.0, 2.0, 3.0);

        $angle = 90.0;
        $this->shouldCall('_atan2f')->with(0.0, 2.33)->andReturn($angle);
        $rollAngle = (int) $this->f32($this->f32($angle * 65536.0) / 6.283184) + 777;
        $this->shouldCall('_njRollCameraInterest')->with($this->addressOf('_var_camera_8c1bb904'), $rollAngle);

        $this->expectTail('_drawFrontBusModel_8c024cc8');
    }

    public function test_y_button_press_blocked_when_ramping_and_mode_two_or_higher(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->seedCommon($base);
        $this->initUint32($this->addressOf('_var_cameraCueState_8c227da4'), 0);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, self::PDD_DGT_TY);
        $this->initUint32($base + 0x2b4, 1); // driveState_0x2b4: driving
        $this->initUint32($this->addressOf('_var_cameraCueBusy_8c227dac'), 1); // ramping
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 2); // >= 2

        $this->call('_BusCameraUpdateCamera_8c025078')->with();

        // No advance, no BusCameraApplyCameraMode_8c024f32 call.
        $this->expectAlwaysAndTail();

        $this->shouldCall('_positionCamera_8c024d6c')->with(18.0, 0.0, 0.5);

        $this->expectTail('_BusCameraDrawBusModel_8c024bb8');
    }

    public function test_positioning_mode0_progress_flag_clear(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->seedCommon($base);
        $this->initUint32($this->addressOf('_var_cameraCueState_8c227da4'), 0);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 0);
        $this->initUint32($base + 0x078, 0); // pitchAngle_0x078
        $this->initUint32($base + 0x07c, 321); // rollAngle_0x07c
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xc7 + 1, 0); // progress flag clear

        $this->initFloat($base + 0x0f4, 1.0);
        $this->initFloat($base + 0x0f8, 2.0);
        $this->initFloat($base + 0x0fc, 3.0);
        $this->initUint32($base + 0x258, 300); // target = 100
        $this->initUint32($base + 0x3c8, 10); // below target: += 60 branch
        $this->initFloat($base + 0x11c, 0.0); // posHistory[2].y
        $this->initFloat($base + 0x128, 0.0); // posHistory[3].y

        $this->call('_BusCameraUpdateCamera_8c025078')->with();

        $this->expectAlwaysAndTail();

        $this->shouldWriteFloat($base + 0x2fc, 1.0);
        $this->shouldWriteFloat($base + 0x304, 3.0);
        $this->shouldWriteFloat($base + 0x300, $this->f32(2.0 + 2.0));

        $this->shouldCall('__divls');
        $this->shouldWriteLong($base + 0x3c8, 70); // 10 + 60, below 100 target: no clamp

        $this->shouldCall('_njSetMatrix')->with($this->addressOf('_var_scratchMatrix_8c1bc46c'), $base + 0x084);
        $this->shouldCall('_njRotateY')->with($this->addressOf('_var_scratchMatrix_8c1bc46c'), 70);
        $this->shouldWriteFloat($this->addressOf('_var_groundQueryPoint_8c1bc460'), 0.0);
        $this->shouldWriteFloat($this->addressOf('_var_groundQueryPoint_8c1bc460') + 4, 2.0);
        $this->shouldWriteFloat($this->addressOf('_var_groundQueryPoint_8c1bc460') + 8, -1.0);
        $groundPt = $this->addressOf('_var_groundQueryPoint_8c1bc460');
        $this->shouldCall('_njCalcPoint')
            ->with($this->addressOf('_var_scratchMatrix_8c1bc46c'), $groundPt, $groundPt)
            ->do(function () use ($groundPt) {
                $this->memory->writeUInt32($groundPt + 0x0, U32::of(fdec(0.5)));
                $this->memory->writeUInt32($groundPt + 0x4, U32::of(fdec(0.6)));
                $this->memory->writeUInt32($groundPt + 0x8, U32::of(fdec(0.7)));
            });
        $this->shouldWriteFloat($base + 0x308, $this->f32(1.0 - 0.5));
        $this->shouldWriteFloat($base + 0x30c, $this->f32(2.0 - 0.6));
        $this->shouldWriteFloat($base + 0x310, $this->f32(3.0 - 0.7));
        $this->shouldWriteFloat($base + 0x314, 1.0);

        $this->shouldCall('_njTranslateCameraPosition')->with($this->addressOf('_var_camera_8c1bb904'), 1.0, $this->f32(2.0 + 2.0), 3.0);
        $this->shouldCall('_njPointCameraInterest')->with($this->addressOf('_var_camera_8c1bb904'), 0.5, 0.6, 0.7);

        $angle = 45.0;
        $this->shouldCall('_atan2f')->with(0.0, 2.33)->andReturn($angle);
        $rollAngle = (int) $this->f32($this->f32($angle * 65536.0) / 6.283184) + 321;
        $this->shouldCall('_njRollCameraInterest')->with($this->addressOf('_var_camera_8c1bb904'), $rollAngle);

        $this->expectTail('_drawFrontBusModel_8c024cc8');
    }

    public function test_positioning_mode0_progress_flag_set_overrides_ang_and_turn(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->seedCommon($base);
        $this->initUint32($this->addressOf('_var_cameraCueState_8c227da4'), 0);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 0);
        $this->initUint32($base + 0x078, 0); // pitchAngle_0x078
        $this->initUint32($base + 0x07c, 999); // rollAngle_0x07c (overridden to 0)
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xc7 + 1, 1); // progress flag set

        $this->initFloat($base + 0x0f4, 1.0);
        $this->initFloat($base + 0x0f8, 2.0);
        $this->initFloat($base + 0x0fc, 3.0);
        $this->initUint32($base + 0x258, 600); // target = 200
        $this->initUint32($base + 0x3c8, 400); // above target: -= 60 branch
        $this->initFloat($base + 0x11c, 0.0); // posHistory[2].y
        $this->initFloat($base + 0x128, 0.0); // posHistory[3].y

        $this->call('_BusCameraUpdateCamera_8c025078')->with();

        $this->expectAlwaysAndTail();

        $this->shouldWriteFloat($base + 0x2fc, 1.0);
        $this->shouldWriteFloat($base + 0x304, 3.0);
        // turnFactor forced to 0.0 by the progress flag.
        $this->shouldWriteFloat($base + 0x300, $this->f32(2.0 + 0.0 + 2.0));

        $this->shouldCall('__divls');
        $this->shouldWriteLong($base + 0x3c8, 340); // 400 - 60, above 200 target: no clamp

        $this->shouldCall('_njSetMatrix')->with($this->addressOf('_var_scratchMatrix_8c1bc46c'), $base + 0x084);
        $this->shouldCall('_njRotateY')->with($this->addressOf('_var_scratchMatrix_8c1bc46c'), 340);
        $groundPt = $this->addressOf('_var_groundQueryPoint_8c1bc460');
        $this->shouldWriteFloat($groundPt, 0.0);
        $this->shouldWriteFloat($groundPt + 4, 2.0);
        $this->shouldWriteFloat($groundPt + 8, -1.0);
        $this->shouldCall('_njCalcPoint')
            ->with($this->addressOf('_var_scratchMatrix_8c1bc46c'), $groundPt, $groundPt)
            ->do(function () use ($groundPt) {
                $this->memory->writeUInt32($groundPt + 0x0, U32::of(fdec(0.1)));
                $this->memory->writeUInt32($groundPt + 0x4, U32::of(fdec(0.2)));
                $this->memory->writeUInt32($groundPt + 0x8, U32::of(fdec(0.3)));
            });
        $this->shouldWriteFloat($base + 0x308, $this->f32(1.0 - 0.1));
        $this->shouldWriteFloat($base + 0x30c, $this->f32(2.0 - 0.2));
        $this->shouldWriteFloat($base + 0x310, $this->f32(3.0 - 0.3));
        $this->shouldWriteFloat($base + 0x314, 1.0);

        $this->shouldCall('_njTranslateCameraPosition')->with($this->addressOf('_var_camera_8c1bb904'), 1.0, $this->f32(2.0 + 2.0), 3.0);
        $this->shouldCall('_njPointCameraInterest')->with($this->addressOf('_var_camera_8c1bb904'), 0.1, 0.2, 0.3);

        $angle = -30.0;
        $this->shouldCall('_atan2f')->with(0.0, 2.33)->andReturn($angle);
        // ang forced to 0 by the progress flag.
        $rollAngle = (int) $this->f32($this->f32($angle * 65536.0) / 6.283184) + 0;
        $this->shouldCall('_njRollCameraInterest')->with($this->addressOf('_var_camera_8c1bb904'), $rollAngle);

        $this->expectTail('_drawFrontBusModel_8c024cc8');
    }

    public function test_positioning_mode1_bump_sway_follow(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->seedCommon($base);
        $this->initUint32($this->addressOf('_var_cameraCueState_8c227da4'), 0);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 1);
        $this->initUint32($base + 0x078, 0);
        $this->initUint32($base + 0x07c, 42);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xc7 + 1, 0);

        $this->initFloat($base + 0x0f4, 10.0);
        $this->initFloat($base + 0x0f8, 20.0);
        $this->initFloat($base + 0x0fc, 30.0);
        $this->initFloat($base + 0x274, 2.0);
        $this->initFloat($base + 0x278, -3.0);
        $this->initFloat($base + 0x26c, 4.0);
        $this->initFloat($base + 0x270, 8.0);
        $this->initFloat($base + 0x11c, 0.0); // posHistory[2].y
        $this->initFloat($base + 0x128, 0.0); // posHistory[3].y

        $this->call('_BusCameraUpdateCamera_8c025078')->with();

        $this->expectAlwaysAndTail();

        $this->shouldWriteFloat($base + 0x308, 2.0);
        $this->shouldWriteFloat($base + 0x310, -3.0);
        $this->shouldWriteFloat($base + 0x314, 2.0);
        $this->shouldWriteFloat($base + 0x2fc, $this->f32(10.0 + 2.0));
        $this->shouldWriteFloat($base + 0x304, $this->f32(30.0 + -3.0));

        $ratio = $this->f32(4.0 / 8.0);
        $posY = $this->f32($this->f32(2.0 * $ratio) + 20.0 + 0.0 + 2.0);
        $this->shouldWriteFloat($base + 0x300, $posY);

        $this->shouldCall('_njTranslateCameraPosition')->with($this->addressOf('_var_camera_8c1bb904'), $this->f32(10.0 + 2.0), $posY, $this->f32(30.0 + -3.0));
        $this->shouldCall('_njPointCameraInterest')->with($this->addressOf('_var_camera_8c1bb904'), 10.0, $this->f32(20.0 + 2.0), 30.0);

        $angle = 12.0;
        $this->shouldCall('_atan2f')->with(0.0, 2.33)->andReturn($angle);
        $rollAngle = (int) $this->f32($this->f32($angle * 65536.0) / 6.283184) + 42;
        $this->shouldCall('_njRollCameraInterest')->with($this->addressOf('_var_camera_8c1bb904'), $rollAngle);

        $this->expectTail(null);
    }

    public function test_positioning_mode4_fixed_interest_point(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->seedCommon($base);
        $this->initUint32($this->addressOf('_var_cameraCueState_8c227da4'), 0);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 4);

        $this->initFloat($base + 0x2fc, 50.0);
        $this->initFloat($base + 0x300, 60.0);
        $this->initFloat($base + 0x304, 70.0);

        $d90 = $this->addressOf('_var_fixedCameraTarget_8c227d90');
        $this->initFloat($d90 + 0x0, 5.0);
        $this->initFloat($d90 + 0x4, 6.0);
        $this->initFloat($d90 + 0x8, 7.0);

        $this->call('_BusCameraUpdateCamera_8c025078')->with();

        $this->expectAlwaysAndTail();

        $this->shouldWriteFloat($base + 0x308, $this->f32(50.0 - 5.0));
        $this->shouldWriteFloat($base + 0x310, $this->f32(70.0 - 7.0));

        $this->shouldCall('_njTranslateCameraPosition')->with($this->addressOf('_var_camera_8c1bb904'), 50.0, 60.0, 70.0);
        $this->shouldCall('_njPointCameraInterest')->with($this->addressOf('_var_camera_8c1bb904'), 5.0, 6.0, 7.0);

        // No roll for mode 4.
        $this->expectTail(null);
    }
};
