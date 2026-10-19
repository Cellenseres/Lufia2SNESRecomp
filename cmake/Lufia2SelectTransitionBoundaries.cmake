include_guard(GLOBAL)

function(lufia2_select_transition_boundaries_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/select_phase.h")
    if(NOT EXISTS "${_header}")
        return()
    endif()
    file(READ "${_header}" _api)
    if(NOT _api MATCHES "Lufia2SelectTransitionStep[(]")
        return()
    endif()
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml" _metadata)
    foreach(_pair IN ITEMS
            "Lufia2SelectScreenFadeIn|86:8305"
            "Lufia2SelectScreenFadeOut|86:830F"
            "Lufia2SelectScreenWaitForInput|86:8339"
            "Lufia2SelectScreenWaitForObjects|86:82C8"
            "Lufia2SelectScreenResumeFade|86:8333"
            "Lufia2SelectScreenResumeInputWait|86:8352"
            "Lufia2SelectScreenResumeObjectWait|86:82CE"
            "Lufia2SelectScreenRepeatObjectWait|86:82FC")
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
