include_guard(GLOBAL)

function(lufia2_menu_leaves_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/menu.h")
    set(_metadata_file "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_metadata_file}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_metadata_file}" _metadata)
    foreach(_item IN ITEMS
        "80:8AF7|Lufia2MenuResolvePointerOffset|menu_string"
        "82:F4D7|Lufia2MenuCycleNameEntrySelection|name_entry")
        string(REPLACE "|" ";" _fields "${_item}")
        list(GET _fields 0 _address)
        list(GET _fields 1 _name)
        list(GET _fields 2 _stem)
        set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/menu/${_stem}.c")
        if(NOT EXISTS "${_source}")
            return()
        endif()
        file(READ "${_source}" _implementation)
        string(REGEX MATCH
            "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"${_name}\"[^[]*"
            _contract "${_metadata}")
        if(NOT _api MATCHES "${_name}[(]" OR
           NOT _implementation MATCHES "${_name}[(]" OR
           NOT _contract MATCHES "address[ \t]*=[ \t]*\"${_address}\"" OR
           NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"" OR
           NOT _contract MATCHES "source[ \t]*=[ \t]*\"src/menu/${_stem}[.]c\"" OR
           NOT _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" OR
           NOT _contract MATCHES "exit_mx[ \t]*=[ \t]*\"M1X0\"")
            return()
        endif()
    endforeach()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
