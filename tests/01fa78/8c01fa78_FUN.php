<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * FUN_8c01fa78(a, b): writes both args into var_8c2264a8 -- field_0x00 = a,
 * field_0x08 = b. field_0x04/0x0c untouched.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_8c2264a8', 0x10);
    }

    public function test_writes_both_fields(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_8c2264a8');

        $this->call('_FUN_8c01fa78')->with(0x1e, 10);

        $this->shouldWriteLong($base + 0x00, 0x1e);
        $this->shouldWriteLong($base + 0x08, 10);
    }
};
