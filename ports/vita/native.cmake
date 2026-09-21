# Pinned so a build is reproducible. Follow upstream for one configure with
# -DLUFIA2_SNESRECOMP_REVISION=<sha> or -DLUFIA2_RECOMP_UI_REVISION=<sha>.
if(NOT DEFINED LUFIA2_SNESRECOMP_REVISION)
    set(LUFIA2_SNESRECOMP_REVISION
        "f7ce10cdcdb9bd6c56b7aedd597ca8cf0626fb25")
endif()
if(NOT DEFINED LUFIA2_RECOMP_UI_REVISION)
    set(LUFIA2_RECOMP_UI_REVISION
        "f0166e1ee4b1c40c07db799671973f11304ea43b")
endif()
set(LUFIA2_SDL_VERSION "3.4.14")

set(LUFIA2_SNESRECOMP_ROOT "" CACHE PATH
    "Optional local snesrecomp source tree")
set(LUFIA2_RECOMP_UI_ROOT "" CACHE PATH
    "Optional local recomp-ui source tree")
set(LUFIA2_SDL3_ROOT "" CACHE PATH
    "Optional local SDL3 source tree")
set(LUFIA2_PLATFORM_ROOT "${CMAKE_SOURCE_DIR}/lib/snesrecomp-platform"
    CACHE PATH "Portable snesrecomp-platform source tree")
set(LUFIA2_VITA_ADAPTER_ROOT "" CACHE PATH
    "Vita adapter source tree; defaults to the workspace sibling when present")
set(LUFIA2_ROM "${CMAKE_SOURCE_DIR}/lufia2.sfc" CACHE FILEPATH
    "Path to the supported Lufia II USA ROM")
set(LUFIA2_VITA_FALLBACK_DATA_ADDRESS "0x81460000" CACHE STRING
    "Fallback Vita RW-segment address when the linker lacks SCE headroom support")

if(NOT LUFIA2_VITA_FALLBACK_DATA_ADDRESS MATCHES "^0x[0-9A-Fa-f]+$")
    message(FATAL_ERROR
        "LUFIA2_VITA_FALLBACK_DATA_ADDRESS must be a hexadecimal address; got "
        "'${LUFIA2_VITA_FALLBACK_DATA_ADDRESS}'")
endif()

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
        "https://github.com/RetroPortingToolKit/snesrecomp/archive/${LUFIA2_SNESRECOMP_REVISION}.tar.gz"
        "snesrecomp-${LUFIA2_SNESRECOMP_REVISION}.tar.gz"
        "snesrecomp-${LUFIA2_SNESRECOMP_REVISION}"
        "${SNESRECOMP_ROOT}"
        "runner/runner.cmake"
    )
endif()

find_package(Python3 COMPONENTS Interpreter REQUIRED)

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
            --cfg-dir "${CMAKE_SOURCE_DIR}/recomp"
            --out-dir "${LUFIA2_GENERATED_DIR}"
            --analysis-backend python
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

if(NOT LUFIA2_VITA_ADAPTER_ROOT)
    get_filename_component(_lufia2_workspace_root
        "${CMAKE_SOURCE_DIR}/.." ABSOLUTE)
    set(LUFIA2_VITA_ADAPTER_ROOT
        "${_lufia2_workspace_root}/snesrecomp-platform-vita"
        CACHE PATH "Vita adapter source tree" FORCE)
endif()
if(NOT EXISTS "${LUFIA2_VITA_ADAPTER_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR
        "LUFIA2_VITA_ADAPTER_ROOT is not a snesrecomp-platform-vita tree: "
        "${LUFIA2_VITA_ADAPTER_ROOT}")
endif()
if(NOT EXISTS "${LUFIA2_PLATFORM_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR
        "LUFIA2_PLATFORM_ROOT is not a snesrecomp-platform tree: "
        "${LUFIA2_PLATFORM_ROOT}")
endif()
message(STATUS "Lufia2 portable platform: ${LUFIA2_PLATFORM_ROOT}")
message(STATUS "Lufia2 Vita adapter: ${LUFIA2_VITA_ADAPTER_ROOT}")

add_subdirectory("${LUFIA2_VITA_ADAPTER_ROOT}" "${CMAKE_BINARY_DIR}/adapter")
snesrecomp_vita_prepare()
set(SNESRECOMP_PLATFORM_ENABLE_OPENGL OFF CACHE BOOL "" FORCE)
enable_testing()
add_subdirectory("${LUFIA2_PLATFORM_ROOT}" "${CMAKE_BINARY_DIR}/platform")

# The runtime library uses strict C11. VitaSDK/newlib hides the POSIX
# setenv declaration in that mode unless its feature level is requested.
target_compile_definitions(snesrecomp_platform_runtime PRIVATE
    _POSIX_C_SOURCE=200809L)

include("${CMAKE_SOURCE_DIR}/cmake/Lufia2Msu.cmake")

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
include("${CMAKE_SOURCE_DIR}/ports/vita/InterpreterHostCosts.cmake")
lufia2_prepare_interp_host_costs(
    SNESRECOMP_RUNNER_SOURCES "${SNESRECOMP_ROOT}")
message(STATUS
    "Lufia2 Vita interpreter host costs: game overlay stacked after portable Platform bridge")
include("${CMAKE_SOURCE_DIR}/ports/vita/PpuPixelOffload.cmake")
lufia2_prepare_ppu_pixel_offload(
    SNESRECOMP_RUNNER_SOURCES "${SNESRECOMP_ROOT}")
if(LUFIA2_ENABLE_DMA_HOST_FASTFORWARD)
    include("${CMAKE_SOURCE_DIR}/cmake/Lufia2DmaHostFastForward.cmake")
    lufia2_prepare_dma_host_fastforward(
        SNESRECOMP_RUNNER_SOURCES "${SNESRECOMP_ROOT}")
endif()
include("${CMAKE_SOURCE_DIR}/cmake/Lufia2DmaOverlay.cmake")
lufia2_prepare_dma_overlay(
    SNESRECOMP_RUNNER_SOURCES "${SNESRECOMP_ROOT}")

list(FILTER SNESRECOMP_RUNNER_SOURCES EXCLUDE REGEX "launcher(_picker)?\\.c$")
list(FILTER SNESRECOMP_RUNNER_SOURCES EXCLUDE REGEX "keybinds\\.c$")

file(GLOB LUFIA2_GENERATED_SOURCES CONFIGURE_DEPENDS
    "${LUFIA2_GENERATED_DIR}/*.c")

if(NOT LUFIA2_GENERATED_SOURCES)
    message(FATAL_ERROR
        "No generated C files were found in ${LUFIA2_GENERATED_DIR}.")
endif()

set(LUFIA2_HOST_SOURCES
    ports/vita/main.c
    src/desktop_glue.c
    ports/vita/lufia2_game_info.c
    src/lufia2_map_load.c
    src/lufia2_map_widescreen.c
    src/lufia2_msu_driver.c
    src/lufia2_room_data.c
    ports/vita/lufia2_runtime.c
    src/lufia2_sprite_visibility.c
    src/lufia2_video_policy.c
    src/gen_stubs.c
    ports/vita/lufia2_ppu_capture.c
    ports/vita/lufia2_gpu.c
)

if(LUFIA2_ENABLE_PERF_AUDIT)
    include("${CMAKE_SOURCE_DIR}/ports/vita/perf/PerfAudit.cmake")
    lufia2_prepare_perf_sources(SNESRECOMP_RUNNER_SOURCES "${SNESRECOMP_ROOT}" 5)
    lufia2_prepare_perf_sources(LUFIA2_HOST_SOURCES "${SNESRECOMP_ROOT}" 1)
    list(APPEND LUFIA2_HOST_SOURCES ports/vita/perf/lufia2_perf_audit.c)
    message(STATUS "Lufia2 PERF AUDIT ON: 3 x 600 frames after 120 warmup; profile promotion unchanged")
else()
    message(STATUS "Lufia2 PERF AUDIT OFF: production instrumentation disabled")
endif()

if(LUFIA2_ENABLE_NATIVE_WAIT OR LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD OR
   LUFIA2_ENABLE_ACTOR_EARLY_RETURN OR
   LUFIA2_ENABLE_DMA_HOST_FASTFORWARD)
    include("${CMAKE_SOURCE_DIR}/cmake/Lufia2NativePatches.cmake")
    lufia2_prepare_native_patch_sources(SNESRECOMP_RUNNER_SOURCES "${SNESRECOMP_ROOT}")
    list(APPEND LUFIA2_HOST_SOURCES patches/native_patches.c)
endif()

if(LUFIA2_ENABLE_NATIVE_WAIT)
    message(STATUS "Lufia2 native wait step1: guarded CMP/BEQ, original bridge timing; runtime LUFIA2_NATIVE_WAIT=0 selects reference")
endif()

if(LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD)
    message(STATUS
        "Lufia2 native frame wait step5: portable guarded batching at five reviewed waits; runtime 0 restores reference")
endif()

if(LUFIA2_ENABLE_ACTOR_EARLY_RETURN)
    message(STATUS
        "Lufia2 actor fast paths: $83:C7F8 and $83:D508 reviewed early returns; original bus order and fallback retained")
endif()

if(LUFIA2_ENABLE_DMA_HOST_FASTFORWARD)
    message(STATUS
        "Lufia2 DMA host fast-forward: synchronous idle timer iterations collapsed; byte transfers unchanged")
endif()

if(LUFIA2_ENABLE_DMA_DIRECT_SOURCE_READ)
    message(STATUS
        "Lufia2 DMA direct source reads: WRAM/plain-LoROM specialized; MMIO/SRAM/special carts use core fallback")
endif()

add_executable(Lufia2Recomp
    ${SNESRECOMP_RUNNER_SOURCES}
    ${LUFIA2_HOST_SOURCES}
    ${LUFIA2_GENERATED_SOURCES}
)

# vita-elf-create appends its SCE module metadata to the executable RX LOAD
# segment. GNU ld normally places the RW LOAD segment at the next 64 KiB
# boundary, which can leave less room than the metadata needs when the RX image
# happens to end near that boundary. Keep a full extra alignment page between
# the two segments. This changes only virtual placement; it adds no code and no
# emulated or renderer work.
#
# Current VitaSDK builds used by this project expose a small linker-script hook
# named __sce_headroom. Prefer it because it follows future RX growth
# automatically. Stock SDKs without that hook use the explicit address below;
# the value is cache-configurable and intentionally local to this game target.
set(_lufia2_vita_headroom_hook FALSE)
set(_lufia2_vita_linker_script_path
    "${VITASDK}/arm-vita-eabi/lib/ldscripts/armvita.x")
if(EXISTS "${_lufia2_vita_linker_script_path}")
    file(STRINGS "${_lufia2_vita_linker_script_path}"
        _lufia2_vita_headroom_lines REGEX "__sce_headroom")
    if(_lufia2_vita_headroom_lines)
        set(_lufia2_vita_headroom_hook TRUE)
    endif()
endif()

if(_lufia2_vita_headroom_hook)
    target_link_options(Lufia2Recomp PRIVATE
        "-Wl,--defsym=__sce_headroom=1")
    message(STATUS
        "Lufia2 Vita ELF layout: one dynamic 64 KiB SCE metadata headroom page")
else()
    target_link_options(Lufia2Recomp PRIVATE
        "-Wl,-Tdata=${LUFIA2_VITA_FALLBACK_DATA_ADDRESS}")
    message(STATUS
        "Lufia2 Vita ELF layout: RW segment at "
        "${LUFIA2_VITA_FALLBACK_DATA_ADDRESS} (stock-linker fallback)")
endif()

if(LUFIA2_ENABLE_PERF_AUDIT)
    target_compile_definitions(Lufia2Recomp PRIVATE LUFIA2_ENABLE_PERF_AUDIT=1)
endif()
if(LUFIA2_ENABLE_PATCH_REPORTING)
    target_compile_definitions(Lufia2Recomp PRIVATE LUFIA2_ENABLE_PATCH_REPORTING=1)
    message(STATUS "Lufia2 patch reporting ON: counters and scheduler time every 120 boundaries")
endif()
if(LUFIA2_ENABLE_RUNTIME_LOG)
    target_compile_definitions(Lufia2Recomp PRIVATE LUFIA2_ENABLE_RUNTIME_LOG=1)
    target_include_directories(Lufia2Recomp PRIVATE
        "${CMAKE_SOURCE_DIR}/ports/vita/perf")
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

# Honor the Vita adapter's code-size policy for the large AOT translation
# units. Keep the runner and host at their normal Release optimization level.
if(SNESRECOMP_TARGET_GENERATED_COMPILE_OPTIONS)
    set_source_files_properties(${LUFIA2_GENERATED_SOURCES} PROPERTIES
        COMPILE_OPTIONS "${SNESRECOMP_TARGET_GENERATED_COMPILE_OPTIONS}")
endif()

# The pinned core's interp816.c uses timespec/clock_gettime on non-Windows
# hosts without including time.h. Supply that missing include for this unit
# only, leaving both the downloaded core and interpreter behavior untouched.
set_property(SOURCE "${SNESRECOMP_ROOT}/runner/src/snes/interp816.c"
    APPEND PROPERTY COMPILE_OPTIONS "-include" "time.h")

include("${CMAKE_SOURCE_DIR}/cmake/Lufia2MsvcPpu.cmake")
lufia2_target_msvc_ppu(Lufia2Recomp "${SNESRECOMP_ROOT}")

target_include_directories(Lufia2Recomp PRIVATE
    "${CMAKE_SOURCE_DIR}/recomp"
    "${CMAKE_SOURCE_DIR}/src"
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
    ${SNESRECOMP_RUNNER_LIBRARIES}
)

snesrecomp_target_mmx_config(Lufia2Recomp)
snesrecomp_target_adapter_apply(TARGET Lufia2Recomp)
snesrecomp_platform_target_host_boot(Lufia2Recomp)
snesrecomp_target_adapter_package(TARGET Lufia2Recomp
    APP_ID CELL00001 APP_NAME "Lufia II Recompiled" VERSION 01.50
    FILE "${CMAKE_SOURCE_DIR}/assets/widescreen/lufia2.l2rooms"
         assets/widescreen/lufia2.l2rooms)
add_custom_command(TARGET Lufia2Recomp POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory
        "$<TARGET_FILE_DIR:Lufia2Recomp>/assets/widescreen"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CMAKE_SOURCE_DIR}/assets/widescreen/lufia2.l2rooms"
        "$<TARGET_FILE_DIR:Lufia2Recomp>/assets/widescreen/lufia2.l2rooms"
)

if(WIN32 AND TARGET SDL3::SDL3-shared)
    add_custom_command(TARGET Lufia2Recomp POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE:SDL3::SDL3-shared>"
            "$<TARGET_FILE_DIR:Lufia2Recomp>"
    )
endif()
