<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

// Draws the VM SELECT screen sprites from var_resourceGroup + menuState groups.
return new class extends TestCase {
    private function seedSlots(array $statuses): void
    {
        $base = $this->addressOf('_var_vmuStatus_8c226048');
        for ($i = 0; $i < 8; $i++) {
            $this->initUint32($base + $i * 4, $statuses[$i] ?? 0);
        }
    }

    private function seedState(int $state): void
    {
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, $state);
    }

    private function expectFixedDraws(): void
    {
        $res = $this->addressOf('_var_resourceGroup_8c2263a8');
        $ms = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldCall('_SpriteDraw_8c014f54')->with($res, 0, 0.0, 0.0, -6.0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($ms, 1, 0.0, 0.0, -4.3);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($ms, 0, 0.0, 0.0, -7.0);
    }

    public function test_early_phase_no_cursor_no_slots()
    {
        $this->resolveSymbols();
        $this->seedState(2); // < 5: no cursor highlight
        $this->seedSlots([]); // all empty
        $this->call('_drawSelectScreen_8c01be90')->with();
        $this->expectFixedDraws();
    }

    public function test_cursor_and_slots()
    {
        $this->resolveSymbols();
        $this->seedState(5); // >= 5 and != 9: cursor drawn
        $ms = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($ms + 0x20, fdec(120.0)); // cursor.x
        $this->initUint32($ms + 0x24, fdec(60.0));  // cursor.y
        $this->seedSlots([1, 0, 3, 0, 0, 0, 0, 5]); // slots 0,2,7 mounted

        $res = $this->addressOf('_var_resourceGroup_8c2263a8');
        $this->call('_drawSelectScreen_8c01be90')->with();
        $this->shouldCall('_SpriteDraw_8c014f54')->with($res, 9, 120.0, 60.0, -4.0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($res, 1, 0.0, 0.0, -5.0); // slot 0
        $this->shouldCall('_SpriteDraw_8c014f54')->with($res, 3, 0.0, 0.0, -5.0); // slot 2
        $this->shouldCall('_SpriteDraw_8c014f54')->with($res, 8, 0.0, 0.0, -5.0); // slot 7
        $this->expectFixedDraws();
    }

    public function test_phase_9_skips_cursor()
    {
        $this->resolveSymbols();
        $this->seedState(9); // 9 is excluded from the cursor draw
        $this->seedSlots([0, 2, 0, 0, 0, 0, 0, 0]); // slot 1 mounted

        $res = $this->addressOf('_var_resourceGroup_8c2263a8');
        $this->call('_drawSelectScreen_8c01be90')->with();
        $this->shouldCall('_SpriteDraw_8c014f54')->with($res, 2, 0.0, 0.0, -5.0); // slot 1
        $this->expectFixedDraws();
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_var_vmuStatus_8c226048', 0x24);
        $this->setSize('_var_resourceGroup_8c2263a8', 0x0c);
        $this->setSize('_SpriteDraw_8c014f54', 4);
    }
};
