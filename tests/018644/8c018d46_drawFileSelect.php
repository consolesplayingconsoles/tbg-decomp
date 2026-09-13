<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private const BASE = 0x100000;

    private int $ms;

    /* NEW FILE column present, first page: cards drawn from the base image, BACK
     * disabled (page 0), NEXT disabled (no 4th card). */
    public function test_new_file_first_page(): void
    {
        $cards = [0 => 0xa, 1 => 1, 2 => 2];   // NEW FILE card, then two saves; slot 3 stays 0xb
        $this->setup($cards, 0, 0);

        $this->call('_drawFileSelect_8c018d46');

        $this->shouldWriteLongTo('_var_saveBufCursor_8c225fe0', self::BASE);
        $this->expectCard(0xa, 55.0);
        $this->expectCard(1, 237.0);
        $this->expectCard(2, 419.0);
        $this->expectChrome(0, 0, 0xb);
    }

    /* No NEW FILE column, scrolled to page 1: save index = page, BACK enabled,
     * NEXT enabled (a card exists at page+3). */
    public function test_saves_only_page_one(): void
    {
        $cards = [1, 2, 3, 4, 5, 6];   // save indices; slots 6..11 stay 0xb
        $this->setup($cards, 1, 2);

        $this->call('_drawFileSelect_8c018d46');

        $this->shouldWriteLongTo('_var_saveBufCursor_8c225fe0', self::BASE + 0x600);
        $this->expectCard(2, 55.0);
        $this->expectCard(3, 237.0);
        $this->expectCard(4, 419.0);
        $this->expectChrome(2, 1, 5);
    }

    /* NEW FILE column present, scrolled: save index = page - 1 (the NEW FILE card
     * is not backed by a save image). */
    public function test_new_file_scrolled(): void
    {
        $cards = [0 => 0xa, 1 => 1, 2 => 2, 3 => 3, 4 => 4];   // slot 5 stays 0xb
        $this->setup($cards, 2, 1);

        $this->call('_drawFileSelect_8c018d46');

        $this->shouldWriteLongTo('_var_saveBufCursor_8c225fe0', self::BASE + 0x600);
        $this->expectCard(2, 55.0);
        $this->expectCard(3, 237.0);
        $this->expectCard(4, 419.0);
        $this->expectChrome(1, 2, 0xb);
    }

    /* Empty list: the very first card is 0xb, so no cards are drawn. */
    public function test_empty_list(): void
    {
        $this->setup([], 0, 0);

        $this->call('_drawFileSelect_8c018d46');

        $this->shouldWriteLongTo('_var_saveBufCursor_8c225fe0', self::BASE);
        $this->expectChrome(0, 0, 0xb);
    }

    private function setup(array $cards, int $page, int $selected): void
    {
        $this->doNotRandomizeMemory();
        $this->setSize('_var_8c226018', 0x30);
        $this->setSize('_var_menuState_8c1bc7a8', 0x6c);

        $c = $this->addressOf('_var_8c226018');
        $full = array_fill(0, 12, 0xb);
        foreach ($cards as $i => $v) {
            $full[$i] = $v;
        }
        foreach ($full as $i => $v) {
            $this->initUint32($c + $i * 4, $v);
        }

        $this->ms = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($this->ms + 0x38, $selected);   // selected_0x38 (cursor column)
        $this->initUint32($this->ms + 0x3c, $page);        // field_0x3c (page offset)
        $this->initUint32($this->addressOf('_var_8c1ba2e0'), self::BASE);
    }

    private function expectCard(int $card, float $x): void
    {
        $this->shouldCall('_drawFileCard_8c018b4c')->with($card, $x);
    }

    /* cursor sprite, BACK/NEXT arrows, then the three static frame sprites. */
    private function expectChrome(int $selected, int $page, int $nextCard): void
    {
        $b = $this->ms + 0xc;
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($b, 0x2f, 182.0 * $selected + 45.0, 0.0, -3.0);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($b, $page != 0 ? 0x16 : 0x15, 0.0, 0.0, -3.0);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($b, $nextCard == 0xb ? 0x17 : 0x18, 0.0, 0.0, -3.0);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($b, 0x14, 0.0, 0.0, -4.0);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->ms, 1, 0.0, 0.0, -4.3);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->ms, 0, 0.0, 0.0, -5.0);
    }
};
