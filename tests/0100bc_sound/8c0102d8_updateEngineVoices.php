<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    /* These three leave var_hudState_8c22643c.engineRpm_0x2c uninitialized and lean on it
     * reading back as zero -- they should set it instead. */
    public function test_engineOffNothingPlays()
    {
        $this->doNotRandomizeMemory();

        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_engineSound_8c0fcd50'), 0);

        $this->singleCall('_updateEngineVoices_8c0102d8')->run();
    }

    public function test_idleLayerStarts()
    {
        $this->doNotRandomizeMemory();

        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_engineSound_8c0fcd50'), 0xf8);
        $this->initUint32($this->addressOf('_var_engineSound_8c0fcd50') + 0x14, 128);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x2e0), 1);

        $this->shouldCall('_sdMidiSetPitch')
            ->with(0xcafe0006, -200, 0);
        $this->shouldCall('_sdMidiSetVol')
            ->with(0xcafe0006, 1, 0);
        $this->shouldCall('_sdMidiPlay')
            ->with(0xcafe0006, 1, 43, 0);

        $this->shouldWriteLongTo('_var_engineSound_8c0fcd50', 1);

        $this->singleCall('_updateEngineVoices_8c0102d8')->run();
    }

    public function test_idleLayerStops()
    {
        $this->doNotRandomizeMemory();

        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_engineSound_8c0fcd50'), 0xf9);
        $this->initUint32($this->addressOf('_var_engineSound_8c0fcd50') + 0x14, 128);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x2e0), 0);

        $this->shouldCall('_sdMidiSetVol')->with(0xcafe0006, -127, 2000);

        $this->shouldWriteLongTo('_var_engineSound_8c0fcd50', 0xf8);

        $this->singleCall('_updateEngineVoices_8c0102d8')->run();
    }

    public function test_runningLayerStarts()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_engineSound_8c0fcd50'), 0xf8);
        $this->initUint32($this->addressOf('_var_engineSound_8c0fcd50') + 0x14, 128);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x2e0), 0);
        $this->initUint32($this->addressOf('_var_hudState_8c22643c') + 0x2c, fdec(400.1));

        $this->shouldCall('_sdMidiSetPitch')
            ->with(0xcafe0007, 0, 0);
        $this->shouldCall('_sdMidiSetVol')
            ->with(0xcafe0007, -127, 0);
        $this->shouldCall('_sdMidiPlay')
            ->with(0xcafe0007, 1, 44, 0);

        $this->shouldWriteLongTo('_var_engineSound_8c0fcd50', 0xfa);

        $this->singleCall('_updateEngineVoices_8c0102d8')->run();
    }

    public function test_runningLayerStops()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_engineSound_8c0fcd50'), 0b10);
        $this->initUint32($this->addressOf('_var_engineSound_8c0fcd50') + 0x14, 128);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x2e0), 0);
        $this->initUint32($this->addressOf('_var_hudState_8c22643c') + 0x2c, fdec(399.9));

        $this->shouldWriteLongTo('_var_engineSound_8c0fcd50', 0);

        $this->singleCall('_updateEngineVoices_8c0102d8')->run();
    }

    public function test_highRevLayerStarts()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_engineSound_8c0fcd50'), 0b010);
        $this->initUint32($this->addressOf('_var_engineSound_8c0fcd50') + 0x14, 128);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x2e0), 0);
        $this->initUint32($this->addressOf('_var_hudState_8c22643c') + 0x2c, fdec(2100.1));

        $this->shouldCall('_sdMidiSetPitch')
            ->with(0xcafe0006, 0, 0);
        $this->shouldCall('_sdMidiSetVol')
            ->with(0xcafe0006, -127, 0);
        $this->shouldCall('_sdMidiPlay')
            ->with(0xcafe0006, 1, 45, 0);

        $this->shouldWriteLongTo('_var_engineSound_8c0fcd50', 0b110);

        $this->singleCall('_updateEngineVoices_8c0102d8')->run();
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_hudState_8c22643c', 0x3c);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);

        // Functions
        $this->setSize('_sdMidiSetPitch', 4);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_sdMidiSetVol', 4);

        // Basic inits
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28') + 0 * 4, 0xcafe0000);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28') + 1 * 4, 0xcafe0001);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28') + 2 * 4, 0xcafe0002);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28') + 3 * 4, 0xcafe0003);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28') + 4 * 4, 0xcafe0004);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28') + 5 * 4, 0xcafe0005);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28') + 6 * 4, 0xcafe0006);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28') + 7 * 4, 0xcafe0007);
    }
};
