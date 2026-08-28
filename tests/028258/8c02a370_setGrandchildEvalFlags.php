<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function allocNodes(): array
    {
        $nodes = $this->alloc(17 * 4);
        $children = [];
        for ($i = 1; $i <= 16; $i++) {
            $node = $this->alloc(4);
            $this->initUint32($node, 0); // evalflags
            $children[$i] = $node;
            $this->initUint32($nodes + 4 * $i, $node);
        }

        return [$nodes, $children];
    }

    public function test_selector0(): void
    {
        [$nodes, $children] = $this->allocNodes();

        $this->call('_setGrandchildEvalFlags_8c02a370')->with($nodes, 0);

        $this->shouldWriteLong($children[1], 0x37);
        $this->shouldWriteLong($children[2], 0x37);
        $this->shouldWriteLong($children[3], 0x3f);
        $this->shouldWriteLong($children[4], 0x3f);
        $this->shouldWriteLong($children[5], 0x3f);
        $this->shouldWriteLong($children[6], 0x3f);
        $this->shouldWriteLong($children[7], 0x3f);
        $this->shouldWriteLong($children[8], 0x3f);
        $this->shouldWriteLong($children[9], 0x37);
        $this->shouldWriteLong($children[10], 0x37);
        $this->shouldWriteLong($children[11], 0x3f);
        $this->shouldWriteLong($children[12], 0x3f);
        $this->shouldWriteLong($children[13], 0x3f);
        $this->shouldWriteLong($children[14], 0x3f);
        $this->shouldWriteLong($children[15], 0x3f);
        $this->shouldWriteLong($children[16], 0x3f);
    }

    public function test_selector1(): void
    {
        [$nodes, $children] = $this->allocNodes();

        $this->call('_setGrandchildEvalFlags_8c02a370')->with($nodes, 1);

        $this->shouldWriteLong($children[1], 0x3f);
        $this->shouldWriteLong($children[2], 0x3f);
        $this->shouldWriteLong($children[3], 0x37);
        $this->shouldWriteLong($children[4], 0x37);
        $this->shouldWriteLong($children[5], 0x3f);
        $this->shouldWriteLong($children[6], 0x3f);
        $this->shouldWriteLong($children[7], 0x3f);
        $this->shouldWriteLong($children[8], 0x37);
        $this->shouldWriteLong($children[9], 0x3f);
        $this->shouldWriteLong($children[10], 0x3f);
        $this->shouldWriteLong($children[11], 0x37);
        $this->shouldWriteLong($children[12], 0x37);
        $this->shouldWriteLong($children[13], 0x3f);
        $this->shouldWriteLong($children[14], 0x3f);
        $this->shouldWriteLong($children[15], 0x3f);
        $this->shouldWriteLong($children[16], 0x37);
    }

    public function test_selector2(): void
    {
        [$nodes, $children] = $this->allocNodes();

        $this->call('_setGrandchildEvalFlags_8c02a370')->with($nodes, 2);

        $this->shouldWriteLong($children[1], 0x3f);
        $this->shouldWriteLong($children[2], 0x3f);
        $this->shouldWriteLong($children[3], 0x3f);
        $this->shouldWriteLong($children[4], 0x3f);
        $this->shouldWriteLong($children[5], 0x37);
        $this->shouldWriteLong($children[6], 0x37);
        $this->shouldWriteLong($children[7], 0x37);
        $this->shouldWriteLong($children[8], 0x3f);
        $this->shouldWriteLong($children[9], 0x3f);
        $this->shouldWriteLong($children[10], 0x3f);
        $this->shouldWriteLong($children[11], 0x3f);
        $this->shouldWriteLong($children[12], 0x3f);
        $this->shouldWriteLong($children[13], 0x37);
        $this->shouldWriteLong($children[14], 0x37);
        $this->shouldWriteLong($children[15], 0x37);
        $this->shouldWriteLong($children[16], 0x3f);
    }

    public function test_other_selector_is_noop(): void
    {
        [$nodes, $children] = $this->allocNodes();

        $this->call('_setGrandchildEvalFlags_8c02a370')->with($nodes, 3);
    }
};
