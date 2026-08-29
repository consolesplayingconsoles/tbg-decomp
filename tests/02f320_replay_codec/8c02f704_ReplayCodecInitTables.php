<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Resets the per-symbol lookup tables: var_8c228bae becomes an identity
// map, and the four index arrays are NIL-filled, for the first 0x100 slots.
return new class extends TestCase {
    public function test_fills_identity_and_nil_tables(): void
    {
        $this->resolveSymbols();
        $this->doNotRandomizeMemory();

        $this->call('_initTables_8c02f704');

        for ($i = 0; $i < 0x100; $i++) {
            $this->shouldWriteByte($this->addressOf('_var_8c228bae') + $i, $i);
            $this->shouldWriteWord($this->addressOf('_var_8c22dbae') + $i * 2, 0x1000);
            $this->shouldWriteWord($this->addressOf('_var_8c22fbae') + $i * 2, 0x1000);
            $this->shouldWriteWord($this->addressOf('_var_8c22bbae') + $i * 2, 0x1000);
            $this->shouldWriteWord($this->addressOf('_var_8c229bae') + $i * 2, 0x1000);
        }
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_8c228bae', 0x1000);
        $this->setSize('_var_8c229bae', 0x2000);
        $this->setSize('_var_8c22bbae', 0x2000);
        $this->setSize('_var_8c22dbae', 0x2000);
        $this->setSize('_var_8c22fbae', 0x2000);
    }
};
