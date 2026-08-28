<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // {int id, void *pvm, void *dat} entry, matching var_messageAssets_8c228484.
    const ENTRY_SIZE = 0x0c;

    private function resolveSymbols(): void
    {
        $this->setSize('_syFree', 4);
        $this->setSize('_AsqReleaseAndFreeTexlist_8c011e3c', 4);
        $this->setSize('_var_messageAssets_8c228484', self::ENTRY_SIZE * 12);
    }

    public function test_no_entries_and_no_shared_dat_only_frees_textbox(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_messageAssetCount_8c228514'), 0);
        $this->initUint32($this->addressOf('_var_messageTextDat_8c228518'), -1);

        $this->call('_ObjectsFreeMessageAssets_8c02adee');

        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 0);
        $this->shouldCall('_ObjectsFreeTextboxes_8c02af32');
    }

    public function test_shared_dat_present_is_freed_and_reset(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_messageAssetCount_8c228514'), 0);
        $this->initUint32($this->addressOf('_var_messageTextDat_8c228518'), 0x8c210000);

        $this->call('_ObjectsFreeMessageAssets_8c02adee');

        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 0);
        $this->shouldCall('_syFree')->with(0x8c210000);
        $this->shouldWriteLongTo('_var_messageTextDat_8c228518', -1);
        $this->shouldCall('_ObjectsFreeTextboxes_8c02af32');
    }

    public function test_entries_are_released_in_order_then_shared_dat(): void
    {
        $this->resolveSymbols();

        $entries = $this->addressOf('_var_messageAssets_8c228484');
        $this->initUint32($entries + 0 * self::ENTRY_SIZE + 0x00, 1); // id
        $this->initUint32($entries + 0 * self::ENTRY_SIZE + 0x04, 0x8c220000); // pvm
        $this->initUint32($entries + 0 * self::ENTRY_SIZE + 0x08, 0x8c220004); // dat
        $this->initUint32($entries + 1 * self::ENTRY_SIZE + 0x00, 2); // id
        $this->initUint32($entries + 1 * self::ENTRY_SIZE + 0x04, 0x8c220008); // pvm
        $this->initUint32($entries + 1 * self::ENTRY_SIZE + 0x08, 0x8c22000c); // dat

        $this->initUint32($this->addressOf('_var_messageAssetCount_8c228514'), 2);
        $this->initUint32($this->addressOf('_var_messageTextDat_8c228518'), 0x8c210000);

        $this->call('_ObjectsFreeMessageAssets_8c02adee');

        $this->shouldCall('_syFree')->with(0x8c220004);
        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c220000);
        $this->shouldCall('_syFree')->with(0x8c22000c);
        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c220008);
        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 0);
        $this->shouldCall('_syFree')->with(0x8c210000);
        $this->shouldWriteLongTo('_var_messageTextDat_8c228518', -1);
        $this->shouldCall('_ObjectsFreeTextboxes_8c02af32');
    }
};
