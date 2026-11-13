include_guard(GLOBAL)

function(lufia2_battle_command_tiles_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle_command_tiles.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_command.c")
    set(_metadata "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}" OR NOT EXISTS "${_metadata}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_source}" _implementation)
    file(READ "${_metadata}" _functions)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${_header}" "${_source}" "${_metadata}")
    if(NOT _api MATCHES "#define LUFIA2_BATTLE_COMMAND_TILE_API 1" OR
       NOT _api MATCHES "Lufia2BattlePrepareCommandTile[(]" OR
       NOT _implementation MATCHES "Lufia2BattlePrepareCommandTile[(][^;{]*[{]")
        return()
    endif()
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2BattleChooseCommand\"[^[]*"
        _contract "${_functions}")
    if(_contract MATCHES "address[ \t]*=[ \t]*\"81:CB77\"" AND
       _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" AND
       _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()

function(lufia2_selected_command_tile_available output)
    set(${output} FALSE PARENT_SCOPE)
    lufia2_battle_command_tiles_available(_base)
    if(NOT _base)
        return()
    endif()
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle_command_tiles.h" _api)
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_command.c" _implementation)
    if(_api MATCHES "#define LUFIA2_BATTLE_SELECTED_COMMAND_TILE_API 1" AND
       _api MATCHES "Lufia2BattlePrepareSelectedCommandTile[(]" AND
       _implementation MATCHES "Lufia2BattlePrepareSelectedCommandTile[(][^;{]*[{]")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()

function(lufia2_party_command_tiles_available output)
    set(${output} FALSE PARENT_SCOPE)
    lufia2_battle_command_tiles_available(_base)
    if(NOT _base)
        return()
    endif()
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_party_commands.c")
    if(NOT EXISTS "${_source}")
        return()
    endif()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_source}")
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle_command_tiles.h" _api)
    file(READ "${_source}" _implementation)
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml" _functions)
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2BattleChoosePartyAction\"[^[]*"
        _contract "${_functions}")
    if(_api MATCHES "#define LUFIA2_BATTLE_PARTY_COMMAND_TILE_API 1" AND
       _api MATCHES "Lufia2BattlePreparePartyCommandTile[(]" AND
       _api MATCHES "Lufia2BattleContinuePartyCommandTiles[(]" AND
       _implementation MATCHES "Lufia2BattlePreparePartyCommandTile[(][^;{]*[{]" AND
       _implementation MATCHES "Lufia2BattleContinuePartyCommandTiles[(][^;{]*[{]" AND
       _contract MATCHES "address[ \t]*=[ \t]*\"81:CC2E\"" AND
       _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" AND
       _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()
