<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_fadeArrivalGate_8c226560', 4);
        $this->setSize('_var_fadePhase_8c227d7c', 4);
        $this->setSize('_var_fadeRequest_8c226564', 4);
        $this->setSize('_var_fadeProgress_8c227d80', 4);
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_var_fadeCompleteCallback_8c22656c', 4);
        $this->setSize('_var_fadeArrivalVariant_8c22655c', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x268 + 4);
        $this->setSize('_var_fadeCamera_8c226558', 4);
        $this->setSize('_var_8c1bb944', 64);
        $this->setSize('_var_8c1bb904', 64);
        $this->setSize('_var_8c1bb984', 76);
        $this->setSize('_var_mirrorSelect_8c1bbc38', 20);
        $this->setSize('_var_busStopTexlist_8c1bc424', 4);
        $this->setSize('_njUserClipping', 4);
        $this->setSize('_njControl3D', 4);
        $this->setSize('_njSetScreen', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njRenderTextureNumG', 4);
        $this->setSize('_njDrawTexture', 4);
        $this->setSize('_njDrawPolygon', 4);
        $this->setSize('_njSetCamera', 4);
        $this->setSize('_TxtDrawSprite_8c014f54', 4);
    }

    public function test_draws_the_no_mirror_arrival_overlay(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_fadeArrivalGate_8c226560'), 1);
        $this->initUint32($this->addressOf('_var_fadeArrivalVariant_8c22655c'), 0);
        $this->initUint32($this->addressOf('_var_mirrorSelect_8c1bbc38'), 0);
        $this->initUint32($this->addressOf('_var_fadePhase_8c227d7c'), 0);
        $this->initUint32($this->addressOf('_var_fadeRequest_8c226564'), 0);

        $init8c1bb904 = $this->addressOf('_var_8c1bb904');

        $this->call('_FadeUpdate_8c022560');

        $this->shouldCall('_njControl3D')->with(0x100);
        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipMirrorView_8c045578'));
        $this->shouldCall('_njSetScreen')->with($this->addressOf('_init_screenFull_8c0455e8'));
        $this->shouldWriteLong($this->addressOf('_var_fadeCamera_8c226558'), $init8c1bb904);
        $this->shouldCall('_fadeDraw_8c022464')->with(0);
        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipLayer2_8c045598'));
    }
};
