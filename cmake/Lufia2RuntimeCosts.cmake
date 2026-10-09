include_guard(GLOBAL)

option(LUFIA2_ENABLE_LOGO_WAIT_FASTFORWARD "Batch the original logo NMI wait within frame deadlines" ON)
option(LUFIA2_ENABLE_PPU_PIXEL_CACHE "Cache software-renderer CGRAM conversion" ON)
option(LUFIA2_AUDIO_COUNTERS_ONLY "Keep audio counters without diagnostic PCM/event histories" ON)
# Set before every platform includes runner.cmake. This changes diagnostics,
# not SPC/DSP execution, the playback ring, NDSP or MSU streaming.
if(LUFIA2_AUDIO_COUNTERS_ONLY)
    set(SNESRECOMP_AUDIO_TRACE_HISTORY COUNTERS CACHE STRING "Audio diagnostic history mode" FORCE)
elseif(NOT DEFINED SNESRECOMP_AUDIO_TRACE_HISTORY OR SNESRECOMP_AUDIO_TRACE_HISTORY STREQUAL "COUNTERS")
    set(SNESRECOMP_AUDIO_TRACE_HISTORY SMALL CACHE STRING "Audio diagnostic history mode" FORCE)
endif()
