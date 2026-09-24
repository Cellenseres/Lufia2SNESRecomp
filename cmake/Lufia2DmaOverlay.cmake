include_guard(GLOBAL)

# Single owner of the fetched dma.c: overlays cannot be chained, so every
# seam that file needs is applied here, into one generated copy.
function(lufia2_prepare_dma_overlay sources_var core_root)
    set(_original "${core_root}/runner/src/snes/dma.c")
    if(NOT EXISTS "${_original}")
        message(FATAL_ERROR "Missing pinned dma.c: ${_original}")
    endif()

    if(NOT LUFIA2_ENABLE_DMA_DIRECT_SOURCE_READ)
        return()
    endif()

    file(SHA256 "${_original}" _hash)
    if(NOT _hash STREQUAL
           "3ad3add226c91330c756906617dc0094be7135f7f64d06af6b21adcd9641fa85")
        message(FATAL_ERROR
            "The dma.c overlay requires the reviewed pinned dma.c; review the "
            "bus contract, then accept ${_hash}")
    endif()

    file(READ "${_original}" _text)
    string(REPLACE "
" "
" _text "${_text}")
    set(_prologue)

    # Specialize only side-effect-free DMA source reads used by Lufia II's plain
    # LoROM cartridge. MMIO, SRAM, B-to-A DMA and all special cartridges keep
    # the upstream snes_read() path.
    if(LUFIA2_ENABLE_DMA_DIRECT_SOURCE_READ)
        set(_old "    val = snes_read(dma->snes, (aBank << 16) | aAdr);")
        set(_new [=[    if (!L2DmaHostDirectSourceRead(dma->snes, aBank, aAdr, &val))
      val = snes_read(dma->snes, (aBank << 16) | aAdr);]=])
        string(REPLACE "${_old}" "" _removed "${_text}")
        string(LENGTH "${_text}" _before)
        string(LENGTH "${_removed}" _after)
        string(LENGTH "${_old}" _old_length)
        math(EXPR _delta "${_before} - ${_after}")
        if(NOT _delta EQUAL _old_length)
            message(FATAL_ERROR
                "Pinned A-to-B DMA source read must occur exactly once")
        endif()
        string(REPLACE "${_old}" "${_new}" _text "${_text}")
        set(_prologue "#include \"patches/dma_host_fastforward.h\"
")
    endif()

    set(_directory "${CMAKE_BINARY_DIR}/generated/dma-overlay")
    set(_overlay "${_directory}/dma.c")
    file(MAKE_DIRECTORY "${_directory}")
    file(WRITE "${_overlay}" "${_prologue}${_text}")

    set(_sources "${${sources_var}}")
    list(FIND _sources "${_original}" _index)
    if(_index EQUAL -1)
        message(FATAL_ERROR "The dma.c overlay expected the original exactly once")
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
