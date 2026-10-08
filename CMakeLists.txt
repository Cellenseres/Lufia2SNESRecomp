cmake_minimum_required(VERSION 3.24)
project(Lufia2Recomp C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("${CMAKE_SOURCE_DIR}/cmake/Lufia2Decomp.cmake")

option(LUFIA2_ENABLE_PERF_AUDIT "Finite Vita performance audit; no AOT promotion" OFF)
if(LUFIA2_ENABLE_PERF_AUDIT AND NOT VITA)
    message(FATAL_ERROR "LUFIA2_ENABLE_PERF_AUDIT currently supports the Vita host only")
endif()

option(LUFIA2_ENABLE_GAMEPLAY_CAPTURE "Observational desktop gameplay corpus; no AOT promotion" OFF)
if(LUFIA2_ENABLE_GAMEPLAY_CAPTURE AND VITA)
    message(FATAL_ERROR "Use the desktop host for GAMEPLAY_CAPTURE; Vita has PERF_AUDIT")
endif()

option(LUFIA2_ENABLE_NATIVE_WAIT "Reviewed native frame-wait instruction payload (experimental)" OFF)
# Runtime default when LUFIA2_NATIVE_WAIT is unset. A console has no
# launcher script, so an opt-in build measures both arms by itself.
set(LUFIA2_NATIVE_WAIT_DEFAULT "auto" CACHE STRING
    "Payload mode when the environment selects none: auto, native or original")
set_property(CACHE LUFIA2_NATIVE_WAIT_DEFAULT PROPERTY STRINGS auto native original)
if(LUFIA2_NATIVE_WAIT_DEFAULT STREQUAL "auto")
    set(LUFIA2_NATIVE_WAIT_DEFAULT_MODE 2)
elseif(LUFIA2_NATIVE_WAIT_DEFAULT STREQUAL "native")
    set(LUFIA2_NATIVE_WAIT_DEFAULT_MODE 1)
elseif(LUFIA2_NATIVE_WAIT_DEFAULT STREQUAL "original")
    set(LUFIA2_NATIVE_WAIT_DEFAULT_MODE 0)
else()
    message(FATAL_ERROR "LUFIA2_NATIVE_WAIT_DEFAULT must be auto, native or original")
endif()
if(LUFIA2_ENABLE_NATIVE_WAIT AND (LUFIA2_ENABLE_GAMEPLAY_CAPTURE OR LUFIA2_ENABLE_PERF_AUDIT))
    message(FATAL_ERROR "Use a separate NativeWait build; interpreter-only audits do not count native payload instructions")
endif()

# Reviewed native patches that belong to the ordinary build. The audit
# experiments below measure against them and so must be configured with these
# OFF, which is also how the reference arm is built.
option(LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD
    "Guarded portable batching for reviewed Lufia II frame waits" ON)

option(LUFIA2_ENABLE_ACTOR_EARLY_RETURN
    "Interpreter actor fast paths for non-decomp/reference builds" ON)

option(LUFIA2_ENABLE_DMA_HOST_FASTFORWARD
    "Collapse host-only idle iterations in synchronous SNES DMA" ON)

option(LUFIA2_ENABLE_DMA_DIRECT_SOURCE_READ
    "Direct side-effect-free WRAM/LoROM DMA source reads" ON)

# Only cfg entries whose 65816 call/return contract was reviewed belong here.
# This deliberately does not consume a runtime coverage JSON/JSONL file.
option(LUFIA2_ENABLE_REVIEWED_AOT
    "Compile reviewed Lufia II CFG roots without profile promotion" ON)

option(LUFIA2_ENABLE_BRIDGE_AUDIT "Sampled desktop bridge phase diagnosis" OFF)
if(LUFIA2_ENABLE_BRIDGE_AUDIT AND (VITA OR NOT LUFIA2_ENABLE_NATIVE_WAIT))
    message(FATAL_ERROR "BridgeAudit requires desktop NativeWait; it does not enable the native runtime mode")
endif()

option(LUFIA2_ENABLE_QUIESCENCE_INDEX "Exact candidate index for bridge state ring (experimental)" OFF)
# Optional scheduler reporting for development measurements.
option(LUFIA2_ENABLE_PATCH_REPORTING
    "Report patch counters and scheduler time every 120 boundaries" OFF)
# Progress and state tracing from the game code itself: map changes, MSU track
# selection, widescreen decisions and the periodic CPU state dump. A shipped
# build leaves this OFF; errors are reported either way.
option(LUFIA2_ENABLE_RUNTIME_LOG "Progress and state tracing to stderr" OFF)
# Per-PC interpreter histogram at exit (SNESRECOMP_INTERP_MS_PROF=1 adds
# host time). A compile definition, not a C flag: the Visual Studio
# generator drops C-only flags from targets that also compile C++.
option(LUFIA2_ENABLE_INTERP_PROFILE "Interpreter PC histogram (profiling builds)" OFF)
if(LUFIA2_ENABLE_INTERP_PROFILE OR CMAKE_C_FLAGS MATCHES "SNESRECOMP_INTERP_PROFILE")
    add_compile_definitions(SNESRECOMP_INTERP_PROFILE)
endif()
# Desktop only, and not because of the compiler: the Vita bridge overlay in
# ports/vita/InterpreterHostCosts.cmake already filters the same ring
# with a fingerprint array and a refcounted presence map. Layering this
# index on top would measure nothing and its anchors no longer match.
if(LUFIA2_ENABLE_QUIESCENCE_INDEX AND (VITA OR NOT LUFIA2_ENABLE_NATIVE_WAIT OR LUFIA2_ENABLE_BRIDGE_AUDIT))
    message(FATAL_ERROR "QuiescenceIndex needs desktop NativeWait with BridgeAudit OFF; the Vita bridge already filters the ring")
endif()

# Keep the ordinary build separate from the earlier measurement builds.
# Place this check after every referenced option has been declared so defaults
# and command-line cache values are treated identically.
if(LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD AND
   (LUFIA2_ENABLE_NATIVE_WAIT OR LUFIA2_ENABLE_GAMEPLAY_CAPTURE OR
    LUFIA2_ENABLE_PERF_AUDIT OR LUFIA2_ENABLE_BRIDGE_AUDIT OR
    LUFIA2_ENABLE_QUIESCENCE_INDEX))
    message(FATAL_ERROR
        "FrameWaitFastForward is part of the ordinary build; disable audit experiments")
endif()

# Runtime default when LUFIA2_QUIESCENCE_INDEX is unset. The Vita has no
# launcher script, so its measurement build alternates both modes by itself.
set(LUFIA2_QUIESCENCE_INDEX_DEFAULT "auto" CACHE STRING
    "Index mode when the environment selects none: auto, indexed or original")
set_property(CACHE LUFIA2_QUIESCENCE_INDEX_DEFAULT PROPERTY STRINGS auto indexed original)
if(LUFIA2_QUIESCENCE_INDEX_DEFAULT STREQUAL "auto")
    set(LUFIA2_QUIESCENCE_INDEX_DEFAULT_MODE 2)
elseif(LUFIA2_QUIESCENCE_INDEX_DEFAULT STREQUAL "indexed")
    set(LUFIA2_QUIESCENCE_INDEX_DEFAULT_MODE 1)
elseif(LUFIA2_QUIESCENCE_INDEX_DEFAULT STREQUAL "original")
    set(LUFIA2_QUIESCENCE_INDEX_DEFAULT_MODE 0)
else()
    message(FATAL_ERROR "LUFIA2_QUIESCENCE_INDEX_DEFAULT must be auto, indexed or original")
endif()

if(VITA)
    include("${CMAKE_SOURCE_DIR}/ports/vita/native.cmake")
    return()
endif()

# Pinned so a build is reproducible. Follow upstream for one configure with
# -DLUFIA2_SNESRECOMP_REVISION=<sha> or -DLUFIA2_RECOMP_UI_REVISION=<sha>.
# The fork revision tracks upstream native analysis plus the per-entry exit-MX
# CFG directive required by the reviewed Lufia II entry contracts.
if(NOT DEFINED LUFIA2_SNESRECOMP_REPOSITORY)
    set(LUFIA2_SNESRECOMP_REPOSITORY "Cellenseres/snesrecomp")
endif()
set(_lufia2_snesrecomp_pin "678b0656de909ffb9b762ff32dabc9e13425c811")
# Older Windows build trees cached this revision. Upgrade that known stale pin
# so regenerating an existing build uses the current native analyzer.
if(NOT DEFINED LUFIA2_SNESRECOMP_REVISION OR
   LUFIA2_SNESRECOMP_REVISION STREQUAL
       "f57204394d5a4f5619ea2714aac982c5d86e5027")
    set(LUFIA2_SNESRECOMP_REVISION "${_lufia2_snesrecomp_pin}"
        CACHE STRING "Pinned snesrecomp revision" FORCE)
endif()
if(NOT DEFINED LUFIA2_RECOMP_UI_REVISION)
    set(LUFIA2_RECOMP_UI_REVISION
        "1b3292f8b93c71843d9503540c6b3dea8404a904")
endif()
set(LUFIA2_SDL_VERSION "3.4.14")

set(LUFIA2_SNESRECOMP_ROOT "" CACHE PATH
    "Optional local snesrecomp source tree")
set(LUFIA2_RECOMP_UI_ROOT "" CACHE PATH
    "Optional local recomp-ui source tree")
set(LUFIA2_SDL3_ROOT "" CACHE PATH
    "Optional local SDL3 source tree")
set(LUFIA2_ROM "${CMAKE_SOURCE_DIR}/lufia2.sfc" CACHE FILEPATH
    "Path to the supported Lufia II USA ROM")

set(_deps_root "${CMAKE_BINARY_DIR}/_deps")
set(_download_root "${_deps_root}/downloads")
file(MAKE_DIRECTORY "${_deps_root}")
file(MAKE_DIRECTORY "${_download_root}")

function(lufia2_download_source url archive_name extracted_name destination required_file)
    if(EXISTS "${destination}/${required_file}")
        return()
    endif()

    set(_archive "${_download_root}/${archive_name}")
    set(_extract_root "${_deps_root}/extract-${archive_name}")

    if(NOT EXISTS "${_archive}")
        message(STATUS "Downloading ${archive_name}")
        file(
            DOWNLOAD
            "${url}"
            "${_archive}"
            STATUS _download_status
            SHOW_PROGRESS
            TLS_VERIFY ON
        )
        list(GET _download_status 0 _download_code)
        list(GET _download_status 1 _download_message)
        if(NOT _download_code EQUAL 0)
            file(REMOVE "${_archive}")
            message(FATAL_ERROR
                "Download failed: ${url}\n${_download_message}")
        endif()
    endif()

    file(REMOVE_RECURSE "${_extract_root}")
    file(MAKE_DIRECTORY "${_extract_root}")

    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E tar xzf "${_archive}"
        WORKING_DIRECTORY "${_extract_root}"
        RESULT_VARIABLE _extract_result
    )
    if(NOT _extract_result EQUAL 0)
        message(FATAL_ERROR "Could not extract ${archive_name}")
    endif()

    if(NOT EXISTS "${_extract_root}/${extracted_name}/${required_file}")
        message(FATAL_ERROR
            "Archive ${archive_name} did not contain ${required_file}")
    endif()

    file(REMOVE_RECURSE "${destination}")
    file(RENAME
        "${_extract_root}/${extracted_name}"
        "${destination}"
    )
    file(REMOVE_RECURSE "${_extract_root}")
endfunction()

if(LUFIA2_SNESRECOMP_ROOT)
    set(SNESRECOMP_ROOT "${LUFIA2_SNESRECOMP_ROOT}")
else()
    # Key the extracted dependency by its immutable revision. Changing the pin
    # must never silently reuse a source tree fetched for an older core.
    set(SNESRECOMP_ROOT
        "${_deps_root}/snesrecomp-${LUFIA2_SNESRECOMP_REVISION}")
    lufia2_download_source(
        "https://github.com/${LUFIA2_SNESRECOMP_REPOSITORY}/archive/${LUFIA2_SNESRECOMP_REVISION}.tar.gz"
        "snesrecomp-${LUFIA2_SNESRECOMP_REVISION}.tar.gz"
        "snesrecomp-${LUFIA2_SNESRECOMP_REVISION}"
        "${SNESRECOMP_ROOT}"
        "runner/runner.cmake"
    )
endif()

if(LUFIA2_RECOMP_UI_ROOT)
    set(RECOMP_UI_ROOT "${LUFIA2_RECOMP_UI_ROOT}")
else()
    set(RECOMP_UI_ROOT
        "${_deps_root}/recomp-ui-${LUFIA2_RECOMP_UI_REVISION}")
    lufia2_download_source(
        "https://github.com/mstan/recomp-ui/archive/${LUFIA2_RECOMP_UI_REVISION}.tar.gz"
        "recomp-ui-${LUFIA2_RECOMP_UI_REVISION}.tar.gz"
        "recomp-ui-${LUFIA2_RECOMP_UI_REVISION}"
        "${RECOMP_UI_ROOT}"
        "recomp_ui.cmake"
    )
endif()

if(NOT EXISTS "${SNESRECOMP_ROOT}/runner/runner.cmake")
    message(FATAL_ERROR "Invalid snesrecomp source tree: ${SNESRECOMP_ROOT}")
endif()

if(NOT EXISTS "${RECOMP_UI_ROOT}/recomp_ui.cmake")
    message(FATAL_ERROR "Invalid recomp-ui source tree: ${RECOMP_UI_ROOT}")
endif()

find_package(Python3 COMPONENTS Interpreter REQUIRED)

lufia2_prepare_decomp(
    CFG_DIR "${CMAKE_SOURCE_DIR}/recomp"
    OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated/effective-cfg"
    REPORT "${CMAKE_BINARY_DIR}/generated/decomp_coverage.json"
    TEXT_REPORT "${CMAKE_BINARY_DIR}/generated/LUFIA2_FUNCTION_SOURCES.txt"
    OUT_CFG_DIR LUFIA2_EFFECTIVE_CFG_DIR)

# The verified C7F8/D508 replacements own actor semantics in normal builds.
# Shadow the cache option rather than erasing the user's fallback preference.
if(LUFIA2_ENABLE_DECOMP AND NOT LUFIA2_DECOMP_REFERENCE_ONLY)
    set(LUFIA2_ENABLE_ACTOR_EARLY_RETURN OFF)
    message(STATUS "Interpreter actor fast paths excluded: semantic decomp selected")
endif()
if(LUFIA2_ENABLE_ACTOR_EARLY_RETURN AND
   (LUFIA2_ENABLE_NATIVE_WAIT OR LUFIA2_ENABLE_GAMEPLAY_CAPTURE OR
    LUFIA2_ENABLE_PERF_AUDIT OR LUFIA2_ENABLE_BRIDGE_AUDIT OR
    LUFIA2_ENABLE_QUIESCENCE_INDEX))
    message(FATAL_ERROR
        "Disable interpreter actor fast paths for audit experiment builds")
endif()


# Full-width TGAs stay outside release artifacts.
file(GLOB LUFIA2_BATTLE_MARGIN_AUTHORING CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/assets-src/battle_margins/battle_*.tga")
set(LUFIA2_WIDESCREEN_ASSET_DIR
    "${CMAKE_SOURCE_DIR}/assets/widescreen")
set(LUFIA2_ROOM_DATA
    "${LUFIA2_WIDESCREEN_ASSET_DIR}/lufia2.l2rooms")
set(LUFIA2_MARGIN_PACK
    "${LUFIA2_WIDESCREEN_ASSET_DIR}/lufia2.l2mp")
if(LUFIA2_BATTLE_MARGIN_AUTHORING)
    set(LUFIA2_MARGIN_PACK
        "${CMAKE_BINARY_DIR}/assets/widescreen/lufia2.l2mp")
    add_custom_command(
        OUTPUT "${LUFIA2_MARGIN_PACK}"
        COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_SOURCE_DIR}/scripts/margin_asset_tool.py"
            --no-intro
            --output "${LUFIA2_MARGIN_PACK}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        DEPENDS
            "${CMAKE_SOURCE_DIR}/scripts/margin_asset_tool.py"
            ${LUFIA2_BATTLE_MARGIN_AUTHORING}
        COMMENT "Packing sparse Lufia II widescreen margins"
        VERBATIM
    )
    add_custom_target(lufia2_margin_pack DEPENDS "${LUFIA2_MARGIN_PACK}")
endif()

set(LUFIA2_GENERATED_DIR "${CMAKE_SOURCE_DIR}/src/gen")
if(EXISTS "${LUFIA2_ROM}")
    file(SHA256 "${LUFIA2_ROM}" _rom_sha256)
    string(TOUPPER "${_rom_sha256}" _rom_sha256)
    if(NOT _rom_sha256 STREQUAL
       "7C34ECB16C10F551120ED7B86CFBC947042F479B52EE74BB3C40E92FDD192B3A")
        message(FATAL_ERROR
            "Unsupported ROM. Expected the clean Lufia II USA release.")
    endif()

    set(LUFIA2_GENERATED_DIR "${CMAKE_BINARY_DIR}/lufia2-gen")
    set(_lufia2_reviewed_aot_args)
    if(LUFIA2_ENABLE_REVIEWED_AOT)
        list(APPEND _lufia2_reviewed_aot_args --cfg-roots)
        message(STATUS
            "Lufia2 reviewed AOT roots ON: static CFG only; profile promotion disabled")
    endif()
    execute_process(
        COMMAND
            "${CMAKE_COMMAND}" -E env
            "SNESRECOMP_ROOT=${SNESRECOMP_ROOT}"
            "PYTHONDONTWRITEBYTECODE=1"
            "${Python3_EXECUTABLE}"
            "${SNESRECOMP_ROOT}/tools/v2_emit.py"
            --rom "${LUFIA2_ROM}"
            --cfg-dir "${LUFIA2_EFFECTIVE_CFG_DIR}"
            --out-dir "${LUFIA2_GENERATED_DIR}"
            --analysis-backend native
            --no-host-root-scan
            ${_lufia2_reviewed_aot_args}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE _regen_result
        COMMAND_ECHO STDOUT
    )

    if(NOT _regen_result EQUAL 0)
        message(FATAL_ERROR "Lufia II recompilation failed.")
    endif()
elseif(NOT EXISTS "${CMAKE_SOURCE_DIR}/src/gen/dispatch_v2.c")
    message(FATAL_ERROR
        "lufia2.sfc was not found. Place the supported ROM in the repository "
        "root or set LUFIA2_ROM to its path.")
endif()


set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)

if(LUFIA2_SDL3_ROOT)
    set(SDL3_SOURCE_DIR "${LUFIA2_SDL3_ROOT}")
else()
    set(SDL3_SOURCE_DIR "${_deps_root}/SDL3-src")
    lufia2_download_source(
        "https://github.com/libsdl-org/SDL/releases/download/release-${LUFIA2_SDL_VERSION}/SDL3-${LUFIA2_SDL_VERSION}.tar.gz"
        "SDL3-${LUFIA2_SDL_VERSION}.tar.gz"
        "SDL3-${LUFIA2_SDL_VERSION}"
        "${SDL3_SOURCE_DIR}"
        "CMakeLists.txt"
    )
endif()

if(NOT EXISTS "${SDL3_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Invalid SDL3 source tree: ${SDL3_SOURCE_DIR}")
endif()

add_subdirectory(
    "${SDL3_SOURCE_DIR}"
    "${CMAKE_BINARY_DIR}/_deps/sdl3-build"
    EXCLUDE_FROM_ALL
)

# Project-owned, game-neutral host-platform layer. Its runtime policy target is
# SDL-free; its presentation target consumes only public SDL types and raw video
# frames. Neither target depends on Lufia or generated game code.
set(SNESRECOMP_PLATFORM_ENABLE_OPENGL ON CACHE BOOL "" FORCE)
set(SNESRECOMP_PLATFORM_GL_CORE_DIR
    "${RECOMP_UI_ROOT}/src/third_party" CACHE PATH "" FORCE)

# The native Vulkan backend is opt-in: it needs a Vulkan SDK for the headers,
# the loader and glslc, and OpenGL stays the shipped default renderer. Turn it
# on with -DLUFIA2_ENABLE_VULKAN=ON, and point SNESRECOMP_PLATFORM_GLSLC at a
# glslc binary if it is not inside $VULKAN_SDK.
option(LUFIA2_ENABLE_VULKAN
    "Build the native Vulkan renderer and offer it in the launcher" OFF)
set(SNESRECOMP_PLATFORM_ENABLE_VULKAN ${LUFIA2_ENABLE_VULKAN}
    CACHE BOOL "" FORCE)

option(LUFIA2_VULKAN_VALIDATION
    "Request the Vulkan validation layer (development builds only)" OFF)
set(SNESRECOMP_PLATFORM_VULKAN_VALIDATION ${LUFIA2_VULKAN_VALIDATION}
    CACHE BOOL "" FORCE)

# Consumer Release does not build Platform's optional smoke tests. Old build
# trees may still cache this option as ON from a standalone test run.
set(SNESRECOMP_PLATFORM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_SOURCE_DIR}/lib/snesrecomp-platform")


set(SNESRECOMP_SDL_BACKEND "SDL3" CACHE STRING "" FORCE)
set(SNESRECOMP_ENABLE_MODS OFF CACHE BOOL "" FORCE)
set(SNESRECOMP_ENABLE_TRACE OFF CACHE BOOL "" FORCE)
set(SNES_COSIM OFF CACHE BOOL "" FORCE)

include("${SNESRECOMP_ROOT}/runner/runner.cmake")

# Replace the framework bridge translation unit with an immutable build-tree
# overlay that calls the game-neutral Platform runtime policy. The fetched
# snesrecomp source tree remains byte-identical to its pinned archive.
snesrecomp_platform_prepare_runner_sources(
    SNESRECOMP_RUNNER_SOURCES "${SNESRECOMP_ROOT}")
include("${CMAKE_SOURCE_DIR}/cmake/Lufia2ConfigOverlay.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/Lufia2PpuMosaic.cmake")
lufia2_prepare_ppu_mosaic_overlay(
    SNESRECOMP_RUNNER_SOURCES "${SNESRECOMP_ROOT}")

if(LUFIA2_ENABLE_DMA_HOST_FASTFORWARD)
    include("${CMAKE_SOURCE_DIR}/cmake/Lufia2DmaHostFastForward.cmake")
    lufia2_prepare_dma_host_fastforward(
        SNESRECOMP_RUNNER_SOURCES "${SNESRECOMP_ROOT}")
endif()
include("${CMAKE_SOURCE_DIR}/cmake/Lufia2DmaOverlay.cmake")
lufia2_prepare_dma_overlay(
    SNESRECOMP_RUNNER_SOURCES "${SNESRECOMP_ROOT}")

# The pinned interpreter clock calls clock_gettime() without including <time.h>,
# so CLOCK_MONOTONIC is undeclared on POSIX. Supply the header from the build
# instead of editing the pinned tree.
if(NOT WIN32)
    set_source_files_properties(
        "${SNESRECOMP_ROOT}/runner/src/snes/interp816.c"
        PROPERTIES COMPILE_OPTIONS "-include;time.h")
endif()

list(FILTER SNESRECOMP_RUNNER_SOURCES EXCLUDE REGEX "launcher(_picker)?\\.c$")
list(FILTER SNESRECOMP_RUNNER_SOURCES EXCLUDE REGEX "keybinds\\.c$")
# Replaced by src/lufia2_savestate_menu.c, which carries slot metadata.
# Only host_main.c consumed it, and this host has its own main.
list(FILTER SNESRECOMP_RUNNER_SOURCES EXCLUDE REGEX "snes_savestate_menu\\.c$")

file(GLOB LUFIA2_GENERATED_SOURCES CONFIGURE_DEPENDS
    "${LUFIA2_GENERATED_DIR}/*.c")

if(NOT LUFIA2_GENERATED_SOURCES)
    message(FATAL_ERROR
        "No generated C files were found in ${LUFIA2_GENERATED_DIR}.")
endif()

# Guarded: older cores have no snes_set_hdma_beam_enabled().
# This host walks the HDMA tables itself; switchable for an A/B.
option(LUFIA2_ENABLE_HDMA_BEAM_SWITCH
    "Disable the core's beam-timeline HDMA; this host replays the tables" ON)
if(EXISTS "${SNESRECOMP_ROOT}/runner/src/snes/snes.h")
    file(READ "${SNESRECOMP_ROOT}/runner/src/snes/snes.h" _snes_h)
    if(_snes_h MATCHES "snes_set_hdma_beam_enabled" AND LUFIA2_ENABLE_HDMA_BEAM_SWITCH)
        add_compile_definitions(LUFIA2_HAVE_HDMA_BEAM_SWITCH=1)
    endif()
    unset(_snes_h)
endif()

set(LUFIA2_HOST_SOURCES
    src/main.c
    src/decomp_bridge/host_events.c
    src/desktop_glue.c
    src/lufia2_battle.c
    src/lufia2_battle_widescreen.c
    src/lufia2_game_info.c
    patches/lufia2_intro_mode7_world.c
    patches/lufia2_mode7_substep.c
    patches/lufia2_sound_menu.c
    src/lufia2_map_load.c
    src/lufia2_equip_names.c
    src/lufia2_drop_text.c
    src/lufia2_cave_eggs.c
    src/lufia2_cave_stairs.c
    src/lufia2_spell_price.c
    src/lufia2_play_time.c
    src/lufia2_party_limits.c
    src/lufia2_map_widescreen.c
    src/lufia2_ui_margins.c
    src/lufia2_map_names.c
    src/lufia2_overlay_ui.c
    src/lufia2_savestate_menu.c
    src/lufia2_resource.c
    src/lufia2_ui_assets.c
    src/lufia2_msu_driver.c
    src/lufia2_ppu_capture.c
    src/lufia2_battle_effects.c
    src/lufia2_battle_ui.c
    src/lufia2_menu_ui.c
    src/lufia2_room_data.c
    src/lufia2_intro_margins.c
    src/lufia2_intro_trees.c
    src/lufia2_intro_widescreen.c
    src/lufia2_margin_assets.c
    src/lufia2_runtime.c
    src/lufia2_splash_credit.c
    src/lufia2_tga.c
    src/lufia2_video_handoff.c
    src/lufia2_video_policy.c
    src/gen_stubs.c
)

# Widescreen policy uses the portable semantic builder when available.
if(LUFIA2_ENABLE_DECOMP)
    list(APPEND LUFIA2_HOST_SOURCES src/lufia2_sprite_visibility.c)
endif()

if(LUFIA2_ENABLE_GAMEPLAY_CAPTURE)
    include("${CMAKE_SOURCE_DIR}/cmake/Lufia2GameplayCapture.cmake")
    lufia2_prepare_gameplay_capture(SNESRECOMP_RUNNER_SOURCES "${SNESRECOMP_ROOT}")
    list(APPEND LUFIA2_HOST_SOURCES src/diagnostics/lufia2_gameplay_capture.c)
    message(STATUS "Lufia2 gameplay capture ON: Ctrl+Shift+F10 toggles bounded recording; CFG unchanged")
endif()

if(LUFIA2_ENABLE_NATIVE_WAIT OR LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD OR
   LUFIA2_ENABLE_ACTOR_EARLY_RETURN)
    include("${CMAKE_SOURCE_DIR}/cmake/Lufia2NativePatches.cmake")
    lufia2_prepare_native_patch_sources(SNESRECOMP_RUNNER_SOURCES "${SNESRECOMP_ROOT}")
    list(APPEND LUFIA2_HOST_SOURCES patches/native_patches.c)
endif()

if(LUFIA2_ENABLE_DMA_HOST_FASTFORWARD)
    message(STATUS
        "Lufia2 DMA host fast-forward: synchronous idle timer iterations collapsed; byte transfers unchanged")
endif()
if(LUFIA2_ENABLE_DMA_DIRECT_SOURCE_READ)
    message(STATUS
        "Lufia2 DMA direct source reads: WRAM/plain-LoROM specialized; MMIO/SRAM/special carts use core fallback")
endif()

if(LUFIA2_ENABLE_NATIVE_WAIT)
    message(STATUS "Lufia2 native wait step1: guarded CMP/BEQ, original bridge timing; runtime LUFIA2_NATIVE_WAIT=0 selects reference")
endif()

if(LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD)
    message(STATUS
        "Lufia2 native frame wait step5: portable guarded batching at five reviewed waits including the Mode 7 world-map wait; runtime 0 restores reference")
endif()

if(LUFIA2_ENABLE_ACTOR_EARLY_RETURN)
    message(STATUS
        "Lufia2 actor fast paths: reviewed $83:C7F8 and $83:D508 write-free exits; original bus order and fallback retained")
endif()

if(LUFIA2_ENABLE_BRIDGE_AUDIT)
    list(APPEND LUFIA2_HOST_SOURCES src/diagnostics/lufia2_bridge_audit.c)
    message(STATUS "BridgeAudit: Ctrl+Shift+F9, 120 warmup + 600 boundaries, randomized iteration sampling; gameplay unchanged")
endif()


add_executable(Lufia2Recomp
    ${SNESRECOMP_RUNNER_SOURCES}
    ${LUFIA2_HOST_SOURCES}
    ${LUFIA2_GENERATED_SOURCES}
)
if(TARGET lufia2_margin_pack)
    add_dependencies(Lufia2Recomp lufia2_margin_pack)
endif()
include("${CMAKE_SOURCE_DIR}/cmake/Lufia2BuildIdentity.cmake")
lufia2_add_build_identity(Lufia2Recomp)

if(LUFIA2_ENABLE_GAMEPLAY_CAPTURE)
    target_compile_definitions(Lufia2Recomp PRIVATE LUFIA2_ENABLE_GAMEPLAY_CAPTURE=1)
    target_include_directories(Lufia2Recomp PRIVATE "${CMAKE_SOURCE_DIR}/src/diagnostics")
endif()

if(LUFIA2_ENABLE_NATIVE_WAIT OR LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD OR
   LUFIA2_ENABLE_ACTOR_EARLY_RETURN OR
   LUFIA2_ENABLE_DMA_HOST_FASTFORWARD OR
   LUFIA2_ENABLE_DMA_DIRECT_SOURCE_READ)
    target_include_directories(Lufia2Recomp PRIVATE "${CMAKE_SOURCE_DIR}")
endif()

if(LUFIA2_ENABLE_NATIVE_WAIT)
    target_compile_definitions(Lufia2Recomp PRIVATE LUFIA2_ENABLE_NATIVE_WAIT=1)
    if(NOT LUFIA2_ENABLE_QUIESCENCE_INDEX)
        target_compile_definitions(Lufia2Recomp PRIVATE
            LUFIA2_NATIVE_WAIT_DEFAULT_MODE=${LUFIA2_NATIVE_WAIT_DEFAULT_MODE})
        message(STATUS "NativeWait runtime default: ${LUFIA2_NATIVE_WAIT_DEFAULT}")
    endif()
endif()

if(LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD)
    target_compile_definitions(Lufia2Recomp PRIVATE
        LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD=1)
endif()

if(LUFIA2_ENABLE_ACTOR_EARLY_RETURN)
    target_compile_definitions(Lufia2Recomp PRIVATE
        LUFIA2_ENABLE_ACTOR_EARLY_RETURN=1)
endif()

if(LUFIA2_ENABLE_DMA_HOST_FASTFORWARD)
    target_compile_definitions(Lufia2Recomp PRIVATE
        LUFIA2_ENABLE_DMA_HOST_FASTFORWARD=1)
endif()

if(LUFIA2_ENABLE_DMA_DIRECT_SOURCE_READ)
    target_compile_definitions(Lufia2Recomp PRIVATE
        LUFIA2_ENABLE_DMA_DIRECT_SOURCE_READ=1)
endif()

if(LUFIA2_ENABLE_BRIDGE_AUDIT)
    target_compile_definitions(Lufia2Recomp PRIVATE LUFIA2_ENABLE_BRIDGE_AUDIT=1)
endif()
if(LUFIA2_ENABLE_RUNTIME_LOG)
    target_compile_definitions(Lufia2Recomp PRIVATE LUFIA2_ENABLE_RUNTIME_LOG=1)
    message(STATUS "Lufia2 runtime log ON: map, MSU, widescreen and periodic state tracing")
endif()
if(LUFIA2_ENABLE_PATCH_REPORTING)
    target_compile_definitions(Lufia2Recomp PRIVATE LUFIA2_ENABLE_PATCH_REPORTING=1)
    message(STATUS "Lufia2 patch reporting ON: counters and scheduler time every 120 boundaries")
endif()

if(LUFIA2_ENABLE_QUIESCENCE_INDEX)
    target_compile_definitions(Lufia2Recomp PRIVATE
        LUFIA2_ENABLE_QUIESCENCE_INDEX=1
        LUFIA2_QUIESCENCE_INDEX_DEFAULT_MODE=${LUFIA2_QUIESCENCE_INDEX_DEFAULT_MODE})
    message(STATUS "QuiescenceIndex: candidate filter only; original equality, ordering, yields and guest timing retained")
    message(STATUS "QuiescenceIndex runtime default: ${LUFIA2_QUIESCENCE_INDEX_DEFAULT}; NativeWait payload defaults to OFF here")
endif()

include("${CMAKE_SOURCE_DIR}/cmake/Lufia2MsvcPpu.cmake")
lufia2_target_msvc_ppu(Lufia2Recomp "${SNESRECOMP_ROOT}")

target_include_directories(Lufia2Recomp PRIVATE
    "${CMAKE_SOURCE_DIR}/recomp"
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/patches"
    ${SNESRECOMP_RUNNER_INCLUDE_DIRS}
)

target_compile_definitions(Lufia2Recomp PRIVATE
    SNESRECOMP_SDL3=1
    LNG_SDL3=1
    SDL_MAIN_HANDLED
    SYSTEM_VOLUME_MIXER_AVAILABLE=0
    SNESRECOMP_TRACE=0
    SNESRECOMP_REVERSE_DEBUG=0
    SNESRECOMP_LLE_BOUNCE_DEFAULT=1
)

if(MSVC)
    target_compile_options(Lufia2Recomp PRIVATE /W0)
    set_source_files_properties(${LUFIA2_HOST_SOURCES} PROPERTIES
        COMPILE_OPTIONS /W4
    )
else()
    target_compile_options(Lufia2Recomp PRIVATE
        -w
        -Wno-implicit-function-declaration
        -Wno-error=implicit-function-declaration
    )
endif()

target_link_libraries(Lufia2Recomp PRIVATE
    SDL3::SDL3
    snesrecomp::platform
    snesrecomp::runtime
    Lufia2::ResourceFormat
    ${SNESRECOMP_RUNNER_LIBRARIES}
)
lufia2_add_decomp_bridge(Lufia2Recomp "${SNESRECOMP_ROOT}")

lufia2_target_config_overlay(Lufia2Recomp "${SNESRECOMP_ROOT}")

if(NOT VITA)
    add_executable(Lufia2BattleEffectsTest EXCLUDE_FROM_ALL
        scripts/tests/battle_effects.c
        src/lufia2_battle_effects.c src/lufia2_battle_ui.c src/lufia2_menu_ui.c src/lufia2_battle.c
        src/lufia2_battle_widescreen.c src/lufia2_video_handoff.c src/lufia2_video_policy.c
        "${SNESRECOMP_ROOT}/runner/src/snes/ws_shadow.c")
    target_include_directories(Lufia2BattleEffectsTest PRIVATE
        "${CMAKE_BINARY_DIR}/generated/lufia2-ppu-fixes/snes"
        "${CMAKE_SOURCE_DIR}/src" "${SNESRECOMP_ROOT}/runner/src"
        "${SNESRECOMP_ROOT}/runner/src/snes")
    target_link_libraries(Lufia2BattleEffectsTest PRIVATE snesrecomp_platform)
    lufia2_target_msvc_ppu(Lufia2BattleEffectsTest "${SNESRECOMP_ROOT}")
    if(MSVC)
        target_compile_options(Lufia2BattleEffectsTest PRIVATE /UNDEBUG /Gy)
        target_link_options(Lufia2BattleEffectsTest PRIVATE /OPT:REF)
    else()
        target_compile_options(Lufia2BattleEffectsTest PRIVATE -UNDEBUG)
    endif()
endif()
snesrecomp_platform_target_glsl_presets(Lufia2Recomp)

set(RECOMP_UI_ENABLE_MODS OFF CACHE BOOL "" FORCE)
include("${RECOMP_UI_ROOT}/recomp_ui.cmake")

recomp_target_launcher_ui(Lufia2Recomp
    CONSOLE snes
)

snesrecomp_platform_target_launcher_overlays(
    Lufia2Recomp "${RECOMP_UI_ROOT}")
include("${CMAKE_SOURCE_DIR}/cmake/Lufia2LauncherFeatures.cmake")
lufia2_target_launcher_features(Lufia2Recomp)

add_custom_command(TARGET Lufia2Recomp POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory
        "$<TARGET_FILE_DIR:Lufia2Recomp>/assets/widescreen"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${LUFIA2_ROOM_DATA}"
        "$<TARGET_FILE_DIR:Lufia2Recomp>/assets/widescreen/lufia2.l2rooms"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${LUFIA2_MARGIN_PACK}"
        "$<TARGET_FILE_DIR:Lufia2Recomp>/assets/widescreen/lufia2.l2mp"
)

# Project-owned presentation presets. They remain ordinary runtime assets;
# the existing GLSL/GLSLP loader owns parsing and shader compilation.
add_custom_command(TARGET Lufia2Recomp POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory
        "$<TARGET_FILE_DIR:Lufia2Recomp>/assets/shaders"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets/shaders"
        "$<TARGET_FILE_DIR:Lufia2Recomp>/assets/shaders"
)

# UI presets contain geometry; original graphics come from the local ROM.
add_custom_command(TARGET Lufia2Recomp POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory
        "$<TARGET_FILE_DIR:Lufia2Recomp>/assets/ui"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CMAKE_SOURCE_DIR}/assets/ui/battle_layout.l2ui"
        "${CMAKE_SOURCE_DIR}/assets/ui/menu_layout.l2ui"
        "$<TARGET_FILE_DIR:Lufia2Recomp>/assets/ui"
)

# Intro margins use their existing compositor.
add_custom_command(TARGET Lufia2Recomp POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory
        "$<TARGET_FILE_DIR:Lufia2Recomp>/assets/img"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CMAKE_SOURCE_DIR}/assets/img/lufia2_intro_margins.tga"
        "$<TARGET_FILE_DIR:Lufia2Recomp>/assets/img/lufia2_intro_margins.tga"
)

if(WIN32 AND TARGET SDL3::SDL3-shared)
    add_custom_command(TARGET Lufia2Recomp POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE:SDL3::SDL3-shared>"
            "$<TARGET_FILE_DIR:Lufia2Recomp>"
    )
endif()

if(UNIX AND NOT APPLE AND TARGET SDL3::SDL3-shared)
    # Ship libSDL3 next to the binary and look for it there first, so a copied
    # build directory runs on a Steam Deck that has no system SDL3.
    set_target_properties(Lufia2Recomp PROPERTIES
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
    )
    add_custom_command(TARGET Lufia2Recomp POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_SONAME_FILE:SDL3::SDL3-shared>"
            "$<TARGET_FILE_DIR:Lufia2Recomp>"
    )
endif()
