include_guard(GLOBAL)

function(lufia2_add_nmi_continuations target core_root)
    if(NOT LUFIA2_ENABLE_DECOMP OR LUFIA2_DECOMP_REFERENCE_ONLY)
        return()
    endif()
    get_target_property(_sources ${target} SOURCES)
    string(REPLACE "\\" "/" _sources "${_sources}")
    if(NOT _sources MATCHES "(^|;)([^;]*/)?src/lufia2_runtime[.]c(;|$)")
        return()
    endif()
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/system.h" _api)
    if(NOT _api MATCHES "Lufia2MainNmiResume[(]")
        return()
    endif()
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml" _metadata)
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2MainNmi\"[^[]*"
        _contract "${_metadata}")
    if(NOT _contract MATCHES "address[ \t]*=[ \t]*\"80:8638\"" OR
       NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        return()
    endif()

    file(READ "${core_root}/runner/src/snes/interp_bridge.c" _pinned)
    string(REPLACE "\r\n" "\n" _pinned "${_pinned}")
    string(SHA256 _hash "${_pinned}")
    if(NOT _hash STREQUAL "f96baa3bef2a19c2c1c1c1b1e5fa4eb10d281bb332b2dac1aabcff10561952af")
        message(FATAL_ERROR "Native NMI timing requires the reviewed interpreter clock contract")
    endif()
    set(_definitions LUFIA2_HAVE_NATIVE_NMI_CONTINUATIONS=1)
    if(_api MATCHES "Lufia2NmiDisplayAndClock[(]" AND
       _api MATCHES "Lufia2NmiRestoreInterruptFrame[(]")
        list(APPEND _definitions LUFIA2_HAVE_NATIVE_NMI_FINISH=1)
    endif()
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2NmiSaveInterruptFrame\"[^[]*"
        _entry_contract "${_metadata}")
    if(_api MATCHES "Lufia2NmiSaveInterruptFrame[(]" AND
       _entry_contract MATCHES "address[ \t]*=[ \t]*\"80:862D\"" AND
       _entry_contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        list(APPEND _definitions LUFIA2_HAVE_NATIVE_NMI_ENTRY=1)
    endif()
    set(_frame_returns FALSE)
    set(_nmi_restore_width FALSE)
    set(_nmi_call_boundaries FALSE)
    set(_nmi_callback_boundaries FALSE)
    set(_frame_wait_boundaries FALSE)
    set(_frame_register_boundaries FALSE)
    set(_menu_input_boundaries FALSE)
    set(_sound_frame_boundaries FALSE)
    set(_field_actor_boundaries FALSE)
    set(_field_frame_boundaries FALSE)
    set(_request_boundaries FALSE)
    set(_select_transition_boundaries FALSE)
    set(_menu_buffer_spans FALSE)
    set(_save_buffer_spans FALSE)
    set(_menu_wait_polls FALSE)
    set(_scene_rebuild_spans FALSE)
    set(_cursor_pixel_spans FALSE)
    set(_session_actor_spans FALSE)
    set(_music_cache_spans FALSE)
    set(_sound_source_spans FALSE)
    set(_sound_progress_spans FALSE)
    set(_decompress_flag_steps FALSE)
    set(_decompress_reference_steps FALSE)
    set(_battle_input_steps FALSE)
    set(_battle_swirl_parts FALSE)
    set(_ripple_steps FALSE)
    if(_api MATCHES "Lufia2FrameWaitRestoreRegisters[(]" AND
       EXISTS "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/menu.h")
        file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/menu.h" _menu_api)
        if(_menu_api MATCHES "Lufia2SpriteFrameRestoreRegisters[(]")
            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2FrameReturns.cmake")
            lufia2_frame_returns_available(_frame_returns ${target})
            if(_frame_returns)
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleRippleSteps.cmake")
                lufia2_battle_ripple_steps_available(_ripple_steps)
                if(_ripple_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_RIPPLE_STEPS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleSwirlParts.cmake")
                lufia2_battle_swirl_parts_available(_battle_swirl_parts)
                if(_battle_swirl_parts)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_SWIRL_PARTS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleInputSteps.cmake")
                lufia2_battle_input_steps_available(_battle_input_steps)
                if(_battle_input_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_INPUT_STEPS=1>")
                endif()
                list(APPEND _definitions LUFIA2_HAVE_NATIVE_FRAME_RETURNS=1)
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2MenuReplayEntries.cmake")
                lufia2_menu_replay_entries_available(_menu_replay_entries ${target})
                if(_menu_replay_entries)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_MENU_REPLAY=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2MinigameResultEntries.cmake")
                lufia2_minigame_result_entries_available(_minigame_result_entries ${target})
                if(_minigame_result_entries)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_MINIGAME_RESULTS=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BoardGroupEntry.cmake")
                lufia2_board_group_entries_available(_board_group_entry ${target})
                if(_board_group_entry)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_BOARD_GROUP=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2PokerRunEntry.cmake")
                lufia2_poker_run_entries_available(_poker_run_entry ${target})
                if(_poker_run_entry)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_POKER_RUN=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2StakeDialogEntries.cmake")
                lufia2_stake_dialog_entries_available(_stake_dialog_entries ${target})
                if(_stake_dialog_entries)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_STAKE_DIALOGS=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2GameSessionEntries.cmake")
                lufia2_game_session_entries_available(_game_session_entries ${target})
                if(_game_session_entries)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_GAME_SESSIONS=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2RoundSelectionEntries.cmake")
                lufia2_round_selection_entries_available(_round_selection_entries ${target})
                if(_round_selection_entries)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_ROUND_SELECTION=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2PokerStakeEntries.cmake")
                lufia2_poker_stake_entries_available(_poker_stake_entries ${target})
                if(_poker_stake_entries)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_POKER_STAKE=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2PokerAnimationEntries.cmake")
                lufia2_poker_animation_entries_available(_poker_animation_entries ${target})
                if(_poker_animation_entries)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_POKER_ANIMATION=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2CardOpeningEntries.cmake")
                lufia2_card_opening_entries_available(_card_opening_entries ${target})
                if(_card_opening_entries)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_CARD_OPENING=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2PaletteEntries.cmake")
                lufia2_palette_entries_available(_palette_entries ${target})
                if(_palette_entries)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_PALETTE_ENTRIES=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2WorldPalette.cmake")
                lufia2_world_palette_available(_world_palette ${target})
                if(_world_palette)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_WORLD_PALETTE=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BootController.cmake")
                lufia2_boot_controller_available(_boot_controller ${target})
                if(_boot_controller)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_BOOT_CONTROLLER=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2SpecialMenuRun.cmake")
                lufia2_special_menu_run_available(_special_menu_run ${target})
                if(_palette_entries AND _special_menu_run)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_SPECIAL_RUN=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2EventRegionEntries.cmake")
                lufia2_event_region_entries_available(_event_region_entries ${target})
                if(_event_region_entries)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_EVENT_REGION_ENTRIES=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2TextWindowEntry.cmake")
                lufia2_text_window_entry_available(_text_window_entry ${target})
                if(_text_window_entry)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_TEXT_WINDOW_ENTRY=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2InterruptEntries.cmake")
                lufia2_interrupt_entries_available(_interrupt_entries ${target})
                if(_interrupt_entries)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_INTERRUPT_ENTRIES=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2ResetEntry.cmake")
                lufia2_reset_entry_available(_reset_entry ${target})
                if(_reset_entry)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_RESET_ENTRY=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2DecompressSteps.cmake")
                lufia2_decompress_steps_available(_decompress_steps)
                if(_decompress_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_DECOMPRESS_STEPS=1>")
                    lufia2_decompress_flag_steps_available(_decompress_flag_steps)
                    lufia2_decompress_reference_steps_available(_decompress_reference_steps)
                    if(_decompress_reference_steps)
                        set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                            APPEND PROPERTY COMPILE_DEFINITIONS
                                "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_DECOMPRESS_REFERENCE_STEPS=1>")
                    endif()
                    if(_decompress_flag_steps)
                        set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                            APPEND PROPERTY COMPILE_DEFINITIONS
                                "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_DECOMPRESS_FLAG_STEPS=1>")
                    endif()
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleBackgroundSteps.cmake")
                lufia2_battle_background_steps_available(_battle_background_steps)
                if(_battle_background_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_BACKGROUND_STEPS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleTransitionSteps.cmake")
                lufia2_battle_transition_steps_available(_battle_transition_steps)
                if(_battle_transition_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_TRANSITION_STEPS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleTimerSteps.cmake")
                lufia2_battle_timer_steps_available(_battle_timer_steps)
                if(_battle_timer_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_TIMER_STEPS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleWaveFillSteps.cmake")
                lufia2_battle_wave_fill_steps_available(_battle_wave_fill_steps)
                if(_battle_wave_fill_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_WAVE_FILL_STEPS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleEffectSlotSteps.cmake")
                lufia2_battle_effect_slot_steps_available(_battle_effect_slot_steps)
                if(_battle_effect_slot_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_SLOT_STEPS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleEffectDispatchSteps.cmake")
                lufia2_battle_effect_dispatch_steps_available(_battle_effect_dispatch_steps)
                if(_battle_effect_dispatch_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_DISPATCH_STEPS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleEffectCleanupSteps.cmake")
                lufia2_battle_effect_cleanup_steps_available(_battle_effect_cleanup_steps)
                if(_battle_effect_cleanup_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_CLEANUP_STEPS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleQueueSteps.cmake")
                lufia2_battle_queue_steps_available(_battle_queue_steps)
                if(_battle_queue_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_QUEUE_STEPS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2MenuFadeContinuations.cmake")
                lufia2_menu_fade_continuations_available(_menu_fade_continuations)
                if(_menu_fade_continuations)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_MENU_FADE_CONTINUATIONS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2EquipmentGroupSteps.cmake")
                lufia2_equipment_group_steps_available(_equipment_group_steps)
                if(_equipment_group_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS LUFIA2_HAVE_NATIVE_EQUIPMENT_GROUP_STEPS=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2ItemRecordSteps.cmake")
                lufia2_item_record_steps_available(_item_record_steps)
                if(_item_record_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS LUFIA2_HAVE_NATIVE_ITEM_RECORD_STEPS=1)
                endif()
                set(_calls_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/nmi_calls.h")
                if(EXISTS "${_calls_header}")
                    file(READ "${_calls_header}" _calls_api)
                    if(_calls_api MATCHES "Lufia2NmiBeginChild[(]" AND
                       _calls_api MATCHES "Lufia2NmiReturnInterrupt[(]")
                        set(_nmi_call_boundaries TRUE)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2FrameWaitBoundaries.cmake")
                        lufia2_frame_wait_boundaries_available(_frame_wait_boundaries)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2FrameRegisterBoundaries.cmake")
                        lufia2_frame_register_boundaries_available(_frame_register_boundaries)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2MenuInputBoundaries.cmake")
                        lufia2_menu_input_boundaries_available(_menu_input_boundaries)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2SoundFrameBoundaries.cmake")
                        lufia2_sound_frame_boundaries_available(_sound_frame_boundaries)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2SelectTransitionBoundaries.cmake")
                        lufia2_select_transition_boundaries_available(_select_transition_boundaries)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2RequestBoundaries.cmake")
                        lufia2_request_boundaries_available(_request_boundaries)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2FieldFrameBoundaries.cmake")
                        lufia2_field_frame_boundaries_available(_field_frame_boundaries)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2FieldActorBoundaries.cmake")
                        lufia2_field_actor_boundaries_available(_field_actor_boundaries)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2MenuBufferSpans.cmake")
                        lufia2_menu_buffer_spans_available(_menu_buffer_spans)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2SaveBufferSpans.cmake")
                        lufia2_save_buffer_spans_available(_save_buffer_spans)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2MenuWaitPolls.cmake")
                        lufia2_menu_wait_polls_available(_menu_wait_polls)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2SceneRebuildSpans.cmake")
                        lufia2_scene_rebuild_spans_available(_scene_rebuild_spans)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2CursorPixelSpans.cmake")
                        lufia2_cursor_pixel_spans_available(_cursor_pixel_spans)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2SessionActorSpans.cmake")
                        lufia2_session_actor_spans_available(_session_actor_spans)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2MusicCacheSpans.cmake")
                        lufia2_music_cache_spans_available(_music_cache_spans)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2SoundSourceSpans.cmake")
                        lufia2_sound_source_spans_available(_sound_source_spans)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2SoundProgressSpans.cmake")
                        lufia2_sound_progress_spans_available(_sound_progress_spans)
                        if(_calls_api MATCHES "Lufia2NmiInvokeCallback[(]" AND
                           _calls_api MATCHES "Lufia2NmiReturnCallback[(]")
                            set(_nmi_callback_boundaries TRUE)
                        endif()
                    endif()
                endif()
                set(_finish_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/nmi_finish.h")
                if(_api MATCHES "Lufia2NmiDisplayAndClock[(]" AND
                   _api MATCHES "Lufia2NmiRestoreInterruptFrame[(]" AND
                   EXISTS "${_finish_header}")
                    file(READ "${_finish_header}" _finish_api)
                    if(_finish_api MATCHES "Lufia2NmiPrepareFrameRestore[(]")
                        set(_nmi_restore_width TRUE)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_NMI_RESTORE_WIDTH=1)
                    endif()
                endif()
                set(_selection_resumptions TRUE)
                if(NOT _menu_api MATCHES "Lufia2SelectScreenStepResumption[(]")
                    set(_selection_resumptions FALSE)
                endif()
                foreach(_resume_address IN ITEMS "86:8333" "86:8352" "86:82CE" "86:82FC")
                    string(REGEX MATCH
                        "\\[\\[function\\]\\][^[]*address[ \t]*=[ \t]*\"${_resume_address}\"[^[]*"
                        _resume_contract "${_metadata}")
                    if(NOT _resume_contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
                        set(_selection_resumptions FALSE)
                    endif()
                endforeach()
                if(_selection_resumptions)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_SELECT_RESUMPTIONS=1)
                endif()
                set(_sound_polls TRUE)
                if(NOT _api MATCHES "Lufia2SoundPollStep[(]")
                    set(_sound_polls FALSE)
                endif()
                foreach(_sound_address IN ITEMS "80:9383" "80:9C0F" "80:9945"
                                                "80:98A5" "80:97E5" "80:9A0A")
                    string(REGEX MATCH
                        "\\[\\[function\\]\\][^[]*address[ \t]*=[ \t]*\"${_sound_address}\"[^[]*"
                        _sound_contract "${_metadata}")
                    if(NOT _sound_contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
                        set(_sound_polls FALSE)
                    endif()
                endforeach()
                if(_sound_polls)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_SOUND_POLLS=1)
                    if(_api MATCHES "Lufia2SoundUploadStep[(]")
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_SOUND_UPLOAD_STEPS=1)
                        set(_frame_polls FALSE)
                        if(_api MATCHES "Lufia2FrameWaitPollStep[(]")
                            set(_frame_polls TRUE)
                            foreach(_frame_address IN ITEMS "82:93C2" "83:900C" "8E:B550"
                                                            "86:8B48" "86:974F")
                                string(REGEX MATCH
                                    "\\[\\[function\\]\\][^[]*address[ \t]*=[ \t]*\"${_frame_address}\"[^[]*"
                                    _frame_contract "${_metadata}")
                                if(NOT _frame_contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
                                    set(_frame_polls FALSE)
                                endif()
                            endforeach()
                        endif()
                        if(_frame_polls)
                            list(APPEND _definitions LUFIA2_HAVE_NATIVE_FRAME_POLL_RUNS=1)
                            string(REGEX MATCH
                                "\\[\\[function\\]\\][^[]*address[ \t]*=[ \t]*\"84:8D4D\"[^[]*"
                                _cave_wait_contract "${_metadata}")
                            if(_api MATCHES "Lufia2FrameWaitCounterStep[(]" AND
                               _cave_wait_contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
                                list(APPEND _definitions LUFIA2_HAVE_NATIVE_FRAME_COUNTER_STEPS=1)
                                string(REGEX MATCH
                                    "\\[\\[function\\]\\][^[]*address[ \t]*=[ \t]*\"80:BF0B\"[^[]*"
                                    _text_wait_contract "${_metadata}")
                                if(_api MATCHES "Lufia2TextWaitForNextFrame[(]" AND
                                   _text_wait_contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
                                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_TEXT_FRAME_WAIT=1)
                                endif()
                                set(_additional_waits FALSE)
                                if(_api MATCHES "Lufia2FrameWaitAdditionalStep[(]")
                                    set(_additional_waits TRUE)
                                    foreach(_wait_address IN ITEMS "80:9378" "84:A5A8" "8E:B288"
                                                                  "84:8363" "85:851C" "85:EC81" "8E:C1DE")
                                        string(REGEX MATCH
                                            "\\[\\[function\\]\\][^[]*address[ \t]*=[ \t]*\"${_wait_address}\"[^[]*"
                                            _wait_contract "${_metadata}")
                                        if(NOT _wait_contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
                                            set(_additional_waits FALSE)
                                        endif()
                                    endforeach()
                                endif()
                                if(_additional_waits)
                                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_ADDITIONAL_FRAME_WAITS=1)
                                endif()
                            endif()
                        endif()
                        set(_cursor_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/menu_input_phase.h")
                        if(_frame_polls AND EXISTS "${_cursor_header}")
                            file(READ "${_cursor_header}" _cursor_api)
                            string(REGEX MATCH
                                "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2MenuInputLoop\"[^[]*"
                                _cursor_contract "${_metadata}")
                            if(_cursor_api MATCHES "Lufia2MenuPrepareCursorPoll[(]" AND
                               _cursor_contract MATCHES "address[ \t]*=[ \t]*\"82:8B08\"" AND
                               _cursor_contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
                                list(APPEND _definitions LUFIA2_HAVE_NATIVE_MENU_CURSOR_POLL=1)
                            endif()
                        endif()
                        set(_save_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/frame_wait.h")
                        if(_frame_polls AND EXISTS "${_save_header}")
                            file(READ "${_save_header}" _save_api)
                            set(_save_steps TRUE)
                            if(NOT _save_api MATCHES "Lufia2FrameWaitSaveRegistersStep[(]")
                                set(_save_steps FALSE)
                            endif()
                            foreach(_save_owner IN ITEMS
                                    "Lufia2WaitMenuFrameChange|82:93C2"
                                    "Lufia2MenuWaitForNextFrame|86:8B48"
                                    "Lufia2WorldWaitForNextFrame|86:974F"
                                    "Lufia2WaitFramePreserveStatus|80:9378"
                                    "Lufia2WaitFramePreserveAccumulator|84:A5A8")
                                string(REPLACE "|" ";" _save_fields "${_save_owner}")
                                list(GET _save_fields 0 _save_name)
                                list(GET _save_fields 1 _save_address)
                                string(REGEX MATCH
                                    "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"${_save_name}\"[^[]*"
                                    _save_contract "${_metadata}")
                                if(NOT _save_contract MATCHES "address[ \t]*=[ \t]*\"${_save_address}\"" OR
                                   NOT _save_contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
                                    set(_save_steps FALSE)
                                endif()
                            endforeach()
                            if(_save_steps)
                                list(APPEND _definitions LUFIA2_HAVE_NATIVE_FRAME_SAVE_STEPS=1)
                            endif()
                        endif()
                        set(_rectangle_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/menu_rectangle.h")
                        if(_frame_polls AND EXISTS "${_rectangle_header}")
                            file(READ "${_rectangle_header}" _rectangle_api)
                            string(REGEX MATCH
                                "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2MenuClearLayerRectangle\"[^[]*"
                                _rectangle_contract "${_metadata}")
                            if(_rectangle_api MATCHES "Lufia2MenuClearLayerRectangleStep[(]" AND
                               _rectangle_contract MATCHES "address[ \t]*=[ \t]*\"82:83B5\"" AND
                               _rectangle_contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
                                list(APPEND _definitions LUFIA2_HAVE_NATIVE_MENU_RECTANGLE_STEPS=1)
                            endif()
                        endif()
                        if(_api MATCHES "Lufia2SoundPollIteration[(]" AND
                           _api MATCHES "Lufia2SoundPollBranchTarget[(]")
                            list(APPEND _definitions LUFIA2_HAVE_NATIVE_SOUND_POLL_RUNS=1)
                        endif()
                        if(_api MATCHES "Lufia2SoundPollBranchStep[(]" AND
                           _api MATCHES "Lufia2SoundPollBranchTarget[(]" AND
                           _api MATCHES "Lufia2BootSoundTransferStep[(]")
                            list(APPEND _definitions LUFIA2_HAVE_NATIVE_SOUND_HANDSHAKE_STEPS=1)
                            if(_api MATCHES "Lufia2SoundUploadOpcodeStep[(]" AND
                               _api MATCHES "Lufia2BootSoundOpcodeStep[(]")
                                list(APPEND _definitions LUFIA2_HAVE_NATIVE_SOUND_TRANSFER_OPCODES=1)
                            endif()
                        endif()
                    endif()
                endif()
            endif()
        endif()
    endif()
    set(_result)
    set(_count 0)
    foreach(_source IN LISTS _sources)
        get_filename_component(_name "${_source}" NAME)
        if(_name STREQUAL "interp_bridge.c")
            file(READ "${_source}" _text)
            if(_text MATCHES "Lufia2DecompNmiAccountCycles|decomp_bridge/nmi_cycles[.]inc")
                message(FATAL_ERROR "Native NMI timing was already installed")
            endif()
            set(_output "${CMAKE_BINARY_DIR}/generated/decomp-nmi/interp_bridge.c")
            file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated/decomp-nmi")
            set(_timing_suffix "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/nmi_cycles.inc\"\n")
            if(_boot_controller)
                set(_boot_entry_marker "        /* IRQs are sampled between instructions.")
                string(FIND "${_text}" "${_boot_entry_marker}" _boot_entry_offset)
                if(_boot_entry_offset EQUAL -1)
                    message(FATAL_ERROR "Boot entry requires the reviewed scheduler boundary")
                endif()
                string(REPLACE "${_boot_entry_marker}"
                    "        if (auto_quiescent && !stop_on_rti && pc_before == 0x808035u) {\n            const int boot = Lufia2DecompRunBootEntry(&in, cpu, pc_before);\n            if (boot == 1) continue;\n            if (boot) return boot > 0;\n        }\n${_boot_entry_marker}"
                    _text "${_text}")
                string(PREPEND _text
                    "#include \"interp816.h\"\n#include \"cpu_state.h\"\nstatic int Lufia2DecompRunBootEntry(Interp816 *, CpuState *, uint32_t);\n")
                string(APPEND _timing_suffix
                    "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/boot_entry_boundary.inc\"\n")
            endif()
            if(_nmi_call_boundaries)
                set(_original_opcode "        int _cyc = interp816_runOpcode(&in);   /* executes the opcode; pushes/pops frames */")
                string(FIND "${_text}" "${_original_opcode}" _opcode_offset)
                if(_opcode_offset EQUAL -1)
                    message(FATAL_ERROR "Native NMI calls require reviewed opcode execution")
                endif()
                set(_native_opcode "        int _cyc = Lufia2DecompNmiRunCallBoundary(\n            &in, pc_before, op, stop_on_rti, auto_quiescent);\n")
                if(_frame_wait_boundaries)
                    string(APPEND _native_opcode "        if (!_cyc)\n            _cyc = Lufia2DecompRunFrameWaitBoundary(&in, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunFrameWaitBoundary(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/frame_wait_boundaries.inc\"\n")
                endif()
                if(_frame_register_boundaries)
                    string(APPEND _native_opcode "        if (!_cyc)\n            _cyc = Lufia2DecompRunFrameRegisterBoundary(&in, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunFrameRegisterBoundary(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/frame_register_boundaries.inc\"\n")
                endif()
                if(_menu_input_boundaries)
                    string(APPEND _native_opcode "        if (!_cyc)\n            _cyc = Lufia2DecompRunMenuInputBoundary(&in, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunMenuInputBoundary(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/menu_input_boundaries.inc\"\n")
                endif()
                if(_sound_progress_spans)
                    string(APPEND _native_opcode "        if (!_cyc && !n_stop && (!yield_pc || auto_quiescent) &&\n            (!g_snes || (!g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n             (!g_snes->nmiEnabled || s_lle_master_deadline))))\n            _cyc = Lufia2DecompRunSoundProgressSpan(&in, cpu, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\n#include \"cpu_state.h\"\nstatic int Lufia2DecompRunSoundProgressSpan(Interp816 *, CpuState *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/sound_progress_spans.inc\"\n")
                endif()
                if(_sound_frame_boundaries)
                    string(APPEND _native_opcode "        if (!_cyc)\n            _cyc = Lufia2DecompRunSoundFrameBoundary(&in, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunSoundFrameBoundary(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/sound_frame_boundaries.inc\"\n")
                endif()
                if(_select_transition_boundaries)
                    string(APPEND _native_opcode "        if (!_cyc)\n            _cyc = Lufia2DecompRunSelectTransitionBoundary(&in, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunSelectTransitionBoundary(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/select_transition_boundaries.inc\"\n")
                endif()
                if(_request_boundaries)
                    string(APPEND _native_opcode "        if (!_cyc)\n            _cyc = Lufia2DecompRunRequestBoundary(&in, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunRequestBoundary(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/request_boundaries.inc\"\n")
                endif()
                if(_field_frame_boundaries)
                    string(APPEND _native_opcode "        if (!_cyc)\n            _cyc = Lufia2DecompRunFieldFrameBoundary(&in, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunFieldFrameBoundary(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/field_frame_boundaries.inc\"\n")
                endif()
                if(_sound_source_spans)
                    string(APPEND _native_opcode "        if (!_cyc && !n_stop && (!yield_pc || auto_quiescent) &&\n            (!g_snes || (!g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n             (!g_snes->nmiEnabled || s_lle_master_deadline))))\n            _cyc = Lufia2DecompRunSoundSourceSpan(&in, cpu, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\n#include \"cpu_state.h\"\nstatic int Lufia2DecompRunSoundSourceSpan(Interp816 *, CpuState *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/sound_source_spans.inc\"\n")
                endif()
                if(_music_cache_spans)
                    string(APPEND _native_opcode "        if (!_cyc && !n_stop && (!yield_pc || auto_quiescent) &&\n            (!g_snes || (!g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n             (!g_snes->nmiEnabled || s_lle_master_deadline))))\n            _cyc = Lufia2DecompRunMusicCacheSpan(&in, cpu, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\n#include \"cpu_state.h\"\nstatic int Lufia2DecompRunMusicCacheSpan(Interp816 *, CpuState *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/music_cache_spans.inc\"\n")
                endif()
                if(_session_actor_spans)
                    string(APPEND _native_opcode "        if (!_cyc && !n_stop && (!yield_pc || auto_quiescent))\n            _cyc = Lufia2DecompRunSessionActorSpan(&in, cpu, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\n#include \"cpu_state.h\"\nstatic int Lufia2DecompRunSessionActorSpan(Interp816 *, CpuState *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/session_actor_spans.inc\"\n")
                endif()
                if(_field_actor_boundaries)
                    string(APPEND _native_opcode "        if (!_cyc)\n            _cyc = Lufia2DecompRunFieldActorBoundary(&in, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunFieldActorBoundary(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/field_actor_boundaries.inc\"\n")
                endif()
                if(_menu_buffer_spans)
                    string(APPEND _native_opcode "        if (!_cyc && !n_stop && (!yield_pc || auto_quiescent))\n            _cyc = Lufia2DecompRunMenuBufferSpan(&in, cpu, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\n#include \"cpu_state.h\"\nstatic int Lufia2DecompRunMenuBufferSpan(Interp816 *, CpuState *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/menu_buffer_spans.inc\"\n")
                endif()
                if(_save_buffer_spans)
                    string(APPEND _native_opcode "        if (!_cyc && !n_stop && (!yield_pc || auto_quiescent))\n            _cyc = Lufia2DecompRunSaveBufferSpan(&in, cpu, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\n#include \"cpu_state.h\"\nstatic int Lufia2DecompRunSaveBufferSpan(Interp816 *, CpuState *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/save_buffer_spans.inc\"\n")
                endif()
                if(_menu_wait_polls)
                    string(APPEND _native_opcode "        if (!_cyc && !n_stop && (!yield_pc || auto_quiescent))\n            _cyc = Lufia2DecompRunMenuWaitPoll(&in, cpu, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\n#include \"cpu_state.h\"\nstatic int Lufia2DecompRunMenuWaitPoll(Interp816 *, CpuState *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/menu_wait_polls.inc\"\n")
                endif()
                if(_cursor_pixel_spans)
                    string(PREPEND _native_opcode "        int _lufia2_cursor_accounted = 0;\n")
                    string(APPEND _native_opcode "        if (!_cyc && !n_stop && (!yield_pc || auto_quiescent) &&\n            (!g_snes || (!g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n             (!g_snes->nmiEnabled || s_lle_master_deadline)))) {\n            _cyc = Lufia2DecompRunCursorPixelSpan(&in, cpu, pc_before, op, Lufia2CursorPixelAccountInstruction);\n            _lufia2_cursor_accounted = _cyc != 0;\n        }\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\n#include \"cpu_state.h\"\nstatic void Lufia2CursorPixelAccountInstruction(CpuState *, unsigned);\nstatic int Lufia2DecompRunCursorPixelSpan(Interp816 *, CpuState *, uint32_t, uint8_t, void (*)(CpuState *, unsigned));\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/cursor_pixel_spans.inc\"\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/cursor_pixel_clock.inc\"\n")
                    set(_clock_block "        {\n            unsigned _internal = (unsigned)_cyc > s_interp_bus_cycles")
                    string(FIND "${_text}" "${_clock_block}" _clock_offset)
                    if(_clock_offset EQUAL -1)
                        message(FATAL_ERROR "Cursor pixels require the reviewed instruction clock")
                    endif()
                    string(REPLACE "${_clock_block}"
                        "        if (!_lufia2_cursor_accounted) {\n            unsigned _internal = (unsigned)_cyc > s_interp_bus_cycles"
                        _text "${_text}")
                endif()
                if(_scene_rebuild_spans)
                    string(APPEND _native_opcode "        if (!_cyc && !n_stop && (!yield_pc || auto_quiescent))\n            _cyc = Lufia2DecompRunSceneRebuildSpan(&in, cpu, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\n#include \"cpu_state.h\"\nstatic int Lufia2DecompRunSceneRebuildSpan(Interp816 *, CpuState *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/scene_rebuild_spans.inc\"\n")
                endif()
                string(APPEND _native_opcode "        if (!_cyc)\n            _cyc = interp816_runOpcode(&in);")
                string(REPLACE "${_original_opcode}" "${_native_opcode}"
                    _text "${_text}")
                string(PREPEND _text
                    "#include \"interp816.h\"\nstatic int Lufia2DecompNmiRunCallBoundary(Interp816 *, uint32_t, uint8_t, int, int);\n")
                if(_nmi_callback_boundaries)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_NMI_CALLBACK_BOUNDARIES 1\n")
                endif()
                string(APPEND _timing_suffix
                    "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/nmi_calls.inc\"\n")
            endif()
            if(_frame_returns)
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2OpcodeHookPages.cmake")
                lufia2_filter_opcode_hook_pages(_text "${_text}")
                set(_hook_call "                    s_pre_opcode_hooks[hi].hook(cpu, pc_before);")
                set(_redirect "            if (redirected) continue;")
                foreach(_required IN ITEMS "${_hook_call}" "${_redirect}")
                    string(FIND "${_text}" "${_required}" _offset)
                    if(_offset EQUAL -1)
                        message(FATAL_ERROR "Native frame returns require reviewed progress checkpoints")
                    endif()
                endforeach()
                string(REPLACE "${_hook_call}"
                    "                    s_frame_return_auto_quiescent = auto_quiescent;\n                    {\n                        const int blocked = s_frame_return_interrupt_blocked;\n                        const unsigned old_poll_budget = s_frame_return_poll_opcode_budget;\n                        s_frame_return_interrupt_blocked = in.waiting || in.stopped ||\n                            in.nmiWanted || in.irqWanted;\n                    s_frame_return_batch_allowed = !s_frame_return_interrupt_blocked && auto_quiescent && s_interp_bridge_depth == 1 &&\n                        steps >= 0 && step_cap > steps && step_cap - steps >= 16 &&\n                        !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n                        !g_interp_bridge_pc_hook && !s_interp_bus_timing_active && s_interp_continuous_read_epoch <= UINT64_MAX - 16u;\n                        s_frame_return_poll_opcode_budget = s_frame_return_batch_allowed ? (unsigned)((step_cap - steps) > 128 ? 128 : (step_cap - steps)) : 0u;\n${_hook_call}\n                        s_frame_return_poll_opcode_budget = old_poll_budget;\n                        s_frame_return_interrupt_blocked = blocked;\n                    }"
                    _text "${_text}")
                string(REPLACE "${_redirect}"
                    "            if (redirected) {\n                Lufia2DecompFrameReturnContinue(&steps, auto_quiescent,\n                    &progress_write_epoch, &progress_dynamic_epoch);\n                continue;\n            }"
                    _text "${_text}")
                string(PREPEND _text
                    "#include <stdint.h>\nstatic int s_frame_return_auto_quiescent;\nstatic int s_frame_return_batch_allowed;\nstatic unsigned s_frame_return_poll_opcode_budget;\nstatic int s_frame_return_interrupt_blocked = 1;\nstatic void Lufia2DecompFrameReturnContinue(long *, int, uint64_t *, uint64_t *);\n")
                string(APPEND _timing_suffix
                    "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/frame_return_cycles.inc\"\n")
                get_property(_beam_hold_query GLOBAL PROPERTY SNESRECOMP_PLATFORM_BEAM_HOLD_QUERY)
                if(_beam_hold_query)
                    string(PREPEND _text "#define SNESRECOMP_PLATFORM_BEAM_HOLD_QUERY 1\n")
                endif()
                if(_nmi_restore_width)
                    string(REPLACE "${_hook_call}"
                        "                    {\n                        const int interrupt_owner = s_native_hook_interrupt;\n                        s_native_hook_interrupt = Lufia2DecompNmiHookHasOwner(\n                            pc_before, stop_on_rti, auto_quiescent, step_cap - steps);\n${_hook_call}\n                        s_native_hook_interrupt = interrupt_owner;\n                    }"
                        _text "${_text}")
                    string(PREPEND _text "#include <stdint.h>\nstatic int s_native_hook_interrupt;\nstatic int Lufia2DecompNmiHookHasOwner(uint32_t, int, int, long);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/nmi_owner.inc\"\n")
                endif()
                if(_ripple_steps)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_RIPPLE_STEPS 1\n")
                    string(REPLACE "${_hook_call}"
                        "                    if (((pc_before & 0x7fffffu) != 0x05a736u ||\n                         step_cap - steps >= 15) &&\n                        ((pc_before & 0x7fffffu) != 0x05a74fu ||\n                         step_cap - steps >= 8) &&\n                        ((pc_before & 0x7fffffu) != 0x05a760u ||\n                         step_cap - steps >= 6)) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
                if(_battle_swirl_parts)
                    string(REPLACE "${_hook_call}"
                        "                    if (((pc_before & 0x7fffffu) != 0x048ce7u ||\n                         step_cap - steps >= 4) &&\n                        ((pc_before & 0x7fffffu) != 0x048cf3u ||\n                         step_cap - steps >= 12)) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
                if(_battle_input_steps)
                    string(REPLACE "${_hook_call}"
                        "                    if (((pc_before & 0x7fffffu) != 0x05ec9au ||\n                         step_cap - steps >= 14) &&\n                        ((pc_before & 0x7fffffu) != 0x05ecbdu ||\n                         step_cap - steps >= 15)) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
                if(_battle_timer_steps)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_BATTLE_TIMER_STEPS 1\n")
                    string(REPLACE "${_hook_call}"
                        "                    if (((pc_before & 0x7fffffu) != 0x058f1fu ||\n                         step_cap - steps >= 10) &&\n                        ((pc_before & 0x7fffffu) != 0x058f3eu ||\n                         step_cap - steps >= 6)) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
                if(_battle_wave_fill_steps)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_BATTLE_WAVE_FILL_STEPS 1\n")
                    string(REPLACE "${_hook_call}"
                        "                    if ((pc_before & 0x7fffffu) != 0x05aef5u ||\n                        step_cap - steps >= 15) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
                if(_battle_effect_slot_steps)
                    string(REPLACE "${_hook_call}"
                        "                    if ((((pc_before & 0x7fffffu) != 0x018a7eu &&\n                          (pc_before & 0x7fffffu) != 0x018ac1u) ||\n                         step_cap - steps >= 14) &&\n                        (((pc_before & 0x7fffffu) != 0x018aaau &&\n                          (pc_before & 0x7fffffu) != 0x018b1bu) ||\n                         step_cap - steps >= 8)) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
                if(_battle_effect_dispatch_steps)
                    string(REPLACE "${_hook_call}"
                        "                    if (((pc_before & 0x7fffffu) != 0x018a95u &&\n                         (pc_before & 0x7fffffu) != 0x018b08u) ||\n                        step_cap - steps >= 9) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
                if(_battle_effect_cleanup_steps)
                    string(REPLACE "${_hook_call}"
                        "                    if ((pc_before & 0x7fffffu) != 0x018bfbu ||\n                        step_cap - steps >= 12) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
                if(_decompress_reference_steps)
                    string(REPLACE "${_hook_call}"
                        "                    if ((((pc_before & 0x7fffffu) != 0x008f44u &&\n                          (pc_before & 0x7fffffu) != 0x008fe8u) ||\n                         step_cap - steps >= 18) &&\n                        (((pc_before & 0x7fffffu) != 0x008f6fu &&\n                          (pc_before & 0x7fffffu) != 0x009013u) ||\n                         step_cap - steps >= 27) &&\n                        (((pc_before & 0x7fffffu) != 0x008f5cu &&\n                          (pc_before & 0x7fffffu) != 0x009000u) ||\n                         step_cap - steps >= 7)) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
                if(_decompress_flag_steps)
                    string(REPLACE "${_hook_call}"
                        "                    if ((((pc_before & 0x7fffffu) != 0x008f40u &&\n                          (pc_before & 0x7fffffu) != 0x008fe4u) ||\n                         step_cap - steps >= 14) &&\n                        (((pc_before & 0x7fffffu) != 0x008f14u &&\n                          (pc_before & 0x7fffffu) != 0x008fb8u) ||\n                         step_cap - steps >= 10)) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
                if(_decompress_steps)
                    string(REPLACE "${_hook_call}"
                        "                    if (((pc_before & 0x7fffffu) != 0x008eefu &&\n                         (pc_before & 0x7fffffu) != 0x008f93u &&\n                         (pc_before & 0x7fffffu) != 0x008efau &&\n                         (pc_before & 0x7fffffu) != 0x008f9eu &&\n                         (pc_before & 0x7fffffu) != 0x008f66u &&\n                         (pc_before & 0x7fffffu) != 0x00900au) ||\n                        step_cap - steps >= 11) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
                if(_battle_background_steps)
                    string(REPLACE "${_hook_call}"
                        "                    if ((pc_before & 0x7fffffu) != 0x01ba59u ||\n                        step_cap - steps >= 7) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
                if(_battle_transition_steps)
                    string(REPLACE "${_hook_call}"
                        "                    if (((pc_before & 0x7fffffu) != 0x048c91u &&\n                         (pc_before & 0x7fffffu) != 0x048cddu) ||\n                        step_cap - steps >= 19) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
            endif()
            file(WRITE "${_output}.tmp" "${_text}${_timing_suffix}")
            execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${_output}.tmp" "${_output}" COMMAND_ERROR_IS_FATAL ANY)
            foreach(_prop IN ITEMS COMPILE_DEFINITIONS COMPILE_OPTIONS COMPILE_FLAGS
                    INCLUDE_DIRECTORIES LANGUAGE COMPILE_DEFINITIONS_RELEASE
                    COMPILE_DEFINITIONS_DEBUG COMPILE_DEFINITIONS_RELWITHDEBINFO
                    COMPILE_DEFINITIONS_MINSIZEREL)
                get_source_file_property(_value "${_source}" "${_prop}")
                if(NOT _value STREQUAL "NOTFOUND")
                    set_property(SOURCE "${_output}" PROPERTY "${_prop}" "${_value}")
                endif()
            endforeach()
            get_filename_component(_parent "${_source}" DIRECTORY)
            set_property(SOURCE "${_output}" APPEND PROPERTY INCLUDE_DIRECTORIES
                "${_parent};${core_root}/runner/src;${core_root}/runner/src/snes")
            list(APPEND _result "${_output}")
            math(EXPR _count "${_count} + 1")
        else()
            list(APPEND _result "${_source}")
        endif()
        if(_source MATCHES "(^|/)src/(decomp_bridge/actor_bridge|lufia2_runtime)[.]c$")
            foreach(_definition IN LISTS _definitions)
                set_property(SOURCE "${_source}" APPEND PROPERTY COMPILE_DEFINITIONS
                    "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:${_definition}>")
            endforeach()
        endif()
    endforeach()
    if(NOT _count EQUAL 1)
        message(FATAL_ERROR "Native NMI timing expected one interpreter bridge")
    endif()
    set_property(TARGET ${target} PROPERTY SOURCES "${_result}")
endfunction()
