include_guard(GLOBAL)
option(LUFIA2_ENABLE_SOUND_CLOCK_BARRIERS "Experimental native sound clock barriers" OFF)

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
    set(_battle_nmi_callbacks FALSE)
    set(_ripple_word_phases FALSE)
    set(_nmi_scroll_finish FALSE)
    set(_nmi_tilemap_steps FALSE)
    set(_nmi_joypad_gate FALSE)
    set(_nmi_dma_setup FALSE)
    set(_nmi_upload_checks FALSE)
    set(_nmi_pad_steps FALSE)
    set(_nmi_tilemap_loads FALSE)
    set(_nmi_scroll_fronts FALSE)
    set(_nmi_listed_dma_prepare FALSE)
    set(_nmi_restore_clock FALSE)
    set(_nmi_callback_clock FALSE)
    set(_nmi_call_boundaries FALSE)
    set(_player_continuations FALSE)
    set(_nmi_callback_boundaries FALSE)
    set(_nmi_child_boundaries FALSE)
    set(_screen_fade_request FALSE)
    set(_screen_fade_steps FALSE)
    set(_frame_wait_boundaries FALSE)
    set(_frame_register_boundaries FALSE)
    set(_frame_exit_steps FALSE)
    set(_battle_command_steps FALSE)
    set(_party_input_steps FALSE)
    set(_action_record_copy_spans FALSE)
    set(_battle_target_name_setup FALSE)
    set(_nmi_upload_control_steps FALSE)
    set(_nmi_control_steps FALSE)
    set(_battle_target_control_steps FALSE)
    set(_battle_control_steps FALSE)
    set(_field_region_control_steps FALSE)
    set(_field_control_steps FALSE)
    set(_world_frame_control_steps FALSE)
    set(_menu_input_boundaries FALSE)
    set(_sound_frame_boundaries FALSE)
    set(_field_actor_boundaries FALSE)
    set(_actor_slot_calls FALSE)
    set(_battle_effect_command_steps FALSE)
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
    set(_battle_status FALSE)
    set(_battle_target_name_steps FALSE)
    set(_battle_target_cursor FALSE)
    set(_battle_target_input FALSE)
    set(_battle_result_steps FALSE)
    set(_effect_clear_window FALSE)
    set(_battle_target_marker_clock FALSE)
    set(_battle_target_selection FALSE)
    set(_filled_windows FALSE)
    set(_second_battler_stat FALSE)
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
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2MenuLeafHelpers.cmake")
                lufia2_menu_leaves_available(_menu_leaves)
                if(_menu_leaves)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_MENU_LEAVES=1)
                    lufia2_second_battler_stat_available(_second_battler_stat)
                    if(_second_battler_stat)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_SECOND_BATTLER_STAT=1)
                    endif()
                    lufia2_battle_status_available(_battle_status)
                    if(_battle_status)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_STATUS=1)
                        lufia2_filled_windows_available(_filled_windows)
                        if(_filled_windows)
                            list(APPEND _definitions LUFIA2_HAVE_NATIVE_FILLED_WINDOWS=1)
                        endif()
                    endif()
                    lufia2_battle_clear_available(_battle_clear)
                    if(_battle_clear)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_CLEAR=1)
                    endif()
                    lufia2_raster_edge_available(_raster_edge)
                    if(_raster_edge)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_RASTER_EDGE=1)
                    endif()
                    lufia2_tile_spell_available(_tile_spell)
                    if(_tile_spell)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_TILE_SPELL=1)
                    endif()
                    lufia2_post_return_leaves_available(_post_return_leaves)
                    if(_post_return_leaves)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_POST_RETURN_LEAVES=1)
                    endif()
                    lufia2_fixed_cursor_available(_fixed_cursor)
                    if(_fixed_cursor)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_FIXED_CURSOR=1)
                    endif()
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2MenuWidth.cmake")
                lufia2_menu_width_available(_menu_width)
                if(_menu_width)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_MENU_WIDTH=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2MessageFormat.cmake")
                lufia2_message_format_available(_message_format)
                if(_message_format)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_MESSAGE_FORMAT=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2FixedMessageWaits.cmake")
                lufia2_fixed_message_waits_available(_fixed_message_waits)
                if(_fixed_message_waits)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_FIXED_MESSAGE_WAITS=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleRippleSteps.cmake")
                lufia2_battle_ripple_steps_available(_ripple_steps)
                lufia2_battle_ripple_word_steps_available(_ripple_word_steps)
                lufia2_battle_ripple_word_phases_available(_ripple_word_phases)
                if(_ripple_word_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_RIPPLE_WORD_STEPS=1>")
                endif()
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
                list(APPEND _definitions LUFIA2_HAVE_NATIVE_FRAME_RETURNS=1
                    LUFIA2_HAVE_NATIVE_FIELD_TICK_CLOCK=1
                    LUFIA2_HAVE_NATIVE_FIELD_CHILD_CLOCK=1
                    LUFIA2_HAVE_NATIVE_EVENT_SLOT_CLOCK=1
                    LUFIA2_HAVE_NATIVE_FIELD_IDLE_CLOCK=1
                    LUFIA2_HAVE_NATIVE_FIELD_STAIR_CLOCK=1
                    LUFIA2_HAVE_NATIVE_FIELD_RECTANGLE_CLOCK=1
                    LUFIA2_HAVE_NATIVE_FIELD_TRIGGER_CLOCK=1
                    LUFIA2_HAVE_NATIVE_FIELD_REQUEST_CLOCK=1)
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2ActorSlotPhases.cmake")
                lufia2_actor_slot_phases_available(_actor_slot_phases)
                if(_actor_slot_phases)
                    lufia2_actor_slot_calls_available(_actor_slot_calls)
                    lufia2_actor_slot_boundaries_available(_actor_slot_boundaries)
                    if(_actor_slot_boundaries)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_ACTOR_SLOT_BOUNDARIES=1)
                    endif()
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_ACTOR_SLOT_PHASES=1)
                    lufia2_actor_slot_child_return_available(_actor_slot_child_return)
                    if(_actor_slot_child_return)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_ACTOR_SLOT_CHILD_RETURN=1)
                    endif()
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2PlayerControl.cmake")
                lufia2_player_control_available(_player_control)
                if(_player_control)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_PLAYER_CONTROL_CLOCK=1)
                    lufia2_player_continuations_available(_player_continuations)
                endif()
                set(_field_map_clock_definition "")
                set(_item_region_definition "")
                set(_item_region_return_clause "")
                set(_vehicle_terrain_definition "")
                set(_vehicle_terrain_return_clause "")
                lufia2_actor_slot_traversal_available(_have_actor_slot_traversal)
                if(_have_actor_slot_traversal)
                    list(APPEND _definitions
                        LUFIA2_HAVE_NATIVE_ACTOR_SLOT_TRAVERSAL=1
                        LUFIA2_HAVE_NATIVE_FIELD_MAP_CLOCK=1)
                    set(_field_map_clock_definition "#define LUFIA2_HAVE_NATIVE_FIELD_MAP_CLOCK 1\n")
                    include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2ItemRegion.cmake")
                    lufia2_item_region_available(_item_region)
                    if(_item_region)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_ITEM_REGION_CLOCK=1)
                        set(_item_region_definition "#define LUFIA2_HAVE_NATIVE_ITEM_REGION_CLOCK 1\n")
                        set(_item_region_return_clause "                         (target & 0x7fffffu) == 0x0eb63bu ||\n")
                    endif()
                    include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2VehicleTerrain.cmake")
                    lufia2_vehicle_terrain_available(_vehicle_terrain)
                    if(_vehicle_terrain)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_VEHICLE_TERRAIN=1)
                        set(_vehicle_terrain_definition "#define LUFIA2_HAVE_NATIVE_VEHICLE_TERRAIN 1\n")
                        set(_vehicle_terrain_return_clause "                         (target & 0x7fffffu) == 0x0ebbafu ||\n")
                    endif()
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2FieldFrameDecision.cmake")
                lufia2_field_frame_decision_available(_field_frame_decision)
                if(_field_frame_decision)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_FIELD_FRAME_DECISION=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2FieldTriggerFronts.cmake")
                lufia2_field_trigger_fronts_available(_field_trigger_fronts)
                if(_field_trigger_fronts)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_FIELD_TRIGGER_FRONTS=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2FieldRegionPhases.cmake")
                lufia2_field_region_phases_available(_field_region_phases)
                if(_field_region_phases)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_FIELD_REGION_PHASES=1)
                endif()
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
                    lufia2_poker_stake_entries_available(_poker_window_entry ${target} window)
                    if(_poker_window_entry)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_POKER_WINDOW=1)
                    endif()
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
                lufia2_boot_nmi_wait_available(_boot_nmi_wait ${target})
                if(_boot_nmi_wait)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_BOOT_NMI_WAIT=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2SceneOperation.cmake")
                lufia2_scene_operation_available(_scene_operation ${target})
                if(_scene_operation)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_SCENE_OPERATION=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2ContinuationEntries.cmake")
                lufia2_continuation_entries_available(_continuation_entries ${target})
                if(_continuation_entries)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_CONTINUATION_ENTRIES=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2TableOutcomes.cmake")
                lufia2_table_outcomes_available(_table_outcomes ${target})
                if(_table_outcomes)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_TABLE_OUTCOMES=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2TableSearch.cmake")
                lufia2_table_search_available(_table_search ${target})
                if(_table_search)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_TABLE_SEARCH=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2TableSymbols.cmake")
                lufia2_table_symbols_available(_table_symbols ${target})
                if(_table_symbols)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_TABLE_SYMBOLS=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2TableRounds.cmake")
                lufia2_table_rounds_available(_table_rounds ${target})
                if(_table_rounds)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_TABLE_ROUNDS=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2TableLines.cmake")
                lufia2_table_lines_available(_table_lines ${target})
                if(_table_lines)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_TABLE_LINES=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2TableMotion.cmake")
                lufia2_table_motion_available(_table_motion ${target})
                if(_table_motion)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_TABLE_MOTION=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2TableServices.cmake")
                lufia2_table_services_available(_table_services ${target})
                if(_table_services)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_TABLE_SERVICES=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2MemberScreens.cmake")
                lufia2_member_screens_available(_member_screens ${target})
                if(_member_screens)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_MEMBER_SCREENS=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2MainMenus.cmake")
                lufia2_main_menus_available(_main_menus ${target})
                if(_main_menus)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_MAIN_MENUS=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2PanelInput.cmake")
                lufia2_panel_input_available(_panel_input ${target})
                if(_panel_input)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_PANEL_INPUT=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2CursorInput.cmake")
                lufia2_cursor_input_available(_cursor_input ${target})
                if(_cursor_input)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_CURSOR_INPUT=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2CursorPayout.cmake")
                lufia2_cursor_payout_available(_cursor_payout ${target})
                if(_cursor_payout)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_CURSOR_PAYOUT=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2CardTable.cmake")
                lufia2_card_table_available(_card_table ${target})
                if(_card_table)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_CARD_TABLE=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2PokerPlayer.cmake")
                lufia2_poker_player_available(_poker_player ${target})
                if(_poker_player)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_POKER_PLAYER=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2SpecialArithmetic.cmake")
                lufia2_special_arithmetic_available(_special_arithmetic ${target})
                if(_special_arithmetic)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_SPECIAL_ARITHMETIC=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2SpecialWindows.cmake")
                lufia2_special_windows_available(_special_windows ${target})
                if(_special_windows)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_SPECIAL_WINDOWS=1)
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2MenuClear.cmake")
                lufia2_menu_clear_available(_menu_clear ${target})
                if(_menu_clear)
                    list(APPEND _definitions LUFIA2_HAVE_NATIVE_MENU_CLEAR=1)
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
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleTimerCalls.cmake")
                lufia2_battle_timer_calls_available(_battle_timer_calls)
                lufia2_battle_nmi_callbacks_available(_battle_nmi_callbacks)
                if(_battle_timer_calls)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_TIMER_CALLS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2SpriteOverlays.cmake")
                lufia2_sprite_overlays_available(_sprite_overlays)
                if(_sprite_overlays)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_SPRITE_OVERLAYS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleHdmaBuilders.cmake")
                lufia2_battle_hdma_builders_available(_battle_hdma_builders)
                if(_battle_hdma_builders)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_HDMA_BUILDERS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleWaveFillSteps.cmake")
                lufia2_battle_wave_fill_steps_available(_battle_wave_fill_steps)
                if(_battle_wave_fill_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_WAVE_FILL_STEPS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleTargetNameSteps.cmake")
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleRowSpans.cmake")
                lufia2_battle_row_spans_available(_battle_row_spans)
                if(_battle_row_spans AND _battle_status AND _menu_leaves)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_ROW_SPANS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleTargetMarkerSpans.cmake")
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleCommandTiles.cmake")
                lufia2_battle_command_tiles_available(_battle_command_tiles)
                lufia2_selected_command_tile_available(_selected_command_tile)
                lufia2_party_command_tiles_available(_party_command_tiles)
                if(_battle_command_tiles)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_COMMAND_TILES=1>")
                    if(_party_command_tiles)
                        set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                            APPEND PROPERTY COMPILE_DEFINITIONS
                                "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_PARTY_COMMAND_TILES=1>")
                    endif()
                    if(_selected_command_tile)
                        set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                            APPEND PROPERTY COMPILE_DEFINITIONS
                                "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_SELECTED_COMMAND_TILE=1>")
                    endif()
                endif()
                lufia2_battle_target_marker_spans_available(_battle_target_marker_spans)
                if(_battle_target_marker_spans AND _battle_status AND _menu_leaves)
                    set(_battle_target_marker_clock TRUE)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_TARGET_MARKER_SPANS=1>")
                endif()
                lufia2_battle_target_name_steps_available(_battle_target_name_steps)
                lufia2_battle_target_cursor_available(_battle_target_cursor)
                lufia2_battle_target_selection_available(_battle_target_selection)
                lufia2_battle_target_input_available(_battle_target_input)
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleResultSteps.cmake")
                lufia2_battle_result_steps_available(_battle_result_steps)
                if(_battle_target_name_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_TARGET_NAME_STEPS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleEffectSlotSteps.cmake")
                lufia2_battle_effect_slot_steps_available(_battle_effect_slot_steps)
                if(_battle_effect_slot_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_SLOT_STEPS=1>")
                endif()
                lufia2_battle_effect_slot_cursors_available(_battle_effect_slot_cursors)
                if(_battle_effect_slot_cursors)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_SLOT_CURSORS=1>")
                endif()
                lufia2_battle_effect_slot_headers_available(_battle_effect_slot_headers)
                if(_battle_effect_slot_headers)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_SLOT_HEADERS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleEffectDispatchSteps.cmake")
                lufia2_battle_effect_dispatch_steps_available(_battle_effect_dispatch_steps)
                lufia2_battle_effect_command_steps_available(_battle_effect_command_steps)
                if(_battle_effect_dispatch_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_DISPATCH_STEPS=1>")
                endif()
                lufia2_battle_effect_frame_steps_available(_battle_effect_frame_steps)
                if(_battle_effect_frame_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_FRAME_STEPS=1>")
                endif()
                include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleEffectCleanupSteps.cmake")
                lufia2_battle_effect_cleanup_steps_available(_battle_effect_cleanup_steps)
                if(_battle_effect_cleanup_steps)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_CLEANUP_STEPS=1>")
                endif()
                lufia2_battle_effect_clear_spans_available(_battle_effect_clear_spans)
                if(_battle_effect_clear_spans)
                    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                        APPEND PROPERTY COMPILE_DEFINITIONS
                            "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_CLEAR_SPANS=1>")
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
                        if(_frame_register_boundaries)
                            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2NmiControlSteps.cmake")
                            lufia2_nmi_control_steps_available(_nmi_control_steps)
                            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2NmiUploadControlSteps.cmake")
                            lufia2_nmi_upload_control_steps_available(_nmi_upload_control_steps)
                            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleTargetNameSetup.cmake")
                            lufia2_battle_target_name_setup_available(_battle_target_name_setup)
                            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleControlSteps.cmake")
                            lufia2_battle_control_steps_available(_battle_control_steps)
                            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleTargetControlSteps.cmake")
                            lufia2_battle_target_control_steps_available(_battle_target_control_steps)
                            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2FieldControlSteps.cmake")
                            lufia2_field_control_steps_available(_field_control_steps)
                            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2FieldRegionControlSteps.cmake")
                            lufia2_field_region_control_steps_available(_field_region_control_steps)
                            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2WorldFrameControlSteps.cmake")
                            lufia2_world_frame_control_steps_available(_world_frame_control_steps)
                        endif()
                        if(_frame_wait_boundaries AND _frame_register_boundaries)
                            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2FrameExitSteps.cmake")
                            lufia2_frame_exit_steps_available(_frame_exit_steps)
                        endif()
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
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2NmiCallBoundaries.cmake")
                        lufia2_nmi_child_boundaries_available(_nmi_child_boundaries)
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
                        if(_finish_api MATCHES "#define LUFIA2_NMI_RESTORE_INDEX_WIDTHS 2")
                            set(_nmi_restore_clock TRUE)
                            if(_finish_api MATCHES "#define LUFIA2_NMI_DISPLAY_INDEX_WIDTHS 2")
                                set(_nmi_display_clock TRUE)
                                list(APPEND _definitions LUFIA2_HAVE_NATIVE_NMI_DISPLAY_CLOCK=1)
                            endif()
                            if(_calls_api MATCHES "#define LUFIA2_NMI_CALLBACK_INDEX_WIDTHS 2")
                                set(_nmi_callback_clock TRUE)
                                list(APPEND _definitions LUFIA2_HAVE_NATIVE_NMI_CALLBACK_CLOCK=1)
                            endif()
                            list(APPEND _definitions LUFIA2_HAVE_NATIVE_NMI_RESTORE_CLOCK=1)
                        endif()
                        set(_nmi_restore_width TRUE)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_NMI_RESTORE_WIDTH=1)
                        if(_ripple_word_phases)
                            list(APPEND _definitions LUFIA2_HAVE_NATIVE_RIPPLE_WORD_PHASES=1)
                        endif()
                    endif()
                endif()
                if(_nmi_restore_clock AND _nmi_display_clock AND _nmi_callback_clock)
                    include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2NmiJoypadGate.cmake")
                    lufia2_nmi_joypad_gate_available(_nmi_joypad_gate)
                    if(_nmi_joypad_gate)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_NMI_JOYPAD_GATE=1)
                    endif()
                    include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2NmiDmaSetup.cmake")
                    lufia2_nmi_dma_setup_available(_nmi_dma_setup)
                    if(_nmi_dma_setup)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_NMI_DMA_SETUP=1)
                    endif()
                    include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2NmiUploadChecks.cmake")
                    lufia2_nmi_upload_checks_available(_nmi_upload_checks)
                    if(_nmi_upload_checks)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_NMI_UPLOAD_CHECKS=1)
                    endif()
                    include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2NmiPadSteps.cmake")
                    lufia2_nmi_pad_steps_available(_nmi_pad_steps)
                    if(_nmi_pad_steps)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_NMI_PAD_STEPS=1)
                    endif()
                    include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2NmiScrollPairs.cmake")
                    include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2NmiCallBoundaries.cmake")
                    include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2ScreenFadeSteps.cmake")
                    lufia2_screen_fade_steps_available(_screen_fade_steps)
                    if(_screen_fade_steps)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_SCREEN_FADE_STEPS=1)
                    endif()
                    lufia2_screen_fade_request_available(_screen_fade_request)
                    if(_screen_fade_request)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_SCREEN_FADE_REQUEST=1)
                    endif()
                    lufia2_nmi_tilemap_steps_available(_nmi_tilemap_steps)
                    if(_nmi_tilemap_steps)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_NMI_TILEMAP_STEPS=1)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2NmiTilemapLoads.cmake")
                        lufia2_nmi_tilemap_loads_available(_nmi_tilemap_loads)
                        if(_nmi_tilemap_loads)
                            list(APPEND _definitions LUFIA2_HAVE_NATIVE_NMI_TILEMAP_LOADS=1)
                        endif()
                    endif()
                    lufia2_nmi_scroll_pairs_available(_nmi_scroll_pairs)
                    lufia2_nmi_scroll_fronts_available(_nmi_scroll_fronts)
                    if(_nmi_scroll_fronts)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_NMI_SCROLL_FRONTS=1)
                        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2NmiListedDma.cmake")
                        lufia2_nmi_listed_dma_prepare_available(_nmi_listed_dma_prepare)
                        if(_nmi_listed_dma_prepare)
                            list(APPEND _definitions LUFIA2_HAVE_NATIVE_NMI_LISTED_DMA_PREPARE=1)
                        endif()
                        lufia2_nmi_scroll_finish_available(_nmi_scroll_finish)
                        if(_nmi_scroll_finish)
                            list(APPEND _definitions LUFIA2_HAVE_NATIVE_NMI_SCROLL_FINISH=1)
                        endif()
                    endif()
                    if(_nmi_scroll_pairs)
                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_NMI_SCROLL_PAIRS=1)
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
                        set(_session_initializers_ready ${_frame_polls})
                        foreach(_initializer_spec
                            "84:9C9B|Lufia2BattleInitializePreviewBackground|battle.h|battle/battle_frame_setup.c"
                            "86:920E|Lufia2WorldMapRunFixedStartSession|world_map.h|world/world_scene.c")
                            string(REPLACE "|" ";" _initializer_fields "${_initializer_spec}")
                            list(GET _initializer_fields 0 _initializer_address)
                            list(GET _initializer_fields 1 _initializer_name)
                            list(GET _initializer_fields 2 _initializer_header)
                            list(GET _initializer_fields 3 _initializer_source)
                            string(REGEX MATCH
                                "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"${_initializer_name}\"[^[]*"
                                _initializer_contract "${_metadata}")
                            if(NOT _initializer_contract MATCHES "address[ \t]*=[ \t]*\"${_initializer_address}\"" OR
                               NOT _initializer_contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" OR
                               NOT _initializer_contract MATCHES "exit_mx[ \t]*=[ \t]*\"M1X0\"" OR
                               NOT _initializer_contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
                                set(_session_initializers_ready FALSE)
                            endif()
                            foreach(_initializer_path
                                "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/${_initializer_header}"
                                "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/${_initializer_source}")
                                if(EXISTS "${_initializer_path}")
                                    file(READ "${_initializer_path}" _initializer_text)
                                    if(NOT _initializer_text MATCHES "${_initializer_name}[(]")
                                        set(_session_initializers_ready FALSE)
                                    endif()
                                else()
                                    set(_session_initializers_ready FALSE)
                                endif()
                            endforeach()
                        endforeach()
                        if(_session_initializers_ready)
                            list(APPEND _definitions LUFIA2_HAVE_NATIVE_SESSION_INITIALIZERS=1)
                        endif()
                        set(_dissolve_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle.h")
                        set(_dissolve_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_transition_setup.c")
                        if(_frame_polls AND EXISTS "${_dissolve_header}" AND EXISTS "${_dissolve_source}")
                            file(READ "${_dissolve_header}" _dissolve_api)
                            file(READ "${_dissolve_source}" _dissolve_implementation)
                            string(REGEX MATCH
                                "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2BattleRunDissolveEffect\"[^[]*"
                                _dissolve_contract "${_metadata}")
                            string(REGEX MATCH
                                "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2BattleRunDissolveTransition\"[^[]*"
                                _dissolve_transition_contract "${_metadata}")
                            if(_dissolve_api MATCHES "LUFIA2_BATTLE_DISSOLVE_PIXEL_API[ \t]+1" AND
                               _dissolve_api MATCHES "Lufia2BattleRunDissolveEffectWithBudget[(]" AND
                               _dissolve_api MATCHES "Lufia2BattleContinueDissolveMask[(]" AND
                               _dissolve_api MATCHES "Lufia2BattleRunDissolveTransitionWithBudget[(]" AND
                               _dissolve_implementation MATCHES "Lufia2BattleContinueDissolveMask[(][^;{]*[{]" AND
                               _dissolve_contract MATCHES "address[ \t]*=[ \t]*\"85:E7C5\"" AND
                               _dissolve_contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" AND
                               _dissolve_contract MATCHES "exit_mx[ \t]*=[ \t]*\"M1X0\"" AND
                               _dissolve_contract MATCHES "status[ \t]*=[ \t]*\"verified\"" AND
                               _dissolve_transition_contract MATCHES "address[ \t]*=[ \t]*\"85:E8E8\"" AND
                               _dissolve_transition_contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" AND
                               _dissolve_transition_contract MATCHES "exit_mx[ \t]*=[ \t]*\"M1X0\"" AND
                               _dissolve_transition_contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
                                list(APPEND _definitions LUFIA2_HAVE_NATIVE_DISSOLVE_SETUP=1)
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
                                set(_rectangle_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/menu/menu_tile_clear.c")
                                if(EXISTS "${_rectangle_source}" AND _rectangle_api MATCHES "Lufia2MenuClearLayerRectangleColumns[(]")
                                    file(READ "${_rectangle_source}" _rectangle_implementation)
                                    if(_rectangle_implementation MATCHES "Lufia2MenuClearLayerRectangleColumns[(][^;{]*[{]")
                                        list(APPEND _definitions LUFIA2_HAVE_NATIVE_MENU_RECTANGLE_SPANS=1)
                                    endif()
                                endif()
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
    if(_frame_returns AND "LUFIA2_HAVE_NATIVE_SOUND_UPLOAD_STEPS=1" IN_LIST _definitions)
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2SoundBegin.cmake")
        lufia2_sound_begin_available(_sound_begin)
        if(_sound_begin)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_SOUND_BEGIN_PAYLOAD=1)
        endif()
    endif()
    set(_battle_tilemap_spans FALSE)
    if(_frame_returns AND _battle_status)
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleTilemapSpans.cmake")
        lufia2_battle_tilemap_spans_available(_battle_tilemap_spans)
        if(_battle_tilemap_spans)
            set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                APPEND PROPERTY COMPILE_DEFINITIONS
                    "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_BATTLE_TILEMAP_SPANS=1>")
        endif()
    endif()
    set(_action_copy_spans FALSE)
    if(_frame_returns AND _battle_status)
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2ActionCopySpans.cmake")
        lufia2_action_copy_spans_available(_action_copy_spans)
        if(_action_copy_spans)
            set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c"
                APPEND PROPERTY COMPILE_DEFINITIONS
                    "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,${target}>:LUFIA2_HAVE_NATIVE_ACTION_COPY_SPANS=1>")
        endif()
    endif()
    set(_action_clear_steps FALSE)
    if(_frame_returns AND _battle_status)
        set(_action_clear_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle.h")
        set(_action_clear_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_action_work.c")
        if(EXISTS "${_action_clear_header}" AND EXISTS "${_action_clear_source}")
            file(READ "${_action_clear_header}" _action_clear_api)
            file(READ "${_action_clear_source}" _action_clear_body)
            if(_action_clear_api MATCHES "Lufia2BattleClearActionWordStep[(]" AND
               _action_clear_body MATCHES "Lufia2ExecutionResult[ \r\n]+Lufia2BattleClearActionWordStep[(]")
                set(_action_clear_steps TRUE)
                list(APPEND _definitions LUFIA2_HAVE_NATIVE_ACTION_CLEAR_STEPS=1)
            endif()
        endif()
    endif()
    set(_frame_decisions FALSE)
    if(_frame_returns)
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2FrameDecisions.cmake")
        lufia2_frame_decisions_available(_frame_decisions)
        if(_frame_decisions)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_FRAME_DECISIONS=1)
        endif()
    endif()
    set(_world_frame_loop FALSE)
    if(_frame_returns AND _sound_source_spans AND
       "LUFIA2_HAVE_NATIVE_SOUND_UPLOAD_STEPS=1" IN_LIST _definitions)
        list(APPEND _definitions LUFIA2_HAVE_NATIVE_SOUND_SOURCE_CLOCK=1)
    endif()
    if(_frame_returns AND _nmi_restore_width AND _battle_nmi_callbacks)
        list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_NMI_CALLBACKS=1)
    else()
        set(_battle_nmi_callbacks FALSE)
    endif()
    if(_frame_returns AND _battle_effect_cleanup_steps AND _battle_effect_clear_spans)
        set(_effect_clear_window TRUE)
        list(APPEND _definitions LUFIA2_HAVE_NATIVE_EFFECT_CLEAR_WINDOW=1)
    endif()
    if(_frame_returns AND _battle_result_steps)
        list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_RESULT_STEPS=1)
    else()
        set(_battle_result_steps FALSE)
    endif()
    if(_frame_returns AND _battle_target_marker_clock)
        list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_TARGET_MARKER_WINDOW=1)
    endif()
    if(_frame_returns AND _battle_input_steps)
        list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_INPUT_WINDOW=1)
    endif()
    if(_frame_returns AND _battle_target_input)
        list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_TARGET_INPUT=1)
    else()
        set(_battle_target_input FALSE)
    endif()
    if(_frame_returns AND _battle_target_selection)
        list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_TARGET_SELECTION=1)
    else()
        set(_battle_target_selection FALSE)
    endif()
    if(_frame_returns AND _battle_target_cursor)
        list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_TARGET_CURSOR=1)
    else()
        set(_battle_target_cursor FALSE)
    endif()
    if(_frame_returns AND _player_continuations AND
       "LUFIA2_HAVE_NATIVE_FIELD_MAP_CLOCK=1" IN_LIST _definitions)
        list(APPEND _definitions LUFIA2_HAVE_NATIVE_PLAYER_CONTINUATIONS=1)
    else()
        set(_player_continuations FALSE)
    endif()
    if(_frame_returns AND (_battle_tilemap_spans OR _action_copy_spans OR _effect_clear_window OR _battle_result_steps OR _battle_target_marker_clock OR _battle_input_steps OR _battle_target_input OR _battle_target_selection OR _battle_target_cursor OR _player_continuations OR _decompress_steps OR _decompress_reference_steps OR
                           _battle_effect_dispatch_steps OR _action_clear_steps OR
                           _battle_target_name_steps OR _frame_decisions))
        list(APPEND _definitions LUFIA2_HAVE_NATIVE_OPCODE_WINDOW_CONTEXT=1)
    endif()
    if(_frame_returns AND "LUFIA2_HAVE_NATIVE_OPCODE_WINDOW_CONTEXT=1" IN_LIST _definitions)
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2ActorSceneValueSteps.cmake")
        lufia2_actor_scene_value_steps_available(_actor_scene_value_steps)
        if(_actor_scene_value_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_ACTOR_SCENE_VALUE_STEPS=1)
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2ActorScenePresentationSteps.cmake")
        lufia2_actor_scene_presentation_steps_available(_actor_scene_presentation_steps)
        if(_actor_scene_presentation_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_ACTOR_SCENE_PRESENTATION_STEPS=1)
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2FieldActorRebuildSteps.cmake")
        lufia2_field_actor_rebuild_steps_available(_field_actor_rebuild_steps)
        if(_field_actor_rebuild_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_FIELD_ACTOR_REBUILD_STEPS=1)
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2SceneClaimSteps.cmake")
        lufia2_scene_claim_steps_available(_scene_claim_steps)
        if(_scene_claim_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_SCENE_CLAIM_STEPS=1)
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleActionParameterSteps.cmake")
        lufia2_battle_action_parameter_steps_available(_battle_action_parameter_steps)
        if(_battle_action_parameter_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_ACTION_PARAMETER_STEPS=1)
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleTargetRecordSteps.cmake")
        lufia2_battle_target_record_steps_available(_battle_target_record_steps)
        if(_battle_target_record_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_TARGET_RECORD_STEPS=1)
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2EventTimerSteps.cmake")
        lufia2_event_timer_steps_available(_event_timer_steps)
        if(_event_timer_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_EVENT_TIMER_STEPS=1)
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleStatusRollSteps.cmake")
        lufia2_battle_status_roll_steps_available(_battle_status_roll_steps)
        if(_battle_status_roll_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_STATUS_ROLL_STEPS=1)
        endif()
        if(_battle_status AND _menu_leaves)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_MULTIPLY_WINDOW=1)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_FRAME_ENDS_WINDOW=1)
            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleStatusSteps.cmake")
            lufia2_battle_status_steps_available(_battle_status_steps)
            if(_battle_status_steps)
                list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_STATUS_STEPS=1)
            endif()
            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2DecimalDigitsSteps.cmake")
            lufia2_decimal_digits_steps_available(_decimal_digits_steps)
            if(_decimal_digits_steps)
                list(APPEND _definitions LUFIA2_HAVE_NATIVE_DECIMAL_DIGITS_STEPS=1)
            endif()
        endif()
        if(_action_clear_steps)
            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2ActionClearSpans.cmake")
            lufia2_action_clear_spans_available(_action_clear_spans)
            if(_action_clear_spans)
                list(APPEND _definitions LUFIA2_HAVE_NATIVE_ACTION_CLEAR_SPANS=1)
            endif()
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattlePartyInput.cmake")
        lufia2_battle_party_input_steps_available(_party_input_steps)
        if(_party_input_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_PARTY_INPUT_STEPS=1)
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2ActionRecordCopySpans.cmake")
        lufia2_action_record_copy_spans_available(_action_record_copy_spans)
        if(_action_record_copy_spans)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_ACTION_RECORD_COPY_SPANS=1)
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleCommandInput.cmake")
        lufia2_battle_command_input_steps_available(_command_input_steps)
        if(_command_input_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_COMMAND_INPUT_STEPS=1)
        endif()
        if(_battle_command_tiles)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_COMMAND_TILE_WINDOWS=1)
        endif()
        if(_battle_row_spans AND _battle_status AND _menu_leaves)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_ROW_WINDOWS=1)
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2SaveActionSteps.cmake")
        lufia2_save_action_steps_available(_save_action_steps)
        if(_save_action_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_SAVE_ACTION_STEPS=1)
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2FieldSceneObjectSteps.cmake")
        lufia2_field_scene_object_steps_available(_field_scene_object_steps)
        if(_field_scene_object_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_FIELD_SCENE_OBJECT_STEPS=1)
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2SceneSlotResetSteps.cmake")
        lufia2_scene_slot_reset_steps_available(_scene_slot_reset_steps)
        if(_scene_slot_reset_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_SCENE_SLOT_RESET_STEPS=1)
        endif()
        if(_battle_status AND _menu_leaves)
            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleWindowSteps.cmake")
            lufia2_battle_window_steps_available(_battle_window_steps)
            if(_battle_window_steps)
                list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_WINDOW_STEPS=1)
            endif()
            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleGaugeSteps.cmake")
            lufia2_battle_gauge_steps_available(_battle_gauge_steps)
            if(_battle_gauge_steps)
                list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_GAUGE_STEPS=1)
            endif()
        endif()
        if(_battle_status AND _menu_leaves)
            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2DivideBitSpans.cmake")
            lufia2_divide_bit_spans_available(_divide_bit_spans)
            if(_divide_bit_spans)
                list(APPEND _definitions LUFIA2_HAVE_NATIVE_DIVIDE_BIT_SPANS=1)
            endif()
        endif()
        if(_battle_queue_steps)
            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2VramQueuePhases.cmake")
            lufia2_vram_queue_phases_available(_vram_queue_phases)
            if(_vram_queue_phases)
                list(APPEND _definitions LUFIA2_HAVE_NATIVE_VRAM_QUEUE_PHASES=1)
            endif()
        endif()
        if(_battle_background_steps)
            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BackgroundWordSpans.cmake")
            lufia2_background_word_spans_available(_background_word_spans)
            if(_background_word_spans)
                list(APPEND _definitions LUFIA2_HAVE_NATIVE_BACKGROUND_WORD_SPANS=1)
            endif()
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2TurnDisplaySteps.cmake")
        lufia2_turn_display_steps_available(_turn_display_steps)
        if(_turn_display_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_TURN_DISPLAY_STEPS=1)
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleFillSteps.cmake")
        lufia2_battle_fill_steps_available(_battle_fill_steps)
        if(_battle_fill_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_FILL_STEPS=1)
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2EffectBackgroundSteps.cmake")
        lufia2_effect_background_steps_available(_effect_background_steps)
        if(_effect_background_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_BACKGROUND_STEPS=1)
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleCommandSteps.cmake")
        lufia2_battle_command_steps_available(_battle_command_steps)
        if(_battle_command_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_COMMAND_LOOP=1)
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleDisplaySteps.cmake")
        lufia2_battle_display_steps_available(_battle_display_steps)
        if(_battle_display_steps)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_DISPLAY_STEPS=1)
        endif()
    endif()
    if(_frame_returns AND "LUFIA2_HAVE_NATIVE_SOUND_SOURCE_CLOCK=1" IN_LIST _definitions AND
       "LUFIA2_HAVE_NATIVE_SOUND_TRANSFER_OPCODES=1" IN_LIST _definitions)
        list(APPEND _definitions LUFIA2_HAVE_NATIVE_SOUND_PAYLOAD_PAIR=1)
    endif()
    set(_sound_clock_barriers FALSE)
    if(LUFIA2_ENABLE_SOUND_CLOCK_BARRIERS AND
       "LUFIA2_HAVE_NATIVE_SOUND_PAYLOAD_PAIR=1" IN_LIST _definitions)
        if(NOT SNESRECOMP_PLATFORM_APU_CPU_SPANS)
            message(FATAL_ERROR "Sound clock barriers require native APU CPU spans")
        endif()
        set(_apu_program_api "${CMAKE_SOURCE_DIR}/lib/snesrecomp-platform/include/snesrecomp_platform/apu_program.h")
        set(_native_spc "${CMAKE_SOURCE_DIR}/src/lufia2_spc_native.c")
        file(READ "${_apu_program_api}" _clock_api)
        file(READ "${_native_spc}" _clock_owner)
        if(NOT _clock_api MATCHES "snesrecomp_apu_cpu_span_open[(]" OR
           NOT _clock_owner MATCHES "Lufia2SpcNativeCpuSpanReady[(]")
            message(FATAL_ERROR "Sound clock barriers require the reviewed APU owner queries")
        endif()
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
            "${_apu_program_api}" "${_native_spc}")
        set(_sound_clock_barriers TRUE)
        list(APPEND _definitions LUFIA2_HAVE_SOUND_CLOCK_BARRIERS=1)
    endif()
    if(_frame_returns AND "LUFIA2_HAVE_NATIVE_SOUND_UPLOAD_STEPS=1" IN_LIST _definitions)
        include("${CMAKE_SOURCE_DIR}/cmake/Lufia2WorldFrameLoop.cmake")
        lufia2_world_frame_loop_available(_world_frame_loop)
        if(_world_frame_loop)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_WORLD_FRAME_LOOP=1)
            lufia2_world_frame_tail_available(_world_frame_tail)
            if(_world_frame_tail)
                list(APPEND _definitions LUFIA2_HAVE_NATIVE_WORLD_FRAME_TAIL=1)
            endif()
        endif()
    endif()
    set(_field_region_continuations FALSE)
    if(_frame_returns AND _field_region_phases AND
       "LUFIA2_HAVE_NATIVE_SOUND_UPLOAD_STEPS=1" IN_LIST _definitions)
        include("${CMAKE_SOURCE_DIR}/cmake/Lufia2FieldRegionContinuations.cmake")
        lufia2_field_region_continuations_available(_field_region_continuations)
        if(_field_region_continuations)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_FIELD_REGION_CONTINUATIONS=1)
        endif()
    endif()
    set(_battle_upkeep FALSE)
    if(_frame_returns AND "LUFIA2_HAVE_NATIVE_SOUND_UPLOAD_STEPS=1" IN_LIST _definitions)
        include("${CMAKE_SOURCE_DIR}/cmake/Lufia2BattleUpkeep.cmake")
        lufia2_battle_upkeep_available(_battle_upkeep)
        if(_battle_upkeep)
            list(APPEND _definitions LUFIA2_HAVE_NATIVE_BATTLE_UPKEEP_STEPS=1)
        endif()
    endif()
    if(_field_region_continuations OR _battle_upkeep)
        include("${CMAKE_SOURCE_DIR}/cmake/Lufia2NativeCallOwner.cmake")
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
            if(_boot_controller OR _scene_operation OR _continuation_entries)
                set(_boot_entry_marker "        /* IRQs are sampled between instructions.")
                string(FIND "${_text}" "${_boot_entry_marker}" _boot_entry_offset)
                if(_boot_entry_offset EQUAL -1)
                    message(FATAL_ERROR "Owned entries require the reviewed scheduler boundary")
                endif()
                string(REPLACE "${_boot_entry_marker}"
                    "        if (auto_quiescent && !stop_on_rti && !in.waiting && !in.stopped && !in.nmiWanted && !in.irqWanted && (pc_before == 0x808035u || pc_before == 0x80ba39u || pc_before == 0x83e10fu)) {\n            const int owned = Lufia2DecompRunOwnedEntry(&in, cpu, pc_before);\n            if (owned == 1) continue;\n            if (owned) return owned > 0;\n        }\n${_boot_entry_marker}"
                    _text "${_text}")
                string(PREPEND _text
                    "#include \"interp816.h\"\n#include \"cpu_state.h\"\nstatic int Lufia2DecompRunOwnedEntry(Interp816 *, CpuState *, uint32_t);\n")
                string(APPEND _timing_suffix
                    "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/owned_entry_boundary.inc\"\n")
            endif()
            if(_nmi_call_boundaries)
                set(_original_opcode "        int _cyc = interp816_runOpcode(&in);   /* executes the opcode; pushes/pops frames */")
                string(FIND "${_text}" "${_original_opcode}" _opcode_offset)
                if(_opcode_offset EQUAL -1)
                    message(FATAL_ERROR "Native NMI calls require reviewed opcode execution")
                endif()
                set(_native_opcode "        int _cyc = Lufia2DecompNmiRunCallBoundary(\n            &in, pc_before, op, stop_on_rti, auto_quiescent,\n            trace || dtrace || wlog_state_sync || s_interp_pctrace);\n")
                set(_manifest_calls FALSE)
                set(_aot_call_manifest "${LUFIA2_GENERATED_DIR}/program_manifest.json")
                if(LUFIA2_ENABLE_REVIEWED_AOT AND EXISTS "${_aot_call_manifest}" AND EXISTS "${LUFIA2_ROM}")
                    set(_aot_call_script "${CMAKE_SOURCE_DIR}/scripts/aot_call_sites.py")
                    set(_aot_call_table "${CMAKE_BINARY_DIR}/generated/decomp-nmi/aot_call_site_table.inc")
                    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_aot_call_script}")
                    set(_aot_call_decomp_args)
                    if(LUFIA2_ENABLE_DECOMP AND NOT LUFIA2_DECOMP_REFERENCE_ONLY)
                        list(APPEND _aot_call_decomp_args
                            --decomp-functions "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml"
                            --decomp-coverage "${CMAKE_BINARY_DIR}/generated/decomp_coverage.json")
                        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
                            "${CMAKE_SOURCE_DIR}/scripts/decomp_manifest.py")
                    endif()
                    execute_process(COMMAND "${CMAKE_COMMAND}" -E env "PYTHONDONTWRITEBYTECODE=1"
                        "${Python3_EXECUTABLE}" "${_aot_call_script}"
                        "${_aot_call_manifest}" "${LUFIA2_ROM}" "${_aot_call_table}"
                        ${_aot_call_decomp_args}
                        RESULT_VARIABLE _aot_call_result)
                    if(NOT _aot_call_result EQUAL 0)
                        message(FATAL_ERROR "Generating Lufia II AOT call sites failed")
                    endif()
                    set(_manifest_calls TRUE)
                endif()
                if(_nmi_child_boundaries OR _battle_effect_frame_steps OR _cursor_pixel_spans OR
                   _turn_display_steps OR _save_action_steps OR _battle_upkeep OR
                   _scene_claim_steps OR _battle_status OR _world_frame_control_steps OR
                   _battle_command_steps OR _menu_fade_continuations OR _battle_gauge_steps OR _item_record_steps OR _menu_leaves OR _field_actor_rebuild_steps OR _sound_frame_boundaries OR _manifest_calls)
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunDirectCall(Interp816 *, uint32_t, uint8_t, uint32_t, bool);\nstatic int Lufia2DecompRunDirectCallWithWidths(Interp816 *, uint32_t, uint8_t, uint32_t, bool, bool, bool);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/direct_call_opcode.inc\"\n")
                endif()
                set(_actor_record_return FALSE)
                if("LUFIA2_HAVE_NATIVE_FIELD_MAP_CLOCK=1" IN_LIST _definitions OR
                   "LUFIA2_HAVE_NATIVE_ACTOR_SLOT_TRAVERSAL=1" IN_LIST _definitions)
                    set(_actor_record_return TRUE)
                endif()
                set(_battle_multiply_return FALSE)
                if("LUFIA2_HAVE_NATIVE_BATTLE_MULTIPLY_WINDOW=1" IN_LIST _definitions)
                    set(_battle_multiply_return TRUE)
                endif()
                if(_battle_upkeep OR _turn_display_steps OR _actor_record_return OR _battle_multiply_return OR _battle_gauge_steps OR _battle_command_steps OR _divide_bit_spans OR _item_record_steps OR _world_frame_control_steps OR _sound_frame_boundaries OR _manifest_calls OR (_battle_status AND _menu_leaves))
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunReturn(Interp816 *, uint32_t, uint8_t, bool, bool, bool);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/long_return_opcode.inc\"\n")
                endif()
                set(_control_sites "")
                set(_control_cases "")
                if(_turn_display_steps)
                    list(APPEND _control_sites "pc_before == 0x859decu" "pc_before == 0x859e04u")
                    string(APPEND _control_cases
                        "            case 0x859decu:\n"
                        "                _cyc = Lufia2DecompRunDirectCallWithWidths(&in, pc_before, op, 0x85ecdbu, true, false, false);\n"
                        "                break;\n"
                        "            case 0x859e04u:\n"
                        "                _cyc = Lufia2DecompRunReturn(&in, pc_before, op, true, true, false);\n"
                        "                break;\n")
                endif()
                if(_actor_record_return)
                    list(APPEND _control_sites "pc_before == 0x83ab60u")
                    string(APPEND _control_cases
                        "            case 0x83ab60u:\n"
                        "                _cyc = Lufia2DecompRunReturn(&in, pc_before, op, true, true, true);\n"
                        "                break;\n")
                endif()
                if(_battle_multiply_return)
                    list(APPEND _control_sites "pc_before == 0x808377u")
                    string(APPEND _control_cases
                        "            case 0x808377u:\n"
                        "                _cyc = Lufia2DecompRunReturn(&in, pc_before, op, true, true, false);\n"
                        "                break;\n")
                endif()
                if(_battle_gauge_steps)
                    list(APPEND _control_sites "pc_before == 0x81e318u" "pc_before == 0x81e32au" "pc_before == 0x81e38eu")
                    string(APPEND _control_cases
                        "            case 0x81e318u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x80834cu, true);\n"
                        "                break;\n"
                        "            case 0x81e32au:\n"
                        "                _cyc = Lufia2DecompRunDirectCallWithWidths(&in, pc_before, op, 0x808378u, true, false, false);\n"
                        "                break;\n"
                        "            case 0x81e38eu:\n"
                        "                _cyc = Lufia2DecompRunReturn(&in, pc_before, op, false, true, false);\n"
                        "                break;\n")
                endif()
                if(_menu_fade_continuations)
                    list(APPEND _control_sites "pc_before == 0x868b27u" "pc_before == 0x868b3du")
                    string(APPEND _control_cases
                        "            case 0x868b3du:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x868b55u, true);\n"
                        "                break;\n"
                        "            case 0x868b27u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x868b55u, true);\n"
                        "                break;\n")
                endif()
                if(_divide_bit_spans)
                    list(APPEND _control_sites "pc_before == 0x80844fu")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunDivideFrameOpcode(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/divide_frame_opcodes.inc\"\n")
                    string(APPEND _native_opcode
                        "        if (!_cyc && (pc_before == 0x808378u || pc_before == 0x808379u ||\n"
                        "            pc_before == 0x80837bu || pc_before == 0x80844eu) &&\n"
                        "            !stop_on_rti && !n_stop && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n"
                        "            g_snes && !g_snes->inNmi && !g_snes->inIrq)\n"
                        "            _cyc = Lufia2DecompRunDivideFrameOpcode(&in, pc_before, op);\n")
                    string(APPEND _control_cases
                        "            case 0x80844fu:\n"
                        "                _cyc = Lufia2DecompRunReturn(&in, pc_before, op, true, in.mf, false);\n"
                        "                break;\n")
                endif()
                if(_battle_status AND _menu_leaves)
                    list(APPEND _control_sites "pc_before == 0x81e56fu" "pc_before == 0x81e592u")
                    string(APPEND _control_cases
                        "            case 0x81e56fu:\n"
                        "            case 0x81e592u:\n"
                        "                _cyc = Lufia2DecompRunReturn(&in, pc_before, op, false, false, false);\n"
                        "                break;\n")
                endif()
                if(_item_record_steps)
                    list(APPEND _control_sites "pc_before == 0x81f199u" "pc_before == 0x81f1aau")
                    string(APPEND _control_cases
                        "            case 0x81f199u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x81f291u, true);\n"
                        "                break;\n"
                        "            case 0x81f1aau:\n"
                        "                _cyc = Lufia2DecompRunReturn(&in, pc_before, op, true, false, false);\n"
                        "                break;\n")
                endif()
                if(_field_actor_rebuild_steps)
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunActorLoopWidthOpcode(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/actor_loop_width_opcodes.inc\"\n")
                    string(APPEND _native_opcode
                        "        if (!_cyc && (pc_before == 0x83a7b5u || pc_before == 0x83a7b7u ||\n"
                        "            pc_before == 0x83a7c0u || pc_before == 0x83a7c5u || pc_before == 0x83a7c7u) &&\n"
                        "            !stop_on_rti && !n_stop && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n"
                        "            g_snes && !g_snes->inNmi && !g_snes->inIrq)\n"
                        "            _cyc = Lufia2DecompRunActorLoopWidthOpcode(&in, pc_before, op);\n")
                    list(APPEND _control_sites
                        "pc_before == 0x83a783u" "pc_before == 0x83a78eu" "pc_before == 0x83a7aeu" "pc_before == 0x83a7b1u")
                    string(APPEND _control_cases
                        "            case 0x83a783u:\n"
                        "                _cyc = Lufia2DecompRunDirectCallWithWidths(&in, pc_before, op, 0x83ab4fu, true, true, true);\n"
                        "                break;\n"
                        "            case 0x83a78eu:\n"
                        "                _cyc = Lufia2DecompRunDirectCallWithWidths(&in, pc_before, op, 0x83a9bau, true, true, true);\n"
                        "                break;\n"
                        "            case 0x83a7aeu:\n"
                        "                _cyc = Lufia2DecompRunDirectCallWithWidths(&in, pc_before, op, 0x83aae5u, false, true, true);\n"
                        "                break;\n"
                        "            case 0x83a7b1u:\n"
                        "                _cyc = Lufia2DecompRunDirectCallWithWidths(&in, pc_before, op, 0x848193u, true, false, true);\n"
                        "                break;\n")
                endif()
                if(_menu_leaves)
                    list(APPEND _control_sites
                        "pc_before == 0x828b06u" "pc_before == 0x82bd7eu" "pc_before == 0x82eb9cu")
                    string(APPEND _control_cases
                        "            case 0x828b06u:\n"
                        "                _cyc = Lufia2DecompRunReturn(&in, pc_before, op, false, true, false);\n"
                        "                break;\n"
                        "            case 0x82eb9cu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x828b08u, false);\n"
                        "                break;\n"
                        "            case 0x82bd7eu:\n"
                        "                _cyc = Lufia2DecompRunDirectCallWithWidths(&in, pc_before, op, 0x81f194u, true, false, false);\n"
                        "                break;\n")
                endif()
                if(_battle_status AND _menu_leaves)
                    list(APPEND _control_sites
                        "pc_before == 0x81e524u" "pc_before == 0x81e53bu" "pc_before == 0x81e541u" "pc_before == 0x81e834u")
                    string(APPEND _control_cases
                        "            case 0x81e524u:\n"
                        "                _cyc = Lufia2DecompRunDirectCallWithWidths(&in, pc_before, op, 0x81e542u, false, false, false);\n"
                        "                break;\n"
                        "            case 0x81e53bu:\n"
                        "                _cyc = Lufia2DecompRunDirectCallWithWidths(&in, pc_before, op, 0x81e542u, false, false, false);\n"
                        "                break;\n"
                        "            case 0x81e541u:\n"
                        "                _cyc = Lufia2DecompRunReturn(&in, pc_before, op, false, true, false);\n"
                        "                break;\n"
                        "            case 0x81e834u:\n"
                        "                _cyc = Lufia2DecompRunReturn(&in, pc_before, op, false, true, false);\n"
                        "                break;\n")
                endif()
                if(_battle_command_steps)
                    list(APPEND _control_sites
                        "pc_before == 0x818b36u")
                    string(APPEND _control_cases
                        "            case 0x818b36u:\n"
                        "                _cyc = Lufia2DecompRunDirectCallWithWidths(&in, pc_before, op, 0x859bdau, true, false, false);\n"
                        "                break;\n")
                endif()
                if(_sound_frame_boundaries)
                    list(APPEND _control_sites "pc_before == 0x80972cu"
                        "pc_before == 0x8098a4u" "pc_before == 0x8099aeu" "pc_before == 0x8099b1u")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunSoundTableLoopOpcode(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/sound_table_loop_opcodes.inc\"\n")
                    string(APPEND _native_opcode
                        "        if (!_cyc && (pc_before == 0x80972bu || (pc_before >= 0x80972fu && pc_before <= 0x809732u)) &&\n"
                        "            !stop_on_rti && !n_stop && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n"
                        "            g_snes && !g_snes->inNmi && !g_snes->inIrq)\n"
                        "            _cyc = Lufia2DecompRunSoundTableLoopOpcode(&in, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunSoundCpuReturnOpcode(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/sound_cpu_return_opcodes.inc\"\n")
                    string(APPEND _native_opcode
                        "        if (!_cyc && pc_before >= 0x8098a0u && pc_before <= 0x8098a3u &&\n"
                        "            !stop_on_rti && !n_stop && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n"
                        "            g_snes && !g_snes->inNmi && !g_snes->inIrq)\n"
                        "            _cyc = Lufia2DecompRunSoundCpuReturnOpcode(&in, pc_before, op);\n")
                    string(APPEND _control_cases
                        "            case 0x80972cu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x809886u, false);\n"
                        "                break;\n"
                        "            case 0x8099aeu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x809a0au, false);\n"
                        "                break;\n"
                        "            case 0x8098a4u:\n"
                        "            case 0x8099b1u:\n"
                        "                _cyc = Lufia2DecompRunReturn(&in, pc_before, op, false, true, false);\n"
                        "                break;\n")
                endif()
                if(_save_action_steps)
                    list(APPEND _control_sites "pc_before == 0x82e9dfu" "pc_before == 0x82ea3du" "pc_before == 0x82ea54u" "pc_before == 0x82ea7eu")
                    string(APPEND _control_cases
                        "            case 0x82e9dfu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x828b08u, false);\n"
                        "                break;\n"
                        "            case 0x82ea3du:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x828b08u, false);\n"
                        "                break;\n"
                        "            case 0x82ea54u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x82f3ddu, false);\n"
                        "                break;\n"
                        "            case 0x82ea7eu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x82f3fcu, false);\n"
                        "                break;\n")
                endif()
                if(_battle_upkeep)
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunBattleShiftLoopOpcode(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/battle_shift_loop_opcodes.inc\"\n")
                    string(APPEND _native_opcode
                        "        if (!_cyc && (pc_before == 0x85ce08u || pc_before == 0x85ce09u || pc_before == 0x85ce0cu) &&\n"
                        "            !stop_on_rti && !n_stop && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n"
                        "            g_snes && !g_snes->inNmi && !g_snes->inIrq)\n"
                        "            _cyc = Lufia2DecompRunBattleShiftLoopOpcode(&in, pc_before, op);\n")
                    list(APPEND _control_sites "pc_before == 0x81b5c6u" "pc_before == 0x81b5e3u" "pc_before == 0x81b5f6u" "pc_before == 0x81b609u" "pc_before == 0x81b61cu" "pc_before == 0x81b62fu" "pc_before == 0x81b642u" "pc_before == 0x81b656u" "pc_before == 0x81b669u" "pc_before == 0x81b67cu")
                    string(APPEND _control_cases
                        "            case 0x81b5c6u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x81b5a3u, false);\n"
                        "                break;\n"
                        "            case 0x81b5e3u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x81b705u, false);\n"
                        "                break;\n"
                        "            case 0x81b5f6u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x81b705u, false);\n"
                        "                break;\n"
                        "            case 0x81b609u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x81b705u, false);\n"
                        "                break;\n"
                        "            case 0x81b61cu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x81b705u, false);\n"
                        "                break;\n"
                        "            case 0x81b62fu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x81b705u, false);\n"
                        "                break;\n"
                        "            case 0x81b642u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x81b705u, false);\n"
                        "                break;\n"
                        "            case 0x81b656u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x81b705u, false);\n"
                        "                break;\n"
                        "            case 0x81b669u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x81b705u, false);\n"
                        "                break;\n"
                        "            case 0x81b67cu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x81b705u, false);\n"
                        "                break;\n")
                    string(APPEND _native_opcode
                        "        if (!_cyc && pc_before >= 0x858a6cu && pc_before <= 0x858a95u && (is_call || is_ret) &&\n"
                        "            !stop_on_rti && !n_stop && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n"
                        "            g_snes && !g_snes->inNmi && !g_snes->inIrq) {\n"
                        "            switch (pc_before) {\n"
                        "            case 0x858a6cu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x858b4bu, true);\n"
                        "                break;\n"
                        "            case 0x858a70u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x858bc0u, true);\n"
                        "                break;\n"
                        "            case 0x858a74u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x858c27u, true);\n"
                        "                break;\n"
                        "            case 0x858a7du:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x858d2eu, true);\n"
                        "                break;\n"
                        "            case 0x858a95u:\n"
                        "                _cyc = Lufia2DecompRunReturn(&in, pc_before, op, true, true, false);\n"
                        "                break;\n"
                        "            }\n"
                        "        }\n")
                    list(APPEND _control_sites "pc_before == 0x81a6d5u")
                    string(APPEND _control_cases
                        "            case 0x81a6d5u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x85dd63u, true);\n"
                        "                break;\n")
                    list(APPEND _control_sites "pc_before == 0x858d08u" "pc_before == 0x858db0u" "pc_before == 0x8588f1u")
                    string(APPEND _control_cases
                        "            case 0x858d08u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x81bdc8u, true);\n"
                        "                break;\n"
                        "            case 0x858db0u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x81be54u, true);\n"
                        "                break;\n"
                        "            case 0x8588f1u:\n"
                        "                _cyc = Lufia2DecompRunReturn(&in, pc_before, op, true, true, false);\n"
                        "                break;\n")
                    list(APPEND _control_sites "pc_before == 0x858ba4u")
                    string(APPEND _control_cases
                        "            case 0x858ba4u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x81bd47u, true);\n"
                        "                break;\n")
                    list(APPEND _control_sites "pc_before == 0x85ddb0u" "pc_before == 0x85ddcbu" "pc_before == 0x85dde6u" "pc_before == 0x85de01u" "pc_before == 0x81a10eu" "pc_before == 0x81a6a9u")
                    string(APPEND _control_cases
                        "            case 0x85ddb0u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x85de2au, true);\n"
                        "                break;\n"
                        "            case 0x85ddcbu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x80834cu, true);\n"
                        "                break;\n"
                        "            case 0x85dde6u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x85de1eu, true);\n"
                        "                break;\n"
                        "            case 0x85de01u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x80834cu, true);\n"
                        "                break;\n"
                        "            case 0x81a10eu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x85dd63u, true);\n"
                        "                break;\n"
                        "            case 0x81a6a9u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x85dd63u, true);\n"
                        "                break;\n")
                    list(APPEND _control_sites "pc_before == 0x85ec81u" "pc_before == 0x85ec8bu")
                    string(APPEND _control_cases
                        "            case 0x85ec81u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x85919cu, true);\n"
                        "                break;\n"
                        "            case 0x85ec8bu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x81b5c4u, true);\n"
                        "                break;\n")
                endif()
                if(_scene_claim_steps)
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunSceneLoopOpcode(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/scene_loop_opcodes.inc\"\n")
                    string(APPEND _native_opcode
                        "        if (!_cyc && (pc_before == 0x83a68cu || pc_before == 0x83a6c4u || pc_before == 0x83a6c5u) &&\n"
                        "            auto_quiescent && !stop_on_rti && !n_stop && !trace && !dtrace &&\n"
                        "            !wlog_state_sync && !s_interp_pctrace && !g_interp_bridge_pc_hook &&\n"
                        "            !Lufia2NativeObserversActive() && g_snes && !g_snes->inNmi && !g_snes->inIrq)\n"
                        "            _cyc = Lufia2DecompRunSceneLoopOpcode(&in, pc_before, op);\n")
                    list(APPEND _control_sites "pc_before == 0x83a68eu" "pc_before == 0x83a6c1u")
                    string(APPEND _control_cases
                        "            case 0x83a68eu:\n"
                        "                _cyc = Lufia2DecompRunDirectCallWithWidths(&in, pc_before, op, 0x83ab4fu, true, true, true);\n"
                        "                break;\n"
                        "            case 0x83a6c1u:\n"
                        "                _cyc = Lufia2DecompRunDirectCallWithWidths(&in, pc_before, op, 0x83a6dfu, false, true, true);\n"
                        "                break;\n")
                endif()
                if(_cursor_pixel_spans)
                    list(APPEND _control_sites "pc_before == 0x828a7eu" "pc_before == 0x828aafu")
                    string(APPEND _control_cases
                        "            case 0x828a7eu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x828ad8u, false);\n"
                        "                break;\n"
                        "            case 0x828aafu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x828ae9u, false);\n"
                        "                break;\n")
                endif()
                if(_world_frame_control_steps)
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunWorldSceneGateOpcode(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/world_scene_gate_opcodes.inc\"\n")
                    string(APPEND _native_opcode
                        "        if (!_cyc && (pc_before == 0x869e17u || pc_before == 0x869e19u ||\n"
                        "            pc_before == 0x869e1cu || pc_before == 0x869e21u ||\n"
                        "            pc_before == 0x869e24u || pc_before == 0x869e38u) &&\n"
                        "            !stop_on_rti && !n_stop && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n"
                        "            g_snes && !g_snes->inNmi && !g_snes->inIrq)\n"
                        "            _cyc = Lufia2DecompRunWorldSceneGateOpcode(&in, pc_before, op);\n")
                    list(APPEND _control_sites
                        "pc_before == 0x869276u" "pc_before == 0x869279u" "pc_before == 0x86927cu"
                        "pc_before == 0x86927fu" "pc_before == 0x869285u" "pc_before == 0x869290u"
                        "pc_before == 0x869293u" "pc_before == 0x869296u" "pc_before == 0x869299u"
                        "pc_before == 0x869e1eu" "pc_before == 0x869e26u" "pc_before == 0x869e29u"
                        "pc_before == 0x869e2cu" "pc_before == 0x869e2fu" "pc_before == 0x869e32u"
                        "pc_before == 0x869e35u" "pc_before == 0x869e3au")
                    string(APPEND _control_cases
                        "            case 0x869276u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x869e4fu, false);\n"
                        "                break;\n"
                        "            case 0x869279u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x869c71u, false);\n"
                        "                break;\n"
                        "            case 0x86927cu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x869adbu, false);\n"
                        "                break;\n"
                        "            case 0x86927fu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x869758u, false);\n"
                        "                break;\n"
                        "            case 0x869285u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x8699bfu, false);\n"
                        "                break;\n"
                        "            case 0x869290u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x86a7ceu, false);\n"
                        "                break;\n"
                        "            case 0x869293u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x86a791u, false);\n"
                        "                break;\n"
                        "            case 0x869296u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x86e1b9u, false);\n"
                        "                break;\n"
                        "            case 0x869299u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x86973eu, false);\n"
                        "                break;\n"
                        "            case 0x869e1eu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x86d37du, false);\n"
                        "                break;\n"
                        "            case 0x869e26u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x8694d4u, false);\n"
                        "                break;\n"
                        "            case 0x869e29u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x86995bu, false);\n"
                        "                break;\n"
                        "            case 0x869e2cu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x86a7ceu, false);\n"
                        "                break;\n"
                        "            case 0x869e2fu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x86a791u, false);\n"
                        "                break;\n"
                        "            case 0x869e32u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x86973eu, false);\n"
                        "                break;\n"
                        "            case 0x869e35u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x86974fu, false);\n"
                        "                break;\n"
                        "            case 0x869e3au:\n"
                        "                _cyc = Lufia2DecompRunReturn(&in, pc_before, op, false, true, false);\n"
                        "                break;\n")
                endif()
                if(_battle_status)
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunBattleStatusBranch(Interp816 *, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/battle_status_branch_opcode.inc\"\n")
                    list(APPEND _control_sites "pc_before == 0x81e532u" "pc_before == 0x81e535u")
                    string(APPEND _control_cases
                        "            case 0x81e532u:\n"
                        "                _cyc = Lufia2DecompRunDirectCallWithWidths(&in, pc_before, op, 0x81e570u, false, false, false);\n"
                        "                break;\n"
                        "            case 0x81e535u:\n"
                        "                _cyc = Lufia2DecompRunBattleStatusBranch(&in, op);\n"
                        "                break;\n")
                endif()
                if(_battle_command_steps)
                    list(APPEND _control_sites "pc_before == 0x81cbddu" "pc_before == 0x81cbe0u"
                        "pc_before == 0x81cbe4u" "pc_before == 0x81cbedu" "pc_before == 0x818b45u"
                        "pc_before == 0x859bf0u" "pc_before == 0x859c07u" "pc_before == 0x859c1eu")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunBattleDisplayOpcode(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/battle_display_opcodes.inc\"\n")
                    string(APPEND _native_opcode
                        "        if (!_cyc && ((pc_before >= 0x859bdeu && pc_before <= 0x859bedu) ||\n"
                        "            (pc_before >= 0x859bf5u && pc_before <= 0x859c04u) ||\n"
                        "            (pc_before >= 0x859c0cu && pc_before <= 0x859c1bu)) &&\n"
                        "            !stop_on_rti && !n_stop && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n"
                        "            g_snes && !g_snes->inNmi && !g_snes->inIrq)\n"
                        "            _cyc = Lufia2DecompRunBattleDisplayOpcode(&in, pc_before, op);\n")
                    string(APPEND _control_cases
                        "            case 0x81cbddu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x81be58u, false);\n"
                        "                break;\n"
                        "            case 0x81cbe0u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x859dd4u, true);\n"
                        "                break;\n"
                        "            case 0x81cbe4u:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x81d9d0u, false);\n"
                        "                break;\n"
                        "            case 0x81cbedu:\n"
                        "                _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x85ec81u, true);\n"
                        "                break;\n"
                        "            case 0x818b45u:\n"
                        "                _cyc = Lufia2DecompRunDirectCallWithWidths(&in, pc_before, op, 0x859bf1u, true, false, false);\n"
                        "                break;\n"
                        "            case 0x859bf0u:\n"
                        "            case 0x859c07u:\n"
                        "            case 0x859c1eu:\n"
                        "                _cyc = Lufia2DecompRunReturn(&in, pc_before, op, true, false, false);\n"
                        "                break;\n")
                endif()
                if(_actor_scene_presentation_steps)
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunDisplayRegisterLoopOpcode(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/display_register_loop_opcodes.inc\"\n")
                    string(APPEND _native_opcode
                        "        if (!_cyc && pc_before >= 0x818633u && pc_before <= 0x818640u &&\n"
                        "            !stop_on_rti && !n_stop && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n"
                        "            g_snes && !g_snes->inNmi && !g_snes->inIrq)\n"
                        "            _cyc = Lufia2DecompRunDisplayRegisterLoopOpcode(&in, pc_before, op);\n")
                endif()
                set(_battle_width_sites "")
                if(_battle_command_steps)
                    list(APPEND _battle_width_sites "pc_before == 0x818b43u" "pc_before == 0x818b49u")
                endif()
                if(_battle_gauge_steps)
                    list(APPEND _battle_width_sites "pc_before == 0x81e320u")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunGaugeDataOpcode(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/gauge_data_opcodes.inc\"\n")
                    string(APPEND _native_opcode
                        "        if (!_cyc && (pc_before == 0x81e31cu || pc_before == 0x81e31eu ||\n"
                        "            pc_before == 0x81e31fu || pc_before == 0x81e322u ||\n"
                        "            pc_before == 0x81e324u || pc_before == 0x81e326u ||\n"
                        "            pc_before == 0x81e328u || pc_before == 0x81e32eu ||\n"
                        "            pc_before == 0x81e331u ||\n"
                        "            pc_before == 0x81e333u || pc_before == 0x81e335u || pc_before == 0x81e337u) &&\n"
                        "            !stop_on_rti && !n_stop && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n"
                        "            g_snes && !g_snes->inNmi && !g_snes->inIrq)\n"
                        "            _cyc = Lufia2DecompRunGaugeDataOpcode(&in, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunGaugeLabelOpcode(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/gauge_label_opcodes.inc\"\n")
                    string(APPEND _native_opcode
                        "        if (!_cyc && pc_before >= 0x81e2afu && pc_before <= 0x81e2c6u &&\n"
                        "            !stop_on_rti && !n_stop && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n"
                        "            g_snes && !g_snes->inNmi && !g_snes->inIrq)\n"
                        "            _cyc = Lufia2DecompRunGaugeLabelOpcode(&in, pc_before, op);\n")
                endif()
                if(_battle_width_sites)
                    list(JOIN _battle_width_sites " || " _battle_width_guard)
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunBattleWidthOpcode(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/battle_width_opcode.inc\"\n")
                    string(APPEND _native_opcode
                        "        if (!_cyc && (${_battle_width_guard}) &&\n"
                        "            !stop_on_rti && !n_stop && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n"
                        "            g_snes && !g_snes->inNmi && !g_snes->inIrq)\n"
                        "            _cyc = Lufia2DecompRunBattleWidthOpcode(&in, pc_before, op);\n")
                endif()
                set(_menu_opcode_sites "")
                if(_battle_command_steps)
                    list(APPEND _menu_opcode_sites "pc_before == 0x81cb96u" "pc_before == 0x81cbe7u"
                        "pc_before == 0x81cbe9u" "pc_before == 0x81cce2u" "pc_before == 0x81cce4u")
                endif()
                if(_save_action_steps)
                    list(APPEND _menu_opcode_sites "pc_before == 0x82ea4cu" "pc_before == 0x82ea4eu"
                        "pc_before == 0x82ea81u")
                endif()
                if(_menu_leaves)
                    list(APPEND _menu_opcode_sites
                        "(pc_before >= 0x82eb9fu && pc_before <= 0x82ebadu && (pc_before & 1u))")
                endif()
                if(_menu_opcode_sites)
                    list(JOIN _menu_opcode_sites " || " _menu_opcode_guard)
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunMenuOpcode(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/command_menu_opcodes.inc\"\n")
                    string(APPEND _native_opcode
                        "        if (!_cyc && (${_menu_opcode_guard}) &&\n"
                        "            !stop_on_rti && !n_stop && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n"
                        "            g_snes && !g_snes->inNmi && !g_snes->inIrq)\n"
                        "            _cyc = Lufia2DecompRunMenuOpcode(&in, pc_before, op);\n")
                endif()
                if(_control_sites)
                    list(JOIN _control_sites " || " _control_guard)
                    string(APPEND _native_opcode
                        "        if (!_cyc && (is_call || is_ret || op == 0x80u) && (${_control_guard}) &&\n"
                        "            !stop_on_rti && !n_stop && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n"
                        "            g_snes && !g_snes->inNmi && !g_snes->inIrq) {\n"
                        "            switch (pc_before) {\n"
                        "${_control_cases}"
                        "            }\n"
                        "        }\n")
                endif()
                if(_nmi_child_boundaries)
                    string(APPEND _native_opcode
                        "        if (!_cyc && pc_before == 0x8087d7u && stop_on_rti &&\n"
                        "            !auto_quiescent && !n_stop && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n"
                        "            g_snes && g_snes->inNmi && !g_snes->inIrq)\n"
                        "            _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x80884fu, false);\n")
                endif()
                if(_battle_target_name_setup)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_BATTLE_TARGET_NAME_SETUP 1\n")
                endif()
                if(_nmi_upload_control_steps)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_NMI_UPLOAD_CONTROL_STEPS 1\n")
                endif()
                if(_nmi_control_steps)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_NMI_CONTROL_STEPS 1\n")
                endif()
                if(_battle_target_control_steps)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_BATTLE_TARGET_CONTROL_STEPS 1\n")
                endif()
                if(_battle_control_steps)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_BATTLE_CONTROL_STEPS 1\n")
                endif()
                if(_field_region_control_steps)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_FIELD_REGION_CONTROL_STEPS 1\n")
                endif()
                if(_field_control_steps)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_FIELD_CONTROL_STEPS 1\n")
                endif()
                if(_world_frame_control_steps)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_WORLD_FRAME_CONTROL_STEPS 1\n")
                endif()
                if(_frame_exit_steps)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_FRAME_EXIT_STEPS 1\n")
                endif()
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
                if(_battle_effect_command_steps OR _battle_effect_frame_steps)
                    string(APPEND _native_opcode
                        "        if (!_cyc && (pc_before & 0xfffe00u) == 0x818a00u) {\n")
                endif()
                if(_battle_effect_command_steps)
                    string(APPEND _native_opcode
                        "            if (op == 0xfcu || op == 0x80u || op == 0xabu)\n"
                        "                _cyc = Lufia2DecompRunEffectCommandStep(&in, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunEffectCommandStep(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/effect_command_calls.inc\"\n")
                endif()
                if(_battle_effect_frame_steps)
                    string(APPEND _native_opcode
                        "            if ((op == 0x20u || op == 0x22u) &&\n"
                        "                (pc_before & 0xffff00u) == 0x818b00u &&\n"
                        "                !n_stop && !stop_on_rti && !trace && !dtrace && !wlog_state_sync &&\n"
                        "                !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive())\n"
                        "                _cyc = Lufia2DecompRunEffectDirectCall(&in, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunEffectDirectCall(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/effect_direct_calls.inc\"\n")
                endif()
                if(_battle_effect_command_steps OR _battle_effect_frame_steps)
                    string(APPEND _native_opcode "        }\n")
                endif()
                if(_battle_command_steps)
                    string(APPEND _native_opcode
                        "        if (!_cyc && pc_before == 0x81cbafu &&\n"
                        "            !n_stop && !stop_on_rti && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive())\n"
                        "            _cyc = Lufia2DecompRunCommandTileCall(&in, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunCommandTileCall(Interp816 *, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/command_tile_call.inc\"\n")
                endif()
                if(_actor_slot_calls)
                    string(APPEND _native_opcode "        if (!_cyc && (pc_before & 0xffff00u) == 0x83bb00u && (op == 0x20u || op == 0x80u || op == 0x6bu))\n            _cyc = Lufia2DecompRunActorSlotCall(&in, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunActorSlotCall(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/actor_slot_calls.inc\"\n")
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
                    string(APPEND _native_opcode
                        "        if (!_cyc && (pc_before == 0x828a98u || pc_before == 0x828ad3u) &&\n"
                        "            auto_quiescent && !stop_on_rti && !n_stop && !trace && !dtrace &&\n"
                        "            !wlog_state_sync && !s_interp_pctrace && !g_interp_bridge_pc_hook &&\n"
                        "            !Lufia2NativeObserversActive() && g_snes && !g_snes->inNmi && !g_snes->inIrq)\n"
                        "            _cyc = Lufia2DecompRunDirectCall(&in, pc_before, op, 0x828afau, false);\n")
                    string(APPEND _native_opcode "        if (!_cyc && !n_stop && (!yield_pc || auto_quiescent) &&\n            (!g_snes || (!g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n             (!g_snes->nmiEnabled || s_lle_master_deadline)))) {\n            _cyc = Lufia2DecompRunCursorPixelSpan(&in, cpu, pc_before, op, Lufia2SpanAccountInstruction);\n            _lufia2_span_accounted = _cyc != 0;\n        }\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\n#include \"cpu_state.h\"\nstatic void Lufia2SpanAccountInstruction(CpuState *, unsigned);\nstatic int Lufia2DecompRunCursorPixelSpan(Interp816 *, CpuState *, uint32_t, uint8_t, void (*)(CpuState *, unsigned));\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/cursor_pixel_spans.inc\"\n")
                endif()
                if(_cursor_pixel_spans OR _manifest_calls)
                    string(PREPEND _native_opcode "        int _lufia2_span_accounted = 0;\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/span_instruction_clock.inc\"\n")
                    set(_clock_block "        {\n            unsigned _internal = (unsigned)_cyc > s_interp_bus_cycles")
                    string(FIND "${_text}" "${_clock_block}" _clock_offset)
                    if(_clock_offset EQUAL -1)
                        message(FATAL_ERROR "Native spans require the reviewed instruction clock")
                    endif()
                    string(REPLACE "${_clock_block}"
                        "        if (!_lufia2_span_accounted) {\n            unsigned _internal = (unsigned)_cyc > s_interp_bus_cycles"
                        _text "${_text}")
                endif()
                if(_scene_rebuild_spans)
                    string(APPEND _native_opcode "        if (!_cyc && !n_stop && (!yield_pc || auto_quiescent))\n            _cyc = Lufia2DecompRunSceneRebuildSpan(&in, cpu, pc_before, op);\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\n#include \"cpu_state.h\"\nstatic int Lufia2DecompRunSceneRebuildSpan(Interp816 *, CpuState *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/scene_rebuild_spans.inc\"\n")
                endif()
                if(_manifest_calls)
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic int Lufia2DecompRunEffectSlotCursorOpcode(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _native_opcode
                        "        if (!_cyc && ((pc_before >= 0x818aaau && pc_before < 0x818ab4u) ||\n"
                        "            (pc_before >= 0x818b1bu && pc_before < 0x818b25u)) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq)\n"
                        "            _cyc = Lufia2DecompRunEffectSlotCursorOpcode(&in, pc_before, op);\n")
                    # The RAM copy and HDMA command dispatcher need per-instruction bus timing.
                    set(_paired_body "            if (bounce_ok && has_body) {")
                    string(FIND "${_text}" "${_paired_body}" _paired_body_offset)
                    if(_paired_body_offset EQUAL -1)
                        message(FATAL_ERROR "Timed RAM and HDMA execution requires the reviewed AOT bounce")
                    endif()
                    string(REPLACE "${_paired_body}"
                        "            if (bounce_ok && has_body && !Lufia2DecompNeedsTimedCall(pc_before, target, &in)) {"
                        _text "${_text}")
                    string(PREPEND _text
                        "#include \"interp816.h\"\nstatic bool Lufia2DecompIsImmediateDataOpcode(uint8_t);\nstatic bool Lufia2DecompIsMemoryOpcode(uint8_t);\nstatic bool Lufia2DecompIsRegisterOrStackOpcode(uint8_t);\nstatic bool Lufia2DecompIsNmiRomOpcode(uint8_t);\nstatic bool Lufia2DecompNeedsTimedCall(uint32_t, uint32_t, const Interp816 *);\nstatic int Lufia2DecompRunNativeRomControl(Interp816 *, uint32_t, uint8_t);\nstatic int Lufia2DecompRunAdditionalRomOpcode(Interp816 *, uint32_t, uint8_t);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${_aot_call_table}\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/aot_call_sites.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/rom_immediate_opcode.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/effect_slot_cursor_opcode.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/sprite_oam_rotate_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/sprite_oam_clear_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/battle_sprite_tile_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/sprite_slot_scan_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/wram_port_fill_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/field_metatile_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/rom_wram_move_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/wave_hdma_fill_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/actor_menu_clear_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/battle_hdma_call_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/item_record_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/world_menu_idle_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/menu_string_read_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/sprite_piece_offsets_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/sprite_piece_position_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/sprite_piece_tile_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/sprite_piece_finish_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/menu_glyph_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/menu_glyph_variants_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/menu_decimal_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/menu_buttons_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/battle_palette_copy_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/menu_multiply_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/battle_oam_pack_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/battle_color_scale_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/render_prep_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/menu_control_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/menu_branch_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/battle_vector_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/battle_effect_yield_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/battle_status_icon_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/battle_party_tilemap_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/render_buffer_fill_span.inc\"\n"
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/field_attribute_span.inc\"\n")
                    string(PREPEND _text
                        "#include \"interp816.h\"\n#include \"cpu_state.h\"\nstatic int Lufia2DecompRunSpriteOamRotateSpan(Interp816 *, CpuState *, uint32_t, uint8_t, bool);\nstatic int Lufia2DecompRunSpriteOamClearSpan(Interp816 *, CpuState *, uint32_t, uint8_t, bool);\ntypedef struct SpriteSlotScanResult {\n    unsigned cycles;\n    unsigned instructions;\n    bool progress;\n} SpriteSlotScanResult;\n\nstatic SpriteSlotScanResult Lufia2DecompRunSpriteSlotScan(Interp816 *, CpuState *, uint32_t, uint8_t, unsigned, bool, uint64_t);\nstatic int Lufia2DecompRunSpriteSlotAdvance(Interp816 *, CpuState *, uint32_t, uint8_t);\nstatic int Lufia2DecompRunWramPortFill(Interp816 *, CpuState *, uint32_t, uint8_t);\nstatic int Lufia2DecompRunWaveHdmaFill(Interp816 *, CpuState *, uint32_t, uint8_t, bool, unsigned, long *);\nstatic int Lufia2DecompRunWaveFrame(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleHdmaCallFrame(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuStringCallFrame(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuPaletteApply(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuPaletteDispatch(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleOamHideBoundaries(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuTileBlockStart(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuTileBlockRow(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuSlotPaletteSetup(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuSlotPaletteAdvance(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuWindowEdgeBoundaries(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuTileRecolor(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuWindowEdgeTile(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuRecolorRow(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuRecolorColumns(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuRecolorSetup(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuRecolorRedraw(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuSlideTick(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunFieldOamReset(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleTilemapClear(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuSlideRestart(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattlePalettePortCopy(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleTilePair(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleTilePairAdvance(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunGlyphBufferFill(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleGaugeFill(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuLayerTransfer(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleBackgroundTileFill(Interp816 *, CpuState *, uint32_t, uint8_t, long *);\nstatic int Lufia2DecompRunBattleTileBlockRow(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleTileBlockColumn(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleTileBlockBoundaries(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleTilemapClearFrame(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleEffectYield(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleEffectDispatchReturn(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBackwardByteCopy(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleRecordSearch(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattlePaletteResourceCopy(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBrightnessCommandRead(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleMessageTick(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleEffectLoopStart(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleStatusIcon(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBrightnessCommandClamp(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBrightnessCommandApply(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunPaletteBrightnessFrame(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunPaletteBrightnessLevel(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleSpriteEdges(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleSpriteFrameBegin(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuLayerFrame(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleSpriteGroup(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleSpriteOverlayGroup(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleSpriteFrameExit(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuCommandIndex(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuCommandJump(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleSineEntry(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleSineQuadrant(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleSineTable(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleVelocityEntry(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleVelocityAxisBranch(Interp816 *, CpuState *, uint32_t, uint8_t, long *);\nstatic int Lufia2DecompRunBattleVelocityAxisStore(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleVelocityFactor(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleVelocityMultiplySetup(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleVelocityMultiplyResult(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunSceneGlyphEntry(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunSceneGlyphSelect(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunSceneGlyphSetup(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunSceneGlyphRow(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunSceneGlyphReturn(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuCursorFromStream(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattlePartyTilemapSlot(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattlePartyTilemapAttributes(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleEffectStreamParameter(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleEffectPackedFields(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleEffectSetMode(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleEffectDisplacementSetup(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleEffectPosition(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleTilemapReturn(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleTilemapSetup(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleSpriteSelection(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleSingleSpriteStart(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattlePartyGridSetup(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleTilemapFixedStyle(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleTilemapSelect(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunSpriteOverlayVisit(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunSpriteOverlayAssign(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleTilemapCount(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleTilemapLayout(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleTilemapLayoutStyle(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattlePartyTilemapRecord(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattlePartyTilemapMultiply(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleEffectRepeat(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleEffectStreamWord(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuParameterWord(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuNewlineDispatch(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuClearRectangleFrame(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuTileRow(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuTileIteration(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunSpriteAnimationSlotSeed(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleSpriteTileFar(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleSpriteTileSetup(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleSpriteTileFinish(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleSpriteTileRecord(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunBattleSpriteTileAdvance(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunSpriteAnimationFramePointer(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunSpriteAnimationMarker(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunSpriteAnimationFinish(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunSpriteAnimateBoundary(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunSpriteOamClearBoundary(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunSpriteOamBuildBoundary(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunSpriteFrameCall(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunWorldMenuIdle(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunItemRecord(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunActorMenuClear(Interp816 *, CpuState *, uint32_t, uint8_t);\nstatic int Lufia2DecompRunMenuStringRead(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuStringStart(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuStringFinish(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunSpritePieceOffsets(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t);\nstatic int Lufia2DecompRunSpritePiecePosition(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunSpritePieceTile(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t);\nstatic int Lufia2DecompRunSpritePieceHighWord(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t);\nstatic int Lufia2DecompRunSpritePieceFinish(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunMenuGlyph(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunMenuGlyphCall(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuGlyphVariant(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuNumberEntry(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunMenuNumberStream(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunMenuPresentationBoundary(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunMenuButtons(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunMenuNumberContinuation(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunMenuDigitsSetup(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunMenuDecimal(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunMenuNumberRead(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t);\nstatic int Lufia2DecompRunMenuDecimalTrim(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuConditionalCommand(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunMenuBranch(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunMenuComparison(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunMenuDecimalOutput(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunBattlePaletteCopy(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long *);\nstatic int Lufia2DecompRunMenuMultiply(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunBattleOamPack(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunBattleColorScale(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunBattleWramPort(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunForwardBlockMove(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunFieldMetatile(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunSpritePlaneClear(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunMenuNumberSign(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunRenderPrep(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunMenuControl(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\nstatic int Lufia2DecompRunMenuTileClear(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t);\nstatic int Lufia2DecompRunMenuTileBlock(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t);\nstatic int Lufia2DecompRunOamHide(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t);\nstatic int Lufia2DecompRunFieldAttribute(Interp816 *, CpuState *, uint32_t, uint8_t, uint64_t, uint64_t, long, long *);\n")
                    string(APPEND _native_opcode
                        "        if (!_cyc && (pc_before >> 16) == 0x85u && ((pc_before >= 0x85ae74u && pc_before <= 0x85ae92u) || pc_before == 0x85ae68u || pc_before == 0x85ade1u || pc_before == 0x85ae94u || pc_before == 0x85aeb0u || pc_before == 0x85a76du) && !n_stop &&\n"
                        "            !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inIrq && !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n"
                        "            ((stop_on_rti && g_snes->inNmi) || (auto_quiescent && !stop_on_rti &&\n"
                        "              !g_snes->inNmi && (!g_snes->nmiEnabled || s_lle_master_deadline)))) {\n"
                        "            long tail_steps = steps;\n"
                        "            const uint64_t instruction_start = g_interp_bridge_steps;\n"
                        "            _cyc = pc_before >= 0x85ae74u && pc_before <= 0x85ae92u\n"
                        "                ? Lufia2DecompRunWaveHdmaFill(&in, cpu, pc_before, op, auto_quiescent,\n"
                        "                    (unsigned)(step_cap - steps), &tail_steps)\n"
                        "                : step_cap - steps >= (pc_before == 0x85aeb0u || pc_before == 0x85a76du ? 1 : pc_before == 0x85ae94u ? 14 : 7)\n"
                        "                ? Lufia2DecompRunWaveFrame(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                    progress_write_epoch, &tail_steps) : 0;\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                if (pc_before >= 0x85ae74u && pc_before <= 0x85ae92u)\n"
                        "                    steps = tail_steps;\n"
                        "                else\n"
                        "                    steps = auto_quiescent ? (tail_steps < 0 ? steps - tail_steps : tail_steps)\n"
                        "                        : steps + (long)(g_interp_bridge_steps - instruction_start) - 1;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x868c3bu && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) &&\n"
                        "            step_cap - steps >= 6) {\n"
                        "            _cyc = Lufia2DecompRunSpritePieceOffsets(&in, cpu, pc_before, op, progress_dynamic_epoch);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = 3;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x868c45u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) &&\n"
                        "            step_cap - steps >= 15) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSpritePiecePosition(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x868c60u || pc_before == 0x868c70u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) &&\n"
                        "            step_cap - steps >= (pc_before == 0x868c60u ? 10 : 11)) {\n"
                        "            _cyc = pc_before == 0x868c60u\n"
                        "                ? Lufia2DecompRunSpritePieceTile(&in, cpu, pc_before, op, progress_dynamic_epoch, progress_write_epoch)\n"
                        "                : Lufia2DecompRunSpritePieceHighWord(&in, cpu, pc_before, op, progress_dynamic_epoch, progress_write_epoch);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = 0;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc &&\n"
                        "            (pc_before == 0x868c84u || pc_before == 0x868c94u || pc_before == 0x868ca4u ||\n"
                        "             pc_before == 0x868cb5u || pc_before == 0x868cc4u || pc_before == 0x868ccdu) &&\n"
                        "            auto_quiescent && !n_stop && !stop_on_rti && !trace && !dtrace &&\n"
                        "            !wlog_state_sync && !s_interp_pctrace && !g_interp_bridge_pc_hook &&\n"
                        "            !Lufia2NativeObserversActive() && g_snes && !g_snes->inNmi && !g_snes->inIrq &&\n"
                        "            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n"
                        "            (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSpritePieceFinish(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x808db3u || pc_before == 0x808dbeu) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) &&\n"
                        "            step_cap - steps >= 6) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuGlyph(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc &&\n"
                        "            (pc_before == 0x808a01u || pc_before == 0x808a17u ||\n"
                        "             pc_before == 0x808a31u || pc_before == 0x808a4cu) &&\n"
                        "            auto_quiescent && !n_stop && !stop_on_rti && !trace && !dtrace &&\n"
                        "            !wlog_state_sync && !s_interp_pctrace && !g_interp_bridge_pc_hook &&\n"
                        "            !Lufia2NativeObserversActive() && g_snes && !g_snes->inNmi && !g_snes->inIrq &&\n"
                        "            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n"
                        "            (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuDecimal(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x808dcau || pc_before == 0x808dd0u || pc_before == 0x808de4u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) &&\n"
                        "            step_cap - steps >= (pc_before == 0x808dcau ? 3 : pc_before == 0x808dd0u ? 12 : 8)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuGlyphVariant(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc &&\n"
                        "            (pc_before == 0x8088ddu || pc_before == 0x8089fau || pc_before == 0x868c1fu) &&\n"
                        "            auto_quiescent && !n_stop && !stop_on_rti && !trace && !dtrace &&\n"
                        "            !wlog_state_sync && !s_interp_pctrace && !g_interp_bridge_pc_hook &&\n"
                        "            !Lufia2NativeObserversActive() && g_snes && !g_snes->inNmi && !g_snes->inIrq &&\n"
                        "            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n"
                        "            (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunRenderPrep(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc &&\n"
                        "            (pc_before == 0x8088b9u || pc_before == 0x8088bfu || pc_before == 0x8088c8u || pc_before == 0x8088ceu ||\n"
                        "             pc_before == 0x8088d5u || pc_before == 0x8088d7u || pc_before == 0x808bcau ||\n"
                        "             pc_before == 0x808bdcu || pc_before == 0x808bedu || pc_before == 0x868b87u) &&\n"
                        "            auto_quiescent && !n_stop && !stop_on_rti && !trace && !dtrace &&\n"
                        "            !wlog_state_sync && !s_interp_pctrace && !g_interp_bridge_pc_hook &&\n"
                        "            !Lufia2NativeObserversActive() && g_snes && !g_snes->inNmi && !g_snes->inIrq &&\n"
                        "            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n"
                        "            (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuControl(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x8280b3u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) &&\n"
                        "            step_cap - steps >= 6) {\n"
                        "            _cyc = Lufia2DecompRunMenuTileBlock(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = 5;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81b5a3u || pc_before == 0x81b5b2u || pc_before == 0x81b5c1u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x81b5a3u ? 3 : 2)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleOamHideBoundaries(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81b5abu || pc_before == 0x81b5b8u || pc_before == 0x82839du) &&\n"
                        "            auto_quiescent && !n_stop && !stop_on_rti && !trace && !dtrace &&\n"
                        "            !wlog_state_sync && !s_interp_pctrace && !g_interp_bridge_pc_hook &&\n"
                        "            !Lufia2NativeObserversActive() && g_snes && !g_snes->inNmi && !g_snes->inIrq &&\n"
                        "            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n"
                        "            (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 6) {\n"
                        "            _cyc = pc_before != 0x82839du\n"
                        "                ? Lufia2DecompRunOamHide(&in, cpu, pc_before, op, progress_dynamic_epoch, progress_write_epoch)\n"
                        "                : Lufia2DecompRunMenuTileClear(&in, cpu, pc_before, op, progress_dynamic_epoch, progress_write_epoch);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = pc_before == 0x81b5b8u ? 5 : pc_before == 0x81b5abu ? 3 : 4;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc &&\n"
                        "            (pc_before == 0x80edeau || pc_before == 0x80edffu ||\n"
                        "             pc_before == 0x80ee05u || pc_before == 0x80ee17u ||\n"
                        "             pc_before == 0x80ee18u || pc_before == 0x80ee2cu) &&\n"
                        "            auto_quiescent && !n_stop && !stop_on_rti && !trace && !dtrace &&\n"
                        "            !wlog_state_sync && !s_interp_pctrace && !g_interp_bridge_pc_hook &&\n"
                        "            !Lufia2NativeObserversActive() && g_snes && !g_snes->inNmi && !g_snes->inIrq &&\n"
                        "            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n"
                        "            (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunFieldAttribute(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x808c7bu || pc_before == 0x808956u || pc_before == 0x808953u) &&\n"
                        "            auto_quiescent && !n_stop && !stop_on_rti && !trace && !dtrace &&\n"
                        "            !wlog_state_sync && !s_interp_pctrace && !g_interp_bridge_pc_hook &&\n"
                        "            !Lufia2NativeObserversActive() && g_snes && !g_snes->inNmi && !g_snes->inIrq &&\n"
                        "            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n"
                        "            (!g_snes->nmiEnabled || s_lle_master_deadline) &&\n"
                        "            step_cap - steps >= (pc_before == 0x808c7bu ? 7 : pc_before == 0x808953u ? 9 : 8)) {\n"
                        "            long tail_steps = 4;\n"
                        "            _cyc = pc_before == 0x808c7bu\n"
                        "                ? Lufia2DecompRunMenuNumberRead(&in, cpu, pc_before, op, progress_dynamic_epoch, progress_write_epoch)\n"
                        "                : Lufia2DecompRunMenuDecimalTrim(&in, cpu, pc_before, op, progress_dynamic_epoch, progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x80888cu || pc_before == 0x808897u || pc_before == 0x8088e9u) &&\n"
                        "            auto_quiescent && !n_stop && !stop_on_rti && !trace && !dtrace &&\n"
                        "            !wlog_state_sync && !s_interp_pctrace && !g_interp_bridge_pc_hook &&\n"
                        "            !Lufia2NativeObserversActive() && g_snes && !g_snes->inNmi && !g_snes->inIrq &&\n"
                        "            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n"
                        "            (!g_snes->nmiEnabled || s_lle_master_deadline) &&\n"
                        "            step_cap - steps >= (pc_before == 0x80888cu ? 7 : pc_before == 0x808897u ? 9 : 2)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = pc_before == 0x8088e9u\n"
                        "                ? Lufia2DecompRunMenuStringFinish(&in, cpu, pc_before, op, progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps)\n"
                        "                : Lufia2DecompRunMenuStringStart(&in, cpu, pc_before, op, progress_dynamic_epoch, progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x808bfdu || pc_before == 0x808c86u || pc_before == 0x808c90u) &&\n"
                        "            auto_quiescent && !n_stop && !stop_on_rti && !trace && !dtrace &&\n"
                        "            !wlog_state_sync && !s_interp_pctrace && !g_interp_bridge_pc_hook &&\n"
                        "            !Lufia2NativeObserversActive() && g_snes && !g_snes->inNmi && !g_snes->inIrq &&\n"
                        "            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n"
                        "            (!g_snes->nmiEnabled || s_lle_master_deadline) &&\n"
                        "            step_cap - steps >= (pc_before == 0x808bfdu ? 12 : pc_before == 0x808c86u ? 5 : 6)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = pc_before == 0x808bfdu\n"
                        "                ? Lufia2DecompRunMenuConditionalCommand(&in, cpu, pc_before, op, progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps)\n"
                        "                : Lufia2DecompRunMenuNumberSign(&in, cpu, pc_before, op, progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc &&\n"
                        "            (pc_before == 0x808c9eu || pc_before == 0x808cb9u || pc_before == 0x808cc6u ||\n"
                        "             pc_before == 0x808cddu || pc_before == 0x808cf0u) &&\n"
                        "            auto_quiescent && !n_stop && !stop_on_rti && !trace && !dtrace &&\n"
                        "            !wlog_state_sync && !s_interp_pctrace && !g_interp_bridge_pc_hook &&\n"
                        "            !Lufia2NativeObserversActive() && g_snes && !g_snes->inNmi && !g_snes->inIrq &&\n"
                        "            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n"
                        "            (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuBranch(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc &&\n"
                        "            (pc_before == 0x808968u || pc_before == 0x80896fu || pc_before == 0x808979u ||\n"
                        "             pc_before == 0x80897eu || pc_before == 0x808988u || pc_before == 0x808991u ||\n"
                        "             pc_before == 0x80899fu) &&\n"
                        "            auto_quiescent && !n_stop && !stop_on_rti && !trace && !dtrace &&\n"
                        "            !wlog_state_sync && !s_interp_pctrace && !g_interp_bridge_pc_hook &&\n"
                        "            !Lufia2NativeObserversActive() && g_snes && !g_snes->inNmi && !g_snes->inIrq &&\n"
                        "            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n"
                        "            (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuDecimalOutput(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x81b9b7u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) &&\n"
                        "            step_cap - steps >= 4) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattlePaletteCopy(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x828000u || pc_before == 0x82800du || pc_before == 0x828025u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuMultiply(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc &&\n"
                        "            (pc_before == 0x81b705u || pc_before == 0x81b715u || pc_before == 0x81b747u ||\n"
                        "             pc_before == 0x81b75au || pc_before == 0x81b76fu || pc_before == 0x81b780u ||\n"
                        "             pc_before == 0x81b78eu || pc_before == 0x81b79du || pc_before == 0x81b7aeu ||\n"
                        "             pc_before == 0x81b7b6u || pc_before == 0x81b7bau || pc_before == 0x81b7c4u) &&\n"
                        "            auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleOamPack(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc &&\n"
                        "            (pc_before == 0x81b3b1u || pc_before == 0x81b3beu || pc_before == 0x81b3d5u ||\n"
                        "             pc_before == 0x81b3e4u || pc_before == 0x81b3f8u || pc_before == 0x81b414u ||\n"
                        "             pc_before == 0x81b42du) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleColorScale(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc &&\n"
                        "            (pc_before == 0x81bb75u || pc_before == 0x81bb90u || pc_before == 0x81bba7u ||\n"
                        "             pc_before == 0x81bbaeu || pc_before == 0x81bbbau || pc_before == 0x81bbc4u ||\n"
                        "             pc_before == 0x81bbd4u || pc_before == 0x81bbdeu) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleWramPort(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc &&\n"
                        "            (pc_before == 0x80f351u || pc_before == 0x869150u || pc_before == 0x82d038u || pc_before == 0x82d04fu || pc_before == 0x8690ccu || pc_before == 0x8690dfu || pc_before == 0x8690f2u || pc_before == 0x869105u || pc_before == 0x869118u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunForwardBlockMove(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc &&\n"
                        "            (pc_before == 0x80f354u || pc_before == 0x80f6dfu ||\n"
                        "             pc_before == 0x80f6eeu || pc_before == 0x80f6fau ||\n"
                        "             pc_before == 0x80f715u || pc_before == 0x80f72fu) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunFieldMetatile(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc &&\n"
                        "            (pc_before == 0x868e45u || pc_before == 0x868e71u) &&\n"
                        "            auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSpritePlaneClear(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc &&\n"
                        "            (pc_before == 0x808c1cu || pc_before == 0x808c23u || pc_before == 0x808c2fu ||\n"
                        "             pc_before == 0x808c39u || pc_before == 0x808c41u || pc_before == 0x808c51u ||\n"
                        "             pc_before == 0x808c5eu || pc_before == 0x808c6cu) &&\n"
                        "            auto_quiescent && !n_stop && !stop_on_rti && !trace && !dtrace &&\n"
                        "            !wlog_state_sync && !s_interp_pctrace && !g_interp_bridge_pc_hook &&\n"
                        "            !Lufia2NativeObserversActive() && g_snes && !g_snes->inNmi && !g_snes->inIrq &&\n"
                        "            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n"
                        "            (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuComparison(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x8088b4u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) &&\n"
                        "            step_cap - steps >= 2) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuGlyphCall(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x8088adu || pc_before == 0x8088aau) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) &&\n"
                        "            step_cap - steps >= (pc_before == 0x8088aau ? 5 : 4)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuStringRead(&in, cpu, pc_before, op,\n"
                        "                progress_dynamic_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x858d5bu || pc_before == 0x858db5u || pc_before == 0x858c3bu || pc_before == 0x858c82u || pc_before == 0x858b67u || pc_before == 0x858bb0u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x858d5bu || pc_before == 0x858c3bu || pc_before == 0x858b67u ? 2 : 8)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattlePartyTilemapSlot(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x858d60u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 17) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattlePartyTilemapRecord(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x858d80u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 12) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattlePartyTilemapMultiply(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x858d9cu && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 10) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattlePartyTilemapAttributes(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81a69au || pc_before == 0x81a0f4u || pc_before == 0x81a6c6u || (pc_before >= 0x81a03au && pc_before <= 0x81a061u)) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x81a04du || pc_before == 0x81a061u ? 1 : pc_before == 0x81a03au || pc_before == 0x81a04eu ? 10 : 8)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleEffectDisplacementSetup(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81a112u || pc_before == 0x81a6adu || pc_before == 0x81a6d9u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x81a112u ? 10 : 11)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleEffectPosition(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x858b6cu || pc_before == 0x858bd7u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 16) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleTilemapLayout(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x858b8du && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 10) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleTilemapLayoutStyle(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x81a100u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 7) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleEffectStreamParameter(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x81a13au && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 19) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleEffectPackedFields(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before >= 0x81a605u && pc_before <= 0x81a62du) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 6) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleEffectSetMode(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && ((pc_before == 0x858ba8u)) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 4) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleTilemapCount(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x858bbeu || pc_before == 0x858c90u || pc_before == 0x858c96u || pc_before == 0x858dc3u || pc_before == 0x858c22u || pc_before == 0x858c25u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 4) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleTilemapReturn(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && ((pc_before >= 0x858a78u && pc_before <= 0x858a81u) || (pc_before >= 0x858b4bu && pc_before <= 0x858b62u) || (pc_before >= 0x858bcdu && pc_before <= 0x858bd4u) || pc_before == 0x858bc0u || pc_before == 0x858c27u || pc_before == 0x858d2eu || (pc_before >= 0x858d4bu && pc_before <= 0x858d50u)) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= ((pc_before == 0x858b4bu || pc_before == 0x858bc0u || pc_before == 0x858c27u) ? 10 : pc_before == 0x858d2eu ? 9 : pc_before >= 0x858d4bu && pc_before <= 0x858d50u ? 8 : 4)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = (pc_before == 0x858b4bu || pc_before == 0x858bc0u || pc_before == 0x858c27u)\n"
                        "                ? Lufia2DecompRunBattleTilemapSetup(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps)\n"
                        "                : pc_before >= 0x858bcdu && pc_before <= 0x858bd4u\n"
                        "                ? Lufia2DecompRunBattleSingleSpriteStart(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps)\n"
                        "                : pc_before == 0x858d2eu || (pc_before >= 0x858d4bu && pc_before <= 0x858d50u)\n"
                        "                ? Lufia2DecompRunBattlePartyGridSetup(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps)\n"
                        "                : Lufia2DecompRunBattleSpriteSelection(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x858bf7u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 12) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleTilemapFixedStyle(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x858c12u || pc_before == 0x858c1cu) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 3) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleTilemapSelect(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81b685u || pc_before == 0x81b68bu) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 6) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSpriteOverlayAssign(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81b698u || pc_before == 0x81b6a5u || pc_before == 0x81b6f6u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 5) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSpriteOverlayVisit(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81b5c4u || pc_before == 0x81b67fu) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 1) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleSpriteEdges(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x85cf21u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 4) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBackwardByteCopy(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x818f94u || pc_before == 0x818f99u || pc_before == 0x818faeu || pc_before == 0x818fb3u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 10) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleRecordSearch(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x81914au && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 6) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattlePaletteResourceCopy(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x819853u || pc_before == 0x819935u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 10) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBrightnessCommandRead(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x8194ebu || pc_before == 0x819500u || pc_before == 0x819515u || pc_before == 0x81952au) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 10) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleEffectLoopStart(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x85885au || pc_before == 0x8588d4u || pc_before == 0x8588f3u || pc_before == 0x8588f0u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 15) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleStatusIcon(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x85b1cdu && !n_stop &&\n"
                        "            !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && step_cap - steps >= 1 &&\n"
                        "            ((auto_quiescent && !stop_on_rti && !g_snes->inNmi &&\n"
                        "              (!g_snes->nmiEnabled || s_lle_master_deadline)) ||\n"
                        "             (!yield_pc && stop_on_rti && g_snes->inNmi && !s_lle_master_deadline))) {\n"
                        "            long tail_steps;\n"
                        "            const uint64_t instruction_start = g_interp_bridge_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleEffectDispatchReturn(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = auto_quiescent ? (tail_steps < 0 ? steps - tail_steps : tail_steps)\n"
                        "                    : steps + (long)(g_interp_bridge_steps - instruction_start) - 1;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x85ab28u || pc_before == 0x85ab2au || pc_before == 0x85ab2fu || pc_before == 0x85ab34u || pc_before == 0x85ab53u || pc_before == 0x85ab52u || pc_before == 0x85ab5au) && !n_stop &&\n"
                        "            !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && step_cap - steps >= 11 &&\n"
                        "            ((auto_quiescent && !stop_on_rti && !g_snes->inNmi &&\n"
                        "              (!g_snes->nmiEnabled || s_lle_master_deadline)) ||\n"
                        "             (!yield_pc && stop_on_rti && g_snes->inNmi && !s_lle_master_deadline))) {\n"
                        "            long tail_steps;\n"
                        "            const uint64_t instruction_start = g_interp_bridge_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleMessageTick(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = auto_quiescent ? (tail_steps < 0 ? steps - tail_steps : tail_steps)\n"
                        "                    : steps + (long)(g_interp_bridge_steps - instruction_start) - 1;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x819947u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 11) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBrightnessCommandClamp(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x819865u || pc_before == 0x81995fu) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 3) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBrightnessCommandApply(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81b396u || pc_before == 0x81b3f3u || pc_before == 0x81b3c9u || pc_before == 0x81b3d3u || pc_before == 0x81b3f1u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x81b396u ? 11 : pc_before == 0x81b3f3u ? 2 : 1)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunPaletteBrightnessFrame(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81b3a7u || pc_before == 0x81b3abu || pc_before == 0x81b3cbu) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 5) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunPaletteBrightnessLevel(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x81f1ceu && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            _cyc = Lufia2DecompRunActorMenuClear(&in, cpu, pc_before, op);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = 2;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x869adbu && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 8) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunWorldMenuIdle(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x868bcfu || pc_before == 0x868be7u || pc_before == 0x868bf2u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x868bcfu ? 4 : pc_before == 0x868be7u ? 1 : 2)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSpriteOamClearBoundary(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x868b73u || pc_before == 0x868b84u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 2) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSpriteAnimateBoundary(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x868b8du || pc_before == 0x868ba1u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x868b8du ? 11 : 3)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSpriteAnimationSlotSeed(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81bd47u || pc_before == 0x81bd4au || pc_before == 0x81bdc8u || pc_before == 0x81bdcbu || pc_before == 0x81be54u || pc_before == 0x81be57u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 1) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleSpriteTileFar(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81bd4bu || pc_before == 0x81bd70u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x81bd4bu ? 25 : 2)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleSpriteTileSetup(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81bdb3u || pc_before == 0x81bdc2u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x81bdb3u ? 8 : 4)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleSpriteTileFinish(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x81bd74u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 16) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleSpriteTileRecord(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x81bd94u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 16) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleSpriteTileAdvance(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x868ba6u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 7) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSpriteAnimationFramePointer(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x868bb5u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 5) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSpriteAnimationMarker(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x868bbfu || pc_before == 0x868bc3u || pc_before == 0x868bc8u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x868bc8u ? 3 : 2)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSpriteAnimationFinish(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x808d04u || pc_before == 0x808d07u || pc_before == 0x808d0au) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 1) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuNewlineDispatch(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81953fu || pc_before == 0x819553u || pc_before == 0x819567u || pc_before == 0x81957bu) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 10) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleEffectRepeat(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x81a12au && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 9) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleEffectStreamWord(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x808ba7u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 6) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuParameterWord(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x80c7c2u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 8) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSceneGlyphEntry(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x80c7d1u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 7) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSceneGlyphSelect(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x80c7e2u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 14) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSceneGlyphSetup(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x80c7f8u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 9) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSceneGlyphRow(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x80c80bu || pc_before == 0x80c812u || pc_before == 0x80c814u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x80c80bu ? 5 : pc_before == 0x80c812u ? 2 : 1)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSceneGlyphReturn(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x808d0du && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 8) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuCursorFromStream(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x85dd63u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 4) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleVelocityEntry(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x85dd6au && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 8) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleVelocityAxisBranch(&in, cpu, pc_before, op, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x85dd78u || pc_before == 0x85dd88u || pc_before == 0x85dd98u || pc_before == 0x85dda4u || pc_before == 0x85dd87u || pc_before == 0x85dd97u || pc_before == 0x85dda3u || pc_before == 0x85ddafu || pc_before == 0x85de1cu || pc_before == 0x85de1du) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (op == 0x6bu || op == 0x7au ? 1 : pc_before == 0x85dd78u || pc_before == 0x85dd88u ? 8 : 6)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleVelocityAxisStore(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x85ddb4u || pc_before == 0x85ddeau) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 6) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleVelocityFactor(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x85ddc0u || pc_before == 0x85ddf6u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 7) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleVelocityMultiplySetup(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x85ddcfu || pc_before == 0x85de05u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 10) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleVelocityMultiplyResult(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x85de1eu || pc_before == 0x85de2au) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x85de1eu ? 7 : 4)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleSineEntry(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x85de31u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 10) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleSineQuadrant(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x85de55u || pc_before == 0x85de61u || pc_before == 0x85de62u || pc_before == 0x85de6bu) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x85de55u ? 5 : pc_before == 0x85de62u ? 4 : 1)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleSineTable(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81b5d3u || pc_before == 0x81b5e6u || pc_before == 0x81b5f9u || pc_before == 0x81b60cu || pc_before == 0x81b61fu || pc_before == 0x81b632u || pc_before == 0x81b659u || pc_before == 0x81b66cu) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 6) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleSpriteGroup(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x81b645u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 7) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleSpriteOverlayGroup(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81b700u || pc_before == 0x81b704u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x81b700u ? 2 : 1)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleSpriteFrameExit(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x808a6cu && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 10) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuCommandIndex(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x808a7eu && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 1) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuCommandJump(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81915bu || pc_before == 0x819169u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x81915bu ? 7 : 10)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleEffectYield(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x818c58u || pc_before == 0x818c59u || pc_before == 0x819552u || pc_before == 0x819566u || pc_before == 0x81957au || pc_before == 0x81958eu || pc_before == 0x81a128u || pc_before == 0x81a161u || pc_before == 0x81a6c5u || pc_before == 0x818faau || pc_before == 0x818fc4u || pc_before == 0x81b3f7u || pc_before == 0x81986cu || pc_before == 0x81986du || pc_before == 0x819966u || pc_before == 0x819967u || pc_before == 0x8194ffu || pc_before == 0x819514u || pc_before == 0x819529u || pc_before == 0x81953eu || pc_before == 0x858db4u || pc_before == 0x8588f2u || pc_before == 0x81a6f1u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 1) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleEffectDispatchReturn(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x81b5c9u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 6) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleSpriteFrameBegin(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x828345u || pc_before == 0x82836au || pc_before == 0x828366u || pc_before == 0x82838bu) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x828345u || pc_before == 0x82836au ? 8 : 3)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuLayerFrame(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x81bea5u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 11) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleTileBlockRow(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x81be70u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 5) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleTileBlockColumn(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81be58u || pc_before == 0x81bebau) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x81be58u ? 14 : 2)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleTileBlockBoundaries(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81c2fbu || pc_before == 0x81c30eu || pc_before == 0x81c30du || pc_before == 0x81c320u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x81c2fbu || pc_before == 0x81c30eu ? 4 : 1)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleTilemapClearFrame(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x81e5e7u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 40) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleGaugeFill(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x80c7afu && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 7) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunGlyphBufferFill(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x828353u || pc_before == 0x828378u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 8) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuLayerTransfer(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x85a915u && stop_on_rti && !n_stop && in.i &&\n"
                        "            !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace && !g_interp_bridge_pc_hook &&\n"
                        "            !Lufia2NativeObserversActive() && g_snes && g_snes->inNmi && !g_snes->inIrq &&\n"
                        "            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled && step_cap - steps >= 10) {\n"
                        "            long extra_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleBackgroundTileFill(&in, cpu, pc_before, op, &extra_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                steps += extra_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x828afeu && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 3) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuSlideRestart(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x858ac2u || pc_before == 0x858ae0u || pc_before == 0x858b37u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 5) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattlePalettePortCopy(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x81be79u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 12) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleTilePair(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x81be90u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 11) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleTilePairAdvance(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x828afau && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 2) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuSlideTick(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x83ac8cu || pc_before == 0x83aca1u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x83ac8cu ? 9 : 5)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunFieldOamReset(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81c307u || pc_before == 0x81c31au) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 3) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleTilemapClear(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x8280f6u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 6) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuRecolorRow(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x8280e0u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 2) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuRecolorColumns(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x8280cau && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 14) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuRecolorSetup(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x828102u || pc_before == 0x82810bu) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x828102u ? 4 : 2)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuRecolorRedraw(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x8281e6u || pc_before == 0x82820au) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x8281e6u ? 9 : 1)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuWindowEdgeBoundaries(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x8280e4u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 8) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuTileRecolor(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x8281f9u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 9) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuWindowEdgeTile(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x86911fu || pc_before == 0x869121u || pc_before == 0x869130u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x86911fu ? 7 : pc_before == 0x869121u ? 6 : 17)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuSlotPaletteSetup(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x869153u || pc_before == 0x869162u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x869153u ? 8 : 2)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuSlotPaletteAdvance(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x8280a5u && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 5) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuTileBlockStart(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x8280b0u || pc_before == 0x8280bdu || pc_before == 0x8280c9u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x8280bdu ? 8 : pc_before == 0x8280b0u ? 2 : 1)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuTileBlockRow(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x808878u || pc_before == 0x808889u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x808878u ? 8 : 3)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuStringCallFrame(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && pc_before == 0x808d5du && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 9) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuPaletteApply(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x808d22u || pc_before == 0x808d28u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x808d22u ? 3 : 1)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuPaletteDispatch(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x82813eu || pc_before == 0x828142u || pc_before == 0x828408u || pc_before == 0x82840bu) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x82813eu || pc_before == 0x828408u ? 5 : 4)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuTileIteration(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x828135u || pc_before == 0x828147u || pc_before == 0x828404u || pc_before == 0x828410u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x828404u ? 2 : pc_before == 0x828135u ? 5 : 6)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuTileRow(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x8283ebu || pc_before == 0x82841cu) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x8283ebu ? 15 : 2)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuClearRectangleFrame(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x868bf5u || pc_before == 0x868c0du || pc_before == 0x868c1eu) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x868bf5u ? 4 : 1)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSpriteOamBuildBoundary(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x868b5cu || pc_before == 0x868b60u || pc_before == 0x868b64u || pc_before == 0x868b68u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x868b68u ? 3 : 1)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunSpriteFrameCall(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x8192dfu || pc_before == 0x8192e7u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= (pc_before == 0x8192dfu ? 4 : 2)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunBattleHdmaCallFrame(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x8089aau || pc_before == 0x8089d8u || pc_before == 0x8089f4u || pc_before == 0x808a00u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuDigitsSetup(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x8088eeu || pc_before == 0x808903u || pc_before == 0x80890cu) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuNumberStream(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x80892fu || pc_before == 0x8089a6u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuNumberContinuation(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x808df9u || pc_before == 0x808e0fu || pc_before == 0x82c627u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuPresentationBoundary(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x828b4bu || pc_before == 0x828c35u ||\n"
                        "             (pc_before >= 0x828b51u && pc_before < 0x828c35u && (pc_before - 0x828b51u) % 19u == 0)) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuButtons(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x808922u || pc_before == 0x808926u || pc_before == 0x808949u ||\n"
                        "             pc_before == 0x80894cu || pc_before == 0x8089a3u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunMenuNumberEntry(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x81f1c5u || pc_before == 0x81f1d7u || pc_before == 0x81f1dbu || pc_before == 0x81f1e3u ||\n"
                        "             pc_before == 0x81f1e9u || pc_before == 0x81f1f6u || pc_before == 0x81f28eu || pc_before == 0x81f2c2u ||\n"
                        "             pc_before == 0x81f291u || pc_before == 0x81f2a9u || pc_before == 0x81f2b0u ||\n"
                        "             pc_before == 0x81f2d1u || pc_before == 0x81f2d6u || pc_before == 0x81f2e5u ||\n"
                        "             pc_before == 0x81f212u || pc_before == 0x81f231u || pc_before == 0x81f250u || pc_before == 0x81f26fu ||\n"
                        "             pc_before == 0x81f217u || pc_before == 0x81f236u || pc_before == 0x81f255u || pc_before == 0x81f274u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            long tail_steps;\n"
                        "            _cyc = Lufia2DecompRunItemRecord(&in, cpu, pc_before, op, progress_dynamic_epoch,\n"
                        "                progress_write_epoch, step_cap - steps, &tail_steps);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = tail_steps < 0 ? steps - tail_steps : tail_steps;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x858d42u || pc_before == 0x858a51u) && auto_quiescent && !n_stop &&\n"
                        "            !stop_on_rti && !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n"
                        "            !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() && g_snes &&\n"
                        "            !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            _cyc = Lufia2DecompRunWramPortFill(&in, cpu, pc_before, op);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = 2;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc &&\n"
                        "            (pc_before == 0x868b7fu || pc_before == 0x868c07u || pc_before == 0x868c18u) &&\n"
                        "            auto_quiescent && !n_stop && !stop_on_rti && !trace && !dtrace &&\n"
                        "            !wlog_state_sync && !s_interp_pctrace && !g_interp_bridge_pc_hook &&\n"
                        "            !Lufia2NativeObserversActive() && g_snes && !g_snes->inNmi && !g_snes->inIrq &&\n"
                        "            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n"
                        "            (!g_snes->nmiEnabled || s_lle_master_deadline) && step_cap - steps >= 3) {\n"
                        "            _cyc = Lufia2DecompRunSpriteSlotAdvance(&in, cpu, pc_before, op);\n"
                        "            if (_cyc) {\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                steps += 2;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x868b77u || pc_before == 0x868bffu ||\n"
                        "                      pc_before == 0x868c10u) && !n_stop && !stop_on_rti &&\n"
                        "            (!yield_pc || auto_quiescent) && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n"
                        "            g_snes && !g_snes->inNmi && !g_snes->inIrq && !g_snes->hIrqEnabled &&\n"
                        "            !g_snes->vIrqEnabled && (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            const SpriteSlotScanResult span = Lufia2DecompRunSpriteSlotScan(\n"
                        "                &in, cpu, pc_before, op, (unsigned)(step_cap - steps),\n"
                        "                auto_quiescent, progress_dynamic_epoch);\n"
                        "            if (span.instructions) {\n"
                        "                _cyc = (int)span.cycles;\n"
                        "                _lufia2_span_accounted = 1;\n"
                        "                if (span.progress) {\n"
                        "                    steps = 0;\n"
                        "                    progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                    progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                }\n"
                        "                steps += span.instructions - 1u;\n"
                        "            }\n"
                        "        }\n"
                        "        if (!_cyc && (pc_before == 0x868cbfu || pc_before == 0x868bdau ||\n"
                        "                      pc_before == 0x868beau) && !n_stop &&\n"
                        "            (!yield_pc || auto_quiescent) && !stop_on_rti && !trace && !dtrace &&\n"
                        "            !wlog_state_sync && !s_interp_pctrace && !g_interp_bridge_pc_hook &&\n"
                        "            !Lufia2NativeObserversActive() && g_snes && !g_snes->inNmi && !g_snes->inIrq &&\n"
                        "            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&\n"
                        "            (!g_snes->nmiEnabled || s_lle_master_deadline)) {\n"
                        "            _cyc = pc_before == 0x868cbfu\n"
                        "                ? Lufia2DecompRunSpriteOamRotateSpan(&in, cpu, pc_before, op, auto_quiescent)\n"
                        "                : Lufia2DecompRunSpriteOamClearSpan(&in, cpu, pc_before, op, auto_quiescent);\n"
                        "            _lufia2_span_accounted = _cyc != 0;\n"
                        "            if (_cyc && auto_quiescent) {\n"
                        "                progress_write_epoch = g_interp_bridge_write_epoch;\n"
                        "                progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n"
                        "                steps = pc_before == 0x868cbfu ? 1 : 4;\n"
                        "            }\n"
                        "        }\n")
                    string(APPEND _native_opcode
                        "        if (!_cyc && (is_ret || op == 0x20u || op == 0x22u || (op & 0x1fu) == 0x10u || op == 0xc2u || op == 0xe2u ||\n"
                        "                      op == 0x80u || op == 0x82u || op == 0x4cu || op == 0x5cu ||\n"
                        "                      Lufia2DecompIsImmediateDataOpcode(op) || Lufia2DecompIsMemoryOpcode(op) || Lufia2DecompIsRegisterOrStackOpcode(op)) &&\n"
                        "            !n_stop && !trace && !dtrace && !wlog_state_sync &&\n"
                        "            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n"
                        "            g_snes && !g_snes->inIrq && (g_snes->inNmi\n"
                        "                ? Lufia2DecompIsNmiRomOpcode(op) : (!stop_on_rti ||\n                      (pc_before >= 0x8086b8u && pc_before <= 0x8086beu))))\n"
                        "            _cyc = Lufia2DecompRunNativeRomControl(&in, pc_before, op);\n"
                        "        if (!_cyc && !n_stop && !trace && !dtrace && !wlog_state_sync &&\n            !s_interp_pctrace && !g_interp_bridge_pc_hook && !Lufia2NativeObserversActive() &&\n            g_snes && !g_snes->inIrq && (!stop_on_rti || (g_snes->inNmi && op == 0xf4u)))\n            _cyc = Lufia2DecompRunAdditionalRomOpcode(&in, pc_before, op);\n")
                endif()
                string(APPEND _native_opcode "        if (!_cyc)\n            _cyc = interp816_runOpcode(&in);")
                string(REPLACE "${_original_opcode}" "${_native_opcode}"
                    _text "${_text}")
                string(PREPEND _text
                    "#include \"interp816.h\"\nstatic int Lufia2DecompNmiRunCallBoundary(Interp816 *, uint32_t, uint8_t, int, int, int);\n")
                if(_nmi_child_boundaries AND _frame_returns)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_NMI_CHILD_BOUNDARIES 1\n")
                endif()
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
                    "                    s_frame_return_auto_quiescent = auto_quiescent;\n                    {\n                        const int blocked = s_frame_return_interrupt_blocked;\n                        const unsigned old_poll_budget = s_frame_return_poll_opcode_budget;\n                        s_frame_return_interrupt_blocked = in.waiting || in.stopped ||\n                            in.nmiWanted || in.irqWanted;\n                    s_frame_return_batch_allowed = !s_frame_return_interrupt_blocked && auto_quiescent && s_interp_bridge_depth == 1 &&\n                        steps >= 0 && step_cap > steps && step_cap - steps >= 16 &&\n                        !trace && !dtrace && !wlog_state_sync && !s_interp_pctrace &&\n                        !g_interp_bridge_pc_hook && !s_interp_bus_timing_active && s_interp_continuous_read_epoch <= UINT64_MAX - 16u;\n                        s_frame_return_poll_opcode_budget = s_frame_return_batch_allowed ? (unsigned)((step_cap - steps) > LUFIA2_FRAME_POLL_MAX_OPCODES ? LUFIA2_FRAME_POLL_MAX_OPCODES : (step_cap - steps)) : 0u;\n${_hook_call}\n                        s_frame_return_poll_opcode_budget = old_poll_budget;\n                        s_frame_return_interrupt_blocked = blocked;\n                    }"
                    _text "${_text}")
                string(REPLACE "${_redirect}"
                    "            if (redirected) {\n                Lufia2DecompFrameReturnContinue(&steps, auto_quiescent,\n                    &progress_write_epoch, &progress_dynamic_epoch);\n                continue;\n            }"
                    _text "${_text}")
                string(PREPEND _text
                    "#include <stdint.h>\n#include \"decomp_bridge/frame_returns.h\"\nstatic int s_frame_return_auto_quiescent;\nstatic int s_frame_return_batch_allowed;\nstatic unsigned s_frame_return_poll_opcode_budget;\nstatic int s_frame_return_interrupt_blocked = 1;\nstatic void Lufia2DecompFrameReturnContinue(long *, int, uint64_t *, uint64_t *);\n")
                string(APPEND _timing_suffix
                    "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/frame_return_cycles.inc\"\n")
                if(_sound_clock_barriers)
                    string(APPEND _timing_suffix
                        "\n#define LUFIA2_HAVE_SOUND_CLOCK_BARRIERS 1\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/sound_clock_barrier.inc\"\n")
                endif()
                if("LUFIA2_HAVE_NATIVE_OPCODE_WINDOW_CONTEXT=1" IN_LIST _definitions)
                    set(_window_save "                        const unsigned old_poll_budget = s_frame_return_poll_opcode_budget;\n")
                    string(FIND "${_text}" "${_window_save}" _window_index)
                    if(_window_index EQUAL -1)
                        message(FATAL_ERROR "Missing native opcode callback context seam")
                    endif()
                    string(REPLACE "${_window_save}"
                        "${_window_save}                        const int old_native_opcode_window = s_native_opcode_window_ready;\n"
                        _text "${_text}")
                    string(REPLACE "${_hook_call}"
                        "                    s_native_opcode_window_ready = s_frame_return_batch_allowed && !n_stop;\n${_hook_call}\n                    s_native_opcode_window_ready = old_native_opcode_window;"
                        _text "${_text}")
                    string(PREPEND _text
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/native_opcode_window.h\"\nstatic int s_native_opcode_window_ready;\n")
                    string(APPEND _timing_suffix
                        "\nbool Lufia2DecompOpcodeWindowReady(void) { return s_native_opcode_window_ready != 0; }\n")
                endif()
                set(_copy_bounce "                !lle_bounce_target_excluded(target);")
                string(FIND "${_text}" "${_copy_bounce}" _copy_bounce_offset)
                if(_copy_bounce_offset EQUAL -1)
                    message(FATAL_ERROR "Action copy timing requires the reviewed AOT bounce")
                endif()
                string(REPLACE "${_copy_bounce}"
                    "                !lle_bounce_target_excluded(target) &&\n                !FramePairedActionCopyBlocked(target, auto_quiescent,\n                    n_stop || stop_on_rti || trace || dtrace || wlog_state_sync ||\n                    s_interp_pctrace || g_interp_bridge_pc_hook, step_cap - steps);"
                    _text "${_text}")
                set(_paired_begin "                s_lle_next_unwind_is_deadline = 0;")
                set(_paired_end "                s_interp_bounce_owner_depth = _saved_bounce_owner;")
                foreach(_marker IN ITEMS "${_paired_begin}" "${_paired_end}")
                    string(FIND "${_text}" "${_marker}" _paired_offset)
                    if(_paired_offset EQUAL -1)
                        message(FATAL_ERROR "Paired readiness requires the reviewed AOT bounce")
                    endif()
                endforeach()
                string(REPLACE "${_paired_begin}"
                    "                const bool paired_tracked = FramePairedCallTracks(target);\n                FramePairedCallContext paired_previous;\n                uint32_t paired_previous_return_pc;\n                int paired_auto_quiescent;\n                if (paired_tracked) {\n                    paired_previous = s_frame_paired_call;\n                    paired_previous_return_pc = s_frame_paired_return_pc;\n                    s_frame_paired_return_pc =\n                        ((target & 0x7fffffu) == 0x008af7u ||\n                         (target & 0x7fffffu) == 0x009c72u ||\n                         (target & 0x7fffffu) == 0x00c21au ||\n                         (target & 0x7fffffu) == 0x00cbaeu ||\n                         (target & 0x7fffffu) == 0x0380cdu ||\n                         (target & 0x7fffffu) == 0x03b66eu ||\n                         (target & 0x7fffffu) == 0x03b711u ||\n                         (target & 0x7fffffu) == 0x03b747u ||\n                         (target & 0x7fffffu) == 0x03b882u ||\n                         (target & 0x7fffffu) == 0x03ab4fu ||\n                         (target & 0x7fffffu) == 0x03c0efu ||\n                         (target & 0x7fffffu) == 0x03fb71u ||\n${_item_region_return_clause}${_vehicle_terrain_return_clause}                         (target & 0x7fffffu) == 0x03f9d4u ||\n                         (target & 0x7fffffu) == 0x03f9f7u ||\n                         (target & 0x7fffffu) == 0x0381c6u ||\n                         (target & 0x7fffffu) == 0x0383a0u ||\n                         (target & 0x7fffffu) == 0x038103u ||\n                         (target & 0x7fffffu) == 0x048000u ||\n                         (target & 0x7fffffu) == 0x02f4d7u ||\n                         (target & 0x7fffffu) == 0x02a00fu ||\n                         (target & 0x7fffffu) == 0x02f46cu ||\n                         (target & 0x7fffffu) == 0x02f6bcu ||\n                         (target & 0x7fffffu) == 0x05eb5fu ||\n                         (target & 0x7fffffu) == 0x039d57u ||\n                         (target & 0x7fffffu) == 0x01f3e4u ||\n                         (target & 0x7fffffu) == 0x03fec4u ||\n                         FramePairedBattleCall(target))\n                        ? (_l2_menu_return ? _l2_menu_return :\n                           _l2_resume_return ? _l2_resume_return :\n                           (pc_before + (uint32_t)call_len +\n                            (uint32_t)cpu_dispatch_inline_arg_bytes(target)) & 0xffffffu)\n                        : UINT32_MAX;\n                    paired_auto_quiescent = s_frame_return_auto_quiescent;\n                    s_frame_return_auto_quiescent = auto_quiescent;\n                    s_frame_paired_call = (FramePairedCallContext){cpu, target & 0x7fffffu,\n                        in.waiting || in.stopped || in.nmiWanted || in.irqWanted ||\n                        FramePairedCallBlocked(target, stop_on_rti, auto_quiescent,\n                            trace || dtrace || wlog_state_sync || s_interp_pctrace ||\n                            g_interp_bridge_pc_hook ||\n                            ((target & 0x7fffffu) == 0x008703u && n_stop), step_cap - steps),\n                        &steps, &progress_write_epoch, &progress_dynamic_epoch};\n                }\n${_paired_begin}"
                    _text "${_text}")
                if(_battle_status OR _second_battler_stat OR _filled_windows)
                    set(_paired_dispatch "                    : cpu_dispatch_pc_paired(cpu, target, _fs);")
                    string(FIND "${_text}" "${_paired_dispatch}" _paired_offset)
                    if(_paired_offset EQUAL -1)
                        message(FATAL_ERROR "Progress transfer requires the reviewed AOT bounce")
                    endif()
                    string(REPLACE "${_paired_dispatch}"
                        "${_paired_dispatch}\n                /* Callees already consumed these progress checkpoints. */\n                if (auto_quiescent && FramePairedBattleCall(target)) {\n                    progress_write_epoch = g_interp_bridge_write_epoch;\n                    progress_dynamic_epoch = s_interp_dynamic_progress_epoch;\n                }"
                        _text "${_text}")
                endif()
                string(REPLACE "${_paired_end}"
                    "                if (paired_tracked) {\n                    s_frame_paired_call = paired_previous;\n                    s_frame_paired_return_pc = paired_previous_return_pc;\n                    s_frame_return_auto_quiescent = paired_auto_quiescent;\n                }\n${_paired_end}"
                    _text "${_text}")
                string(PREPEND _text
                    "#define LUFIA2_HAVE_NATIVE_FIELD_TICK_CLOCK 1\n#define LUFIA2_HAVE_NATIVE_FIELD_CHILD_CLOCK 1\n#define LUFIA2_HAVE_NATIVE_EVENT_SLOT_CLOCK 1\n#define LUFIA2_HAVE_NATIVE_FIELD_IDLE_CLOCK 1\n#define LUFIA2_HAVE_NATIVE_FIELD_STAIR_CLOCK 1\n#define LUFIA2_HAVE_NATIVE_FIELD_RECTANGLE_CLOCK 1\n${_field_map_clock_definition}${_item_region_definition}${_vehicle_terrain_definition}#define LUFIA2_HAVE_NATIVE_FIELD_TRIGGER_CLOCK 1\n#define LUFIA2_HAVE_NATIVE_FIELD_REQUEST_CLOCK 1\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/paired_call_context.inc\"\n")
                if(_battle_status)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_BATTLE_STATUS 1\n")
                endif()
                if(_second_battler_stat)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_SECOND_BATTLER_STAT 1\n")
                endif()
                if(_filled_windows)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_FILLED_WINDOWS 1\n")
                endif()
                get_property(_beam_hold_query GLOBAL PROPERTY SNESRECOMP_PLATFORM_BEAM_HOLD_QUERY)
                if(_beam_hold_query)
                    string(PREPEND _text "#define SNESRECOMP_PLATFORM_BEAM_HOLD_QUERY 1\n")
                endif()
                if("LUFIA2_HAVE_NATIVE_FIELD_MAP_CLOCK=1" IN_LIST _definitions OR
                   "LUFIA2_HAVE_NATIVE_ACTOR_SLOT_TRAVERSAL=1" IN_LIST _definitions)
                    set(_record_hook_entry "                if (s_pre_opcode_hooks[hi].pc24 == key) {\n")
                    string(FIND "${_text}" "${_record_hook_entry}" _record_hook_index)
                    if(_record_hook_index EQUAL -1)
                        message(FATAL_ERROR "Missing actor record hook admission seam")
                    endif()
                    string(REPLACE "${_record_hook_entry}"
                        "${_record_hook_entry}                    if ((pc_before & 0x7fffffu) == 0x03ab4fu &&\n                        Lufia2DecompActorRecordOffsetsHookOwned(pc_before, s_pre_opcode_hooks[hi].hook)) {\n                        int record_stop = 0;\n                        for (int si = 0; si < n_stop; ++si) {\n                            const uint32_t stop = stop_pcs[si] & 0x7fffffu;\n                            record_stop |= stop >= 0x03ab4fu && stop < 0x03ab60u;\n                        }\n                        if (record_stop || step_cap - steps < 9) break;\n                    }\n"
                        _text "${_text}")
                    string(PREPEND _text
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/actor_record_offsets_clock.h\"\n")
                endif()
                if(_battle_effect_frame_steps)
                    set(_effect_frame_hook "                if (s_pre_opcode_hooks[hi].pc24 == key) {\n")
                    string(FIND "${_text}" "${_effect_frame_hook}" _effect_frame_hook_index)
                    if(_effect_frame_hook_index EQUAL -1)
                        message(FATAL_ERROR "Missing effect frame hook admission seam")
                    endif()
                    string(REPLACE "${_effect_frame_hook}"
                        "${_effect_frame_hook}                    uint32_t effect_end = 0;\n                    switch (key) {\n                    case 0x018a76u: case 0x018ab9u: case 0x018b2du: case 0x018b3cu:\n                    case 0x018b4bu: case 0x018b5au: case 0x018bc1u: case 0x018bcbu:\n                        effect_end = Lufia2DecompEffectFrameHookEnd(pc_before, s_pre_opcode_hooks[hi].hook);\n                        break;\n                    default: break;\n                    }\n                    if (effect_end) {\n                        int effect_stop = 0;\n                        for (int si = 0; si < n_stop; ++si) {\n                            const uint32_t stop = stop_pcs[si] & 0x7fffffu;\n                            effect_stop |= stop >= key && stop < (effect_end & 0x7fffffu);\n                        }\n                        if (effect_stop || step_cap - steps < 4) break;\n                    }\n"
                        _text "${_text}")
                    string(PREPEND _text
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/effect_frame_clock.h\"\n")
                endif()
                if(_nmi_scroll_pairs)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_NMI_SCROLL_PAIRS 1\n")
                    set(_scroll_pair_hook "                if (s_pre_opcode_hooks[hi].pc24 == key) {\n")
                    string(FIND "${_text}" "${_scroll_pair_hook}" _scroll_pair_hook_at)
                    if(_scroll_pair_hook_at EQUAL -1)
                        message(FATAL_ERROR "Native scroll pair hook entry changed")
                    endif()
                    string(REPLACE "${_scroll_pair_hook}"
                        "${_scroll_pair_hook}                    if (key == 0x0087b2u && Lufia2DecompNmiScrollPairHookOwned(\n                            pc_before, s_pre_opcode_hooks[hi].hook)) {\n                        int scroll_stop = 0;\n                        for (int si = 0; si < n_stop; ++si) {\n                            const uint32_t stop = stop_pcs[si] & 0x7fffffu;\n                            scroll_stop |= stop >= 0x0087b2u && stop < 0x0087c4u;\n                        }\n                        if (scroll_stop || step_cap - steps < LUFIA2_NMI_SCROLL_PAIR_OPCODES) break;\n                    }\n"
                        _text "${_text}")
                endif()
                if(_screen_fade_steps)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_SCREEN_FADE_STEPS 1\n")
                endif()
                if(_screen_fade_request)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_SCREEN_FADE_REQUEST 1\n")
                endif()
                if(_nmi_joypad_gate)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_NMI_JOYPAD_GATE 1\n#include \"lufia2/nmi_joypad_gate.h\"\n")
                endif()
                if(_nmi_dma_setup)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_NMI_DMA_SETUP 1\n#include \"lufia2/nmi_dma_setup.h\"\n")
                endif()
                if(_nmi_upload_checks)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_NMI_UPLOAD_CHECKS 1\n#include \"lufia2/nmi_upload_checks.h\"\n")
                endif()
                if(_nmi_pad_steps)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_NMI_PAD_STEPS 1\n#include \"lufia2/nmi_pad_steps.h\"\n")
                endif()
                if(_nmi_tilemap_steps)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_NMI_TILEMAP_STEPS 1\n")
                endif()
                if(_nmi_tilemap_loads)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_NMI_TILEMAP_LOADS 1\n")
                endif()
                if(_nmi_scroll_finish)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_NMI_SCROLL_FINISH 1\n")
                endif()
                if(_nmi_scroll_fronts)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_NMI_SCROLL_FRONTS 1\n")
                endif()
                if(_nmi_listed_dma_prepare)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_NMI_LISTED_DMA_PREPARE 1\n")
                endif()
                if(_nmi_display_clock)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_NMI_DISPLAY_CLOCK 1\n")
                endif()
                if(_nmi_callback_clock)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_NMI_CALLBACK_CLOCK 1\n")
                endif()
                if(_nmi_restore_clock)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_NMI_RESTORE_CLOCK 1\n")
                endif()
                if(_nmi_restore_width)
                    string(REPLACE "${_hook_call}"
                        "                    {\n                        const int interrupt_owner = s_native_hook_interrupt;\n                        s_native_hook_interrupt = Lufia2DecompNmiHookHasOwner(\n                            pc_before, stop_on_rti, auto_quiescent |\n                            ((trace || dtrace || wlog_state_sync) ? LUFIA2_NMI_OWNER_OBSERVED : 0),\n                            step_cap - steps);\n${_hook_call}\n                        s_native_hook_interrupt = interrupt_owner;\n                    }"
                        _text "${_text}")
                    string(PREPEND _text "#include <stdint.h>\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/nmi_owner.h\"\nstatic int s_native_hook_interrupt;\nstatic bool Lufia2DecompNmiHookHasOwner(uint32_t, int, int, long);\n")
                    string(APPEND _timing_suffix
                        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/nmi_owner.inc\"\n")
                endif()
                if(_ripple_word_phases AND _nmi_restore_width)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_RIPPLE_WORD_PHASES 1\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/ripple_word_clock.h\"\n")
                    set(_ripple_word_hook "                if (s_pre_opcode_hooks[hi].pc24 == key) {\n")
                    string(FIND "${_text}" "${_ripple_word_hook}" _ripple_word_hook_at)
                    if(_ripple_word_hook_at EQUAL -1)
                        message(FATAL_ERROR "Native word ripple hook entry changed")
                    endif()
                    string(REPLACE "${_ripple_word_hook}"
                        "${_ripple_word_hook}                    {\n                        const uint32_t ripple_end = Lufia2DecompRippleWordPhaseHookEnd(\n                            pc_before, s_pre_opcode_hooks[hi].hook);\n                        if (ripple_end) {\n                            const uint32_t first = pc_before & 0x7fffffu;\n                            const unsigned count = first == 0x05aa3du\n                                ? LUFIA2_NMI_RIPPLE_WORD_PREPARE_OPCODES\n                                : LUFIA2_NMI_RIPPLE_WORD_FINISH_OPCODES;\n                            int ripple_stop = 0;\n                            for (int si = 0; si < n_stop; ++si) {\n                                const uint32_t stop = stop_pcs[si] & 0x7fffffu;\n                                ripple_stop |= stop >= first && stop < (ripple_end & 0x7fffffu);\n                            }\n                            if (ripple_stop || step_cap - steps < count) break;\n                        }\n                    }\n"
                        _text "${_text}")
                endif()
                if(_action_clear_steps)
                    string(PREPEND _text "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/action_clear_clock.h\"\n")
                    set(_action_clear_hook "                if (s_pre_opcode_hooks[hi].pc24 == key) {\n")
                    string(FIND "${_text}" "${_action_clear_hook}" _action_clear_hook_at)
                    if(_action_clear_hook_at EQUAL -1)
                        message(FATAL_ERROR "Native action clear hook entry changed")
                    endif()
                    string(REPLACE "${_action_clear_hook}"
                        "${_action_clear_hook}                    if (Lufia2DecompActionClearStepHookOwned(\n                            pc_before, s_pre_opcode_hooks[hi].hook) &&\n                        ((pc_before >> 16) != 0x85u || !auto_quiescent ||\n                         s_interp_bridge_depth != 1 || n_stop || stop_on_rti ||\n                         trace || dtrace || wlog_state_sync || s_interp_pctrace ||\n                         g_interp_bridge_pc_hook || Lufia2NativeObserversActive() ||\n                         step_cap - steps < 16))\n                        break;\n"
                        _text "${_text}")
                endif()
                if(_frame_decisions)
                    string(PREPEND _text "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/frame_decision_clock.h\"\n")
                    set(_decision_hook "                if (s_pre_opcode_hooks[hi].pc24 == key) {\n")
                    string(FIND "${_text}" "${_decision_hook}" _decision_hook_at)
                    if(_decision_hook_at EQUAL -1)
                        message(FATAL_ERROR "Native frame-decision hook entry changed")
                    endif()
                    string(REPLACE "${_decision_hook}"
                        "${_decision_hook}                    if (key == 0x05ec85u || key == 0x05ec8fu || key == 0x06925cu ||\n                        key == 0x069264u || key == 0x069288u) {\n                        if (Lufia2DecompFrameDecisionHookOwned(pc_before, s_pre_opcode_hooks[hi].hook) &&\n                            (pc_before < 0x800000u || !auto_quiescent ||\n                             s_interp_bridge_depth != 1 || n_stop || stop_on_rti ||\n                             trace || dtrace || wlog_state_sync || s_interp_pctrace ||\n                             g_interp_bridge_pc_hook || Lufia2NativeObserversActive() ||\n                             step_cap - steps < 16))\n                            break;\n                    }\n"
                        _text "${_text}")
                endif()
                if(_nmi_display_clock OR _nmi_callback_clock OR _nmi_restore_clock)
                    string(PREPEND _text "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/nmi_owner.h\"\n")
                    set(_nmi_timed_hook "                if (s_pre_opcode_hooks[hi].pc24 == key) {\n")
                    string(FIND "${_text}" "${_nmi_timed_hook}" _nmi_timed_hook_at)
                    if(_nmi_timed_hook_at EQUAL -1)
                        message(FATAL_ERROR "Native NMI timed hook entry changed")
                    endif()
                    string(REPLACE "${_nmi_timed_hook}"
                        "${_nmi_timed_hook}                    if ((key == 0x0086c1u || key == 0x00864cu || key == 0x008653u || key == 0x00865au ||\n                         key == 0x008664u || (key == 0x00869bu && !in.xf) ||\n                         key == 0x0087a7u || key == 0x0087c4u || key == 0x0087cfu ||\n                         key == 0x0087dau || key == 0x0087e5u || key == 0x0087f3u ||\n                         key == 0x0087fcu || key == 0x00880du || key == 0x00881cu || key == 0x008829u\n#if defined(LUFIA2_HAVE_NATIVE_NMI_JOYPAD_GATE)\n                         || key == 0x008765u\n#endif\n#if defined(LUFIA2_HAVE_NATIVE_SCREEN_FADE_STEPS)\n                         || key == 0x0086c6u || key == 0x0086dbu || key == 0x0086f2u\n#endif\n#if defined(LUFIA2_HAVE_NATIVE_NMI_LISTED_DMA_PREPARE)\n                         || key == 0x0087c8u || key == 0x0087d3u || key == 0x0087deu || key == 0x0087e9u\n#endif\n#if defined(LUFIA2_HAVE_NATIVE_NMI_DMA_SETUP)\n                         || key == 0x00870cu || key == 0x00873cu\n#endif\n#if defined(LUFIA2_HAVE_NATIVE_NMI_UPLOAD_CHECKS)\n                         || key == 0x008706u || key == 0x008736u\n#endif\n#if defined(LUFIA2_HAVE_NATIVE_NMI_PAD_STEPS)\n                         || key == 0x008770u\n#endif\n#if defined(LUFIA2_HAVE_NATIVE_NMI_TILEMAP_LOADS)\n                         || key == 0x008802u || key == 0x008811u || key == 0x008820u\n#endif\n                         )) {\n                        const uint32_t nmi_end = Lufia2DecompNmiTimedHookEnd(\n                            pc_before, s_pre_opcode_hooks[hi].hook) & 0x7fffffu;\n                        if (nmi_end) {\n                            const unsigned needed =\n#if defined(LUFIA2_HAVE_NATIVE_NMI_JOYPAD_GATE)\n                                key == 0x008765u ? LUFIA2_NMI_JOYPAD_GATE_OPCODES :\n#endif\n#if defined(LUFIA2_HAVE_NATIVE_SCREEN_FADE_STEPS)\n                                (key == 0x0086c6u || key == 0x0086dbu) ? LUFIA2_NMI_FADE_ACCUMULATE_OPCODES :\n                                key == 0x0086f2u ? LUFIA2_NMI_FADE_IN_OPCODES :\n#endif\n#if defined(LUFIA2_HAVE_NATIVE_NMI_LISTED_DMA_PREPARE)\n                                (key == 0x0087c8u || key == 0x0087d3u || key == 0x0087deu || key == 0x0087e9u) ? LUFIA2_NMI_DMA_PREPARE_OPCODES :\n#endif\n#if defined(LUFIA2_HAVE_NATIVE_NMI_DMA_SETUP)\n                                (key == 0x00870cu || key == 0x00873cu) ? LUFIA2_NMI_DMA_SETUP_OPCODES :\n#endif\n#if defined(LUFIA2_HAVE_NATIVE_NMI_UPLOAD_CHECKS)\n                                (key == 0x008706u || key == 0x008736u) ? LUFIA2_NMI_UPLOAD_CHECK_OPCODES :\n#endif\n#if defined(LUFIA2_HAVE_NATIVE_NMI_PAD_STEPS)\n                                key == 0x008770u ? LUFIA2_NMI_PAD_STEP_OPCODES :\n#endif\n                                key == 0x0086c1u ? LUFIA2_NMI_FADE_REQUEST_OPCODES :\n                                key == 0x008664u ? LUFIA2_NMI_DISPLAY_OPCODES :\n                                key == 0x00869bu ? LUFIA2_NMI_RESTORE_OPCODES :\n                                key == 0x00864cu ? LUFIA2_NMI_PRIMARY_CALLBACK_OPCODES :\n                                key == 0x008653u ? LUFIA2_NMI_SECONDARY_OPCODES :\n                                key == 0x0087a7u ? LUFIA2_NMI_SCROLL_SETUP_OPCODES :\n                                key == 0x0087f3u ? LUFIA2_NMI_SCROLL_FINISH_OPCODES :\n                                key == 0x0087fcu ? LUFIA2_NMI_TILEMAP_HEADER_OPCODES :\n                                (key == 0x00880du || key == 0x00881cu || key == 0x008829u\n#if defined(LUFIA2_HAVE_NATIVE_NMI_TILEMAP_LOADS)\n                                 || key == 0x008802u || key == 0x008811u || key == 0x008820u\n#endif\n                                 ) ? LUFIA2_NMI_TILEMAP_STEP_OPCODES :\n                                key == 0x00865au ? LUFIA2_NMI_SCROLL_OPCODES : LUFIA2_NMI_DMA_SELECT_OPCODES;\n                            int nmi_stop = 0;\n                            for (int si = 0; si < n_stop; ++si) {\n                                const uint32_t stop = stop_pcs[si] & 0x7fffffu;\n                                nmi_stop |= stop >= key && stop < nmi_end;\n                            }\n                            if ((pc_before >> 16) != 0x80u || nmi_stop || !stop_on_rti ||\n                                auto_quiescent || trace || dtrace || wlog_state_sync ||\n                                s_interp_pctrace || g_interp_bridge_pc_hook ||\n                                s_interp_bus_timing_active || in.waiting || in.stopped ||\n                                in.nmiWanted || in.irqWanted || Lufia2NativeObserversActive() ||\n                                steps < 0 || step_cap <= steps || step_cap - steps < needed)\n                                break;\n                        }\n                    }\n"
                        _text "${_text}")
                endif()
                if(_battle_target_name_steps)
                    string(PREPEND _text "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/battle_target_name_clock.h\"\n")
                    set(_target_name_hook "                if (s_pre_opcode_hooks[hi].pc24 == key) {\n")
                    string(FIND "${_text}" "${_target_name_hook}" _target_name_hook_at)
                    if(_target_name_hook_at EQUAL -1)
                        message(FATAL_ERROR "Native target-name hook entry changed")
                    endif()
                    string(REPLACE "${_target_name_hook}"
                        "${_target_name_hook}                    if ((key == 0x01d58du || key == 0x01d7c5u || key == 0x01d8ccu) &&\n                        Lufia2DecompTargetNameHookOwned(pc_before, s_pre_opcode_hooks[hi].hook)) {\n                        int target_name_stop = 0;\n                        for (int si = 0; si < n_stop; ++si) {\n                            const uint32_t stop = stop_pcs[si] & 0x7fffffu;\n                            target_name_stop |= stop >= key &&\n                                stop < key + LUFIA2_TARGET_NAME_SPAN_BYTES;\n                        }\n                        if ((pc_before >> 16) != 0x81u || target_name_stop || stop_on_rti ||\n                            trace || dtrace || wlog_state_sync || s_interp_pctrace ||\n                            g_interp_bridge_pc_hook || Lufia2NativeObserversActive() ||\n                            step_cap - steps < LUFIA2_TARGET_NAME_MAX_OPCODES)\n                            break;\n                    }\n"
                        _text "${_text}")
                endif()
                if(_ripple_word_steps)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_RIPPLE_WORD_STEPS 1\n")
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
                if(_battle_target_marker_clock)
                    string(PREPEND _text
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/battle_target_marker_clock.h\"\n")
                    set(_marker_hook "                if (s_pre_opcode_hooks[hi].pc24 == key) {\n")
                    string(FIND "${_text}" "${_marker_hook}" _marker_hook_at)
                    if(_marker_hook_at EQUAL -1)
                        message(FATAL_ERROR "Native target marker hook entry changed")
                    endif()
                    string(REPLACE "${_marker_hook}"
                        "${_marker_hook}                    if (key == 0x01d60eu &&\n                        (n_stop || step_cap - steps < LUFIA2_TARGET_MARKER_MAX_OPCODES) &&\n                        Lufia2DecompTargetMarkerHookOwned(pc_before, s_pre_opcode_hooks[hi].hook))\n                        break;\n"
                        _text "${_text}")
                endif()
                if(_battle_input_steps)
                    string(PREPEND _text
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/battle_input_clock.h\"\n")
                    set(_battle_input_hook "                if (s_pre_opcode_hooks[hi].pc24 == key) {\n")
                    string(FIND "${_text}" "${_battle_input_hook}" _battle_input_hook_at)
                    if(_battle_input_hook_at EQUAL -1)
                        message(FATAL_ERROR "Native battle input hook entry changed")
                    endif()
                    string(REPLACE "${_battle_input_hook}"
                        "${_battle_input_hook}                    if (n_stop && Lufia2DecompBattleInputHookOwned(\n                            pc_before, s_pre_opcode_hooks[hi].hook))\n                        break;\n"
                        _text "${_text}")
                    string(REPLACE "${_hook_call}"
                        "                    if ((((pc_before & 0x7fffffu) != 0x05ec9au ||\n                         step_cap - steps >= 14) &&\n                        ((pc_before & 0x7fffffu) != 0x05ecbdu ||\n                         step_cap - steps >= 15)) ||\n                        !Lufia2DecompBattleInputHookOwned(\n                            pc_before, s_pre_opcode_hooks[hi].hook)) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
                if(_battle_nmi_callbacks)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_BATTLE_NMI_CALLBACKS 1\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/nmi_owner.h\"\n")
                    set(_battle_callback_hook "                if (s_pre_opcode_hooks[hi].pc24 == key) {\n")
                    string(FIND "${_text}" "${_battle_callback_hook}" _battle_callback_hook_at)
                    if(_battle_callback_hook_at EQUAL -1)
                        message(FATAL_ERROR "Native battle callback hook entry changed")
                    endif()
                    string(REPLACE "${_battle_callback_hook}"
                        "${_battle_callback_hook}                    if ((key == 0x058e79u || key == 0x058e7cu || key == 0x058e8eu) &&\n                        Lufia2DecompNmiTimedHookEnd(pc_before, s_pre_opcode_hooks[hi].hook)) {\n                        int callback_stop = 0;\n                        for (int si = 0; si < n_stop; ++si) {\n                            const uint32_t stop = stop_pcs[si] & 0x7fffffu;\n                            callback_stop |= stop >= key && stop < 0x058e97u;\n                        }\n                        if ((pc_before >> 16) != 0x85u || callback_stop || !stop_on_rti ||\n                            auto_quiescent || trace || dtrace || wlog_state_sync ||\n                            s_interp_pctrace || g_interp_bridge_pc_hook ||\n                            s_interp_bus_timing_active || in.waiting || in.stopped ||\n                            in.nmiWanted || in.irqWanted || Lufia2NativeObserversActive() ||\n                            steps < 0 || step_cap <= steps ||\n                            step_cap - steps < LUFIA2_NMI_BATTLE_CALLBACK_OPCODES)\n                            break;\n                    }\n"
                        _text "${_text}")
                endif()
                if(_battle_timer_calls)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_BATTLE_TIMER_CALLS 1\n")
                endif()
                if(_battle_timer_steps)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_BATTLE_TIMER_STEPS 1\n")
                    string(REPLACE "${_hook_call}"
                        "                    if (((pc_before & 0x7fffffu) != 0x058f1fu ||\n                         step_cap - steps >= 10) &&\n                        ((pc_before & 0x7fffffu) != 0x058f3eu ||\n                         step_cap - steps >= 6)) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
                if(_battle_hdma_builders)
                    string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_BATTLE_HDMA_BUILDERS 1\n")
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
                    string(PREPEND _text
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/effect_dispatch_clock.h\"\n")
                    string(REPLACE "${_hook_call}"
                        "                    if (((pc_before & 0x7fffffu) != 0x018a95u &&\n                         (pc_before & 0x7fffffu) != 0x018b08u) ||\n                        !Lufia2DecompEffectDispatchHookOwned(pc_before, s_pre_opcode_hooks[hi].hook) ||\n                        step_cap - steps >= 9) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
                if(_battle_effect_cleanup_steps)
                    string(PREPEND _text
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/effect_clear_clock.h\"\n")
                    set(_clear_hook "                if (s_pre_opcode_hooks[hi].pc24 == key) {\n")
                    string(FIND "${_text}" "${_clear_hook}" _clear_hook_at)
                    if(_clear_hook_at EQUAL -1)
                        message(FATAL_ERROR "Native effect clear hook entry changed")
                    endif()
                    string(REPLACE "${_clear_hook}"
                        "${_clear_hook}                    if (key == 0x018bfbu && n_stop &&\n                        Lufia2DecompEffectClearHookOwned(pc_before, s_pre_opcode_hooks[hi].hook))\n                        break;\n"
                        _text "${_text}")
                    if(NOT _battle_effect_clear_spans)
                        string(REPLACE "${_hook_call}"
                            "                    if ((pc_before & 0x7fffffu) != 0x018bfbu ||\n                        step_cap - steps >= LUFIA2_EFFECT_CLEAR_STEP_OPCODES ||\n                        !Lufia2DecompEffectClearHookOwned(pc_before, s_pre_opcode_hooks[hi].hook)) {\n${_hook_call}\n                    }"
                            _text "${_text}")
                    endif()
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
                if(_battle_command_tiles)
                    string(REPLACE "${_hook_call}"
                        "                    if ((pc_before & 0x7fffffu) != 0x01cb99u ||\n                        !Lufia2DecompCommandTileHookOwned(pc_before, s_pre_opcode_hooks[hi].hook) ||\n                        step_cap - steps >= 10) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
                if(_selected_command_tile)
                    string(REPLACE "${_hook_call}"
                        "                    if ((pc_before & 0x7fffffu) != 0x01cbb8u ||\n                        !Lufia2DecompCommandTileHookOwned(pc_before, s_pre_opcode_hooks[hi].hook) ||\n                        step_cap - steps >= 17) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
                if(_battle_transition_steps)
                    string(REPLACE "${_hook_call}"
                        "                    if (((pc_before & 0x7fffffu) != 0x048c91u &&\n                         (pc_before & 0x7fffffu) != 0x048cddu) ||\n                        step_cap - steps >= 19) {\n${_hook_call}\n                    }"
                        _text "${_text}")
                endif()
            endif()
            if(_world_frame_loop)
                lufia2_compose_world_frame_loop(_text "${_text}")
            endif()
            if(_field_region_continuations)
                lufia2_compose_field_region_continuations(_text "${_text}")
            endif()
            if(_battle_upkeep)
                lufia2_compose_battle_upkeep(_text "${_text}")
            endif()
            if(_field_region_continuations OR _battle_upkeep)
                lufia2_compose_native_call_owner(_text "${_text}")
            endif()
            if(_boot_nmi_wait AND "LUFIA2_HAVE_NATIVE_FRAME_POLL_RUNS=1" IN_LIST _definitions)
                string(PREPEND _text "#define LUFIA2_HAVE_NATIVE_BOOT_NMI_WAIT 1\n")
            endif()
            if(_vehicle_terrain)
                string(APPEND _timing_suffix
                    "\nbool Lufia2DecompFrameSchedulerBounces(void) { return lle_yield_bounce_enabled() != 0; }\n")
            endif()
            if("LUFIA2_HAVE_NATIVE_RESET_ENTRY=1" IN_LIST _definitions)
                set(_interrupt_return "        if (stop_on_rti && op == 0x40) {")
                string(FIND "${_text}" "${_interrupt_return}" _interrupt_return_position)
                if(_interrupt_return_position LESS 0)
                    message(FATAL_ERROR "Missing interrupt return for the native reset handoff")
                endif()
                string(REPLACE "${_interrupt_return}" [=[
        /* Soft reset leaves the NMI handler through the reset vector. */
        if (stop_on_rti && entry_pc24 == 0x00862du && pc_before == 0x8086b1u &&
            op == 0x5cu && in.k == 0u && in.pc == 0x8000u) {
            sync_interp_to_cpu(&in, cpu);
            bridge_apu_flush(cpu);
            return LUFIA2_INTERRUPT_RESET_HANDOFF;
        }
        if (stop_on_rti && op == 0x40) {
]=] _text "${_text}")
                string(PREPEND _text "#include \"decomp_bridge/nmi_resume.h\"\n")
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
