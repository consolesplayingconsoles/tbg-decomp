<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * showMark_8c01fa78(a, b): writes both args into var_hudMark_8c2264a8 -- markSpriteId_0x00 = a,
 * displayTimer_0x08 = b. blinkIconId_0x04/blinkCounter_0x0c untouched.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
    }

    public function test_writes_both_fields(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_hudMark_8c2264a8');

        $this->call('_showMark_8c01fa78')->with(0x1e, 10);

        $this->shouldWriteLong($base + 0x00, 0x1e);
        $this->shouldWriteLong($base + 0x08, 10);
    }
};
