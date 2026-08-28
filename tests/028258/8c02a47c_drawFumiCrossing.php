<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // State offsets, matching fumiCrossingTask_8c02a4f8's test.
    const ST_TRAIN_TEXLIST = 0x40;
    const ST_TRAIN_MODEL = 0x44;
    const ST_TRAIN_MOTION = 0x48;
    const ST_PHASE = 0x54;
    const ST_FRAME = 0x58; // counter (phases 1 and 3), the gate's draw frame
    const ST_TRAIN_FRAME = 0x60; // counter (phase 2), the train's draw frame
    const ST_TRAIN_THRESHOLD = 0x64; // >0 while a passing train is still due

    private function f(float $value): int
    {
        return unpack('L', pack('f', $value))[1];
    }

    private function initFloat(int $address, float $value): void
    {
        $this->initUint32($address, $this->f($value));
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njCnkSimpleDrawMotion', 4);
        $this->setSize('_njCnkSimpleDrawObject', 4);
    }

    /** Allocates a state buffer with the fields drawFumiCrossing_8c02a47c reads,
     * and points the gate/lamp/train globals at fresh handles. Returns
     * [state, gateTexlist, gateModel, closingMotion, openingMotion,
     * lampTexlist, lampModel, trainTexlist]. */
    private function setup(
        int $phase,
        float $frame,
        float $trainThreshold,
        float $trainFrame = 0.0
    ): array
    {
        $gateTexlist = $this->alloc(4);
        $gateModel = $this->alloc(4);
        $closingMotion = $this->alloc(4);
        $openingMotion = $this->alloc(4);
        $lampTexlist = $this->alloc(4);
        $lampModel = $this->alloc(4);
        $trainTexlist = $this->alloc(4);

        $this->initUint32($this->addressOf('_var_fumiTexlist_8c228410'), $gateTexlist);
        $this->initUint32($this->addressOf('_var_fumiGateModel_8c22840c'), $gateModel);
        $this->initUint32($this->addressOf('_var_fumiCloseMotion_8c228414'), $closingMotion);
        $this->initUint32($this->addressOf('_var_fumiOpenMotion_8c228418'), $openingMotion);
        $this->initUint32($this->addressOf('_var_fumiLampTexlist_8c228420'), $lampTexlist);
        $this->initUint32($this->addressOf('_var_fumiLampModel_8c22841c'), $lampModel);
        $this->initUint32($this->addressOf('_var_fumiTrainTexlist_8c228428'), $trainTexlist);

        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_PHASE, $phase);
        $this->initFloat($state + self::ST_FRAME, $frame);
        $this->initFloat($state + self::ST_TRAIN_FRAME, $trainFrame);
        $this->initFloat($state + self::ST_TRAIN_THRESHOLD, $trainThreshold);

        return [$state, $gateTexlist, $gateModel, $closingMotion, $openingMotion,
                $lampTexlist, $lampModel, $trainTexlist];
    }

    public function test_phase0_draws_closing_motion_and_lamp_no_train(): void
    {
        $this->resolveSymbols();

        [$state, $gateTexlist, $gateModel, $closingMotion, , $lampTexlist, $lampModel] =
            $this->setup(0, 0.0, 0.0);

        $this->call('_drawFumiCrossing_8c02a47c')->with($state);

        $this->shouldCall('_njSetTexture')->with($gateTexlist);
        $this->shouldCall('_njCnkSimpleDrawMotion')->with($gateModel, $closingMotion, 0.0);
        $this->shouldCall('_njSetTexture')->with($lampTexlist);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($lampModel);
    }

    public function test_phase1_draws_closing_motion_at_current_frame(): void
    {
        $this->resolveSymbols();

        [$state, $gateTexlist, $gateModel, $closingMotion, , $lampTexlist, $lampModel] =
            $this->setup(1, 3.0, 0.0);

        $this->call('_drawFumiCrossing_8c02a47c')->with($state);

        $this->shouldCall('_njSetTexture')->with($gateTexlist);
        $this->shouldCall('_njCnkSimpleDrawMotion')->with($gateModel, $closingMotion, 3.0);
        $this->shouldCall('_njSetTexture')->with($lampTexlist);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($lampModel);
    }

    public function test_phase2_draws_opening_motion_and_train(): void
    {
        $this->resolveSymbols();

        [$state, $gateTexlist, $gateModel, , $openingMotion, $lampTexlist, $lampModel, $trainTexlist] =
            $this->setup(2, 4.0, 1.0, 2.0);

        $trainModel = $this->alloc(4);
        $trainMotion = $this->alloc(4);
        $this->initUint32($state + self::ST_TRAIN_MODEL, $trainModel);
        $this->initUint32($state + self::ST_TRAIN_MOTION, $trainMotion);

        $this->call('_drawFumiCrossing_8c02a47c')->with($state);

        $this->shouldCall('_njSetTexture')->with($gateTexlist);
        $this->shouldCall('_njCnkSimpleDrawMotion')->with($gateModel, $openingMotion, 4.0);
        $this->shouldCall('_njSetTexture')->with($lampTexlist);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($lampModel);
        $this->shouldCall('_njSetTexture')->with($trainTexlist);
        $this->shouldCall('_njCnkSimpleDrawMotion')->with($trainModel, $trainMotion, 2.0);
    }

    public function test_phase3_draws_opening_motion_no_train(): void
    {
        $this->resolveSymbols();

        [$state, $gateTexlist, $gateModel, , $openingMotion, $lampTexlist, $lampModel] =
            $this->setup(3, 5.0, 0.0);

        $this->call('_drawFumiCrossing_8c02a47c')->with($state);

        $this->shouldCall('_njSetTexture')->with($gateTexlist);
        $this->shouldCall('_njCnkSimpleDrawMotion')->with($gateModel, $openingMotion, 5.0);
        $this->shouldCall('_njSetTexture')->with($lampTexlist);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($lampModel);
    }

    public function test_phase4_draws_opening_motion_frozen_no_train(): void
    {
        $this->resolveSymbols();

        [$state, $gateTexlist, $gateModel, , $openingMotion, $lampTexlist, $lampModel] =
            $this->setup(4, 8.0, 0.0);

        $this->call('_drawFumiCrossing_8c02a47c')->with($state);

        $this->shouldCall('_njSetTexture')->with($gateTexlist);
        $this->shouldCall('_njCnkSimpleDrawMotion')->with($gateModel, $openingMotion, 8.0);
        $this->shouldCall('_njSetTexture')->with($lampTexlist);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($lampModel);
    }

    public function test_phase5_skips_gate_motion_still_draws_lamp(): void
    {
        $this->resolveSymbols();

        [$state, $gateTexlist, , , , $lampTexlist, $lampModel] =
            $this->setup(5, 0.0, 0.0);

        $this->call('_drawFumiCrossing_8c02a47c')->with($state);

        $this->shouldCall('_njSetTexture')->with($gateTexlist);
        $this->shouldCall('_njSetTexture')->with($lampTexlist);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($lampModel);
    }
};
