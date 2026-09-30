<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * drawUnlockGrid_8c01c9f2: draws a lock icon (sprite 2) over every
 * still-locked slot in the 55-slot grid, then the cell cursor (sprite 3)
 * and a background overlay (sprite 1).
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_TxtDrawSprite_8c014f54', 4);
        $this->setSize('__divls', 4);
        $this->setSize('__modls', 4);

        $this->onCall('__divls', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });
        $this->onCall('__modls', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });
    }

    private function initUnlocked(array $unlocked): void
    {
        $base = $this->addressOf('_var_profileUnlocked_8c2263b4');
        foreach ($unlocked as $i => $value) {
            $this->initUint8($base + $i, $value ? 1 : 0);
        }
    }

    private function initCursor(float $x, float $y): void
    {
        $menuState = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($menuState + 0x20, $this->floatToUint32($x));
        $this->initUint32($menuState + 0x24, $this->floatToUint32($y));
    }

    private function floatToUint32(float $value): int
    {
        return unpack('L', pack('f', $value))[1];
    }

    private function expectLockIcon(int $slot): void
    {
        $resGroup = $this->addressOf('_var_menuState_8c1bc7a8') + 0x0c;
        $row = intdiv($slot, 10);
        $col = $slot % 10;

        $this->shouldCall('__divls');
        $this->shouldCall('__modls');
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with(
            $resGroup, 2,
            (float) ($col * 0x2d + 0x60),
            (float) ($row * 0x30 + 0x80),
            -3.0
        );
    }

    private function expectCursorAndOverlay(float $cursorX, float $cursorY): void
    {
        $resGroup = $this->addressOf('_var_menuState_8c1bc7a8') + 0x0c;

        $this->shouldCall('_TxtDrawSprite_8c014f54')->with(
            $resGroup, 3, $cursorX, $cursorY, -2.0
        );

        $this->shouldCall('_TxtDrawSprite_8c014f54')->with(
            $resGroup, 1, 0.0, 0.0, -4.0
        );
    }

    public function test_all_unlocked_draws_no_lock_icons(): void
    {
        $this->resolveSymbols();
        $this->initUnlocked(array_fill(0, 55, 1));
        $this->initCursor(1.0, 2.0);

        $this->expectCursorAndOverlay(1.0, 2.0);

        $this->singleCall('_drawUnlockGrid_8c01c9f2')->run();
    }

    public function test_all_locked_draws_every_lock_icon_in_order(): void
    {
        $this->resolveSymbols();
        $this->initUnlocked(array_fill(0, 55, 0));
        $this->initCursor(3.0, 4.0);

        for ($i = 0; $i < 55; $i++) {
            $this->expectLockIcon($i);
        }
        $this->expectCursorAndOverlay(3.0, 4.0);

        $this->singleCall('_drawUnlockGrid_8c01c9f2')->run();
    }

    public function test_only_locked_slots_get_icons(): void
    {
        $this->resolveSymbols();
        $unlocked = array_fill(0, 55, 1);
        $unlocked[0] = 0;
        $unlocked[12] = 0;
        $unlocked[54] = 0;
        $this->initUnlocked($unlocked);
        $this->initCursor(0.0, 0.0);

        $this->expectLockIcon(0);
        $this->expectLockIcon(12);
        $this->expectLockIcon(54);
        $this->expectCursorAndOverlay(0.0, 0.0);

        $this->singleCall('_drawUnlockGrid_8c01c9f2')->run();
    }
};
