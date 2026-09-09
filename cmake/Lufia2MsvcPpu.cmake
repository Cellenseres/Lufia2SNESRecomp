include_guard(GLOBAL)

# The pinned core uses a GNU-only suffix attribute on PpuPixelPrioBufs. Preserve its
# required alignment on MSVC without modifying the downloaded runner tree.
function(lufia2_target_msvc_ppu target snesrecomp_root)
    if(NOT CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        return()
    endif()

    set(_source "${snesrecomp_root}/runner/src/snes/ppu.h")
    file(READ "${_source}" _header)
    set(_begin "typedef struct PpuPixelPrioBufs {")
    set(_end "} __attribute__((aligned(8))) PpuPixelPrioBufs;")
    foreach(_anchor IN ITEMS "${_begin}" "${_end}"
            "#include \"../types.h\"" "#include \"saveload.h\"")
        string(FIND "${_header}" "${_anchor}" _pos)
        if(_pos EQUAL -1)
            message(FATAL_ERROR
                "The pinned ppu.h MSVC alignment context changed: ${_anchor}")
        endif()
    endforeach()
    string(REPLACE "${_begin}"
        "typedef struct __declspec(align(8)) PpuPixelPrioBufs {"
        _header "${_header}")
    string(REPLACE "${_end}" "} PpuPixelPrioBufs;" _header "${_header}")
    # The relocated header must resolve these two includes in the original tree.
    string(REPLACE "#include \"../types.h\""
        "#include \"${snesrecomp_root}/runner/src/types.h\""
        _header "${_header}")
    string(REPLACE "#include \"saveload.h\""
        "#include \"${snesrecomp_root}/runner/src/snes/saveload.h\""
        _header "${_header}")

    set(_overlay_dir "${CMAKE_BINARY_DIR}/generated/lufia2-msvc")
    set(_overlay "${_overlay_dir}/ppu.h")
    file(MAKE_DIRECTORY "${_overlay_dir}")
    file(WRITE "${_overlay}"
        "#ifdef __cplusplus\nextern \"C\" {\n#endif\n"
        "${_header}\n"
        "#ifdef __cplusplus\n}\n"
        "static_assert(alignof(PpuPixelPrioBufs) >= 8, \"PPU buffer alignment\");\n"
        "#else\n"
        "_Static_assert(__alignof(PpuPixelPrioBufs) >= 8, \"PPU buffer alignment\");\n"
        "#endif\n")
    # /I alone cannot override quoted includes beside the upstream .c files.
    # Load the corrected definition first; PPU_H then guards every later include.
    # Apply it consistently to C and C++ consumers so their Ppu layouts agree.
    target_compile_options(${target} PRIVATE
        "$<$<COMPILE_LANGUAGE:C,CXX>:/FI${_overlay}>")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_source}")
endfunction()
