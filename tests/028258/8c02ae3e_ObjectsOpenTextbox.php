<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // var_messageTextBoxA_8c1bc404/408/40c are consecutive words in the real binary
    // (accessed as (&var_messageTextBoxA_8c1bc404)[idx] elsewhere in this unit); force the
    // same adjacency here since the linker doesn't place separately
    // declared symbols next to each other by default.
    private function resolveSymbols(): void
    {
        $this->setSize('_TxtInit_8c01524c', 4);
        $this->setSize('_TxtCreateTextBox_8c0152fc', 4);

        $base = $this->addressOf('_var_messageTextBoxA_8c1bc404');
        $this->rellocate('_var_messageTextBoxB_8c1bc408', $base + 4);
        $this->rellocate('_var_messageTextBoxIndex_8c1bc40c', $base + 8);
    }

    // x2, y2, enable_offset overflow to the stack (register-passed args are
    // x, y, priority (float, own register bank), width, height). Check them
    // by reading the stack directly, since ->with() can't be trusted to
    // place stack-passed args at the offset their declaration order implies
    // once a call reads them back from its own incoming stack slots.
    private function checkTextBoxStackArgs(int $x2, int $y2, int $enableOffset): callable
    {
        $mask = 0xffffffff;
        $x2 &= $mask;
        $y2 &= $mask;
        $enableOffset &= $mask;

        return function () use ($x2, $y2, $enableOffset) {
            $sp = $this->registers[15]->value;
            $a = $this->memory->readUInt32($sp + 0)->value;
            $b = $this->memory->readUInt32($sp + 4)->value;
            $c = $this->memory->readUInt32($sp + 8)->value;

            if ($a !== $enableOffset || $b !== $y2 || $c !== $x2) {
                throw new \Exception(sprintf(
                    "Unexpected TxtCreateTextBox stack args: got [%d, %d, %d], expecting enable_offset=%d, y2=%d, x2=%d",
                    $a, $b, $c, $enableOffset, $y2, $x2
                ));
            }
        };
    }

    public function test_no_existing_textbox_skips_teardown(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_messageTextBoxA_8c1bc404'), 0xffffffff);

        $box0 = $this->alloc(4);
        $box1 = $this->alloc(4);

        $this->call('_ObjectsOpenTextbox_8c02ae3e')
            ->with(0x20, 0x180, -2.0, 0x240, 0x40, 0, 0, 0xffffffff);

        $this->shouldCall('_TxtInit_8c01524c');

        $this->shouldCall('_TxtCreateTextBox_8c0152fc')
            ->with(0x20, 0x180, -2.0, 0x240, 0x40)
            ->do($this->checkTextBoxStackArgs(0, 0, -1))
            ->andReturn($box0);
        $this->shouldWriteLongTo('_var_messageTextBoxA_8c1bc404', $box0);

        $this->shouldCall('_TxtCreateTextBox_8c0152fc')
            ->with(0x20, 0x180, -2.0, 0x240, 0x40)
            ->do($this->checkTextBoxStackArgs(0, 0, -1))
            ->andReturn($box1);
        $this->shouldWriteLongTo('_var_messageTextBoxB_8c1bc408', $box1);

        $this->shouldWriteLongTo('_var_messageTextBoxIndex_8c1bc40c', 1);
        $this->shouldWriteLongTo('_var_menuTextboxCharLimit_8c225fb8', 0);
    }

    public function test_existing_textbox_is_torn_down_first(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_messageTextBoxA_8c1bc404'), 0x8c300000);

        $box0 = $this->alloc(4);
        $box1 = $this->alloc(4);

        $this->call('_ObjectsOpenTextbox_8c02ae3e')
            ->with(0x20, 0x178, -2.0, 0x240, 0x40, 0, 0, 0xffffffff);

        $this->shouldCall('_ObjectsFreeTextboxes_8c02af32');

        $this->shouldCall('_TxtInit_8c01524c');

        $this->shouldCall('_TxtCreateTextBox_8c0152fc')
            ->with(0x20, 0x178, -2.0, 0x240, 0x40)
            ->do($this->checkTextBoxStackArgs(0, 0, -1))
            ->andReturn($box0);
        $this->shouldWriteLongTo('_var_messageTextBoxA_8c1bc404', $box0);

        $this->shouldCall('_TxtCreateTextBox_8c0152fc')
            ->with(0x20, 0x178, -2.0, 0x240, 0x40)
            ->do($this->checkTextBoxStackArgs(0, 0, -1))
            ->andReturn($box1);
        $this->shouldWriteLongTo('_var_messageTextBoxB_8c1bc408', $box1);

        $this->shouldWriteLongTo('_var_messageTextBoxIndex_8c1bc40c', 1);
        $this->shouldWriteLongTo('_var_menuTextboxCharLimit_8c225fb8', 0);
    }

    public function test_enable_offset_disabled_state_is_passed_through(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_messageTextBoxA_8c1bc404'), 0xffffffff);

        $box0 = $this->alloc(4);
        $box1 = $this->alloc(4);

        $this->call('_ObjectsOpenTextbox_8c02ae3e')
            ->with(0x20, 0x180, -2.0, 0x240, 0x40, 0, 0, 0);

        $this->shouldCall('_TxtInit_8c01524c');

        $this->shouldCall('_TxtCreateTextBox_8c0152fc')
            ->with(0x20, 0x180, -2.0, 0x240, 0x40)
            ->do($this->checkTextBoxStackArgs(0, 0, 0))
            ->andReturn($box0);
        $this->shouldWriteLongTo('_var_messageTextBoxA_8c1bc404', $box0);

        $this->shouldCall('_TxtCreateTextBox_8c0152fc')
            ->with(0x20, 0x180, -2.0, 0x240, 0x40)
            ->do($this->checkTextBoxStackArgs(0, 0, 0))
            ->andReturn($box1);
        $this->shouldWriteLongTo('_var_messageTextBoxB_8c1bc408', $box1);

        $this->shouldWriteLongTo('_var_messageTextBoxIndex_8c1bc40c', 1);
        $this->shouldWriteLongTo('_var_menuTextboxCharLimit_8c225fb8', 0);
    }
};
