include_guard(GLOBAL)

# Replace only the synchronous $420B drain loop.  The downloaded pinned
# source remains untouched; the compiled translation unit lives in the build
# tree and is pinned to the reviewed upstream hash and exact source seam.
function(lufia2_prepare_dma_host_fastforward sources_var core_root)
    set(_original "${core_root}/runner/src/snes/snes.c")
    if(NOT EXISTS "${_original}")
        message(FATAL_ERROR "Missing pinned snes.c: ${_original}")
    endif()

    file(SHA256 "${_original}" _hash)
    if(NOT _hash STREQUAL
           "08d99514a89a86e1dc73b646bf464d9c9501b7939e6d56b87f47ce5aa46e1155")
        message(FATAL_ERROR
            "DMA host fast-forward requires the reviewed pinned snes.c; review the timing contract before updating core")
    endif()

    file(READ "${_original}" _text)
    string(REPLACE "\r\n" "\n" _text "${_text}")
    set(_old "      while (dma_cycle(snes->dma)) {}")
    set(_new [=[      uint64_t l2_dma_idle_ticks = 0;
      while (L2DmaHostFastForwardCycle(snes->dma,
                                             &l2_dma_idle_ticks)) {}
      lufia2_dma_host_idle_ticks_skipped += l2_dma_idle_ticks;]=])

    string(REPLACE "${_old}" "" _removed "${_text}")
    string(LENGTH "${_text}" _before)
    string(LENGTH "${_removed}" _after)
    string(LENGTH "${_old}" _old_length)
    math(EXPR _delta "${_before} - ${_after}")
    if(NOT _delta EQUAL _old_length)
        message(FATAL_ERROR
            "Pinned synchronous DMA loop must occur exactly once")
    endif()
    string(REPLACE "${_old}" "${_new}" _text "${_text}")

    set(_directory "${CMAKE_BINARY_DIR}/generated/dma-host-fastforward")
    set(_overlay "${_directory}/snes.c")
    file(MAKE_DIRECTORY "${_directory}")
    file(WRITE "${_overlay}"
        "#include \"patches/dma_host_fastforward.h\"\n"
        "uint64_t lufia2_dma_host_idle_ticks_skipped = 0;\n"
        "uint64_t Lufia2DmaHostFastForwardIdleTicks(void) {\n"
        "  return lufia2_dma_host_idle_ticks_skipped;\n"
        "}\n"
        "${_text}")

    set(_sources "${${sources_var}}")
    list(FIND _sources "${_original}" _index)
    if(_index EQUAL -1)
        message(FATAL_ERROR
            "DMA host fast-forward expected the original snes.c exactly once")
    endif()
    list(REMOVE_AT _sources ${_index})
    list(INSERT _sources ${_index} "${_overlay}")
    foreach(_prop IN ITEMS COMPILE_DEFINITIONS COMPILE_OPTIONS COMPILE_FLAGS
            INCLUDE_DIRECTORIES LANGUAGE COMPILE_DEFINITIONS_RELEASE
            COMPILE_DEFINITIONS_DEBUG COMPILE_DEFINITIONS_RELWITHDEBINFO
            COMPILE_DEFINITIONS_MINSIZEREL)
        get_source_file_property(_value "${_original}" "${_prop}")
        if(NOT _value STREQUAL "NOTFOUND")
            set_property(SOURCE "${_overlay}" PROPERTY "${_prop}" "${_value}")
        endif()
    endforeach()
    set_property(SOURCE "${_overlay}" APPEND PROPERTY INCLUDE_DIRECTORIES
        "${CMAKE_SOURCE_DIR};${core_root}/runner/src/snes;${core_root}/runner/src")
    set(${sources_var} "${_sources}" PARENT_SCOPE)
endfunction()
