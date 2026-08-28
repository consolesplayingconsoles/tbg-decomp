<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // Table entry: {int type, int dataPtr}; only the type is read here.
    const ENTRY_SIZE = 0x08;

    // var_assetRequestSlots_8c228288 row stride.
    const ROW_SIZE = 0x18;

    // The row pointer and every type-6 global are referenced by the
    // function's compiled code regardless of which branch actually runs
    // this iteration, so all must be resolvable in every test.
    private function resolveSymbols(): void
    {
        $this->setSize('_syFree', 4);
        $this->setSize('_AsqReleaseAndFreeTexlist_8c011e3c', 4);

        $this->addressOf('_var_assetRequestSlots_8c228288');
        $this->addressOf('_var_fumiGateModel_8c22840c');
        $this->addressOf('_var_fumiTexlist_8c228410');
        $this->addressOf('_var_fumiCloseMotion_8c228414');
        $this->addressOf('_var_fumiOpenMotion_8c228418');
        $this->addressOf('_var_fumiLampModel_8c22841c');
        $this->addressOf('_var_fumiLampTexlist_8c228420');
        $this->addressOf('_var_fumiTrainModel_8c228424');
        $this->addressOf('_var_fumiTrainTexlist_8c228428');
        $this->addressOf('_var_fumiTrainMotionA_8c22842c');
        $this->addressOf('_var_fumiTrainMotionB_8c228430');
    }

    public function test_no_table_in_flight_is_a_noop(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), -1);

        $this->call('_ObjectsFreeAssetRequests_8c029cfe');
    }

    public function test_type_0_frees_only_the_slot_at_offset_8(): void
    {
        $this->resolveSymbols();

        $table = $this->alloc(self::ENTRY_SIZE + 4);
        $this->initUint32($table + 0x00, 0);
        $this->initUint32($table + self::ENTRY_SIZE, -1);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $row = $this->addressOf('_var_assetRequestSlots_8c228288');
        $this->initUint32($row + 0x08, 0x8c210008);

        $this->call('_ObjectsFreeAssetRequests_8c029cfe');

        $this->shouldCall('_syFree')->with(0x8c210008);
        $this->shouldWriteLongTo('_var_assetRequestTable_8c228408', -1);
    }

    public function test_type_1_frees_offset_4_and_8_and_releases_offset_0(): void
    {
        $this->resolveSymbols();

        $table = $this->alloc(self::ENTRY_SIZE + 4);
        $this->initUint32($table + 0x00, 1);
        $this->initUint32($table + self::ENTRY_SIZE, -1);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $row = $this->addressOf('_var_assetRequestSlots_8c228288');
        $this->initUint32($row + 0x00, 0x8c210000);
        $this->initUint32($row + 0x04, 0x8c210004);
        $this->initUint32($row + 0x08, 0x8c210008);

        $this->call('_ObjectsFreeAssetRequests_8c029cfe');

        $this->shouldCall('_syFree')->with(0x8c210004);
        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c210000);
        $this->shouldCall('_syFree')->with(0x8c210008);
        $this->shouldWriteLongTo('_var_assetRequestTable_8c228408', -1);
    }

    public function test_type_3_frees_offset_4_and_8_and_releases_offset_0(): void
    {
        $this->resolveSymbols();

        $table = $this->alloc(self::ENTRY_SIZE + 4);
        $this->initUint32($table + 0x00, 3);
        $this->initUint32($table + self::ENTRY_SIZE, -1);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $row = $this->addressOf('_var_assetRequestSlots_8c228288');
        $this->initUint32($row + 0x00, 0x8c210000);
        $this->initUint32($row + 0x04, 0x8c210004);
        $this->initUint32($row + 0x08, 0x8c210008);

        $this->call('_ObjectsFreeAssetRequests_8c029cfe');

        $this->shouldCall('_syFree')->with(0x8c210004);
        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c210000);
        $this->shouldCall('_syFree')->with(0x8c210008);
        $this->shouldWriteLongTo('_var_assetRequestTable_8c228408', -1);
    }

    public function test_type_2_frees_offset_4_and_releases_offset_0_only(): void
    {
        $this->resolveSymbols();

        $table = $this->alloc(self::ENTRY_SIZE + 4);
        $this->initUint32($table + 0x00, 2);
        $this->initUint32($table + self::ENTRY_SIZE, -1);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $row = $this->addressOf('_var_assetRequestSlots_8c228288');
        $this->initUint32($row + 0x00, 0x8c210000);
        $this->initUint32($row + 0x04, 0x8c210004);

        $this->call('_ObjectsFreeAssetRequests_8c029cfe');

        $this->shouldCall('_syFree')->with(0x8c210004);
        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c210000);
        $this->shouldWriteLongTo('_var_assetRequestTable_8c228408', -1);
    }

    public function test_type_4_frees_offset_4_and_releases_offset_0_only(): void
    {
        $this->resolveSymbols();

        $table = $this->alloc(self::ENTRY_SIZE + 4);
        $this->initUint32($table + 0x00, 4);
        $this->initUint32($table + self::ENTRY_SIZE, -1);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $row = $this->addressOf('_var_assetRequestSlots_8c228288');
        $this->initUint32($row + 0x00, 0x8c210000);
        $this->initUint32($row + 0x04, 0x8c210004);

        $this->call('_ObjectsFreeAssetRequests_8c029cfe');

        $this->shouldCall('_syFree')->with(0x8c210004);
        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c210000);
        $this->shouldWriteLongTo('_var_assetRequestTable_8c228408', -1);
    }

    public function test_type_5_frees_offset_4_and_releases_offset_0_only(): void
    {
        $this->resolveSymbols();

        $table = $this->alloc(self::ENTRY_SIZE + 4);
        $this->initUint32($table + 0x00, 5);
        $this->initUint32($table + self::ENTRY_SIZE, -1);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $row = $this->addressOf('_var_assetRequestSlots_8c228288');
        $this->initUint32($row + 0x00, 0x8c210000);
        $this->initUint32($row + 0x04, 0x8c210004);

        $this->call('_ObjectsFreeAssetRequests_8c029cfe');

        $this->shouldCall('_syFree')->with(0x8c210004);
        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c210000);
        $this->shouldWriteLongTo('_var_assetRequestTable_8c228408', -1);
    }

    public function test_multiple_rows_advance_the_row_stride(): void
    {
        $this->resolveSymbols();

        $table = $this->alloc(2 * self::ENTRY_SIZE + 4);
        $this->initUint32($table + 0 * self::ENTRY_SIZE, 0);
        $this->initUint32($table + 1 * self::ENTRY_SIZE, 0);
        $this->initUint32($table + 2 * self::ENTRY_SIZE, -1);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $base = $this->addressOf('_var_assetRequestSlots_8c228288');
        $this->initUint32($base + 0 * self::ROW_SIZE + 0x08, 0x8c210008);
        $this->initUint32($base + 1 * self::ROW_SIZE + 0x08, 0x8c210108);

        $this->call('_ObjectsFreeAssetRequests_8c029cfe');

        $this->shouldCall('_syFree')->with(0x8c210008);
        $this->shouldCall('_syFree')->with(0x8c210108);
        $this->shouldWriteLongTo('_var_assetRequestTable_8c228408', -1);
    }

    public function test_type_6_frees_the_fixed_railway_crossing_assets(): void
    {
        $this->resolveSymbols();

        $table = $this->alloc(self::ENTRY_SIZE + 4);
        $this->initUint32($table + 0x00, 6);
        $this->initUint32($table + self::ENTRY_SIZE, -1);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $this->initUint32($this->addressOf('_var_fumiGateModel_8c22840c'), 0x8c220000);
        $this->initUint32($this->addressOf('_var_fumiTexlist_8c228410'), 0x8c220004);
        $this->initUint32($this->addressOf('_var_fumiCloseMotion_8c228414'), 0x8c220008);
        $this->initUint32($this->addressOf('_var_fumiOpenMotion_8c228418'), 0x8c22000c);
        $this->initUint32($this->addressOf('_var_fumiLampModel_8c22841c'), 0x8c220010);
        $this->initUint32($this->addressOf('_var_fumiLampTexlist_8c228420'), 0x8c220014);
        $this->initUint32($this->addressOf('_var_fumiTrainModel_8c228424'), 0x8c220018);
        $this->initUint32($this->addressOf('_var_fumiTrainTexlist_8c228428'), 0x8c22001c);
        $this->initUint32($this->addressOf('_var_fumiTrainMotionA_8c22842c'), 0x8c220020);
        $this->initUint32($this->addressOf('_var_fumiTrainMotionB_8c228430'), 0x8c220024);

        $this->call('_ObjectsFreeAssetRequests_8c029cfe');

        $this->shouldCall('_syFree')->with(0x8c220000);
        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c220004);
        $this->shouldCall('_syFree')->with(0x8c220008);
        $this->shouldCall('_syFree')->with(0x8c22000c);
        $this->shouldCall('_syFree')->with(0x8c220010);
        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c220014);
        $this->shouldCall('_syFree')->with(0x8c220018);
        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c22001c);
        $this->shouldCall('_syFree')->with(0x8c220020);
        $this->shouldCall('_syFree')->with(0x8c220024);
        $this->shouldWriteLongTo('_var_assetRequestTable_8c228408', -1);
    }
};
