<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_frees_the_glyph_tables()
    {
        $this->resolveSymbols();

        // TODO: calloc
        $var_glyphSlotUsed_8c1bc7a0 = $this->alloc(0x200 * 2);
        $this->initUint16Array($var_glyphSlotUsed_8c1bc7a0, array_fill(0, 0x200, -18));
        $this->initUint32($this->addressOf('_var_glyphSlotUsed_8c1bc7a0'), $var_glyphSlotUsed_8c1bc7a0);
        $this->initUint16($var_glyphSlotUsed_8c1bc7a0 + 0x000 * 2, -19);
        $this->initUint16($var_glyphSlotUsed_8c1bc7a0 + 0x001 * 2, -20);
        $this->initUint16($var_glyphSlotUsed_8c1bc7a0 + 0x199 * 2, -20);
        $var_glyphTexlists_8c1bc790 = $this->alloc(0x200 * 8);
        $this->initUint32($this->addressOf('_var_glyphTexlists_8c1bc790'), $var_glyphTexlists_8c1bc790);

        $this->initUint32($this->addressOf('_var_glyphTexnames_8c1bc78c'), 0xcafe0000);
        $this->initUint32($this->addressOf('_var_glyphBuffer_8c1bc7a4'), 0xcafe0001);

        $this->shouldCall('_njReleaseTexture')->with($var_glyphTexlists_8c1bc790 + 0x001 * 8);
        $this->shouldCall('_njReleaseTexture')->with($var_glyphTexlists_8c1bc790 + 0x199 * 8);

        $this->shouldCall('_syFree')->with($var_glyphTexlists_8c1bc790);
        $this->shouldCall('_syFree')->with(0xcafe0000);
        $this->shouldCall('_syFree')->with(0xcafe0001);
        $this->shouldCall('_syFree')->with($var_glyphSlotUsed_8c1bc7a0);

        $this->singleCall('_TxtDestroy_8c01529c')->run();
    }

    /**
     * A slot in use holds its glyph index (0..GLYPH_COUNT-1); a free one holds
     * 0xffff. The compare is unsigned (the asm EXTU.W's before CMP/GE), so an
     * in-use slot is far below 0xffed and must be released.
     *
     * test_frees_the_glyph_tables above only spans -18/-19/-20, the one neighbourhood where a
     * signed compare agrees with the unsigned one, so it passes either way.
     * Real glyph indices are what tell them apart.
     */
    public function test_releasesSlotsHoldingGlyphIndexes()
    {
        $this->resolveSymbols();

        $var_glyphSlotUsed_8c1bc7a0 = $this->alloc(0x200 * 2);
        $this->initUint16Array($var_glyphSlotUsed_8c1bc7a0, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_glyphSlotUsed_8c1bc7a0'), $var_glyphSlotUsed_8c1bc7a0);

        // Slots holding a loaded glyph, including both ends of the range
        $this->initUint16($var_glyphSlotUsed_8c1bc7a0 + 0x000 * 2, 0x000);
        $this->initUint16($var_glyphSlotUsed_8c1bc7a0 + 0x005 * 2, 0x005);
        $this->initUint16($var_glyphSlotUsed_8c1bc7a0 + 0x1ff * 2, 0x1ff);

        $var_glyphTexlists_8c1bc790 = $this->alloc(0x200 * 8);
        $this->initUint32($this->addressOf('_var_glyphTexlists_8c1bc790'), $var_glyphTexlists_8c1bc790);
        $this->initUint32($this->addressOf('_var_glyphTexnames_8c1bc78c'), 0xcafe0000);
        $this->initUint32($this->addressOf('_var_glyphBuffer_8c1bc7a4'), 0xcafe0001);

        $this->shouldCall('_njReleaseTexture')->with($var_glyphTexlists_8c1bc790 + 0x000 * 8);
        $this->shouldCall('_njReleaseTexture')->with($var_glyphTexlists_8c1bc790 + 0x005 * 8);
        $this->shouldCall('_njReleaseTexture')->with($var_glyphTexlists_8c1bc790 + 0x1ff * 8);

        $this->shouldCall('_syFree')->with($var_glyphTexlists_8c1bc790);
        $this->shouldCall('_syFree')->with(0xcafe0000);
        $this->shouldCall('_syFree')->with(0xcafe0001);
        $this->shouldCall('_syFree')->with($var_glyphSlotUsed_8c1bc7a0);

        $this->singleCall('_TxtDestroy_8c01529c')->run();
    }

    protected function resolveSymbols(): void
    {
        //$this->setSize('_var_fontResourceGroup_8c1bc794', 8);
        // Functions
        // $this->setSize('__divls', 4);
    }

    protected function isAsmObject(): bool
    {
        return str_contains($this->objectFile, '/asm/');
    }

    protected function initUint16Array(int $address, array $values): void
    {
        foreach ($values as $i => $value) {
            $this->initUint16($address + $i * 2, $value);
        }
    }
};
