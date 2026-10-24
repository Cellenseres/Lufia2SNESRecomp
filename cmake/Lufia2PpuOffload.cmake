include_guard(GLOBAL)
function(lufia2_add_ppu_offload target core_root)
    get_target_property(_sources ${target} SOURCES)
    get_target_property(_target_includes ${target} INCLUDE_DIRECTORIES)
    set(_result)
    set(_count 0)
    foreach(_source IN LISTS _sources)
        if(_source MATCHES "/ppu[.]c$")
            set(_output "${CMAKE_BINARY_DIR}/generated/ppu-offload/ppu.c")
            execute_process(COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/scripts/runtime_ppu_offload.py"
                --input "${_source}" --output "${_output}"
                COMMAND_ERROR_IS_FATAL ANY)
            get_source_file_property(_includes "${_source}" INCLUDE_DIRECTORIES)
            if(_includes STREQUAL "NOTFOUND")
                set(_includes "")
            endif()
            set_source_files_properties("${_output}" PROPERTIES INCLUDE_DIRECTORIES
                "${_target_includes};${_includes};${core_root}/runner/src/snes;${core_root}/runner/src;${CMAKE_SOURCE_DIR}/src")
            list(APPEND _result "${_output}")
            math(EXPR _count "${_count} + 1")
        else()
            list(APPEND _result "${_source}")
        endif()
    endforeach()
    if(NOT _count EQUAL 1)
        message(FATAL_ERROR "PPU offload requires exactly one current PPU source")
    endif()
    set_property(TARGET ${target} PROPERTY SOURCES "${_result}")
    file(READ "${core_root}/runner/src/snes/snes.h" _snes_header)
    file(READ "${core_root}/runner/src/snes/dma.h" _dma_header)
    if(NOT _snes_header MATCHES "snes_set_hdma_beam_enabled" OR
       NOT _dma_header MATCHES "dma_hdma_pending_init_set")
        message(FATAL_ERROR "Native PPU timeline requires the pinned core's HDMA ownership API")
    endif()
    target_compile_definitions(${target} PRIVATE LUFIA2_NATIVE_PPU_TIMELINE=1)
    option(LUFIA2_OBJ_DEFER_CHECK "Compare helper sprite rows with serial rows" OFF)
    if(LUFIA2_OBJ_DEFER_CHECK)
        target_compile_definitions(${target} PRIVATE LUFIA2_OBJ_DEFER_CHECK=1)
    endif()
    target_sources(${target} PRIVATE
        "${CMAKE_SOURCE_DIR}/src/lufia2_ppu_timeline.c"
        "${CMAKE_SOURCE_DIR}/src/lufia2_ppu_offload.c"
        "${CMAKE_SOURCE_DIR}/src/lufia2_ppu_obj_defer.c"
        "${CMAKE_SOURCE_DIR}/src/lufia2_intro_margin_job.c"
        "${CMAKE_SOURCE_DIR}/src/lufia2_intro_tree_job.c"
        "${CMAKE_SOURCE_DIR}/src/lufia2_intro_ppu_job.c")
    target_sources(${target} PRIVATE "${CMAKE_SOURCE_DIR}/src/lufia2_battle_ppu_job.c")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/scripts/runtime_ppu_offload.py")
endfunction()
