cmake_minimum_required(VERSION 3.24)
project(Lufia2Recomp C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LUFIA2_SNESRECOMP_REVISION
    "fe6045c22bb023e15d825ec40bfc25387ec9253c")
set(LUFIA2_RECOMP_UI_REVISION
    "ceebf1dfbcce3a1646a50d016ed50aa854836304")
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
    set(SNESRECOMP_ROOT "${_deps_root}/snesrecomp-src")
    lufia2_download_source(
        "https://github.com/mstan/snesrecomp/archive/${LUFIA2_SNESRECOMP_REVISION}.tar.gz"
        "snesrecomp-${LUFIA2_SNESRECOMP_REVISION}.tar.gz"
        "snesrecomp-${LUFIA2_SNESRECOMP_REVISION}"
        "${SNESRECOMP_ROOT}"
        "runner/runner.cmake"
    )
endif()

if(LUFIA2_RECOMP_UI_ROOT)
    set(RECOMP_UI_ROOT "${LUFIA2_RECOMP_UI_ROOT}")
else()
    set(RECOMP_UI_ROOT "${_deps_root}/recomp-ui-src")
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

# recomp-ui currently uses empty initializers in launcher_binds.c. They are
# valid in newer language modes but not in the C11 mode used by this project.
set(_rui_binds_file "${RECOMP_UI_ROOT}/src/common/launcher_binds.c")
if(EXISTS "${_rui_binds_file}")
    file(READ "${_rui_binds_file}" _rui_binds_source)
    string(REPLACE "= {}" "= {0}" _rui_binds_patched "${_rui_binds_source}")
    if(NOT _rui_binds_patched STREQUAL _rui_binds_source)
        file(WRITE "${_rui_binds_file}" "${_rui_binds_patched}")
    endif()
endif()

find_package(Python3 COMPONENTS Interpreter REQUIRED)

if(EXISTS "${LUFIA2_ROM}")
    file(SHA256 "${LUFIA2_ROM}" _rom_sha256)
    string(TOUPPER "${_rom_sha256}" _rom_sha256)
    if(NOT _rom_sha256 STREQUAL
       "7C34ECB16C10F551120ED7B86CFBC947042F479B52EE74BB3C40E92FDD192B3A")
        message(FATAL_ERROR
            "Unsupported ROM. Expected the clean Lufia II USA release.")
    endif()

    execute_process(
        COMMAND
            "${CMAKE_COMMAND}" -E env
            "SNESRECOMP_ROOT=${SNESRECOMP_ROOT}"
            "${Python3_EXECUTABLE}"
            "${SNESRECOMP_ROOT}/tools/v2_emit.py"
            --rom "${LUFIA2_ROM}"
            --cfg-dir "${CMAKE_SOURCE_DIR}/recomp"
            --out-dir "${CMAKE_SOURCE_DIR}/src/gen"
            --analysis-backend python
            --no-host-root-scan
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

set(SNESRECOMP_SDL_BACKEND "SDL3" CACHE STRING "" FORCE)
set(SNESRECOMP_ENABLE_MODS OFF CACHE BOOL "" FORCE)
set(SNESRECOMP_ENABLE_TRACE OFF CACHE BOOL "" FORCE)
set(SNES_COSIM OFF CACHE BOOL "" FORCE)

include("${SNESRECOMP_ROOT}/runner/runner.cmake")

list(FILTER SNESRECOMP_RUNNER_SOURCES EXCLUDE REGEX "launcher(_picker)?\\.c$")
list(FILTER SNESRECOMP_RUNNER_SOURCES EXCLUDE REGEX "keybinds\\.c$")

file(GLOB LUFIA2_GENERATED_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/gen/*.c"
)

if(NOT LUFIA2_GENERATED_SOURCES)
    message(FATAL_ERROR "No generated C files were found in src/gen.")
endif()

add_executable(Lufia2Recomp
    ${SNESRECOMP_RUNNER_SOURCES}
    src/main.c
    src/desktop_glue.c
    src/lufia2_game_info.c
    src/lufia2_runtime.c
    src/gen_stubs.c
    ${LUFIA2_GENERATED_SOURCES}
)

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
else()
    target_compile_options(Lufia2Recomp PRIVATE
        -w
        -Wno-implicit-function-declaration
        -Wno-error=implicit-function-declaration
    )
endif()

target_link_libraries(Lufia2Recomp PRIVATE
    SDL3::SDL3
    ${SNESRECOMP_RUNNER_LIBRARIES}
)

snesrecomp_target_mmx_config(Lufia2Recomp)

set(RECOMP_UI_ENABLE_MODS OFF CACHE BOOL "" FORCE)
include("${RECOMP_UI_ROOT}/recomp_ui.cmake")

recomp_target_launcher_ui(Lufia2Recomp
    CONSOLE snes
)

if(WIN32 AND TARGET SDL3::SDL3-shared)
    add_custom_command(TARGET Lufia2Recomp POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE:SDL3::SDL3-shared>"
            "$<TARGET_FILE_DIR:Lufia2Recomp>"
    )
endif()
