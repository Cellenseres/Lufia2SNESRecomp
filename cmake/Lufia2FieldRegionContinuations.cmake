include_guard(GLOBAL)

function(lufia2_field_region_continuations_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/field_region_continuations.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/field/field_triggers.c")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}")
        return()
    endif()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_header}" "${_source}")
    file(READ "${_header}" _api)
    file(READ "${_source}" _body)
    if(NOT _api MATCHES "#define[ \t]+LUFIA2_FIELD_REGION_CONTINUATIONS[ \t]+1([ \t\r\n]|$)" OR
       NOT _api MATCHES "Lufia2FieldContinueRegionProbe[(]" OR
       NOT _body MATCHES "Lufia2ExecutionResult[ \r\n]+Lufia2FieldContinueRegionProbe[(]")
        return()
    endif()
    set(_metadata "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_metadata}")
        return()
    endif()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_metadata}")
    file(READ "${_metadata}" _contracts)
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2FieldTriggerUpdateWithServices\"[^[]*"
        _contract "${_contracts}")
    if(NOT _contract MATCHES "address[ \t]*=[ \t]*\"83:81C6\"" OR
       NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        return()
    endif()
    set(${output} TRUE PARENT_SCOPE)
endfunction()

function(lufia2_compose_field_region_continuations output source)
    set(_marker "        const uint8_t  op = bridge_bus_read(cpu, pc_before);")
    string(REGEX MATCHALL "const uint8_t  op = bridge_bus_read[(]cpu, pc_before[)]"
        _matches "${source}")
    list(LENGTH _matches _count)
    string(FIND "${source}" "${_marker}" _position)
    if(NOT _count EQUAL 1 OR _position EQUAL -1 OR
       source MATCHES "field_region_owner[.]inc|Lufia2DecompFieldRegionCanResume")
        message(FATAL_ERROR "Field region continuation requires one reviewed opcode boundary")
    endif()
    string(REPLACE "${_marker}"
        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/field_region_owner.inc\"\n${_marker}"
        _text "${source}")
    string(PREPEND _text
        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/field_region_owner.h\"\n")
    set(${output} "${_text}" PARENT_SCOPE)
endfunction()
