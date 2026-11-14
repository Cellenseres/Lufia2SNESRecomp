include_guard(GLOBAL)

function(lufia2_battle_hdma_builders_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_hdma_builders.c")
    set(_metadata_path "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}" OR
       NOT EXISTS "${_metadata_path}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_source}" _implementation)
    set(_entries AF5E AFE0 B058 B0C3 B13B B1A4)
    file(READ "${_metadata_path}" _metadata)
    foreach(_mode RANGE 5 10)
        set(_name "Lufia2BattleHdmaBuildMode${_mode}")
        if(NOT _api MATCHES "${_name}[(]" OR
           NOT _implementation MATCHES "${_name}[(][^;{]*[{]")
            return()
        endif()
        math(EXPR _index "${_mode} - 5")
        list(GET _entries ${_index} _entry)
        string(REGEX MATCH
            "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"${_name}\"[^[]*"
            _contract "${_metadata}")
        if(NOT _contract MATCHES "address[ \t]*=[ \t]*\"85:${_entry}\"" OR
           NOT _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" OR
           NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
            return()
        endif()
    endforeach()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
