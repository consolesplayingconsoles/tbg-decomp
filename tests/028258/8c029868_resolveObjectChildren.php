<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_walks_child_then_sibling_chain()
    {
        $sib2 = 0x12345678; // never dereferenced, only stored

        $sib1 = $this->alloc(0x34);
        $this->initUint32($sib1 + 0x30, $sib2); // sib1->sibling

        $child = $this->alloc(0x34);
        $this->initUint32($child + 0x30, $sib1); // child->sibling

        $root = $this->alloc(0x30);
        $this->initUint32($root + 0x2c, $child); // root->child

        $nodes = $this->alloc(4 * 4);
        $this->initUint32($nodes + 0, $root);

        $this->call('_resolveObjectChildren_8c029868')->with($nodes);

        $this->shouldWriteLong($nodes + 4, $child);
        $this->shouldWriteLong($nodes + 8, $sib1);
        $this->shouldWriteLong($nodes + 12, $sib2);
    }
};
