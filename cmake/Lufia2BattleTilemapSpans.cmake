include_guard(GLOBAL)

function(lufia2_battle_tilemap_spans_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle_tilemap_steps.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_loop_internal.c")
    set(_metadata "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}" OR NOT EXISTS "${_metadata}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_source}" _implementation)
    if(NOT _api MATCHES "#define LUFIA2_BATTLE_TILEMAP_SPANS 1([\r\n]|$)" OR
       NOT _api MATCHES "Lufia2BattleClearTilemapCells[(]" OR
       NOT _implementation MATCHES "Lufia2BattleClearTilemapCells[(][^;{]*[{]" OR
       NOT _implementation MATCHES "program_bank != 0x81u")
        return()
    endif()
    file(READ "${_metadata}" _functions)
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2BattleMainLoop\"[^[]*"
        _contract "${_functions}")
    if(NOT _contract MATCHES "address[ \t]*=[ \t]*\"81:886F\"" OR
       NOT _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" OR
       NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        return()
    endif()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
