include_guard(GLOBAL)

function(lufia2_sprite_overlays_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_sprite_build.c")
    set(_metadata_path "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}" OR
       NOT EXISTS "${_metadata_path}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_source}" _implementation)
    if(NOT _api MATCHES "Lufia2BattleApplySpritePositionOverlays[(]" OR
       NOT _implementation MATCHES "Lufia2BattleApplySpritePositionOverlays[(][^;{]*[{]")
        return()
    endif()
    file(READ "${_metadata_path}" _metadata)
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2BattleBuildSprites\"[^[]*"
        _contract "${_metadata}")
    if(_contract MATCHES "address[ \t]*=[ \t]*\"81:B5C4\"" AND
       _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" AND
       _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()
