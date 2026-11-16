include_guard(GLOBAL)

function(lufia2_nmi_child_boundaries_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/nmi_calls.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/system/nmi.c")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}")
        return()
    endif()
    file(READ "${_header}" _declarations)
    file(READ "${_source}" _definitions)
    if(_declarations MATCHES "#define LUFIA2_NMI_CHILD_BOUNDARIES 1([ \t\r\n]|$)" AND
       _declarations MATCHES "Lufia2NmiReturnChild[(]" AND
       _definitions MATCHES "Lufia2NmiReturnChild[(]" AND
       _definitions MATCHES "case 0x8087f0u:")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()

function(lufia2_screen_fade_request_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/system.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/system/screen.c")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}")
        return()
    endif()
    file(READ "${_header}" _declarations)
    file(READ "${_source}" _definitions)
    if(_declarations MATCHES "Lufia2ScreenFadeCheckRequest[(]" AND
       _definitions MATCHES "Lufia2ScreenFadeCheckRequest[(]")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()
