include_guard(GLOBAL)

function(lufia2_action_copy_spans_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle_action_work_steps.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_action_work.c")
    set(_metadata "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}" OR NOT EXISTS "${_metadata}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_source}" _implementation)
    if(NOT _api MATCHES "#define LUFIA2_BATTLE_ACTION_COPY_SPANS 1([\r\n]|$)" OR
       NOT _api MATCHES "Lufia2BattleCopyActionWorkSpan[(]" OR
       NOT _implementation MATCHES "Lufia2BattleCopyActionWorkSpan[(][^;{]*[{]" OR
       NOT _implementation MATCHES "entry != 0x85cd8fu" OR
       NOT _implementation MATCHES "entry != 0x85cd9eu")
        return()
    endif()
    file(READ "${_metadata}" _functions)
    foreach(_direction IN ITEMS Save Restore)
        string(REGEX MATCH
            "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2Battle${_direction}ActionWork\"[^[]*"
            _contract "${_functions}")
        if(_direction STREQUAL "Save")
            set(_address "85:CD8C")
        else()
            set(_address "85:CD9B")
        endif()
        if(NOT _contract MATCHES "address[ \t]*=[ \t]*\"${_address}\"" OR
           NOT _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" OR
           NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
            return()
        endif()
    endforeach()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
