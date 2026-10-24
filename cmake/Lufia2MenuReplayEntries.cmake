include_guard(GLOBAL)

function(lufia2_menu_replay_entries_available output target)
    set(${output} FALSE PARENT_SCOPE)
    if(NOT target STREQUAL "Lufia2Recomp" OR LUFIA2_DECOMP_REFERENCE_ONLY)
        return()
    endif()
    set(_names Lufia2CardGameChooseReplay Lufia2BoardGameChooseReplay)
    set(_headers card_game.h board_game.h)
    set(_addresses "84:AB23" "84:CAF1")
    set(_bridges Lufia2DecompBridge_84AB23 Lufia2DecompBridge_84CAF1)
    set(_metadata_path "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    set(_bindings_path "${CMAKE_SOURCE_DIR}/recomp/decomp_bindings.toml")
    if(NOT EXISTS "${_metadata_path}" OR NOT EXISTS "${_bindings_path}")
        return()
    endif()
    file(READ "${_metadata_path}" _metadata)
    file(READ "${_bindings_path}" _bindings)
    foreach(_index RANGE 0 1)
        list(GET _names ${_index} _name)
        list(GET _headers ${_index} _header)
        list(GET _addresses ${_index} _address)
        list(GET _bridges ${_index} _bridge)
        set(_path "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/${_header}")
        if(NOT EXISTS "${_path}")
            return()
        endif()
        file(READ "${_path}" _api)
        string(REGEX MATCH
            "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"${_name}\"[^[]*"
            _contract "${_metadata}")
        string(REGEX MATCH
            "\\[\\[binding\\]\\][^[]*address[ \t]*=[ \t]*\"${_address}\"[^[]*"
            _binding "${_bindings}")
        if(NOT _api MATCHES "${_name}[(]" OR
           NOT _contract MATCHES "address[ \t]*=[ \t]*\"${_address}\"" OR
           NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"" OR
           NOT _binding MATCHES "bridge[ \t]*=[ \t]*\"${_bridge}\"")
            return()
        endif()
    endforeach()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
