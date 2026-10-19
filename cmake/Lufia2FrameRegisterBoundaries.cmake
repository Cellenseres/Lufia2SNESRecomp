include_guard(GLOBAL)

function(lufia2_frame_register_boundaries_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/frame_wait.h")
    if(NOT EXISTS "${_header}")
        return()
    endif()
    file(READ "${_header}" _api)
    if(NOT _api MATCHES "Lufia2FrameWaitRegisterStep[(]" OR
       NOT _api MATCHES "Lufia2SpriteFrameRegisterStep[(]")
        return()
    endif()
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml" _metadata)
    foreach(_pair IN ITEMS
            "Lufia2WaitMenuFrameChange|82:93C2"
            "Lufia2MenuWaitForNextFrame|86:8B48"
            "Lufia2WorldWaitForNextFrame|86:974F"
            "Lufia2WaitFramePreserveStatus|80:9378"
            "Lufia2WaitFramePreserveAccumulator|84:A5A8"
            "Lufia2SpriteFrame|86:8B55")
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
