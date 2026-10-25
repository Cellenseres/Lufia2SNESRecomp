include_guard(GLOBAL)

function(lufia2_table_motion_available output target)
    set(${output} FALSE PARENT_SCOPE)
    if(NOT target STREQUAL "Lufia2Recomp" OR LUFIA2_DECOMP_REFERENCE_ONLY)
        return()
    endif()
    set(_names Lufia2CardTableAdvanceColumns Lufia2CardTableRefreshColumns Lufia2CardTableDrawColumn Lufia2CardTableResetRoundBuffers Lufia2CardTableRestoreLineAttributes Lufia2CardTableAnimatePanelAttributes Lufia2CardTableAnimateLineAttributes Lufia2CardTableAnimateAccentAttributes Lufia2CardTablePaintTileAttributes)
    set(_headers card_table.h card_table.h card_table.h card_table.h card_table.h card_table.h card_table.h card_table.h card_table.h)
    set(_addresses "84:E9E3" "84:EA4B" "84:EAA5" "84:ED9E" "84:EDCB" "84:F034" "84:F09C" "84:F0D4" "84:F105")
    set(_bridges Lufia2DecompBridge_84E9E3 Lufia2DecompBridge_84EA4B Lufia2DecompBridge_84EAA5 Lufia2DecompBridge_84ED9E Lufia2DecompBridge_84EDCB Lufia2DecompBridge_84F034 Lufia2DecompBridge_84F09C Lufia2DecompBridge_84F0D4 Lufia2DecompBridge_84F105)
    set(_metadata_path "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    set(_bindings_path "${CMAKE_SOURCE_DIR}/recomp/decomp_bindings.toml")
    if(NOT EXISTS "${_metadata_path}" OR NOT EXISTS "${_bindings_path}")
        return()
    endif()
    file(READ "${_metadata_path}" _metadata)
    file(READ "${_bindings_path}" _bindings)
    foreach(_index RANGE 0 8)
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
