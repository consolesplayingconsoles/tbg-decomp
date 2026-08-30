<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_scrolls_up_and_clamps_to_6()
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x84);
        $menuState = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($menuState + 0x38, 20); // selected_0x38
        $this->initUint32($menuState + 0x44, 0);  // scrollTopRow_0x44

        $this->call('_scrollTowardSelection_8c01ebc8');

        $this->shouldWriteLong($menuState + 0x44, 16);
        $this->shouldWriteLong($menuState + 0x44, 6);
    }

    public function test_scrolls_up_without_clamp()
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x84);
        $menuState = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($menuState + 0x38, 5); // selected_0x38
        $this->initUint32($menuState + 0x44, 0); // scrollTopRow_0x44

        $this->call('_scrollTowardSelection_8c01ebc8');

        $this->shouldWriteLong($menuState + 0x44, 1);
    }

    public function test_scrolls_down_to_selection()
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x84);
        $menuState = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($menuState + 0x38, 2); // selected_0x38
        $this->initUint32($menuState + 0x44, 6); // scrollTopRow_0x44

        $this->call('_scrollTowardSelection_8c01ebc8');

        $this->shouldWriteLong($menuState + 0x44, 2);
    }

    public function test_no_change_when_within_window()
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x84);
        $menuState = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($menuState + 0x38, 4); // selected_0x38
        $this->initUint32($menuState + 0x44, 3); // scrollTopRow_0x44

        $this->call('_scrollTowardSelection_8c01ebc8');
    }
};
