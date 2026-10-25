include_guard(GLOBAL)

function(lufia2_table_symbols_available output target)
    set(${output} FALSE PARENT_SCOPE)
    if(NOT target STREQUAL "Lufia2Recomp" OR LUFIA2_DECOMP_REFERENCE_ONLY)
        return()
    endif()
    set(_names Lufia2CardTableStopColumn Lufia2CardTablePrepareColumnSymbols Lufia2CardTableFindMatchingSymbol Lufia2CardTableRememberRoundSymbol Lufia2CardTableBuildColumnMatches Lufia2CardTableReadMatchColumn Lufia2CardTableAppendMatchSymbol Lufia2CardTableStartColumns)
    set(_headers card_table.h card_table.h card_table.h card_table.h card_table.h card_table.h card_table.h card_table.h)
    set(_addresses "84:E5B5" "84:E640" "84:E673" "84:E817" "84:E829" "84:E906" "84:E91B" "84:E92A")
    set(_bridges Lufia2DecompBridge_84E5B5 Lufia2DecompBridge_84E640 Lufia2DecompBridge_84E673 Lufia2DecompBridge_84E817 Lufia2DecompBridge_84E829 Lufia2DecompBridge_84E906 Lufia2DecompBridge_84E91B Lufia2DecompBridge_84E92A)
    set(_metadata_path "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    set(_bindings_path "${CMAKE_SOURCE_DIR}/recomp/decomp_bindings.toml")
    if(NOT EXISTS "${_metadata_path}" OR NOT EXISTS "${_bindings_path}")
        return()
    endif()
    file(READ "${_metadata_path}" _metadata)
    file(READ "${_bindings_path}" _bindings)
    foreach(_index RANGE 0 7)
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
