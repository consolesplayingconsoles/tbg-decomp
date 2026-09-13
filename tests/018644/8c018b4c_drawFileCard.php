<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private int $ms;
    private int $buf;

    /* NEW FILE card (kind 0xa): a single sprite, no save read. */
    public function test_new_file_card(): void
    {
        $this->doNotRandomizeMemory();
        $this->ms = $this->addressOf('_var_menuState_8c1bc7a8');

        $this->call('_drawFileCard_8c018b4c')->with(0xa, 40.0);

        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->ms + 0xc, 0x13, 40.0, 0.0, -4.0);
    }

    /* Save card, single-digit day: date, unlocked-course icons, counts, two rank-3 markers. */
    public function test_save_card_single_digit_day(): void
    {
        $this->setupSave(5, 55, 99999);
        $this->course(0, 1, 3);   // unlocked, rank 3
        $this->course(1, 1, 3);   // unlocked, rank 3
        $this->course(2, 1, 0);   // unlocked, no rank

        $this->call('_drawFileCard_8c018b4c')->with(0, 100.0);

        $this->expectDate(100.0, 5);
        $this->expectIcon(100.0, 0);
        $this->expectIcon(100.0, 1);
        $this->expectIcon(100.0, 2);
        $this->expectCounts(100.0, 55, 99999);
        $this->expectMarker(0x23, 113.0, 279.0);
        $this->expectMarker(0x23, 137.0, 279.0);
        $this->expectFinal(100.0);
    }

    /* Save card, two-digit day (offset 63), no unlocked courses, no markers. */
    public function test_save_card_two_digit_day_no_markers(): void
    {
        $this->setupSave(15, 7, 42);

        $this->call('_drawFileCard_8c018b4c')->with(0, 100.0);

        $this->expectDate(100.0, 15);
        $this->expectCounts(100.0, 7, 42);
        $this->expectFinal(100.0);
    }

    /* Rank-2 course: draws glyph 0x24 first, then the grid floods with 0x23. */
    public function test_save_card_rank2_marker(): void
    {
        $this->setupSave(5, 1, 2);
        $this->course(0, 0, 2);

        $this->call('_drawFileCard_8c018b4c')->with(0, 100.0);

        $this->expectDate(100.0, 5);
        $this->expectCounts(100.0, 1, 2);
        $this->expectFlood(100.0, 0x24);
        $this->expectFinal(100.0);
    }

    /* Rank-1 course: draws glyph 0x25 first, then the grid floods with 0x23. */
    public function test_save_card_rank1_marker(): void
    {
        $this->setupSave(5, 1, 2);
        $this->course(0, 0, 1);

        $this->call('_drawFileCard_8c018b4c')->with(0, 100.0);

        $this->expectDate(100.0, 5);
        $this->expectCounts(100.0, 1, 2);
        $this->expectFlood(100.0, 0x25);
        $this->expectFinal(100.0);
    }

    private function setupSave(int $days, int $f8c, int $exp): void
    {
        $this->doNotRandomizeMemory();
        $this->buf = $this->alloc(0x100);
        $this->initUint32($this->addressOf('_var_saveBufCursor_8c225fe0'), $this->buf);
        $this->initUint32($this->buf + 0x00, $days);
        $this->initUint32($this->buf + 0x8c, $f8c);
        $this->initUint32($this->buf + 0x90, $exp);

        $this->setSize('__modls', 4);
        $this->onCall('__modls', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });

        $this->ms = $this->addressOf('_var_menuState_8c1bc7a8');
    }

    /* course record i: unlocked flag (byte 0) and story-sprite rank (byte 3). */
    private function course(int $i, int $unlocked, int $story): void
    {
        if ($unlocked) {
            $this->initUint8($this->buf + 0x44 + $i * 8 + 0, $unlocked);
        }
        if ($story) {
            $this->initUint8($this->buf + 0x44 + $i * 8 + 3, $story);
        }
    }

    /* Date number (offset 52 for 1 digit, 63 for 2) plus weekday icon 6 + (day+1)%7. */
    private function expectDate(float $x, int $days): void
    {
        $off = $days >= 10 ? 63.0 : 52.0;
        $this->shouldCall('_drawNumber_8c018aa2')->with($days, $x + $off, 122.0);
        $this->shouldCall('__modls');
        $this->shouldCall('_TxtDrawSprite_8c014f54')
            ->with($this->ms, 6 + ($days + 1) % 7, $x + 84.0, 122.0, -4.0);
    }

    private function expectIcon(float $x, int $i): void
    {
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->ms + 0xc, 0x26 + $i, $x, 0.0, -4.0);
    }

    private function expectCounts(float $x, int $f8c, int $exp): void
    {
        $this->shouldCall('_drawNumber_8c018aa2')->with($f8c, $x + 77.0, 244.0);
        $this->shouldCall('_drawNumber_8c018aa2')->with($exp, $x + 77.0, 264.0);
    }

    private function expectMarker(int $glyph, float $x, float $y): void
    {
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->ms + 0xc, $glyph, $x, $y, -4.5);
    }

    /* Full two-row grid (5 columns at xoff 13..109): first marker $firstGlyph, rest 0x23. */
    private function expectFlood(float $x, int $firstGlyph): void
    {
        $first = true;
        foreach ([279.0, 303.0] as $y) {
            for ($c = 0; $c < 5; $c++) {
                $this->expectMarker($first ? $firstGlyph : 0x23, $x + (13.0 + 24.0 * $c), $y);
                $first = false;
            }
        }
    }

    private function expectFinal(float $x): void
    {
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->ms + 0xc, 0x12, $x, 0.0, -4.5);
        $this->shouldWriteLongTo('_var_saveBufCursor_8c225fe0', $this->buf + 0x600);
    }
};
