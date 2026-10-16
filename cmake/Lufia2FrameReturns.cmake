include_guard(GLOBAL)

function(lufia2_frame_returns_available output target)
    set(${output} FALSE PARENT_SCOPE)
    if(NOT target STREQUAL "Lufia2Recomp")
        return()
    endif()
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml" _metadata)
    set(_names Lufia2WaitMenuFrameChange Lufia2MenuWaitForNextFrame
        Lufia2WorldWaitForNextFrame Lufia2SpriteFrame)
    set(_addresses "82:93C2" "86:8B48" "86:974F" "86:8B55")
    foreach(_index RANGE 0 3)
        list(GET _names ${_index} _name)
        list(GET _addresses ${_index} _address)
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
