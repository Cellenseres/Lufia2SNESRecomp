include_guard(GLOBAL)

function(lufia2_field_frame_boundaries_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/field_phase.h")
    if(NOT EXISTS "${_header}")
        return()
    endif()
    file(READ "${_header}" _api)
    if(NOT _api MATCHES "Lufia2FieldFrameStep[(]")
        return()
    endif()
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml" _metadata)
    foreach(_pair IN ITEMS
            "Lufia2FieldRunLoop|83:8000"
            "Lufia2FieldResumeFrame|83:8007")
        string(REPLACE "|" ";" _parts "${_pair}")
        list(GET _parts 0 _name)
        list(GET _parts 1 _address)
        string(REGEX MATCH
            "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"${_name}\"[^[]*"
            _contract "${_metadata}")
        if(NOT _contract MATCHES "address[ \t]*=[ \t]*\"${_address}\"" OR
           NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
            return()
        endif()
    endforeach()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
