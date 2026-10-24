include_guard(GLOBAL)

function(lufia2_add_ppu_host_support target)
    # Consumers also query the host policy when the optional generator patch is
    # disabled. Link the real shared module in both configurations, once only.
    get_target_property(_sources ${target} SOURCES)
    foreach(_name lufia2_ppu_host lufia2_ppu_obj_capture)
        set(_found FALSE)
        foreach(_source IN LISTS _sources)
            if(_source MATCHES "(^|/)${_name}[.]c$")
                set(_found TRUE)
            endif()
        endforeach()
        if(NOT _found)
            target_sources(${target} PRIVATE "${CMAKE_SOURCE_DIR}/src/${_name}.c")
        endif()
    endforeach()
endfunction()

function(lufia2_add_ppu_host_costs target core_root)
    get_target_property(_sources ${target} SOURCES)
    get_target_property(_target_includes ${target} INCLUDE_DIRECTORIES)
    set(_result)
    set(_count 0)
    foreach(_source IN LISTS _sources)
        if(_source MATCHES "/ppu[.]c$")
            set(_output "${CMAKE_BINARY_DIR}/generated/ppu-host/ppu.c")
            execute_process(COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/scripts/runtime_ppu_host.py"
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
        message(FATAL_ERROR "PPU host costs require exactly one PPU source")
    endif()
    set_property(TARGET ${target} PROPERTY SOURCES "${_result}")
    lufia2_add_ppu_host_support(${target})
    target_compile_definitions(${target} PRIVATE LUFIA2_PPU_HOST_PATCH=1)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/scripts/runtime_ppu_host.py"
        "${CMAKE_SOURCE_DIR}/scripts/runtime_ppu_colour.py")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/scripts/runtime_ppu_menu_colour.py")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/scripts/runtime_ppu_retained.py")
endfunction()
