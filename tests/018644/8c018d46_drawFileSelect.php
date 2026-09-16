<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private const BASE = 0x100000;

    private int $ms;

    /* NEW FILE card present, not scrolled: cards drawn from the base image, left
     * arrow greyed (offset 0), right arrow greyed (no 4th card). */
    public function test_new_file_not_scrolled(): void
    {
        $cards = [0 => 0xa, 1 => 1, 2 => 2];   // NEW FILE card, then two saves; card 3 stays 0xb
        $this->setup($cards, 0, 0);

        $this->call('_drawFileSelect_8c018d46');

        $this->shouldWriteLongTo('_var_saveBufCursor_8c225fe0', self::BASE);
        $this->expectCard(0xa, 55.0);
        $this->expectCard(1, 237.0);
        $this->expectCard(2, 419.0);
        $this->expectChrome(0, 0, 0xb);
    }

    /* No NEW FILE card, scrolled by one: image index = scroll offset, both arrows
     * lit (a card exists at offset+3). */
    public function test_saves_only_scrolled(): void
    {
        $cards = [1, 2, 3, 4, 5, 6];   // VMU file indices; cards 6..11 stay 0xb
        $this->setup($cards, 1, 2);

        $this->call('_drawFileSelect_8c018d46');

        $this->shouldWriteLongTo('_var_saveBufCursor_8c225fe0', self::BASE + 0x600);
        $this->expectCard(2, 55.0);
        $this->expectCard(3, 237.0);
        $this->expectCard(4, 419.0);
        $this->expectChrome(2, 1, 5);
    }

    /* NEW FILE card present, scrolled: image index = offset - 1 (the NEW FILE card
     * is not backed by a save image). */
    public function test_new_file_scrolled(): void
    {
        $cards = [0 => 0xa, 1 => 1, 2 => 2, 3 => 3, 4 => 4];   // card 5 stays 0xb
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

    private function setup(array $cards, int $scroll, int $selected): void
    {
        $this->doNotRandomizeMemory();
        $this->setSize('_var_fileCards_8c226018', 0x30);
        $this->setSize('_var_menuState_8c1bc7a8', 0x6c);

        $c = $this->addressOf('_var_fileCards_8c226018');
        $full = array_fill(0, 12, 0xb);
        foreach ($cards as $i => $v) {
            $full[$i] = $v;
        }
        foreach ($full as $i => $v) {
            $this->initUint32($c + $i * 4, $v);
        }

        $this->ms = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($this->ms + 0x38, $selected);   // selected_0x38 (cursor column, 0-2)
        $this->initUint32($this->ms + 0x3c, $scroll);      // cursorCol_0x3c (leftmost card)
        $this->initUint32($this->addressOf('_var_saveBuf_8c1ba2e0'), self::BASE);
    }

    private function expectCard(int $card, float $x): void
    {
        $this->shouldCall('_drawFileCard_8c018b4c')->with($card, $x);
    }

    /* cursor sprite, left/right arrows, then the three static frame sprites. */
    private function expectChrome(int $selected, int $scroll, int $nextCard): void
    {
        $b = $this->ms + 0xc;
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($b, 0x2f, 182.0 * $selected + 45.0, 0.0, -3.0);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($b, $scroll != 0 ? 0x16 : 0x15, 0.0, 0.0, -3.0);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($b, $nextCard == 0xb ? 0x17 : 0x18, 0.0, 0.0, -3.0);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($b, 0x14, 0.0, 0.0, -4.0);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->ms, 1, 0.0, 0.0, -4.3);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->ms, 0, 0.0, 0.0, -5.0);
    }
};
