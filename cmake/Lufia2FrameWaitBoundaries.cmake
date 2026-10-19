include_guard(GLOBAL)

function(lufia2_frame_wait_boundaries_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/frame_wait.h")
    if(NOT EXISTS "${_header}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/system.h" _system_api)
    if(NOT _api MATCHES "Lufia2FrameWaitReturnStep[(]" OR
       NOT _system_api MATCHES "Lufia2FrameWaitAdditionalStep[(]")
        return()
    endif()
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml" _metadata)
    foreach(_pair IN ITEMS
            "Lufia2WaitMenuFrameChange|82:93C2"
            "Lufia2FieldWaitForNextFrame|83:900C"
            "Lufia2ItemRegionWaitForNextFrame|8E:B550"
            "Lufia2MenuWaitForNextFrame|86:8B48"
            "Lufia2WorldWaitForNextFrame|86:974F"
            "Lufia2TextWaitForNextFrame|80:BF0B"
            "Lufia2AncientCaveWaitForNextFrame|84:8D4D"
            "Lufia2WaitFramePreserveStatus|80:9378"
            "Lufia2FieldWaitSelectionFrame|84:8363"
            "Lufia2WaitFramePreserveAccumulator|84:A5A8"
            "Lufia2FieldDiagnosticsWaitFrame|85:851C"
            "Lufia2BattleFrameInput|85:EC81"
            "Lufia2WaitForNextFrame|8E:B288"
            "Lufia2FieldApplyObjectReward|8E:C1DE")
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
