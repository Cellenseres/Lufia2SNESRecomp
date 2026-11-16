include_guard(GLOBAL)

function(lufia2_battle_target_name_steps_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle_target_name_steps.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_targets.c")
    set(_metadata_path "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}" OR
       NOT EXISTS "${_metadata_path}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_source}" _implementation)
    if(NOT _api MATCHES "Lufia2BattleClearTargetNameSpan[(]" OR
       NOT _implementation MATCHES "Lufia2BattleClearTargetNameSpan[(][^;{]*[{]")
        return()
    endif()
    file(READ "${_metadata_path}" _metadata)
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2BattleChooseTargets\"[^[]*"
        _contract "${_metadata}")
    if(_contract MATCHES "address[ \t]*=[ \t]*\"81:D4E0\"" AND
       _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" AND
       _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()

function(lufia2_battle_target_cursor_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle_target_name_steps.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_targets.c")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_source}" _implementation)
    if(_api MATCHES "Lufia2BattleTargetCursorStep[(]" AND
       _implementation MATCHES "Lufia2BattleTargetCursorStep[(][^;{]*[{]")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()

function(lufia2_battle_target_selection_available output)
    set(${output} FALSE PARENT_SCOPE)
    lufia2_battle_target_cursor_available(_cursor)
    if(NOT _cursor)
        return()
    endif()
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle_target_name_steps.h" _api)
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_targets.c" _implementation)
    if(_api MATCHES "#define LUFIA2_BATTLE_TARGET_SELECTION_PHASES 1([\r\n]|$)" AND
       _implementation MATCHES "case 0x81d593u:" AND
       _implementation MATCHES "case 0x81d5b2u:" AND
       _implementation MATCHES "case 0x81d623u:")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()

function(lufia2_battle_target_input_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle_target_name_steps.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_targets.c")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_source}" _implementation)
    if(_api MATCHES "#define LUFIA2_BATTLE_TARGET_INPUT_PHASES 1([\r\n]|$)" AND
       _api MATCHES "Lufia2BattleTargetInputStep[(]" AND
       _implementation MATCHES "Lufia2BattleTargetInputStep[(][^;{]*[{]" AND
       _implementation MATCHES "case 0x81d638u:" AND
       _implementation MATCHES "case 0x81d648u:" AND
       _implementation MATCHES "case 0x81d64eu:")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()
