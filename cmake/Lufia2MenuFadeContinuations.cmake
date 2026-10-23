include_guard(GLOBAL)

function(lufia2_menu_fade_continuations_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/menu.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/menu/menu_presentation_callers.c")
    set(_metadata_path "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}" OR
       NOT EXISTS "${_metadata_path}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_source}" _implementation)
    if(NOT _api MATCHES "Lufia2MenuContinueDisplayFade[(]" OR
       NOT _implementation MATCHES "Lufia2MenuContinueDisplayFade[(][^;{]*[{]")
        return()
    endif()
    file(READ "${_metadata_path}" _metadata)
    set(_names Lufia2MenuFadeDisplayIn Lufia2MenuFadeDisplayOut)
    set(_addresses "86:8B1C" "86:8B32")
    foreach(_index RANGE 0 1)
        list(GET _names ${_index} _name)
        list(GET _addresses ${_index} _address)
        string(REGEX MATCH
            "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"${_name}\"[^[]*"
            _contract "${_metadata}")
        if(NOT _contract MATCHES "address[ \t]*=[ \t]*\"${_address}\"" OR
           NOT _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" OR
           NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
            return()
        endif()
    endforeach()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
