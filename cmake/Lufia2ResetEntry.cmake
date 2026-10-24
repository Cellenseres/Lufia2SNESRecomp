include_guard(GLOBAL)

function(lufia2_reset_entry_available output target)
    set(${output} FALSE PARENT_SCOPE)
    if(NOT target STREQUAL "Lufia2Recomp" OR LUFIA2_DECOMP_REFERENCE_ONLY)
        return()
    endif()
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/system.h")
    set(_metadata_path "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    set(_bindings_path "${CMAKE_SOURCE_DIR}/recomp/decomp_bindings.toml")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_metadata_path}" OR
       NOT EXISTS "${_bindings_path}")
        return()
    endif()
    file(READ "${_header}" _api)
    if(NOT _api MATCHES "Lufia2ResetSystem[(][^;]*uint8_t[ \t\r\n]+entry_emulation[)]")
        return()
    endif()
    file(READ "${_metadata_path}" _metadata)
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2ResetSystem\"[^[]*"
        _contract "${_metadata}")
    if(NOT _contract MATCHES "address[ \t]*=[ \t]*\"80:8000\"" OR
       NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        return()
    endif()
    file(READ "${_bindings_path}" _bindings)
    string(REGEX MATCH
        "\\[\\[binding\\]\\][^[]*address[ \t]*=[ \t]*\"80:8000\"[^[]*"
        _binding "${_bindings}")
    if(_binding MATCHES "bridge[ \t]*=[ \t]*\"Lufia2DecompBridge_808000\"")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()
