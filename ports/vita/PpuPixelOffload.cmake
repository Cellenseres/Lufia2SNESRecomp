# Narrow adaptation of the development host's advance/render separation.
# Keep the pinned PPU's line setup and sprite/overflow evaluation intact;
# defer only background drawing and final pixel composition to GXM.
include("${CMAKE_SOURCE_DIR}/cmake/Lufia2PpuMosaic.cmake")
function(lufia2_prepare_ppu_pixel_offload sources_var core_root)
    set(_original "${core_root}/runner/src/snes/ppu.c")
    file(READ "${_original}" _source)
    lufia2_fix_ppu_4bpp_mosaic_palette(_source)
    set(_old [=[
    if (ppu->renderFlags & kPpuRenderFlags_NewRenderer) {
      PPU_T0; PpuDrawWholeLine(ppu, line); PPU_ACC(g_ppu_sec_line_ms);
]=])
    string(FIND "${_source}" "${_old}" _at)
    if(_at EQUAL -1)
        message(FATAL_ERROR "Pinned PPU pixel-composition anchor changed")
    endif()
    string(REPLACE "${_old}" "" _without "${_source}")
    # Count by length as REPLACE removes every occurrence.
    string(LENGTH "${_source}" _length)
    string(LENGTH "${_without}" _remaining)
    string(LENGTH "${_old}" _anchor_length)
    math(EXPR _removed "${_length} - ${_remaining}")
    if(NOT _removed EQUAL _anchor_length)
        message(FATAL_ERROR "Pinned PPU pixel-composition anchor is ambiguous")
    endif()
    set(_new [=[
    /* All guest-visible line state and sprite overflow were evaluated above.
     * The host enables this only for stable-memory frames without IRQs.
     * Admitted HDMA register writes are captured at each original line.
     * Unsupported/failed GPU draws replay pixels on a private PPU snapshot. */
    extern bool lufia2_gxm_defer_pixels;
    if (lufia2_gxm_defer_pixels)
      return;
    if (ppu->renderFlags & kPpuRenderFlags_NewRenderer) {
      PPU_T0; PpuDrawWholeLine(ppu, line); PPU_ACC(g_ppu_sec_line_ms);
]=])
    string(REPLACE "${_old}" "${_new}" _patched "${_source}")
    set(_directory "${CMAKE_BINARY_DIR}/generated/vita")
    file(MAKE_DIRECTORY "${_directory}")
    set(_overlay "${_directory}/ppu.c")
    file(WRITE "${_overlay}" "${_patched}")
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
