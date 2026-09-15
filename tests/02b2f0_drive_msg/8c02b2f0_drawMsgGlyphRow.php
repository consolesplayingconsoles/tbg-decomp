<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// drawMsgGlyphRow_8c02b2f0(ids, count, x, y): draws `count` 32x32 glyph
// quads in a row via njDrawQuadTexture, x advancing by 32.0 each; each
// glyph's UV comes from a 16x16 cell atlas keyed by ids[i]'s low nibble
// (column) and high nibble (row). The NJS_QUAD_TEXTURE argument is a
// callee-local stack struct -- its address is unknowable ahead of time, so
// its fields are verified via memory reads in ->do(); njDrawQuadTexture's
// float z argument (a fixed constant) isn't asserted since sh4objtest
// doesn't expose FPU register reads to tests.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_njDrawQuadTexture', 4);
    }

    public function test_twoGlyphs_drawsTwoQuadsAdvancingX(): void {
        $this->resolveSymbols();

        $ids = $this->alloc(4 * 2);
        $this->initUint32($ids + 0, 0x23); // col=3 (3/16=0.1875), row=0x20 (32/256=0.125)
        $this->initUint32($ids + 4, 0xf5); // col=5 (5/16=0.3125), row=0xf0 (240/256=0.9375)

        $this->call('_drawMsgGlyphRow_8c02b2f0')->with($ids, 2, 100.0, 50.0);

        $this->shouldCall('_njDrawQuadTexture')->do(function () {
            $q = $this->registers[4]->value;
            $expected = [
                'x1' => 100.0, 'y1' => 50.0, 'x2' => 132.0, 'y2' => 82.0,
                'u1' => 0.1875, 'v1' => 0.125, 'u2' => 0.25, 'v2' => 0.1875,
            ];
            $offsets = ['x1' => 0x00, 'y1' => 0x04, 'x2' => 0x08, 'y2' => 0x0c,
                        'u1' => 0x10, 'v1' => 0x14, 'u2' => 0x18, 'v2' => 0x1c];
            foreach ($expected as $name => $value) {
                $bits = $this->memory->readUInt32($q + $offsets[$name])->value;
                $actual = unpack('f', pack('L', $bits))[1];
                if (abs($actual - $value) > 0.0001) {
                    throw new \Exception("quad 0 field $name: expected $value, got $actual");
                }
            }
        });
        $this->shouldCall('_njDrawQuadTexture')->do(function () {
            $q = $this->registers[4]->value;
            $expected = [
                'x1' => 132.0, 'y1' => 50.0, 'x2' => 164.0, 'y2' => 82.0,
                'u1' => 0.3125, 'v1' => 0.9375, 'u2' => 0.375, 'v2' => 1.0,
            ];
            $offsets = ['x1' => 0x00, 'y1' => 0x04, 'x2' => 0x08, 'y2' => 0x0c,
                        'u1' => 0x10, 'v1' => 0x14, 'u2' => 0x18, 'v2' => 0x1c];
            foreach ($expected as $name => $value) {
                $bits = $this->memory->readUInt32($q + $offsets[$name])->value;
                $actual = unpack('f', pack('L', $bits))[1];
                if (abs($actual - $value) > 0.0001) {
                    throw new \Exception("quad 1 field $name: expected $value, got $actual");
                }
            }
        });
    }

    public function test_zeroCount_drawsNothing(): void {
        $this->resolveSymbols();

        $ids = $this->alloc(4);

        $this->call('_drawMsgGlyphRow_8c02b2f0')->with($ids, 0, 0.0, 0.0);
    }
};
