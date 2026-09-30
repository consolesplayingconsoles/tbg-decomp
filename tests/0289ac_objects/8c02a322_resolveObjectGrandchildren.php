<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_walks_two_child_sibling_chains()
    {
        // First group: root->child->child, then 7 siblings.
        $gc1_8 = 0x11111118; // never dereferenced, only stored

        $gc1_7 = $this->alloc(0x34);
        $this->initUint32($gc1_7 + 0x30, $gc1_8);

        $gc1_6 = $this->alloc(0x34);
        $this->initUint32($gc1_6 + 0x30, $gc1_7);

        $gc1_5 = $this->alloc(0x34);
        $this->initUint32($gc1_5 + 0x30, $gc1_6);

        $gc1_4 = $this->alloc(0x34);
        $this->initUint32($gc1_4 + 0x30, $gc1_5);

        $gc1_3 = $this->alloc(0x34);
        $this->initUint32($gc1_3 + 0x30, $gc1_4);

        $gc1_2 = $this->alloc(0x34);
        $this->initUint32($gc1_2 + 0x30, $gc1_3);

        $gc1_1 = $this->alloc(0x34);
        $this->initUint32($gc1_1 + 0x30, $gc1_2);

        $childA = $this->alloc(0x34);
        $this->initUint32($childA + 0x2c, $gc1_1); // childA->child

        // Second group: root->child->sibling->child, then 7 siblings.
        $gc2_8 = 0x22222228; // never dereferenced, only stored

        $gc2_7 = $this->alloc(0x34);
        $this->initUint32($gc2_7 + 0x30, $gc2_8);

        $gc2_6 = $this->alloc(0x34);
        $this->initUint32($gc2_6 + 0x30, $gc2_7);

        $gc2_5 = $this->alloc(0x34);
        $this->initUint32($gc2_5 + 0x30, $gc2_6);

        $gc2_4 = $this->alloc(0x34);
        $this->initUint32($gc2_4 + 0x30, $gc2_5);

        $gc2_3 = $this->alloc(0x34);
        $this->initUint32($gc2_3 + 0x30, $gc2_4);

        $gc2_2 = $this->alloc(0x34);
        $this->initUint32($gc2_2 + 0x30, $gc2_3);

        $gc2_1 = $this->alloc(0x34);
        $this->initUint32($gc2_1 + 0x30, $gc2_2);

        $childB = $this->alloc(0x34);
        $this->initUint32($childB + 0x2c, $gc2_1); // childB->child

        $this->initUint32($childA + 0x30, $childB); // childA->sibling

        $root = $this->alloc(0x30);
        $this->initUint32($root + 0x2c, $childA); // root->child

        $nodes = $this->alloc(4 * 17);
        $this->initUint32($nodes + 0, $root);

        $this->call('_resolveObjectGrandchildren_8c02a322')->with($nodes);

        $this->shouldWriteLong($nodes + 4, $gc1_1);
        $this->shouldWriteLong($nodes + 8, $gc1_2);
        $this->shouldWriteLong($nodes + 12, $gc1_3);
        $this->shouldWriteLong($nodes + 16, $gc1_4);
        $this->shouldWriteLong($nodes + 20, $gc1_5);
        $this->shouldWriteLong($nodes + 24, $gc1_6);
        $this->shouldWriteLong($nodes + 28, $gc1_7);
        $this->shouldWriteLong($nodes + 32, $gc1_8);

        $this->shouldWriteLong($nodes + 36, $gc2_1);
        $this->shouldWriteLong($nodes + 40, $gc2_2);
        $this->shouldWriteLong($nodes + 44, $gc2_3);
        $this->shouldWriteLong($nodes + 48, $gc2_4);
        $this->shouldWriteLong($nodes + 52, $gc2_5);
        $this->shouldWriteLong($nodes + 56, $gc2_6);
        $this->shouldWriteLong($nodes + 60, $gc2_7);
        $this->shouldWriteLong($nodes + 64, $gc2_8);
    }
};
