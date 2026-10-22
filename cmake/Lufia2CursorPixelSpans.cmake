include_guard(GLOBAL)

function(lufia2_cursor_pixel_spans_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/menu_cursor_step.h")
    if(NOT EXISTS "${_header}")
        return()
    endif()
    file(READ "${_header}" _api)
    if(NOT _api MATCHES "Lufia2MenuAdvanceCursorSlide[(]")
        return()
    endif()
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml" _metadata)
    foreach(_address IN ITEMS "82:89FA" "82:8AD6" "82:8A9B")
        string(REGEX MATCH
            "\\[\\[function\\]\\][^[]*address[ \t]*=[ \t]*\"${_address}\"[^[]*"
            _contract "${_metadata}")
        if(NOT _contract MATCHES "source[ \t]*=[ \t]*\"src/menu/menu_cursor_slide[.]c\"" OR
           NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"" OR
           NOT _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" OR
           NOT _contract MATCHES "exit_mx[ \t]*=[ \t]*\"M1X0\"")
            return()
        endif()
    endforeach()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
