<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // Table row: {int type, int dataPtr}.
    const ROW_SIZE = 0x08;

    public function test_null_table_just_releases_previous_requests(): void
    {
        $this->setSize('_AsqRequestPvm_8c011ac0', 4);
        $this->setSize('_AsqRequestDat_8c011182', 4);

        $this->call('_ObjectsStartAssetRequests_8c029ad4')->with(0);

        $this->shouldCall('_ObjectsFreeAssetRequests_8c029cfe');
    }

    public function test_same_table_as_in_flight_is_a_noop(): void
    {
        // A real (non-sentinel-only) row: if the early-return guard were
        // dropped or inverted, ObjectsFreeAssetRequests_8c029cfe would run
        // for real (it's a linked intra-object call, not an external stub,
        // so the DSL can't just assert it wasn't called) and this type-0 row
        // would make it attempt a genuine, unmocked _syFree call, which the
        // harness rejects as an unexpected call -- making the guard's effect
        // observable without a shouldNotCall API.
        $src = $this->alloc(0x10);
        $njName = $this->allocString('O_TEST_00.njd');
        $this->initUint32($src + 0x0c, $njName);

        $table = $this->alloc(self::ROW_SIZE + 4);
        $this->initUint32($table + 0x00, 0);
        $this->initUint32($table + 0x04, $src);
        $this->initUint32($table + self::ROW_SIZE, -1);

        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $this->call('_ObjectsStartAssetRequests_8c029ad4')->with($table);

        // No expectations at all: any call (AsqRequestNj, syFree, ...) that
        // the broken path would make now surfaces as an unexpected-call
        // failure, proving the early return actually fired.
    }

    public function test_type_0_requests_nj_from_source_struct_offset_0xc(): void
    {
        $this->setSize('_AsqRequestPvm_8c011ac0', 4);
        $this->setSize('_AsqRequestDat_8c011182', 4);

        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), 0);

        $src = $this->alloc(0x10);
        $njName = $this->allocString('O_TEST_00.njd');
        $this->initUint32($src + 0x0c, $njName);

        $table = $this->alloc(self::ROW_SIZE + 4);
        $this->initUint32($table + 0x00, 0);
        $this->initUint32($table + 0x04, $src);
        $this->initUint32($table + self::ROW_SIZE, -1);

        $this->call('_ObjectsStartAssetRequests_8c029ad4')->with($table);

        $this->shouldCall('_ObjectsFreeAssetRequests_8c029cfe');
        $this->shouldWriteLongTo('_var_assetRequestTable_8c228408', $table);
        $this->shouldCall('_AsqRequestNj_8c011492')->with(
            $this->addressOf('_var_commonDirCopy_8c18ad8c'),
            $njName,
            $this->addressOf('_var_assetRequestSlots_8c228288') + 0x08,
            0,
        );
    }

    public function test_multiple_rows_use_the_row_stride_for_the_slot_base(): void
    {
        $this->setSize('_AsqRequestPvm_8c011ac0', 4);
        $this->setSize('_AsqRequestDat_8c011182', 4);

        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), 0);

        $src0 = $this->alloc(0x10);
        $njName0 = $this->allocString('O_TEST_00.njd');
        $this->initUint32($src0 + 0x0c, $njName0);

        $src1 = $this->alloc(0x10);
        $njName1 = $this->allocString('O_TEST_01.njd');
        $this->initUint32($src1 + 0x0c, $njName1);

        $table = $this->alloc(2 * self::ROW_SIZE + 4);
        $this->initUint32($table + 0 * self::ROW_SIZE + 0x00, 0);
        $this->initUint32($table + 0 * self::ROW_SIZE + 0x04, $src0);
        $this->initUint32($table + 1 * self::ROW_SIZE + 0x00, 0);
        $this->initUint32($table + 1 * self::ROW_SIZE + 0x04, $src1);
        $this->initUint32($table + 2 * self::ROW_SIZE, -1);

        $this->call('_ObjectsStartAssetRequests_8c029ad4')->with($table);

        $this->shouldCall('_ObjectsFreeAssetRequests_8c029cfe');
        $this->shouldWriteLongTo('_var_assetRequestTable_8c228408', $table);
        $this->shouldCall('_AsqRequestNj_8c011492')->with(
            $this->addressOf('_var_commonDirCopy_8c18ad8c'),
            $njName0,
            $this->addressOf('_var_assetRequestSlots_8c228288') + 0x08,
            0,
        );
        // Row 1 must target slots + 0x18 (one row stride) + 0x08, not slots + 0x08 again.
        $this->shouldCall('_AsqRequestNj_8c011492')->with(
            $this->addressOf('_var_commonDirCopy_8c18ad8c'),
            $njName1,
            $this->addressOf('_var_assetRequestSlots_8c228288') + 0x18 + 0x08,
            0,
        );
    }

    public function test_type_1_requests_nj_pvm_and_dat(): void
    {
        $this->setSize('_AsqRequestPvm_8c011ac0', 4);
        $this->setSize('_AsqRequestDat_8c011182', 4);

        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), 0);

        $njName = $this->allocString('O_TEST_00.njd');
        $pvmName = $this->allocString('O_TEST.pvm');
        $datName = $this->allocString('O_TEST.dat');

        $src = $this->alloc(0x10);
        $this->initUint32($src + 0x00, $njName);
        $this->initUint32($src + 0x04, $pvmName);
        $this->initUint32($src + 0x08, 3);
        $this->initUint32($src + 0x0c, $datName);

        $table = $this->alloc(self::ROW_SIZE + 4);
        $this->initUint32($table + 0x00, 1);
        $this->initUint32($table + 0x04, $src);
        $this->initUint32($table + self::ROW_SIZE, -1);

        $this->call('_ObjectsStartAssetRequests_8c029ad4')->with($table);

        $this->shouldCall('_ObjectsFreeAssetRequests_8c029cfe');
        $this->shouldWriteLongTo('_var_assetRequestTable_8c228408', $table);
        $this->shouldCall('_AsqRequestNj_8c011492')->with(
            $this->addressOf('_var_commonDirCopy_8c18ad8c'),
            $njName,
            $this->addressOf('_var_assetRequestSlots_8c228288') + 0x04,
            0,
        );
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $this->addressOf('_var_commonDirCopy_8c18ad8c'),
            $pvmName,
            $this->addressOf('_var_assetRequestSlots_8c228288') + 0x00,
            3,
            0,
        );
        $this->shouldCall('_AsqRequestDat_8c011182')->with(
            $this->addressOf('_var_commonDirCopy_8c18ad8c'),
            $datName,
            $this->addressOf('_var_assetRequestSlots_8c228288') + 0x08,
        );
    }

    public function test_type_2_requests_nj_and_pvm_then_copies_attr_bytes(): void
    {
        $this->setSize('_AsqRequestPvm_8c011ac0', 4);
        $this->setSize('_AsqRequestDat_8c011182', 4);

        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), 0);

        $njName = $this->allocString('O_TEST_00.njd');
        $pvmName = $this->allocString('O_TEST.pvm');

        $src = $this->alloc(0x10);
        $this->initUint32($src + 0x00, $njName);
        $this->initUint32($src + 0x04, $pvmName);
        $this->initUint32($src + 0x08, 5);
        $this->initUint8($src + 0x0c, 0x11);
        $this->initUint8($src + 0x0d, 0x22);
        $this->initUint8($src + 0x0e, 0x33);
        $this->initUint8($src + 0x0f, 0x44);

        $table = $this->alloc(self::ROW_SIZE + 4);
        $this->initUint32($table + 0x00, 2);
        $this->initUint32($table + 0x04, $src);
        $this->initUint32($table + self::ROW_SIZE, -1);

        $this->call('_ObjectsStartAssetRequests_8c029ad4')->with($table);

        $this->shouldCall('_ObjectsFreeAssetRequests_8c029cfe');
        $this->shouldWriteLongTo('_var_assetRequestTable_8c228408', $table);
        $this->shouldCall('_AsqRequestNj_8c011492')->with(
            $this->addressOf('_var_commonDirCopy_8c18ad8c'),
            $njName,
            $this->addressOf('_var_assetRequestSlots_8c228288') + 0x04,
            0,
        );
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $this->addressOf('_var_commonDirCopy_8c18ad8c'),
            $pvmName,
            $this->addressOf('_var_assetRequestSlots_8c228288') + 0x00,
            5,
            0,
        );
        $this->shouldWriteByte($this->addressOf('_var_assetRequestSlots_8c228288') + 0x14, 0x11);
        $this->shouldWriteByte($this->addressOf('_var_assetRequestSlots_8c228288') + 0x15, 0x22);
        $this->shouldWriteByte($this->addressOf('_var_assetRequestSlots_8c228288') + 0x16, 0x33);
        $this->shouldWriteByte($this->addressOf('_var_assetRequestSlots_8c228288') + 0x17, 0x44);
    }

    public function test_type_3_requests_nj_pvm_nj_then_copies_attr_bytes(): void
    {
        $this->setSize('_AsqRequestPvm_8c011ac0', 4);
        $this->setSize('_AsqRequestDat_8c011182', 4);

        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), 0);

        $njName = $this->allocString('O_TEST_00.njd');
        $pvmName = $this->allocString('O_TEST.pvm');
        $njmName = $this->allocString('O_TEST_00.njm');

        $src = $this->alloc(0x14);
        $this->initUint32($src + 0x00, $njName);
        $this->initUint32($src + 0x04, $pvmName);
        $this->initUint32($src + 0x08, 7);
        $this->initUint32($src + 0x0c, $njmName);
        $this->initUint8($src + 0x10, 0x11);
        $this->initUint8($src + 0x11, 0x22);
        $this->initUint8($src + 0x12, 0x33);
        $this->initUint8($src + 0x13, 0x44);

        $table = $this->alloc(self::ROW_SIZE + 4);
        $this->initUint32($table + 0x00, 3);
        $this->initUint32($table + 0x04, $src);
        $this->initUint32($table + self::ROW_SIZE, -1);

        $this->call('_ObjectsStartAssetRequests_8c029ad4')->with($table);

        $this->shouldCall('_ObjectsFreeAssetRequests_8c029cfe');
        $this->shouldWriteLongTo('_var_assetRequestTable_8c228408', $table);
        $this->shouldCall('_AsqRequestNj_8c011492')->with(
            $this->addressOf('_var_commonDirCopy_8c18ad8c'),
            $njName,
            $this->addressOf('_var_assetRequestSlots_8c228288') + 0x04,
            0,
        );
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $this->addressOf('_var_commonDirCopy_8c18ad8c'),
            $pvmName,
            $this->addressOf('_var_assetRequestSlots_8c228288') + 0x00,
            7,
            0,
        );
        $this->shouldCall('_AsqRequestNj_8c011492')->with(
            $this->addressOf('_var_commonDirCopy_8c18ad8c'),
            $njmName,
            $this->addressOf('_var_assetRequestSlots_8c228288') + 0x08,
            0,
        );
        $this->shouldWriteByte($this->addressOf('_var_assetRequestSlots_8c228288') + 0x14, 0x11);
        $this->shouldWriteByte($this->addressOf('_var_assetRequestSlots_8c228288') + 0x15, 0x22);
        $this->shouldWriteByte($this->addressOf('_var_assetRequestSlots_8c228288') + 0x16, 0x33);
        $this->shouldWriteByte($this->addressOf('_var_assetRequestSlots_8c228288') + 0x17, 0x44);
    }

    public function test_type_4_requests_nj_and_pvm_then_writes_one_attr_byte(): void
    {
        $this->setSize('_AsqRequestPvm_8c011ac0', 4);
        $this->setSize('_AsqRequestDat_8c011182', 4);

        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), 0);

        $njName = $this->allocString('O_TEST_00.njd');
        $pvmName = $this->allocString('O_TEST.pvm');

        $src = $this->alloc(0x10);
        $this->initUint32($src + 0x00, $njName);
        $this->initUint32($src + 0x04, $pvmName);
        $this->initUint32($src + 0x08, 9);
        $this->initUint32($src + 0x0c, 0x55);

        $table = $this->alloc(self::ROW_SIZE + 4);
        $this->initUint32($table + 0x00, 4);
        $this->initUint32($table + 0x04, $src);
        $this->initUint32($table + self::ROW_SIZE, -1);

        $this->call('_ObjectsStartAssetRequests_8c029ad4')->with($table);

        $this->shouldCall('_ObjectsFreeAssetRequests_8c029cfe');
        $this->shouldWriteLongTo('_var_assetRequestTable_8c228408', $table);
        $this->shouldCall('_AsqRequestNj_8c011492')->with(
            $this->addressOf('_var_commonDirCopy_8c18ad8c'),
            $njName,
            $this->addressOf('_var_assetRequestSlots_8c228288') + 0x04,
            0,
        );
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $this->addressOf('_var_commonDirCopy_8c18ad8c'),
            $pvmName,
            $this->addressOf('_var_assetRequestSlots_8c228288') + 0x00,
            9,
            0,
        );
        $this->shouldWriteByte($this->addressOf('_var_assetRequestSlots_8c228288') + 0x15, 0x55);
    }

    public function test_type_5_requests_nj_and_pvm_then_copies_trailing_8_bytes(): void
    {
        $this->setSize('_AsqRequestPvm_8c011ac0', 4);
        $this->setSize('_AsqRequestDat_8c011182', 4);
        $this->setSize('__quick_evn_mvn', 4);

        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), 0);

        $njName = $this->allocString('O_TEST_00.njd');
        $pvmName = $this->allocString('O_TEST.pvm');

        $src = $this->alloc(0x14);
        $this->initUint32($src + 0x00, $njName);
        $this->initUint32($src + 0x04, $pvmName);
        $this->initUint32($src + 0x08, 11);
        $this->initUint32($src + 0x0c, 0x11223344);
        $this->initUint32($src + 0x10, 0x55667788);

        $table = $this->alloc(self::ROW_SIZE + 4);
        $this->initUint32($table + 0x00, 5);
        $this->initUint32($table + 0x04, $src);
        $this->initUint32($table + self::ROW_SIZE, -1);

        $this->call('_ObjectsStartAssetRequests_8c029ad4')->with($table);

        $this->shouldCall('_ObjectsFreeAssetRequests_8c029cfe');
        $this->shouldWriteLongTo('_var_assetRequestTable_8c228408', $table);
        $this->shouldCall('_AsqRequestNj_8c011492')->with(
            $this->addressOf('_var_commonDirCopy_8c18ad8c'),
            $njName,
            $this->addressOf('_var_assetRequestSlots_8c228288') + 0x04,
            0,
        );
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $this->addressOf('_var_commonDirCopy_8c18ad8c'),
            $pvmName,
            $this->addressOf('_var_assetRequestSlots_8c228288') + 0x00,
            11,
            0,
        );
        // The trailing 8-byte struct copy compiles to a call to the SHC
        // runtime helper __quick_evn_mvn; its memory effect isn't visible to
        // this harness (see sortAndLoadNjQueue_8c0116b6's use of
        // __quick_odd_mvn for the same reason), so just assert the call.
        $this->shouldCall('__quick_evn_mvn');
    }

    public function test_type_6_requests_the_fixed_railway_crossing_assets(): void
    {
        $this->setSize('_AsqRequestPvm_8c011ac0', 4);
        $this->setSize('_AsqRequestDat_8c011182', 4);

        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), 0);
        $this->addressOf('_var_assetRequestSlots_8c228288');

        $table = $this->alloc(self::ROW_SIZE + 4);
        $this->initUint32($table + 0x00, 6);
        $this->initUint32($table + 0x04, 0);
        $this->initUint32($table + self::ROW_SIZE, -1);

        $this->call('_ObjectsStartAssetRequests_8c029ad4')->with($table);

        $this->shouldCall('_ObjectsFreeAssetRequests_8c029cfe');
        $this->shouldWriteLongTo('_var_assetRequestTable_8c228408', $table);
        $commonDir = $this->addressOf('_var_commonDirCopy_8c18ad8c');
        $this->shouldCall('_AsqRequestNj_8c011492')->with(
            $commonDir, 'O_FUMI_00.njd', $this->addressOf('_var_fumiGateModel_8c22840c'), 0,
        );
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $commonDir, 'O_FUMI.pvm', $this->addressOf('_var_fumiTexlist_8c228410'), 2, 0,
        );
        $this->shouldCall('_AsqRequestNj_8c011492')->with(
            $commonDir, 'O_FUMI_00.njm', $this->addressOf('_var_fumiCloseMotion_8c228414'), 0,
        );
        $this->shouldCall('_AsqRequestNj_8c011492')->with(
            $commonDir, 'O_FUMI_01.njm', $this->addressOf('_var_fumiOpenMotion_8c228418'), 0,
        );
        $this->shouldCall('_AsqRequestNj_8c011492')->with(
            $commonDir, 'O_FUMI_LAMP.njd', $this->addressOf('_var_fumiLampModel_8c22841c'), 0,
        );
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $commonDir, 'O_FUMI_LAMP.pvm', $this->addressOf('_var_fumiLampTexlist_8c228420'), 0x10, 0,
        );
        $this->shouldCall('_AsqRequestNj_8c011492')->with(
            $commonDir, 'od_chu00.njd', $this->addressOf('_var_fumiTrainModel_8c228424'), 0,
        );
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $commonDir, 'od_chu00.pvm', $this->addressOf('_var_fumiTrainTexlist_8c228428'), 0x10, 0,
        );
        $this->shouldCall('_AsqRequestNj_8c011492')->with(
            $commonDir, 'o01_tra0.njm', $this->addressOf('_var_fumiTrainMotionA_8c22842c'), 0,
        );
        $this->shouldCall('_AsqRequestNj_8c011492')->with(
            $commonDir, 'o01_tra1.njm', $this->addressOf('_var_fumiTrainMotionB_8c228430'), 0,
        );
    }
};
