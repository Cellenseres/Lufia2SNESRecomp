include_guard(GLOBAL)

function(lufia2_text_window_entry_available output target)
    set(${output} FALSE PARENT_SCOPE)
    if(NOT target STREQUAL "Lufia2Recomp" OR LUFIA2_DECOMP_REFERENCE_ONLY)
        return()
    endif()
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/text.h")
    set(_metadata_path "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    set(_bindings_path "${CMAKE_SOURCE_DIR}/recomp/decomp_bindings.toml")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_metadata_path}" OR
       NOT EXISTS "${_bindings_path}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_metadata_path}" _metadata)
    file(READ "${_bindings_path}" _bindings)
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2TextOpenWindowWithServices\"[^[]*"
        _contract "${_metadata}")
    string(REGEX MATCH
        "\\[\\[binding\\]\\][^[]*address[ \t]*=[ \t]*\"80:9D3E\"[^[]*"
        _binding "${_bindings}")
    if(NOT _api MATCHES "Lufia2TextOpenWindowWithServices[(]" OR
       NOT _contract MATCHES "address[ \t]*=[ \t]*\"80:9D3E\"" OR
       NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"" OR
       NOT _binding MATCHES "bridge[ \t]*=[ \t]*\"Lufia2DecompBridge_809D3E\"")
        return()
    endif()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
