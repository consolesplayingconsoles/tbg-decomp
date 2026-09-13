<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private int $b14;
    private int $b18;

    private function setup(int $selectedVm, int $status, int $count, array $saves): void
    {
        $this->setSize('_var_8c226014', 4);
        $this->setSize('_var_8c226018', 0x30);
        $this->setSize('_var_vmuStatus_8c226048', 0x24);
        $this->setSize('_var_selectedVm_8c1ba34c', 4);
        $this->setSize('_var_loadedSaveCount_8c22600c', 4);
        $this->setSize('_var_8c225fe4', 0x28);

        $this->b14 = $this->addressOf('_var_8c226014');
        $this->b18 = $this->addressOf('_var_8c226018');

        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), $selectedVm);
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048') + $selectedVm * 4, $status);
        $this->initUint32($this->addressOf('_var_loadedSaveCount_8c22600c'), $count);
        foreach ($saves as $i => $v) {
            $this->initUint32($this->addressOf('_var_8c225fe4') + $i * 4, $v);
        }
    }

    /* Appends the saved slots, then fills the tail of the 12-slot list with 0xb. */
    private function expectBody(int $dst, array $saves, int $count): void
    {
        foreach ($saves as $i => $v) {
            $this->shouldWriteLong($this->b18 + $dst * 4, $v);
            $dst++;
        }
        for (; $dst < 12; $dst++) {
            $this->shouldWriteLong($this->b18 + $dst * 4, 0xb);
        }
        $this->shouldWriteLong($this->b14, $count);   // leading (0/1) + count
    }

    /* Status 4: VMU takes a NEW FILE card (0xa), then two saves. */
    public function test_status4_prepends_new_file_card(): void
    {
        $saves = [100, 200];
        $this->setup(0, 4, 2, $saves);

        $this->call('_buildFileList_8c018a22');

        $this->shouldWriteLong($this->b14, 0);
        $this->shouldWriteLong($this->b18 + 0, 0xa);   // NEW FILE card
        $this->shouldWriteLong($this->b14, 1);
        $this->expectBody(1, $saves, 1 + 2);
    }

    /* Status 6 with room (< 10 saves): also prepends the NEW FILE card. */
    public function test_status6_with_room_prepends_card(): void
    {
        $saves = [11, 22, 33];
        $this->setup(1, 6, 3, $saves);

        $this->call('_buildFileList_8c018a22');

        $this->shouldWriteLong($this->b14, 0);
        $this->shouldWriteLong($this->b18 + 0, 0xa);
        $this->shouldWriteLong($this->b14, 1);
        $this->expectBody(1, $saves, 1 + 3);
    }

    /* Status other than 4/6: no card, saves start at slot 0. */
    public function test_other_status_no_card(): void
    {
        $saves = [300, 400];
        $this->setup(2, 0, 2, $saves);

        $this->call('_buildFileList_8c018a22');

        $this->shouldWriteLong($this->b14, 0);
        $this->expectBody(0, $saves, 2);
    }

    /* Status 6 but full (10 saves): no card; list fills exactly, two 0xb tail slots. */
    public function test_status6_full_no_card(): void
    {
        $saves = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
        $this->setup(3, 6, 10, $saves);

        $this->call('_buildFileList_8c018a22');

        $this->shouldWriteLong($this->b14, 0);
        $this->expectBody(0, $saves, 10);
    }
};
