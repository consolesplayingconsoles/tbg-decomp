<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /** Allocates a BusState-sized buffer and wires field_0x018/01c/020/024/
     * 028/02c/030/034/038/03c/040/044/048 to fresh scratch words, returning
     * [$bus, $targets] where $targets maps each field offset to the address
     * it points at. */
    private function makeBus(): array
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x400);
        $bus = $this->addressOf('_var_busState_8c1bb9d0');
        $this->doNotRandomizeMemory();

        $targets = [];
        foreach ([0x018, 0x01c, 0x020, 0x024, 0x028] as $off) {
            // These are dereferenced at +0x14/+0x18/+0x1c, so give them
            // enough room.
            $t = $this->alloc(0x20);
            $this->initUint32($bus + $off, $t);
            $targets[$off] = $t;
        }
        foreach ([0x02c, 0x030, 0x034, 0x038, 0x03c, 0x040, 0x044, 0x048] as $off) {
            $t = $this->alloc(4);
            $this->initUint32($bus + $off, $t);
            $targets[$off] = $t;
        }

        return [$bus, $targets];
    }

    /** Sets up the four copied source fields and the blinker mask, and
     * queues the ordered expectations for the seven unconditional copies
     * plus the five blinker-bit lights that run before the field_0x000
     * switch. */
    private function assertBaseline(array $t, int $distance, int $ang074, int $acc078, int $ang07c, int $blinker): void
    {
        $this->shouldWriteLong($t[0x018] + 0x14, $acc078);
        $this->shouldWriteLong($t[0x018] + 0x1c, $ang07c);
        $this->shouldWriteLong($t[0x01c] + 0x14, $distance);
        $this->shouldWriteLong($t[0x01c] + 0x18, $ang074);
        $this->shouldWriteLong($t[0x020] + 0x14, $distance);
        $this->shouldWriteLong($t[0x020] + 0x18, $ang074);
        $this->shouldWriteLong($t[0x024] + 0x14, $distance);

        $this->shouldWriteLong($t[0x02c], ($blinker & 0x1) ? 0x37 : 0x3f);
        $this->shouldWriteLong($t[0x030], ($blinker & 0x2) ? 0x37 : 0x3f);
        $this->shouldWriteLong($t[0x034], ($blinker & 0x4) ? 0x37 : 0x3f);
        $this->shouldWriteLong($t[0x038], ($blinker & 0x8) ? 0x37 : 0x3f);
        $this->shouldWriteLong($t[0x03c], ($blinker & 0x10) ? 0x37 : 0x3f);
    }

    /** Sets up a bus, seeds field_0x000/070/074/078/07c/080, calls the
     * function, and asserts the baseline writes. Returns $t so the caller
     * can add phase-specific assertions -- but assertBaseline() already
     * queued its writes, so the caller's phase-specific shouldWriteLong()
     * calls append after them, matching actual execution order. */
    private function runPhase(int $phase, int $distance, int $ang074, int $acc078, int $ang07c, int $blinker): array
    {
        [$bus, $t] = $this->makeBus();

        $this->initUint32($bus + 0x000, $phase);
        $this->initUint32($bus + 0x070, $distance);
        $this->initUint32($bus + 0x074, $ang074);
        $this->initUint32($bus + 0x078, $acc078);
        $this->initUint32($bus + 0x07c, $ang07c);
        $this->initUint32($bus + 0x080, $blinker);

        $this->call('_BusDrawUpdateModels_8c027958')->with($bus);

        $this->assertBaseline($t, $distance, $ang074, $acc078, $ang07c, $blinker);

        return $t;
    }

    public function test_copies_and_all_blinkers_off_default_phase(): void
    {
        $this->runPhase(1, 0x11, 0x22, 0x33, 0x44, 0); // phase 1: unmatched, no extra write
    }

    public function test_all_blinkers_on(): void
    {
        $this->runPhase(8, 0, 0, 0, 0, 0x1f); // phase 8: unmatched, no extra write
    }

    public function test_phase_0x14_bit0x40_clear(): void
    {
        $t = $this->runPhase(0x14, 0x55, 0, 0, 0, 0); // bit 0x40 clear
        $this->shouldWriteLong($t[0x028] + 0x14, 0x55);
        $this->shouldWriteLong($t[0x044], 0x3f);
    }

    public function test_phase_0x16_bit0x40_set(): void
    {
        $t = $this->runPhase(0x16, 0x66, 0, 0, 0, 0x40); // bit 0x40 set
        $this->shouldWriteLong($t[0x028] + 0x14, 0x66);
        $this->shouldWriteLong($t[0x044], 0x37);
    }

    public function test_phase_0x0e_no_extra_light(): void
    {
        $t = $this->runPhase(0x0e, 0x77, 0, 0, 0, 0);
        $this->shouldWriteLong($t[0x028] + 0x14, 0x77);
    }

    public function test_phase_0x1c_bit0x20_clear(): void
    {
        $t = $this->runPhase(0x1c, 0, 0, 0, 0, 0); // bit 0x20 clear
        $this->shouldWriteLong($t[0x040], 0x3f);
    }

    public function test_phase_0x1e_bit0x20_set(): void
    {
        $t = $this->runPhase(0x1e, 0, 0, 0, 0, 0x20); // bit 0x20 set
        $this->shouldWriteLong($t[0x040], 0x37);
    }

    public function test_phase_0x1a_bit0x40_set(): void
    {
        $t = $this->runPhase(0x1a, 0, 0, 0, 0, 0x40); // bit 0x40 set
        $this->shouldWriteLong($t[0x048], 0x37);
    }

    public function test_phase_0x1a_bit0x40_clear(): void
    {
        $t = $this->runPhase(0x1a, 0, 0, 0, 0, 0); // bit 0x40 clear
        $this->shouldWriteLong($t[0x048], 0x3f);
    }
};
