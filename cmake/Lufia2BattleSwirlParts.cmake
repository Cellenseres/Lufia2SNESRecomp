include_guard(GLOBAL)

function(lufia2_battle_swirl_parts_available output)
    set(${output} FALSE PARENT_SCOPE)
    include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Lufia2BattleTransitionSteps.cmake")
    lufia2_battle_transition_steps_available(_base)
    if(NOT _base)
        return()
    endif()
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle_swirl_steps.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_transition.c")
    set(_metadata_path "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}" OR NOT EXISTS "${_metadata_path}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_source}" _implementation)
    foreach(_name IN ITEMS Lufia2BattleSwirlHdmaPrepareStep Lufia2BattleSwirlHdmaStoreStep Lufia2BattleSwirlHdmaAdvanceStep)
        if(NOT _api MATCHES "${_name}[(]" OR
           NOT _implementation MATCHES "${_name}[(][^;{]*[{]")
            return()
        endif()
    endforeach()
    file(READ "${_metadata_path}" _metadata)
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2BattleVisualTransition\"[^[]*"
        _contract "${_metadata}")
    if(_contract MATCHES "address[ \t]*=[ \t]*\"84:8BC7\"" AND
       _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" AND
       _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()
