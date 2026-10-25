include_guard(GLOBAL)

function(lufia2_table_services_available output target)
    set(${output} FALSE PARENT_SCOPE)
    if(NOT target STREQUAL "Lufia2Recomp" OR LUFIA2_DECOMP_REFERENCE_ONLY)
        return()
    endif()
    set(_names Lufia2CardTableUpdateHdma Lufia2CardTableBuildHdma Lufia2CardTableLoadAlternateTiles Lufia2CardTableInitialize Lufia2CardTableLoadGraphics Lufia2CardTableLoadAlternateGraphics Lufia2CardTableWaitFrame)
    set(_headers card_table.h card_table.h card_table.h card_table.h card_table.h card_table.h card_table.h)
    set(_addresses "84:F12F" "84:F173" "84:F78F" "84:EC4F" "84:EDFA" "84:F7D3" "84:E9B1")
    set(_bridges Lufia2DecompBridge_84F12F Lufia2DecompBridge_84F173 Lufia2DecompBridge_84F78F Lufia2DecompBridge_84EC4F Lufia2DecompBridge_84EDFA Lufia2DecompBridge_84F7D3 Lufia2DecompBridge_84E9B1)
    set(_metadata_path "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    set(_bindings_path "${CMAKE_SOURCE_DIR}/recomp/decomp_bindings.toml")
    if(NOT EXISTS "${_metadata_path}" OR NOT EXISTS "${_bindings_path}")
        return()
    endif()
    file(READ "${_metadata_path}" _metadata)
    file(READ "${_bindings_path}" _bindings)
    foreach(_index RANGE 0 6)
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
