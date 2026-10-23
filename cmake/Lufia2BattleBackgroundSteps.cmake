include_guard(GLOBAL)

function(lufia2_battle_background_steps_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle_background_steps.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_background_internal.c")
    set(_metadata_path "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}" OR
       NOT EXISTS "${_metadata_path}")
        return()
    endif()
    set(_owner "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_background.c")
    if(NOT EXISTS "${_owner}")
        return()
    endif()
    file(READ "${_owner}" _root_implementation)
    if(NOT _root_implementation MATCHES "Lufia2BattleBackgroundPrepare[(][^;{]*[{]")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_source}" _implementation)
    foreach(_name IN ITEMS Lufia2BattleDecodeBackgroundWordStep)
        if(NOT _api MATCHES "${_name}[(]" OR
           NOT _implementation MATCHES "${_name}[(][^;{]*[{]")
            return()
        endif()
    endforeach()
    file(READ "${_metadata_path}" _metadata)
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2BattleBackgroundPrepare\"[^[]*"
        _contract "${_metadata}")
    if(_contract MATCHES "address[ \t]*=[ \t]*\"81:B9C7\"" AND
       _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" AND
       _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()
