<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// DriveMsgDraw_8c02b388(unused): DrawCallback1 for the drive-message HUD
// banner -- see src/02b2f0_drive_msg.h for the full contract.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_runState_8c2285c4', 0x9c);
        $this->setSize('_var_markTexlist_8c1bc418', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njQuadTextureStart', 4);
        $this->setSize('_njSetQuadTextureG', 4);
        $this->setSize('_njQuadTextureEnd', 4);
        $this->setSize('_SpriteDraw_8c014f54', 4);
    }

    private function initFloat(int $addr, float $value): void {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    private function makeSlot(int $index, int $ids, int $glyphCount, float $x, int $holdFrames): void {
        $base = $this->addressOf('_var_driveMsgQueue_8c228564') + $index * 0x18;
        $this->initUint32($base + 0x04, $ids);
        $this->initFloat($base + 0x08, $x);
        $this->initUint32($base + 0x0c, $glyphCount);
        $this->initUint32($base + 0x14, $holdFrames);
    }

    // var_runState_8c2285c4.runPassed_0x04 != 0: the run is over and passed, so this just
    // draws the run-passed mark and returns -- none of the HUD's own nj*
    // calls or drawMsgGlyphRow_8c02b2f0 happen.
    public function test_runPassed_drawsMarkSpriteOnly(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x04, 1);

        $this->call('_DriveMsgDraw_8c02b388')->with(0);

        // &var_markTexlist_8c1bc418 is the mark ResourceGroup's own address
        // (its first field is the texlist), not a cast of its value.
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $this->addressOf('_var_markTexlist_8c1bc418'), 0x78, 0.0, 0.0, -1.16
        );
    }

    // var_runState_8c2285c4.runPassed_0x04 == 0: draws the HUD normally. Slots 0 and 2 are active
    // (holdFrames != 0); 1 and 3 are idle and skipped. Each active slot's
    // row is drawn 32.0 apart starting at y=192.0, regardless of which
    // slots are skipped in between.
    public function test_normalDraw_drawsActiveSlotsOnly(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x04, 0);

        $texlist = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_markTexlist_8c1bc418'), $texlist);

        $ids0 = $this->alloc(4);
        $ids2 = $this->alloc(4);
        $this->makeSlot(0, $ids0, 3, 10.0, 5);
        $this->makeSlot(1, 0, 0, 0.0, 0); // idle
        $this->makeSlot(2, $ids2, 7, 20.0, 60);
        $this->makeSlot(3, 0, 0, 0.0, 0); // idle

        $this->call('_DriveMsgDraw_8c02b388')->with(0);

        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njQuadTextureStart')->with(1);
        $this->shouldCall('_njSetQuadTextureG')->with(0x0a8d, 0xffffffff);
        $this->shouldCall('_drawMsgGlyphRow_8c02b2f0')->with($ids0, 3, 10.0, 192.0);
        $this->shouldCall('_drawMsgGlyphRow_8c02b2f0')->with($ids2, 7, 20.0, 128.0);
        $this->shouldCall('_njQuadTextureEnd');
    }
};
