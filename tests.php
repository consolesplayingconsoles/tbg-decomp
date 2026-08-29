<?php

return [
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
                "build/output_test/010fe8_heap_src.obj",
                "build/output_test/010fe8_heap_c.obj",
            ],
        ],
        [
            'tests' => [
                "tests/012324/12324_task.php",
            ],
            'objects' => [
                "build/output_test/012324_peripheral_support_src.obj",
                "build/output_test/012324_peripheral_support_c.obj",
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
                "build/output_test/014f54_text_src.obj",
                "build/output_test/014f54_text_c.obj",
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
                "build/output_test/0100bc_sound_src.obj",
                "build/output_test/0100bc_sound_c.obj",
            ],
        ],
        [
            'tests' => ["tests/015ab8_title.php"],
            'objects' => [
                "build/output_test/015ab8_title_src.obj",
                "build/output_test/015ab8_title_c.obj",
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
                "build/output_test/0193c8_vm_menu_src.obj",
                "build/output_test/0193c8_vm_menu_c.obj",
            ],
        ],
        [
            'tests' => [
                "tests/0207d4.php",
            ],
            'objects' => [
                "build/output_test/0207d4_src.obj",
                "build/output_test/0207d4_c.obj",
            ],
        ],
        [
            'tests' => [
                "tests/016c58.php"
            ],
            'objects' => [
                "build/output_test/016c58_prompt_src.obj",
                "build/output_test/016c58_prompt_c.obj",
            ],
        ],
        [
            'tests' => [
                "tests/012f44_game.php",
            ],
            'objects' => [
                "build/output_test/012f44_game_src.obj",
                "build/output_test/012f44_game_c.obj",
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
                "build/output_test/011120_asset_queues_src.obj",
                "build/output_test/011120_asset_queues_c.obj",
            ],
        ],
        [
            "tests" => [
                "tests/019e98_main_menu/19e98_MainMenuTask.php",
                "tests/019e98_main_menu/1a09a_switchToMainMenuTask.php",
            ],
            "objects" => [
                "build/output_test/019e98_main_menu_src.obj",
                "build/output_test/019e98_main_menu_c.obj",
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
                "build/output_test/016d2c_course_menu_src.obj",
                "build/output_test/016d2c_course_menu_c.obj",
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
                "build/output_test/012504_input_src.obj",
                "build/output_test/012504_input_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/016bf4_demo_input/8c016bf4_DemoInputTask.php"
            ],
            "objects" => [
                "build/output_test/016bf4_demo_input_src.obj",
                "build/output_test/016bf4_demo_input_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01d290_album/1d290_AlbumDrawGrid.php",
                "tests/01d290_album/1d300_AlbumMenuTask.php",
                "tests/01d290_album/1d6e2_AlbumSwitchFromTask.php",
            ],
            "objects" => [
                "build/output_test/01d290_album_src.obj",
                "build/output_test/01d290_album_c.obj",
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
                "build/output_test/01d7fc_results_src.obj",
                "build/output_test/01d7fc_results_c.obj",
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
                "build/output_test/02af78_event_src.obj",
                "build/output_test/02af78_event_c.obj",
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
                "build/output_test/02c884_bus_stop_src.obj",
                "build/output_test/02c884_bus_stop_c.obj",
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
                "build/output_test/013ae8_route_load_src.obj",
                "build/output_test/013ae8_route_load_c.obj",
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
                "build/output_test/0129cc_pause_src.obj",
                "build/output_test/0129cc_pause_c.obj",
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
                "build/output_test/01614c_debug_menu_src.obj",
                "build/output_test/01614c_debug_menu_c.obj",
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
                "build/output_test/018644_file_menu_src.obj",
                "build/output_test/018644_file_menu_c.obj",
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
                "build/output_test/01a148_option_src.obj",
                "build/output_test/01a148_option_c.obj",
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
                "build/output_test/01b19c_system_menu_src.obj",
                "build/output_test/01b19c_system_menu_c.obj",
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
                "build/output_test/01bb48_vm_game_src.obj",
                "build/output_test/01bb48_vm_game_c.obj",
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
                "build/output_test/01c980_profile_file_src.obj",
                "build/output_test/01c980_profile_file_c.obj",
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
                "tests/01e27c/8c01f114_practiceLessonStart.php",
                "tests/01e27c/8c01f21c_practiceLessonRetry.php",
            ],
            "objects" => [
                "build/output_test/01e27c_practice_menu_src.obj",
                "build/output_test/01e27c_practice_menu_c.obj",
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
                "build/output_test/01f3c0_ending_src.obj",
                "build/output_test/01f3c0_ending_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01fa78/8c01fa78_FUN.php",
                "tests/01fa78/8c01fa80_FUN.php",
                "tests/01fa78/8c01fbac_FUN.php",
                "tests/01fa78/8c01ff48_FUN.php",
                "tests/01fa78/8c02018c_FUN.php",
            ],
            "objects" => [
                "build/output_test/01fa78_src.obj",
                "build/output_test/01fa78_c.obj",
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
                "build/output_test/02171c_tile_stream_src.obj",
                "build/output_test/02171c_tile_stream_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/fadecmd/8c0222dc_FUN.php",
                "tests/fadecmd/8c02239c_FUN.php",
                "tests/fadecmd/8c0223ea_FadeCmdPushCall1.php",
                "tests/fadecmd/8c022420_FadeCmdPushCall2.php",
            ],
            "objects" => [
                "build/output_test/0222dc_fadecmd_src.obj",
                "build/output_test/0222dc_fadecmd_c.obj",
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
                "build/output_test/022464_fade_src.obj",
                "build/output_test/022464_fade_c.obj",
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
                "build/output_test/028258_objects_src.obj",
                "build/output_test/028258_objects_c.obj",
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
                "build/output_test/026710_traffic_src.obj",
                "build/output_test/026710_traffic_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/020914/8c020914_GroundQueryFindPolygon.php",
            ],
            "objects" => [
                "build/output_test/020914_ground_query_src.obj",
                "build/output_test/020914_ground_query_c.obj",
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
                "build/output_test/020b6c_ground_probe_src.obj",
                "build/output_test/020b6c_ground_probe_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/0206f0_intersect/8c0206f0_IntersectSegments.php",
            ],
            "objects" => [
                "build/output_test/0206f0_intersect_src.obj",
                "build/output_test/0206f0_intersect_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02786c_vehicle_parts/8c02786c_VehPartsBind.php",
            ],
            "objects" => [
                "build/output_test/02786c_vehicle_parts_src.obj",
                "build/output_test/02786c_vehicle_parts_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02e400_collision/8c02e400_CollideFindTaskHit.php",
                "tests/02e400_collision/8c02e486_CollideQueueReset.php",
                "tests/02e400_collision/8c02e48e_CollideQueueAdd.php",
                "tests/02e400_collision/8c02e4ac_CollideQueueTest.php",
            ],
            "objects" => [
                "build/output_test/02e400_collision_src.obj",
                "build/output_test/02e400_collision_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02f0c8/8c02f0c8_FUN.php",
                "tests/02f0c8/8c02f212_FUN.php",
                "tests/02f0c8/8c02f28a_FUN.php",
            ],
            "objects" => [
                "build/output_test/02f0c8_src.obj",
                "build/output_test/02f0c8_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02b464_drive_points/8c02b464_adjust.php",
                "tests/02b464_drive_points/8c02b578_armCooldowns.php",
                "tests/02b464_drive_points/8c02b6d4_handleBump.php",
                "tests/02b464_drive_points/8c02b7ea_handleFlags.php",
                "tests/02b464_drive_points/8c02b864_FUN.php",
                "tests/02b464_drive_points/8c02b886_FUN.php",
                "tests/02b464_drive_points/8c02b8b8_FUN.php",
                "tests/02b464_drive_points/8c02b986_FUN.php",
                "tests/02b464_drive_points/8c02bb1c_FUN.php",
                "tests/02b464_drive_points/8c02bcd8_FUN.php",
                "tests/02b464_drive_points/8c02c072_taskCallback.php",
                "tests/02b464_drive_points/8c02c46a_DrivePointsReset.php",
                "tests/02b464_drive_points/8c02c624_FUN.php",
                "tests/02b464_drive_points/8c02c76a_FUN.php",
                "tests/02b464_drive_points/8c02c586_FUN.php",
                "tests/02b464_drive_points/8c02c69a_FUN.php",
                "tests/02b464_drive_points/8c02c738_FUN.php",
                "tests/02b464_drive_points/8c02c784_FUN.php",
            ],
            "objects" => [
                "build/output_test/02b464_drive_points_src.obj",
                "build/output_test/02b464_drive_points_c.obj",
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
                "build/output_test/02f320_replay_codec_src.obj",
                "build/output_test/02f320_replay_codec_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/023310_bus_init/8c023310_busInitPlaceBus.php",
                "tests/023310_bus_init/8c023610_BusInitStart.php",
            ],
            "objects" => [
                "build/output_test/023310_bus_init_src.obj",
                "build/output_test/023310_bus_init_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/022bdc_bus/8c022bdc_BusTask.php",
            ],
            "objects" => [
                "build/output_test/022bdc_bus_src.obj",
                "build/output_test/022bdc_bus_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/024b4c_bus_render/8c024b4c_FUN.php",
                "tests/024b4c_bus_render/8c024b86_FUN.php",
                "tests/024b4c_bus_render/8c024f32_FUN.php",
                "tests/024b4c_bus_render/8c024bb8_FUN.php",
                "tests/024b4c_bus_render/8c024d6c_positionCamera.php",
                "tests/024b4c_bus_render/8c025078_BusRenderUpdateCamera.php",
                "tests/024b4c_bus_render/8c025604_BusRenderUpdateMirrorCamera.php",
            ],
            "objects" => [
                "build/output_test/024b4c_bus_render_src.obj",
                "build/output_test/024b4c_bus_render_c.obj",
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
                "build/output_test/023938_bus_drive_src.obj",
                "build/output_test/023938_bus_drive_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/025870/8c025870_FUN.php",
                "tests/025870/8c0258ba_FUN.php",
                "tests/025870/8c025906_demoUpdateCamera.php",
                "tests/025870/8c0259e8_stopTextboxTask.php",
                "tests/025870/8c025af4_FUN.php",
            ],
            "objects" => [
                "build/output_test/025870_src.obj",
                "build/output_test/025870_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02e51c/8c02e51c_FUN.php",
                "tests/02e51c/8c02eab4_FUN.php",
                "tests/02e51c/8c02e69c_FUN.php",
                "tests/02e51c/8c02ec50_FUN.php",
                "tests/02e51c/8c02f08a_FUN.php",
            ],
            "objects" => [
                "build/output_test/02e51c_src.obj",
                "build/output_test/02e51c_c.obj",
            ]
        ],
    ],
];
