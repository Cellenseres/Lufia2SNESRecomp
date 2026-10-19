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
    if(_api MATCHES "Lufia2FrameWaitRestoreRegisters[(]" AND
       EXISTS "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/menu.h")
        file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/menu.h" _menu_api)
        if(_menu_api MATCHES "Lufia2SpriteFrameRestoreRegisters[(]")
            include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2FrameReturns.cmake")
            lufia2_frame_returns_available(_frame_returns ${target})
            if(_frame_returns)
                list(APPEND _definitions LUFIA2_HAVE_NATIVE_FRAME_RETURNS=1)
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
                    "                    s_frame_return_auto_quiescent = auto_quiescent;\n${_hook_call}"
                    _text "${_text}")
                string(REPLACE "${_redirect}"
                    "            if (redirected) {\n                Lufia2DecompFrameReturnContinue(&steps, auto_quiescent,\n                    &progress_write_epoch, &progress_dynamic_epoch);\n                continue;\n            }"
                    _text "${_text}")
                string(PREPEND _text
                    "#include <stdint.h>\nstatic int s_frame_return_auto_quiescent;\nstatic void Lufia2DecompFrameReturnContinue(long *, int, uint64_t *, uint64_t *);\n")
                string(APPEND _timing_suffix
                    "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/frame_return_cycles.inc\"\n")
            endif()
            if(_frame_polls)
                set(_wait_hook_guard "                    _l2_hooked = 1;")
                string(FIND "${_text}" "${_wait_hook_guard}" _wait_hook_offset)
                if(NOT _wait_hook_offset EQUAL -1)
                    string(REPLACE "${_wait_hook_guard}"
                        "                    if (Lufia2DecompFramePollHookIsNative(s_pre_opcode_hooks[_l2_hi].hook))\n                        continue;\n${_wait_hook_guard}"
                        _text "${_text}")
                    string(PREPEND _text
                        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/frame_returns.h\"\n")
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
