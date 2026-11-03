include_guard(GLOBAL)

function(lufia2_menu_clear_available output target)
    set(${output} FALSE PARENT_SCOPE)
    if(NOT target STREQUAL "Lufia2Recomp" OR LUFIA2_DECOMP_REFERENCE_ONLY)
        return()
    endif()
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/special_menu.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/menu/special_menu.c")
    set(_metadata_path "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    set(_bindings_path "${CMAKE_SOURCE_DIR}/recomp/decomp_bindings.toml")
    foreach(_file IN ITEMS "${_header}" "${_source}" "${_metadata_path}" "${_bindings_path}")
        if(NOT EXISTS "${_file}")
            return()
        endif()
    endforeach()
    file(READ "${_header}" _api)
    file(READ "${_source}" _body)
    file(READ "${_metadata_path}" _metadata)
    file(READ "${_bindings_path}" _bindings)
    if(NOT _api MATCHES "Lufia2SpecialMenuClearStep[(]" OR
       NOT _body MATCHES "Lufia2ExecutionResult[ \r\n]+Lufia2SpecialMenuClearStep[(]")
        return()
    endif()
    set(_addresses "84:A4DD" "84:A4FD")
    set(_names "Lufia2SpecialMenuClearTilemaps" "Lufia2SpecialMenuHideSprites")
    set(_bridges "Lufia2DecompBridge_84A4DD" "Lufia2DecompBridge_84A4FD")
    foreach(_index RANGE 0 1)
        list(GET _addresses ${_index} _address)
        list(GET _names ${_index} _name)
        list(GET _bridges ${_index} _bridge)
        string(REGEX MATCH
            "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"${_name}\"[^[]*"
            _contract "${_metadata}")
        string(REGEX MATCH
            "\\[\\[binding\\]\\][^[]*address[ \t]*=[ \t]*\"${_address}\"[^[]*"
            _binding "${_bindings}")
        if(NOT _api MATCHES "${_name}[(]" OR
           NOT _body MATCHES "Lufia2ExecutionResult[ \r\n]+${_name}[(]" OR
           NOT _contract MATCHES "address[ \t]*=[ \t]*\"${_address}\"" OR
           NOT _contract MATCHES "source[ \t]*=[ \t]*\"src/menu/special_menu[.]c\"" OR
           NOT _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" OR
           NOT _contract MATCHES "exit_mx[ \t]*=[ \t]*\"dynamic\"" OR
           NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"" OR
           NOT _binding MATCHES "bridge[ \t]*=[ \t]*\"${_bridge}\"")
            return()
        endif()
    endforeach()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
