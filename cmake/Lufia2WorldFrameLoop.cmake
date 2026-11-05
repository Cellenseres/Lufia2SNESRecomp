include_guard(GLOBAL)

function(lufia2_world_frame_loop_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/world_scene_returns.h")
    if(NOT EXISTS "${_header}")
        return()
    endif()
    file(READ "${_header}" _api)
    if(NOT _api MATCHES "#define[ \t]+LUFIA2_WORLD_FRAME_LOOP_API[ \t]+1" OR
       NOT _api MATCHES "Lufia2WorldMapContinueSession[(]")
        return()
    endif()
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml" _metadata)
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2WorldMapRunSession\"[^[]*"
        _contract "${_metadata}")
    if(_contract MATCHES "address[ \t]*=[ \t]*\"86:919E\"" AND
       _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()

function(lufia2_compose_world_frame_loop output source)
    set(_marker "        const uint8_t  op = bridge_bus_read(cpu, pc_before);")
    string(REGEX MATCHALL "const uint8_t  op = bridge_bus_read[(]cpu, pc_before[)]"
        _matches "${source}")
    list(LENGTH _matches _count)
    string(FIND "${source}" "${_marker}" _position)
    if(NOT _count EQUAL 1 OR _position EQUAL -1 OR
       source MATCHES "world_frame_owner[.]inc|Lufia2DecompWorldFrameCanResume")
        message(FATAL_ERROR "World frame loop requires one reviewed opcode boundary")
    endif()
    string(REPLACE "${_marker}"
        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/world_frame_owner.inc\"\n${_marker}"
        _text "${source}")
    string(PREPEND _text
        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/world_frame_owner.h\"\n")
    set(${output} "${_text}" PARENT_SCOPE)
endfunction()
