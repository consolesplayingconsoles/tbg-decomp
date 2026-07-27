<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private const RECORDS = 0x44; // 9 course records, stride 8
    private const EXP = 0x90;
    private const SIZE = 0x94;

    /* Day 1, empty records, EXP at the cap -> plausible. */
    public function test_valid_minimum(): void
    {
        $this->doNotRandomizeMemory();
        $save = $this->alloc(self::SIZE);
        $this->initUint32($save + 0x00, 1);
        $this->initUint32($save + self::EXP, 99999);

        $this->call('_FileMenuIsSaveValid_8c018804')->with($save);

        $this->shouldReturn(1);
    }

    /* Day 30 and every record field at its upper bound (flags 1, ranks 3) -> still valid. */
    public function test_valid_upper_bounds(): void
    {
        $this->doNotRandomizeMemory();
        $save = $this->alloc(self::SIZE);
        $this->initUint32($save + 0x00, 30);
        for ($i = 0; $i < 9; $i++) {
            $rec = $save + self::RECORDS + $i * 8;
            $this->initUint8($rec + 0, 1);
            $this->initUint8($rec + 1, 1);
            $this->initUint8($rec + 2, 1);
            $this->initUint8($rec + 3, 3);
            $this->initUint8($rec + 4, 3);
        }
        $this->initUint32($save + self::EXP, 99999);

        $this->call('_FileMenuIsSaveValid_8c018804')->with($save);

        $this->shouldReturn(1);
    }

    /* Day 0 is below range. */
    public function test_invalid_day_zero(): void
    {
        $this->doNotRandomizeMemory();
        $save = $this->alloc(self::SIZE);
        $this->initUint32($save + 0x00, 0);

        $this->call('_FileMenuIsSaveValid_8c018804')->with($save);

        $this->shouldReturn(0);
    }

    /* Day 31 is above range. */
    public function test_invalid_day_too_high(): void
    {
        $this->doNotRandomizeMemory();
        $save = $this->alloc(self::SIZE);
        $this->initUint32($save + 0x00, 31);

        $this->call('_FileMenuIsSaveValid_8c018804')->with($save);

        $this->shouldReturn(0);
    }

    /* A flag byte > 1 in the first record fails validation. */
    public function test_invalid_flag_out_of_range(): void
    {
        $this->doNotRandomizeMemory();
        $save = $this->alloc(self::SIZE);
        $this->initUint32($save + 0x00, 1);
        $this->initUint32($save + self::EXP, 0);
        $this->initUint8($save + self::RECORDS + 0, 2);

        $this->call('_FileMenuIsSaveValid_8c018804')->with($save);

        $this->shouldReturn(0);
    }

    /* A rank byte > 3 in the last record fails (proves the loop reaches record 8). */
    public function test_invalid_rank_in_last_record(): void
    {
        $this->doNotRandomizeMemory();
        $save = $this->alloc(self::SIZE);
        $this->initUint32($save + 0x00, 1);
        $this->initUint32($save + self::EXP, 0);
        $this->initUint8($save + self::RECORDS + 8 * 8 + 3, 4);

        $this->call('_FileMenuIsSaveValid_8c018804')->with($save);

        $this->shouldReturn(0);
    }

    /* EXP over the 99999 cap fails. */
    public function test_invalid_exp_over_cap(): void
    {
        $this->doNotRandomizeMemory();
        $save = $this->alloc(self::SIZE);
        $this->initUint32($save + 0x00, 1);
        $this->initUint32($save + self::EXP, 100000);

        $this->call('_FileMenuIsSaveValid_8c018804')->with($save);

        $this->shouldReturn(0);
    }
};
