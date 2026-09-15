<?php

return [
    'callBlocklist' => ['/^LAB_/', '/^L\d+$/'],
    'sourcePaths' => [
        'Z:\\app\\src' => 'src',
    ],
    'groups' => [
        [
            'tests' => [
                "tests/010fe8_heap/8c010fe8_heapInit.php",
                "tests/010fe8_heap/8c01102a_heapAlloc.php",
                "tests/010fe8_heap/8c0110c4_heapFree.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/010fe8_heap.obj",
                "build/output_test/src/010fe8_heap.obj",
            ],
        ],
        [
            'tests' => [
                "tests/012324/12324_task.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/012324_peripheral_support.obj",
                "build/output_test/src/012324_peripheral_support.obj",
            ],
        ],
        [
            'tests' => [
                "tests/014f54_text/14f54_drawSprite.php",
                "tests/014f54_text/15034_getGlyphDatOffset.php",
                "tests/014f54_text/15110_unpackGlyphTexture.php",
                "tests/014f54_text/1524c_TxtInit.php",
                "tests/014f54_text/1529c_TxtDestroy.php",
                "tests/014f54_text/152fc_TxtCreateTextBox.php",
                "tests/014f54_text/1543a_TxtPrepareTextBoxLayout.php",
                "tests/014f54_text/155e0_TxtDrawTextbox.php",
                "tests/014f54_text/1594c_FUN.php",
                "tests/014f54_text/159ac_FUN_demo.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/014f54_text.obj",
                "build/output_test/src/014f54_text.obj",
            ],
        ],
        [
            'tests' => [
                "tests/0100bc_sound/0100bc_initUknVol.php",
                "tests/0100bc_sound/010128_midiSetVol.php",
                "tests/0100bc_sound/0102d8_FUN.php",
                "tests/0100bc_sound/010972_setAdxVol.php",
                "tests/0100bc_sound/010a40_FUN_adxVol.php",
                "tests/0100bc_sound/010bae_FUN.php",
                "tests/0100bc_sound/010c2c_FUN.php",
                "tests/0100bc_sound/010cd6_snd.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/0100bc_sound.obj",
                "build/output_test/src/0100bc_sound.obj",
            ],
        ],
        [
            'tests' => ["tests/015ab8_title.php"],
            'objects' => [
                "build/output_test/src/asm/decompiled/015ab8_title.obj",
                "build/output_test/src/015ab8_title.obj",
            ],
        ],
        [
            'tests' => [
                "tests/0193c8_vm_menu/198a0_VmMenuTask.php",
                "tests/0193c8_vm_menu/19852_drawVmuWarning.php",
                "tests/0193c8_vm_menu/193c8_TaskWaitForVmsReady.php",
                "tests/0193c8_vm_menu/1940e_VmMenuMountVms.php",
                "tests/0193c8_vm_menu/1946a_TaskUnmountVms.php",
                "tests/0193c8_vm_menu/194de_VmMenuUnmountVms.php",
                "tests/0193c8_vm_menu/19504_VmMenuFreeAndClear.php",
                "tests/0193c8_vm_menu/19550_fetchVmusStatus.php",
                "tests/0193c8_vm_menu/19e44_VmMenuSwitchFromTask.php",
                "tests/0193c8_vm_menu/1967c_VmMenuUpdateVmuStatus.php",
                "tests/0193c8_vm_menu/19730_saveFileExists.php",
                "tests/0193c8_vm_menu/19788_initCursorLerp.php",
                "tests/0193c8_vm_menu/197c0_drawVmMenu.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/0193c8_vm_menu.obj",
                "build/output_test/src/0193c8_vm_menu.obj",
            ],
        ],
        [
            'tests' => [
                "tests/0207d4.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/0207d4.obj",
                "build/output_test/src/0207d4.obj",
            ],
        ],
        [
            'tests' => [
                "tests/020594/8c020594_VehicleModelPlace.php",
                "tests/020594/8c020676_unused.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/020594.obj",
                "build/output_test/src/020594.obj",
            ],
        ],
        [
            'tests' => [
                "tests/02081c/8c02081c_GeomDistanceXZ.php",
                "tests/02081c/8c020842_GeomQuadOverlap.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/02081c.obj",
                "build/output_test/src/02081c.obj",
            ],
        ],
        [
            'tests' => [
                "tests/016c58.php"
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/016c58_prompt.obj",
                "build/output_test/src/016c58_prompt.obj",
            ],
        ],
        [
            'tests' => [
                "tests/012f44_game.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/012f44_game.obj",
                "build/output_test/src/012f44_game.obj",
            ],
        ],
        [
            'tests' => [
                "tests/011120/4338_initDatQueue_8c011124.php",
                "tests/011120/4384_AsqNop_11120.php",
                "tests/011120/4458_resetDatQueue_8c01116a.php",
                "tests/011120/4532_taskLoadQueuedDats_8c0111b4.php",
                "tests/011120/4880_sortAndLoadDatQueue_8c011310.php",
                "tests/011120/5324_taskLoadQueuedNjs_8c0114cc.php",
                "tests/011120/5814_sortAndLoadNjQueue_8c0116b6.php",
                "tests/011120/6052_freeNjQueue_8c0117a4.php",
                "tests/011120/6072_initTexlistQueue_8c0117b8.php",
                "tests/011120/6142_resetTexlistQueue_8c0117fe.php",
                "tests/011120/6172_AsqRequestTexlist_1181c.php",
                "tests/011120/6206_taskLoadQueuedTexlists_8c01183e.php",
                "tests/011120/6648_loadTexlistQueue_8c0119f8.php",
                "tests/011120/6722_texlistQueueIsIdle_8c011a42.php",
                "tests/011120/6728_freeTexlistQueue_8c011a48.php",
                "tests/011120/6748_initPvmQueue_8c011a5c.php",
                "tests/011120/6848_AsqRequestPvm_11ac0.php",
                "tests/011120/6912_taskLoadQueuedPvms_8c011b00.php",
                "tests/011120/7460_sortAndLoadPvmQueue_8c011d24.php",
                "tests/011120/7714_pvmQueueIsIdle_8c011e22.php",
                "tests/011120/7720_freePvmQueue_8c011e28.php",
                "tests/011120/7740_AsqReleaseAndFreeTexlist_11e3c.php",
                "tests/011120/7776_AsqFreeTexlist_11e60.php",
                "tests/011120/7808_taskProcessQueues_8c011e80.php",
                "tests/011120/7990_AsqInitQueues_11f36.php",
                "tests/011120/8044_AsqResetQueues_11f6c.php",
                "tests/011120/8062_AsqFreeQueues_11f7e.php",
                "tests/011120/8160_AsqProcessQueues_11fe0.php",
                "tests/011120/8240_AsqRequestModels_12030.php",
                "tests/011120/8446_AsqFreeModels_120fe.php",
                "tests/011120/8544_AsqSetSeedA_12160.php",
                "tests/011120/8550_AsqGetRandomA_12166.php",
                "tests/011120/8568_AsqGetRandomInRangeA_12178.php",
                "tests/011120/8610_AsqSetSeedB_121a2.php",
                "tests/011120/8616_AsqGetRandomB_121a8.php",
                "tests/011120/8638_AsqGetRandomInRangeB_121be.php",
                "tests/011120/8680_AsqApplyButtonConfig_121e8.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/011120_asset_queues.obj",
                "build/output_test/src/011120_asset_queues.obj",
            ],
        ],
        [
            "tests" => [
                "tests/019e98_main_menu/19e98_MainMenuTask.php",
                "tests/019e98_main_menu/1a09a_switchToMainMenuTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/019e98_main_menu.obj",
                "build/output_test/src/019e98_main_menu.obj",
            ]
        ],
        [
            "tests" => [
                "tests/016d2c_course_menu/8c016d2c_CourseMenuInterpolateCursor.php",
                "tests/016d2c_course_menu/8c016dc6_cursorOffTarget.php",
                "tests/016d2c_course_menu/8c016e6c_drawInteger.php",
                "tests/016d2c_course_menu/8c016ed2_getWeekDayIndex.php",
                "tests/016d2c_course_menu/8c016ee6_CourseMenuDrawDateAndExp.php",
                "tests/016d2c_course_menu/8c016f98_instructorDialogTask.php",
                "tests/016d2c_course_menu/8c0170c6_FUN_pushDialogTask.php",
                "tests/016d2c_course_menu/8c017108_swapDialogMessageBox.php",
                "tests/016d2c_course_menu/8c017126_handleCourseMenuInput.php",
                "tests/016d2c_course_menu/8c0172dc_CourseMenuBuildCourseUnlockList.php",
                "tests/016d2c_course_menu/8c0173e6_CourseMenuApplyUnlocks.php",
                "tests/016d2c_course_menu/8c017420_buildCourseMenuDialogFlow.php",
                "tests/016d2c_course_menu/8c017590_drawCourseButtons.php",
                "tests/016d2c_course_menu/8c017718_StoryMenuTask.php",
                "tests/016d2c_course_menu/8c017a20_buildFreeRunMenuDialogFlow.php",
                "tests/016d2c_course_menu/8c017ada_FreeRunMenuTask.php",
                "tests/016d2c_course_menu/8c017d54_FUN.php",
                "tests/016d2c_course_menu/8c017e18_CourseMenuSwitchFromTask.php",
                "tests/016d2c_course_menu/8c017ef2_CourseMenuReturn.php",
                "tests/016d2c_course_menu/8c01803e_drawFixedInteger.php",
                "tests/016d2c_course_menu/8c018118_drawRouteInfo.php",
                "tests/016d2c_course_menu/8c0181b6_courseConfirmMenuTask.php",
                "tests/016d2c_course_menu/8c0184cc_courseMenuConfirmInit.php",
                "tests/016d2c_course_menu/8c01852c_requestCommonResources.php",
                "tests/016d2c_course_menu/8c018568_requestSysResgrp.php",
                "tests/016d2c_course_menu/8c0185c4_freeResourceGroup.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/016d2c_course_menu.obj",
                "build/output_test/src/016d2c_course_menu.obj",
            ]
            ],
        [
            "tests" => [
                "tests/012504_input/8c012504_task.php",
                "tests/012504_input/8c012718_inputTaskAlt.php",
                "tests/012504_input/8c0128cc_InputPushTask.php",
                "tests/012504_input/8c012970_InputDispatchTask.php",
                "tests/012504_input/8c012984_InputSetName.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/012504_input.obj",
                "build/output_test/src/012504_input.obj",
            ]
        ],
        [
            "tests" => [
                "tests/016bf4_demo_input/8c016bf4_DemoInputTask.php"
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/016bf4_demo_input.obj",
                "build/output_test/src/016bf4_demo_input.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01d290_album/1d290_AlbumDrawGrid.php",
                "tests/01d290_album/1d300_AlbumMenuTask.php",
                "tests/01d290_album/1d6e2_AlbumSwitchFromTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01d290_album.obj",
                "build/output_test/src/01d290_album.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01d7fc_results/8c01d7fc_drawScoreDigits.php",
                "tests/01d7fc_results/8c01d864_drawTextboxSprite.php",
                "tests/01d7fc_results/8c01df8e_startResultsTask.php",
                "tests/01d7fc_results/8c01e0b4_ResultShowPassedRun.php",
                "tests/01d7fc_results/8c01e24e_ResultShowFailedRun.php",
                "tests/01d7fc_results/8c01d8e0_resultsTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01d7fc_results.obj",
                "build/output_test/src/01d7fc_results.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02af78_event/8c02af78_setProgressFlag.php",
                "tests/02af78_event/8c02afbe_hasProgressFlag.php",
                "tests/02af78_event/8c02aff0_hasProgressFlagAlt.php",
                "tests/02af78_event/8c02b022_setRunEventFlag.php",
                "tests/02af78_event/8c02b030_hasRunEventFlag.php",
                "tests/02af78_event/8c02b03c_EventScanCandidates.php",
                "tests/02af78_event/8c02b170_EventPickForSegment.php",
                "tests/02af78_event/8c02b292_EventApplyFlags.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02af78_event.obj",
                "build/output_test/src/02af78_event.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02c884/8c02c884_resetStopState.php",
                "tests/02c884/8c02c8ae_pickWaitingPassengers.php",
                "tests/02c884/8c02ca96_BusStopFreeTaskGroup.php",
                "tests/02c884/8c02caba_BusStopSetup.php",
                "tests/02c884/8c02ccae_advanceStopSegment.php",
                "tests/02c884/8c02cd6a_BusStopGetSegment.php",
                "tests/02c884/8c02cd7a_BusStopGetStopArea.php",
                "tests/02c884/8c02ccc6_BusStopUpdateStopHeadings.php",
                "tests/02c884/8c02cd92_drawStopMarker.php",
                "tests/02c884/8c02ce48_BusStopUpdateArrival.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02c884_bus_stop.obj",
                "build/output_test/src/02c884_bus_stop.obj",
            ]
        ],
        [
            "tests" => [
                "tests/013ae8_route_load/13ae8_requestVehicleAssets.php",
                "tests/013ae8_route_load/13b5a_freeVehicleAssets.php",
                "tests/013ae8_route_load/13bbc_RouteLoadClearModelSlots.php",
                "tests/013ae8_route_load/13c34_syncRouteModelAssets.php",
                "tests/013ae8_route_load/13d42_finishAssetLoad.php",
                "tests/013ae8_route_load/13d78_RouteLoadStartRouteModelLoadPass.php",
                "tests/013ae8_route_load/13dae_RouteLoadFreeAllRouteModels.php",
                "tests/013ae8_route_load/13df6_syncPedestrianAssets.php",
                "tests/013ae8_route_load/13ee4_RouteLoadFreePedestrianAssets.php",
                "tests/013ae8_route_load/13f22_freeSegmentModels.php",
                "tests/013ae8_route_load/13f78_syncSegmentModels.php",
                "tests/013ae8_route_load/14088_loadRouteModels.php",
                "tests/013ae8_route_load/14322_RouteLoadResetPvmReady.php",
                "tests/013ae8_route_load/1432a_RouteLoadIsPvmReady.php",
                "tests/013ae8_route_load/14330_RouteLoadSetPvmReady.php",
                "tests/013ae8_route_load/14338_routeLoadTask.php",
                "tests/013ae8_route_load/144fc_RouteLoadPushTask.php",
                "tests/013ae8_route_load/14550_unknownSegmentReloadTask.php",
                "tests/013ae8_route_load/1468e_RouteLoadPushSegmentReloadTask.php",
                "tests/013ae8_route_load/14784_RouteLoadUnusedTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/013ae8_route_load.obj",
                "build/output_test/src/013ae8_route_load.obj",
            ]
        ],
        [
            "tests" => [
                "tests/0129cc_pause/8c0129cc_Update.php",
                "tests/0129cc_pause/8c012cbc_PauseTask.php",
                "tests/0129cc_pause/8c012d06_PauseToggleTask.php",
                "tests/0129cc_pause/8c012d5a_PauseDemoEndTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/0129cc_pause.obj",
                "build/output_test/src/0129cc_pause.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01614c_debug_menu/8c01614c_FUN.php",
                "tests/01614c_debug_menu/8c016182_DebugMenuFreeSessionAssets.php",
                "tests/01614c_debug_menu/8c01628c_saveMenuTask.php",
                "tests/01614c_debug_menu/8c016636_openSaveMenu.php",
                "tests/01614c_debug_menu/8c01666a_listMenuTask.php",
                "tests/01614c_debug_menu/8c01673a_DebugMenuOpen.php",
                "tests/01614c_debug_menu/8c016770_FUN.php",
                "tests/01614c_debug_menu/8c01677e_DebugMenuDemoRecordTask.php",
                "tests/01614c_debug_menu/8c0167c0_FUN.php",
                "tests/01614c_debug_menu/8c0167ca_replaySaveTask.php",
                "tests/01614c_debug_menu/8c016924_startReplaySave.php",
                "tests/01614c_debug_menu/8c0169bc_replayLoadTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01614c_debug_menu.obj",
                "build/output_test/src/01614c_debug_menu.obj",
            ]
        ],
        [
            "tests" => [
                "tests/018644/8c018644_loadFileTask.php",
                "tests/018644/8c018784_startVmLoad.php",
                "tests/018644/8c0187d0_FileMenuFreeBuffers.php",
                "tests/018644/8c018804_FileMenuIsSaveValid.php",
                "tests/018644/8c018862_FileMenuResetControlDefaults.php",
                "tests/018644/8c0188bc_FileMenuResetViewDefaults.php",
                "tests/018644/8c0188dc_FileMenuResetSoundDefaults.php",
                "tests/018644/8c01890a_FileMenuResetProgress.php",
                "tests/018644/8c01895e_FileMenuResetNewGame.php",
                "tests/018644/8c0189d2_FileMenuResetOptionDefaults.php",
                "tests/018644/8c0189fc_FileMenuApplySoundSettings.php",
                "tests/018644/8c018a22_buildFileList.php",
                "tests/018644/8c018aa2_drawNumber.php",
                "tests/018644/8c018b4c_drawFileCard.php",
                "tests/018644/8c018d46_drawFileSelect.php",
                "tests/018644/8c018e7e_fileSelectTask.php",
                "tests/018644/8c019334_FileMenuSwitchFromTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/018644_file_menu.obj",
                "build/output_test/src/018644_file_menu.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01a148_option/8c01a148_settingTask.php",
                "tests/01a148_option/8c01a3c0_switchToSetting.php",
                "tests/01a148_option/8c01a3da_cycleValue.php",
                "tests/01a148_option/8c01a42a_drawSensitivityBar.php",
                "tests/01a148_option/8c01a4b4_keyConfigEditExit.php",
                "tests/01a148_option/8c01a50c_keyConfigTask.php",
                "tests/01a148_option/8c01a89c_switchToKeyConfig.php",
                "tests/01a148_option/8c01a8b6_audioEditValue.php",
                "tests/01a148_option/8c01a904_soundTestFieldRead.php",
                "tests/01a148_option/8c01a926_soundTestFieldAdjust.php",
                "tests/01a148_option/8c01aaaa_soundTestFieldDraw.php",
                "tests/01a148_option/8c01ab08_audioTask.php",
                "tests/01a148_option/8c01afd8_switchToAudio.php",
                "tests/01a148_option/8c01b00a_topMenuTask.php",
                "tests/01a148_option/8c01b122_OptionSwitchToTopMenu.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01a148_option.obj",
                "build/output_test/src/01a148_option.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01b19c_system_menu/8c01b19c_SystemMenuApplyLoadedProgress.php",
                "tests/01b19c_system_menu/8c01b1c0_writeDecimalDigits.php",
                "tests/01b19c_system_menu/8c01b206_updateVmuIconText.php",
                "tests/01b19c_system_menu/8c01b26c_SystemMenuWriteToVmu.php",
                "tests/01b19c_system_menu/8c01b3ac_saveTask.php",
                "tests/01b19c_system_menu/8c01ba64_SystemMenuSwitchFromTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01b19c_system_menu.obj",
                "build/output_test/src/01b19c_system_menu.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01bb48_vm_game/8c01bb48_advanceLcdAnim.php",
                "tests/01bb48_vm_game/8c01bc44_pollBupOp.php",
                "tests/01bb48_vm_game/8c01bd30_saveExecFile.php",
                "tests/01bb48_vm_game/8c01be90_drawSelectScreen.php",
                "tests/01bb48_vm_game/8c01bf2a_selectSlot.php",
                "tests/01bb48_vm_game/8c01bfec_vmGameTask.php",
                "tests/01bb48_vm_game/8c01bde4_defragDisk.php",
                "tests/01bb48_vm_game/8c01be30_loadFileEx.php",
                "tests/01bb48_vm_game/8c01be60_rewriteExecFile.php",
                "tests/01bb48_vm_game/8c01c880_VmGameSwitchToTopMenu.php",
                "tests/01bb48_vm_game/8c01c8dc_VmGameResetLcdAnims.php",
                "tests/01bb48_vm_game/8c01c8fc_VmGameSetLcdSlot.php",
                "tests/01bb48_vm_game/8c01c910_VmGameUpdateLcd.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01bb48_vm_game.obj",
                "build/output_test/src/01bb48_vm_game.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01c980_profile_file/8c01c980_ProfileFileUpdateUnlocks.php",
                "tests/01c980_profile_file/8c01c9f2_drawUnlockGrid.php",
                "tests/01c980_profile_file/8c01cac8_drawEpisodeChecklist.php",
                "tests/01c980_profile_file/8c01cbec_updatePageLoad.php",
                "tests/01c980_profile_file/8c01ccec_menuTask.php",
                "tests/01c980_profile_file/8c01d1c4_ProfileFilePushTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01c980_profile_file.obj",
                "build/output_test/src/01c980_profile_file.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01e27c/8c01e27c_FUN.php",
                "tests/01e27c/8c01e576_initDescriptionReveal.php",
                "tests/01e27c/8c01e992_buildDialogQueue.php",
                "tests/01e27c/8c01e920_practiceCancelReturn.php",
                "tests/01e27c/8c01ead8_drawDigits.php",
                "tests/01e27c/8c01e63c_showLesson.php",
                "tests/01e27c/8c01ebc8_scrollTowardSelection.php",
                "tests/01e27c/8c01ebf2_FUN.php",
                "tests/01e27c/8c01f114_PracticeMenuLessonStart.php",
                "tests/01e27c/8c01f21c_PracticeMenuLessonRetry.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01e27c_practice_menu.obj",
                "build/output_test/src/01e27c_practice_menu.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01f3c0_ending/8c01f3c0_selectEndingDialog.php",
                "tests/01f3c0_ending/8c01f42c_updateEndingOverlay.php",
                "tests/01f3c0_ending/8c01f50e_scrollCreditsText.php",
                "tests/01f3c0_ending/8c01f658_creditsTask.php",
                "tests/01f3c0_ending/8c01f954_EndingStart.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01f3c0_ending.obj",
                "build/output_test/src/01f3c0_ending.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01fa78/8c01fa78_showMark.php",
                "tests/01fa78/8c01fa80_drawTimeDigits.php",
                "tests/01fa78/8c01fbac_drawHud.php",
                "tests/01fa78/8c01ff48_hudUpdateTask.php",
                "tests/01fa78/8c02018c_HudReset.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01fa78.obj",
                "build/output_test/src/01fa78.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02171c/8c02171c_TileStreamClearUnknownVar.php",
                "tests/02171c/8c021724_TileStreamTeardown.php",
                "tests/02171c/8c02175a_TileStreamInit.php",
                "tests/02171c/8c0217de_lookupTile.php",
                "tests/02171c/8c021810_TileStreamLoad.php",
                "tests/02171c/8c02190a_TileStreamRequestUpload.php",
                "tests/02171c/8c021a24_TileStreamReleaseAll.php",
                "tests/02171c/8c021b34_TileStreamDrawTile.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02171c_tile_stream.obj",
                "build/output_test/src/02171c_tile_stream.obj",
            ]
        ],
        [
            "tests" => [
                "tests/0222dc_fadecmd/8c0222dc_FUN.php",
                "tests/0222dc_fadecmd/8c02239c_FUN.php",
                "tests/0222dc_fadecmd/8c0223ea_FadeCmdPushCall1.php",
                "tests/0222dc_fadecmd/8c022420_FadeCmdPushCall2.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/0222dc_fadecmd.obj",
                "build/output_test/src/0222dc_fadecmd.obj",
            ]
        ],
        [
            "tests" => [
                "tests/022464/8c022464_fadeDraw_type0.php",
                "tests/022464/8c022464_fadeDraw_types1to4.php",
                "tests/022464/8c022464_fadeDraw_types5to6.php",
                "tests/022464/8c022464_fadeDraw_multi.php",
                "tests/022464/8c022560_FadeUpdate_idle.php",
                "tests/022464/8c022560_FadeUpdate_fade_start.php",
                "tests/022464/8c022560_FadeUpdate_fade_complete.php",
                "tests/022464/8c022560_FadeUpdate_fade_in.php",
                "tests/022464/8c022560_FadeUpdate_arrival_no_mirror.php",
                "tests/022464/8c022560_FadeUpdate_arrival_variants.php",
                "tests/022464/8c022560_FadeUpdate_edge_states.php",
                "tests/022464/8c0228a2_FadeStartRunTransition_fade_out_start.php",
                "tests/022464/8c022910_FadeUpdatePlain_idle.php",
                "tests/022464/8c022910_FadeUpdatePlain_fade_start.php",
                "tests/022464/8c022910_FadeUpdatePlain_fade_complete.php",
                "tests/022464/8c022910_FadeUpdatePlain_gate_draw.php",
                "tests/022464/8c022910_FadeUpdatePlain_edge_states.php",
                "tests/022464/8c022a54_fadeInTask.php",
                "tests/022464/8c022a9c_FadePushIn.php",
                "tests/022464/8c022ad0_fadeOutTask.php",
                "tests/022464/8c022b60_FadePushOut.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/022464_fade.obj",
                "build/output_test/src/022464_fade.obj",
            ]
        ],
        [
            "tests" => [
                "tests/028258/8c028258_trafficSignalTask.php",
                "tests/028258/8c02840c_snapPointToGround.php",
                "tests/028258/8c0288b2_ObjectsGetTrafficSignal.php",
                "tests/028258/8c028900_ObjectsGetTrafficSignalFrame.php",
                "tests/028258/8c0288be_ObjectsFreeTrafficSignals.php",
                "tests/028258/8c02890c_clearPedCrossingFlags.php",
                "tests/028258/8c028958_ObjectsFUN.php",
                "tests/028258/8c02897a_markPedCrossing.php",
                "tests/028258/8c028984_ObjectsFUN.php",
                "tests/028258/8c02898e_isCrossingOccupied.php",
                "tests/028258/8c028998_ObjectsFUN.php",
                "tests/028258/8c02833c_linkedTrafficSignalTask.php",
                "tests/028258/8c0283d4_setTrafficSignalLightCallback.php",
                "tests/028258/8c0283e8_execTrafficSignalGroupTask.php",
                "tests/028258/8c02845a_ObjectsInitTrafficSignals.php",
                "tests/028258/8c0289ac_advancePedPathPos.php",
                "tests/028258/8c028a38_drawPedestriansMirror.php",
                "tests/028258/8c028b74_drawPedestrians.php",
                "tests/028258/8c028dd0_FUN.php",
                "tests/028258/8c028de8_FUN.php",
                "tests/028258/8c028e00_pedestrianTask.php",
                "tests/028258/8c02903e_pedStaticObjectTask.php",
                "tests/028258/8c029078_pedGroupTask.php",
                "tests/028258/8c0293f6_pedestriansTask.php",
                "tests/028258/8c0296d6_ObjectsInitPedestrianGroups.php",
                "tests/028258/8c0297da_ObjectsFreePedestrianGroups.php",
                "tests/028258/8c029868_resolveObjectChildren.php",
                "tests/028258/8c029878_drawBlinkers.php",
                "tests/028258/8c029904_routeBlinkerTask.php",
                "tests/028258/8c029920_ObjectsInitBlinkers.php",
                "tests/028258/8c029acc_ObjectsClearAssetRequestTable.php",
                "tests/028258/8c029ad4_ObjectsStartAssetRequests.php",
                "tests/028258/8c029cfe_ObjectsFreeAssetRequests.php",
                "tests/028258/8c029e46_drawFlyByModel.php",
                "tests/028258/8c029e68_flyByModelTask.php",
                "tests/028258/8c029e94_rowFlyByTask.php",
                "tests/028258/8c029f42_initDatBlob.php",
                "tests/028258/8c029f54_advanceDatBlob.php",
                "tests/028258/8c029f2a_drawDatModel.php",
                "tests/028258/8c029fcc_rowDatTask.php",
                "tests/028258/8c02a048_drawRowModel.php",
                "tests/028258/8c02a08a_rowModelTask.php",
                "tests/028258/8c02a0d6_drawRowMotionModel.php",
                "tests/028258/8c02a120_rowMotionModelTask.php",
                "tests/028258/8c02a1b2_drawRowSimpleModel.php",
                "tests/028258/8c02a1f0_rowSimpleModelTask.php",
                "tests/028258/8c02a206_drawRowMaterialModel.php",
                "tests/028258/8c02a27c_rowMaterialModelTask.php",
                "tests/028258/8c02a47c_drawFumiCrossing.php",
                "tests/028258/8c02a4f8_fumiCrossingTask.php",
                "tests/028258/8c02a322_resolveObjectGrandchildren.php",
                "tests/028258/8c02a370_setGrandchildEvalFlags.php",
                "tests/028258/8c02a5d0_setSimpleLightCallback.php",
                "tests/028258/8c02a60e_execRowTaskGroupTask.php",
                "tests/028258/8c02a6ac_ObjectsPushTasks.php",
                "tests/028258/8c02a9fc_FUN.php",
                "tests/028258/8c02aa28_ObjectsClearMessageAssets.php",
                "tests/028258/8c02aa36_ObjectsRequestMessageAssets.php",
                "tests/028258/8c02ab7a_messageBoxTask.php",
                "tests/028258/8c02ad8c_ObjectsStartMessageBox.php",
                "tests/028258/8c02adee_ObjectsFreeMessageAssets.php",
                "tests/028258/8c02ae3e_ObjectsOpenTextbox.php",
                "tests/028258/8c02aefc_ObjectsSwapMessageBoxFor.php",
                "tests/028258/8c02af1c_ObjectsMenuTextboxText.php",
                "tests/028258/8c02af32_ObjectsFreeTextboxes.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/028258_objects.obj",
                "build/output_test/src/028258_objects.obj",
            ]
        ],
        [
            "tests" => [
                "tests/026710_traffic/8c026710_TrafficReadScriptArgs.php",
                "tests/026710_traffic/8c026748_initEntryState.php",
                "tests/026710_traffic/8c026bc4_TrafficUpdateHeading.php",
                "tests/026710_traffic/8c026ca2_TrafficAdvanceOnPath.php",
                "tests/026710_traffic/8c026da4_TrafficRelocatePlacementTable.php",
                "tests/026710_traffic/8c026dcc_TrafficMarkSignalIdsInUse.php",
                "tests/026710_traffic/8c026eaa_TrafficComputeBlockedSpeed.php",
                "tests/026710_traffic/8c026f7e_TrafficUpdateFrameFlags.php",
                "tests/026710_traffic/8c026fb0_TrafficRemainingPathDistance.php",
                "tests/026710_traffic/8c026fcc_TrafficSeekPathRecord.php",
                "tests/026710_traffic/8c027012_TrafficRunEntryScript.php",
                "tests/026710_traffic/8c0272b8_spawnEntry.php",
                "tests/026710_traffic/8c02756a_applyTrafficLighting.php",
                "tests/026710_traffic/8c0275d4_trafficUpdateTask.php",
                "tests/026710_traffic/8c02769e_TrafficInit.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/026710_traffic.obj",
                "build/output_test/src/026710_traffic.obj",
            ]
        ],
        [
            "tests" => [
                "tests/020914/8c020914_GroundQueryFindPolygon.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/020914_ground_query.obj",
                "build/output_test/src/020914_ground_query.obj",
            ]
        ],
        [
            "tests" => [
                "tests/020b6c/8c020b6c_GroundProbeTrackPolygon.php",
                "tests/020b6c/8c020f7e_GroundProbeInterpolateHeight.php",
                "tests/020b6c/8c020fe4_GroundProbeFindPolygonAtHeight.php",
                "tests/020b6c/8c021290_GroundProbeTrackPolygonAtHeight.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/020b6c_ground_probe.obj",
                "build/output_test/src/020b6c_ground_probe.obj",
            ]
        ],
        [
            "tests" => [
                "tests/0206f0_intersect/8c0206f0_IntersectSegments.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/0206f0_intersect.obj",
                "build/output_test/src/0206f0_intersect.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02786c_vehicle_parts/8c02786c_VehPartsBind.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02786c_vehicle_parts.obj",
                "build/output_test/src/02786c_vehicle_parts.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02e400_collision/8c02e400_CollisionFindTaskHit.php",
                "tests/02e400_collision/8c02e486_CollisionQueueReset.php",
                "tests/02e400_collision/8c02e48e_CollisionQueueAdd.php",
                "tests/02e400_collision/8c02e4ac_CollisionQueueTest.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02e400_collision.obj",
                "build/output_test/src/02e400_collision.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02f0c8_traffic_path_scan/8c02f0c8_TrafficPathScanBuild.php",
                "tests/02f0c8_traffic_path_scan/8c02f212_TrafficPathScanNext.php",
                "tests/02f0c8_traffic_path_scan/8c02f28a_TrafficPathScanJunctionOccupied.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02f0c8_traffic_path_scan.obj",
                "build/output_test/src/02f0c8_traffic_path_scan.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02b2f0_drive_msg/8c02b2f0_drawMsgGlyphRow.php",
                "tests/02b2f0_drive_msg/8c02b388_DriveMsgDraw.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02b2f0_drive_msg.obj",
                "build/output_test/src/02b2f0_drive_msg.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02b464_drive_points/8c02b464_adjust.php",
                "tests/02b464_drive_points/8c02b578_armCooldowns.php",
                "tests/02b464_drive_points/8c02b6d4_handleBump.php",
                "tests/02b464_drive_points/8c02b7ea_gradeWallHit.php",
                "tests/02b464_drive_points/8c02b864_gradeOffCourseSevere.php",
                "tests/02b464_drive_points/8c02b886_gradeOffCourse.php",
                "tests/02b464_drive_points/8c02b8b8_gradeSignals.php",
                "tests/02b464_drive_points/8c02b986_gradeLaneUse.php",
                "tests/02b464_drive_points/8c02bb1c_gradeIntersection.php",
                "tests/02b464_drive_points/8c02bcd8_gradeFrame.php",
                "tests/02b464_drive_points/8c02c072_taskCallback.php",
                "tests/02b464_drive_points/8c02c46a_DrivePointsReset.php",
                "tests/02b464_drive_points/8c02c624_onFadeStopEnded.php",
                "tests/02b464_drive_points/8c02c76a_onFadeRunFailed.php",
                "tests/02b464_drive_points/8c02c586_DrivePointsRunComplete.php",
                "tests/02b464_drive_points/8c02c69a_driveEndFadeTask.php",
                "tests/02b464_drive_points/8c02c738_beginDriveEnd.php",
                "tests/02b464_drive_points/8c02c784_DrivePointsOnFadeDriveEnd.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02b464_drive_points.obj",
                "build/output_test/src/02b464_drive_points.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02df3c_traffic_lookahead/8c02df3c_TrafficLookaheadInit.php",
                "tests/02df3c_traffic_lookahead/8c02dfca_TrafficLookaheadScan.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02df3c_traffic_lookahead.obj",
                "build/output_test/src/02df3c_traffic_lookahead.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02f320_replay_codec/8c02f320_ReplayCodecInit.php",
                "tests/02f320_replay_codec/8c02f3a0_ReplayCodecGetBit.php",
                "tests/02f320_replay_codec/8c02f3e0_ReplayCodecGetBits.php",
                "tests/02f320_replay_codec/8c02f49c_ReplayCodecPutBit.php",
                "tests/02f320_replay_codec/8c02f4da_ReplayCodecPutBits.php",
                "tests/02f320_replay_codec/8c02f556_ReplayCodecSwapNodes.php",
                "tests/02f320_replay_codec/8c02f58a_ReplayCodecListInsert.php",
                "tests/02f320_replay_codec/8c02f636_ReplayCodecLzwFindChild.php",
                "tests/02f320_replay_codec/8c02f668_ReplayCodecLzwInsertChild.php",
                "tests/02f320_replay_codec/8c02f6ac_ReplayCodecLzwRemoveChild.php",
                "tests/02f320_replay_codec/8c02f704_ReplayCodecInitTables.php",
                "tests/02f320_replay_codec/8c02f740_ReplayCodecExtendDict.php",
                "tests/02f320_replay_codec/8c02f824_ReplayCodecWriteCode.php",
                "tests/02f320_replay_codec/8c02f892_ReplayCodecReadCode.php",
                "tests/02f320_replay_codec/8c02f934_ReplayCodecPack.php",
                "tests/02f320_replay_codec/8c02fa14_ReplayCodecUnpack.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02f320_replay_codec.obj",
                "build/output_test/src/02f320_replay_codec.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02e2dc_bus_collision/8c02e2dc_BusCollisionFindHit.php",
                "tests/02e2dc_bus_collision/8c02e35a_findNextHit.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02e2dc_bus_collision.obj",
                "build/output_test/src/02e2dc_bus_collision.obj",
            ]
        ],
        [
            "tests" => [
                "tests/023310_bus_init/8c023310_busInitPlaceBus.php",
                "tests/023310_bus_init/8c023610_BusInitStart.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/023310_bus_init.obj",
                "build/output_test/src/023310_bus_init.obj",
            ]
        ],
        [
            "tests" => [
                "tests/022bdc_bus/8c022bdc_BusTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/022bdc_bus.obj",
                "build/output_test/src/022bdc_bus.obj",
            ]
        ],
        [
            "tests" => [
                "tests/024b4c_bus_render/8c024b4c_BusRenderSaveCameraState.php",
                "tests/024b4c_bus_render/8c024b86_BusRenderRestoreCameraState.php",
                "tests/024b4c_bus_render/8c024f32_BusRenderApplyCameraMode.php",
                "tests/024b4c_bus_render/8c024bb8_BusRenderDrawBusModel.php",
                "tests/024b4c_bus_render/8c024d6c_positionCamera.php",
                "tests/024b4c_bus_render/8c025078_BusRenderUpdateCamera.php",
                "tests/024b4c_bus_render/8c025604_BusRenderUpdateMirrorCamera.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/024b4c_bus_render.obj",
                "build/output_test/src/024b4c_bus_render.obj",
            ]
        ],
        [
            "tests" => [
                "tests/023938_bus_drive/8c023bce_BusDriveStop.php",
                "tests/023938_bus_drive/8c023bea_busDriveDecelerate.php",
                "tests/023938_bus_drive/8c023938_FUN.php",
                "tests/023938_bus_drive/8c023cba_FUN.php",
                "tests/023938_bus_drive/8c023e7e_FUN.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/023938_bus_drive.obj",
                "build/output_test/src/023938_bus_drive.obj",
            ]
        ],
        [
            "tests" => [
                "tests/025870_demo/8c025870_DemoBoardingCamera.php",
                "tests/025870_demo/8c0258ba_applyShotPosition.php",
                "tests/025870_demo/8c025906_DemoUpdateCamera.php",
                "tests/025870_demo/8c0259e8_demoShotTask.php",
                "tests/025870_demo/8c025af4_DemoStartTour.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/025870_demo.obj",
                "build/output_test/src/025870_demo.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02e51c_attr_query/8c02e51c_AttrQueryFindConvexPolygon.php",
                "tests/02e51c_attr_query/8c02eab4_AttrQueryFindConvexPolygonAtHeight.php",
                "tests/02e51c_attr_query/8c02e69c_AttrQueryFindPolygon.php",
                "tests/02e51c_attr_query/8c02ec50_AttrQueryFindPolygonAtHeight.php",
                "tests/02e51c_attr_query/8c02f08a_AttrQueryRegionOccupied.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02e51c_attr_query.obj",
                "build/output_test/src/02e51c_attr_query.obj",
            ]
        ],
        [
            "tests" => [
                "tests/027958_bus_draw/8c027958_BusDrawUpdateModels.php",
                "tests/027958_bus_draw/8c0281ac_BusDrawSignal.php",
                "tests/027958_bus_draw/8c028206_BusDrawSignalAttachment.php",
                "tests/027958_bus_draw/8c028022_BusDrawFadeLights.php",
                "tests/027958_bus_draw/8c027a88_drawAhead.php",
                "tests/027958_bus_draw/8c027bac_drawMirror.php",
                "tests/027958_bus_draw/8c027c3c_BusDrawPlaceEntity.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/027958_bus_draw.obj",
                "build/output_test/src/027958_bus_draw.obj",
            ]
        ],
        [
            "tests" => [
                "tests/025b98_traffic_drive/8c02656a_TrafficDriveDecoration.php",
                "tests/025b98_traffic_drive/8c025b98_TrafficDriveVehicle.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/025b98_traffic_drive.obj",
                "build/output_test/src/025b98_traffic_drive.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02d19c_passenger/8c02d19c_drawPassengerSprite.php",
                "tests/02d19c_passenger/8c02d1f4_drawInterior.php",
                "tests/02d19c_passenger/8c02d5ca_PassengerSeatedTask.php",
                "tests/02d19c_passenger/8c02d5d8_setCountUpStep.php",
                "tests/02d19c_passenger/8c02d21c_PassengerBoardTask.php",
                "tests/02d19c_passenger/8c02d46c_PassengerExitTask.php",
                "tests/02d19c_passenger/8c02d644_PassengerStopSceneTask.php",
                "tests/02d19c_passenger/8c02d8f0_PassengerSkipStopTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02d19c_passenger.obj",
                "build/output_test/src/02d19c_passenger.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02d968_stop_spawn/8c02d968_StopSpawnInit.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02d968_stop_spawn.obj",
                "build/output_test/src/02d968_stop_spawn.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02d06c_stop_draw/8c02d06c_StopDrawWaitingPassengers.php",
                "tests/02d06c_stop_draw/8c02d0fc_StopDrawLightBegin.php",
                "tests/02d06c_stop_draw/8c02d146_StopDrawLightEnd.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02d06c_stop_draw.obj",
                "build/output_test/src/02d06c_stop_draw.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02412c/8c02412c_BusLineAdvance.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02412c.obj",
                "build/output_test/src/02412c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/020214/8c020214_DriveCueTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/020214.obj",
                "build/output_test/src/020214.obj",
            ]
        ],
        [
            "tests" => [
                "tests/021b9c/8c021b9c_drawTileGrid.php",
                "tests/021b9c/8c021ec4_drawTileGridMirror.php",
                "tests/021b9c/8c0221d0_TileDrawEnqueueTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/021b9c.obj",
                "build/output_test/src/021b9c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/024280_bus_input/8c024280_BusInputCapMirrorTraffic.php",
                "tests/024280_bus_input/8c0242ce_debugGearOverride.php",
                "tests/024280_bus_input/8c024320_applyThrottle.php",
                "tests/024280_bus_input/8c024530_applyBraking.php",
                "tests/024280_bus_input/8c024606_applyBrakingSfx.php",
                "tests/024280_bus_input/8c0246b2_BusInputUpdate.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/024280_bus_input.obj",
                "build/output_test/src/024280_bus_input.obj",
            ]
        ],
    ],
];
