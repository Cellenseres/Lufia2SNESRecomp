include_guard(GLOBAL)

function(lufia2_session_actor_spans_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/session_actors.h")
    if(NOT EXISTS "${_header}")
        return()
    endif()
    file(READ "${_header}" _api)
    foreach(_function IN ITEMS Lufia2FieldRestoreSessionActor
            Lufia2FieldRestoreReservedActor Lufia2FieldSelectSessionPresentation)
        if(NOT _api MATCHES "${_function}[(]")
            return()
        endif()
    endforeach()
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml" _metadata)
    foreach(_pair IN ITEMS "Lufia2FieldBeginSessionSetup|83:ACB7"
            "Lufia2FieldResumeSessionSetup|83:AD23" "Lufia2FieldSelectSessionSetup|83:ACEF")
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
