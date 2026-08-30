<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _FUN_8c024f32(void): turn-blink state machine driven by var_cameraMode_8c227d9c.
 * 0 -> zeroes busState.cameraYawEase_0x3c8 and returns; 1/4 -> no-op; 2/3 -> scale
 * busState's spawn-stop direction (headingDirX_0x274/0x278, always read/scaled by
 * 18.0/30.0) and accumulate into busState.posX_0x2fc/posZ_0x304; var_8c227df0
 * gets var_8c227de0 when var_8c227da4 != 0, else a fixed 5.0/18.0.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_cameraMode_8c227d9c', 4);
        $this->setSize('_var_8c227da4', 4);
        $this->setSize('_var_8c227de0', 4);
        $this->setSize('_var_8c227df0', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
    }

    private function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }

    private function initFloat(int $address, float $value): void
    {
        $this->initUint32($address, unpack('L', pack('f', $value))[1]);
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }

    /*
     * The asm's two duplicated switch statements make cases 2/3 write
     * var_8c227df0 and busState.posX_0x2fc twice each (identical values);
     * the collapsed C only writes them once. Assert the duplicates only
     * against the asm object.
     */
    private function assertTurnRateWrites(int $base, float $df0, float $posX): void
    {
        $this->shouldWriteFloat($this->addressOf('_var_8c227df0'), $df0);
        $this->shouldWriteFloat($base + 0x2fc, $posX);
        if ($this->isAsmObject()) {
            $this->shouldWriteFloat($this->addressOf('_var_8c227df0'), $df0);
            $this->shouldWriteFloat($base + 0x2fc, $posX);
        }
    }

    public function test_state_zero_clears_and_returns(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 0);
        $base = $this->addressOf('_var_busState_8c1bb9d0');

        $this->call('_FUN_8c024f32')->with();

        $this->shouldWriteLong($base + 0x3c8, 0);
    }

    public function test_state_one_is_noop(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 1);

        $this->call('_FUN_8c024f32')->with();
    }

    public function test_state_four_is_noop(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 4);

        $this->call('_FUN_8c024f32')->with();
    }

    public function test_state_two_flag_zero_uses_constant_rate(): void
    {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 2);
        $this->initUint32($this->addressOf('_var_8c227da4'), 0);
        $this->initFloat($base + 0x274, 0.5);
        $this->initFloat($base + 0x278, -0.25);
        $this->initFloat($base + 0xf4, 10.0);
        $this->initFloat($base + 0xfc, -20.0);

        $this->call('_FUN_8c024f32')->with();

        $dx = $this->f32($this->f32(0.5) * 18.0);
        $dz = $this->f32($this->f32(-0.25) * 18.0);
        $this->assertTurnRateWrites($base, 5.0, $this->f32($this->f32(10.0) + $dx));
        $this->shouldWriteFloat($base + 0x304, $this->f32($this->f32(-20.0) + $dz));
    }

    public function test_state_two_flag_nonzero_scales_by_direction(): void
    {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 2);
        $this->initUint32($this->addressOf('_var_8c227da4'), 1);
        $this->initFloat($base + 0x274, 0.5);
        $this->initFloat($base + 0x278, -0.25);
        $this->initFloat($this->addressOf('_var_8c227de0'), 3.5);
        $this->initFloat($base + 0xf4, 10.0);
        $this->initFloat($base + 0xfc, -20.0);

        $this->call('_FUN_8c024f32')->with();

        $dx = $this->f32($this->f32(0.5) * 18.0);
        $dz = $this->f32($this->f32(-0.25) * 18.0);
        $this->assertTurnRateWrites($base, 3.5, $this->f32($this->f32(10.0) + $dx));
        $this->shouldWriteFloat($base + 0x304, $this->f32($this->f32(-20.0) + $dz));
    }

    public function test_state_three_flag_zero_uses_constant_rate(): void
    {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 3);
        $this->initUint32($this->addressOf('_var_8c227da4'), 0);
        $this->initFloat($base + 0x274, 0.5);
        $this->initFloat($base + 0x278, -0.25);
        $this->initFloat($base + 0xf4, 1.0);
        $this->initFloat($base + 0xfc, 2.0);

        $this->call('_FUN_8c024f32')->with();

        $dx = $this->f32($this->f32(0.5) * 30.0);
        $dz = $this->f32($this->f32(-0.25) * 30.0);
        $this->assertTurnRateWrites($base, 18.0, $this->f32($this->f32(1.0) + $dx));
        $this->shouldWriteFloat($base + 0x304, $this->f32($this->f32(2.0) + $dz));
    }

    public function test_state_three_flag_nonzero_scales_by_direction(): void
    {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 3);
        $this->initUint32($this->addressOf('_var_8c227da4'), 1);
        $this->initFloat($base + 0x274, 0.5);
        $this->initFloat($base + 0x278, -0.25);
        $this->initFloat($this->addressOf('_var_8c227de0'), -1.5);
        $this->initFloat($base + 0xf4, 1.0);
        $this->initFloat($base + 0xfc, 2.0);

        $this->call('_FUN_8c024f32')->with();

        $dx = $this->f32($this->f32(0.5) * 30.0);
        $dz = $this->f32($this->f32(-0.25) * 30.0);
        $this->assertTurnRateWrites($base, -1.5, $this->f32($this->f32(1.0) + $dx));
        $this->shouldWriteFloat($base + 0x304, $this->f32($this->f32(2.0) + $dz));
    }
};
