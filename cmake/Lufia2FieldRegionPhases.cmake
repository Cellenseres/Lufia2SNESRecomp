include_guard(GLOBAL)

function(lufia2_field_region_phases_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/field.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/field/field_triggers.c")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}")
        return()
    endif()
    file(READ "${_header}" _declarations)
    file(READ "${_source}" _definitions)
    foreach(_symbol IN ITEMS Lufia2FieldPrepareRegionProbe
                            Lufia2FieldSelectRegionScript
                            Lufia2FieldSelectRegionScene)
        if(NOT _declarations MATCHES "${_symbol}\\(" OR
           NOT _definitions MATCHES "${_symbol}\\(")
            return()
        endif()
    endforeach()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
