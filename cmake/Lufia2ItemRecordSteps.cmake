include_guard(GLOBAL)

function(lufia2_item_record_steps_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/item_record_steps.h")
    if(NOT EXISTS "${_header}")
        return()
    endif()
    file(READ "${_header}" _api)
    if(NOT _api MATCHES "Lufia2ItemBeginRecordRead[(]" OR
       NOT _api MATCHES "Lufia2ItemFinishRecordRead[(]")
        return()
    endif()
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml" _metadata)
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2ItemRecordByte\"[^[]*"
        _contract "${_metadata}")
    if(_contract MATCHES "address[ \t]*=[ \t]*\"81:F194\"" AND
       _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()
