include_guard(GLOBAL)

function(_lufia2_count_literal out_var haystack needle)
    string(LENGTH "${haystack}" _before)
    string(REPLACE "${needle}" "" _without "${haystack}")
    string(LENGTH "${_without}" _after)
    string(LENGTH "${needle}" _needle_length)
    if(_needle_length EQUAL 0)
        message(FATAL_ERROR "Cannot count an empty PPU overlay anchor")
    endif()
    math(EXPR _count "(${_before} - ${_after}) / ${_needle_length}")
    set(${out_var} "${_count}" PARENT_SCOPE)
endfunction()

# Validate the palette-base calculation only inside the 4-bpp mosaic drawer.
# Older cores used the 2-bpp shift (8), which turns CGRAM palette offsets from
# 16-colour steps into 4-colour steps. The current pin already carries the
# corrected shift (6); retaining this fail-closed check prevents regression.
function(lufia2_fix_ppu_4bpp_mosaic_palette source_var)
    set(_source "${${source_var}}")
    set(_function_start "static void PpuDrawBackground_4bpp_mosaic(")
    set(_function_next "static void PpuDrawBackground_4bpp_policy(")

    string(FIND "${_source}" "${_function_start}" _start)
    string(FIND "${_source}" "${_function_next}" _end)
    if(_start EQUAL -1 OR _end EQUAL -1 OR _end LESS_EQUAL _start)
        message(FATAL_ERROR
            "The pinned 4-bpp mosaic function boundaries changed")
    endif()

    string(SUBSTRING "${_source}" 0 ${_start} _prefix)
    math(EXPR _body_length "${_end} - ${_start}")
    string(SUBSTRING "${_source}" ${_start} ${_body_length} _body)
    string(SUBSTRING "${_source}" ${_end} -1 _suffix)

    set(_wrong "  enum { kPaletteShift = 8 };")
    set(_correct "  enum { kPaletteShift = 6 };")
    _lufia2_count_literal(_wrong_count "${_body}" "${_wrong}")
    _lufia2_count_literal(_correct_count "${_body}" "${_correct}")

    if(_wrong_count EQUAL 1 AND _correct_count EQUAL 0)
        string(REPLACE "${_wrong}" "${_correct}" _body "${_body}")
    elseif(_wrong_count EQUAL 0 AND _correct_count EQUAL 1)
        # A future core may already contain the fix.
    else()
        message(FATAL_ERROR
            "The pinned 4-bpp mosaic palette anchor is missing or ambiguous "
            "(wrong=${_wrong_count}, correct=${_correct_count})")
    endif()

    set(${source_var} "${_prefix}${_body}${_suffix}" PARENT_SCOPE)
endfunction()

# Desktop builds compile the corrected immutable copy. Vita folds the same
# source transformation into its existing pixel-offload overlay.
function(lufia2_prepare_ppu_mosaic_overlay sources_var core_root)
    set(_original "${core_root}/runner/src/snes/ppu.c")
    if(NOT EXISTS "${_original}")
        message(FATAL_ERROR "Missing snesrecomp PPU source: ${_original}")
    endif()

    file(READ "${_original}" _source)
    lufia2_fix_ppu_4bpp_mosaic_palette(_source)

    set(_directory "${CMAKE_BINARY_DIR}/generated/lufia2-ppu-fixes/snes")
    set(_overlay "${_directory}/ppu.c")
    file(MAKE_DIRECTORY "${_directory}")
    file(WRITE "${_overlay}" "${_source}")
    set_source_files_properties("${_overlay}" PROPERTIES INCLUDE_DIRECTORIES
        "${core_root}/runner/src/snes;${core_root}/runner/src")

    set(_sources "${${sources_var}}")
    list(FIND _sources "${_original}" _index)
    if(_index EQUAL -1)
        message(FATAL_ERROR "Expected original PPU translation unit missing")
    endif()
    list(REMOVE_ITEM _sources "${_original}")
    list(APPEND _sources "${_overlay}")
    set(${sources_var} "${_sources}" PARENT_SCOPE)
endfunction()
