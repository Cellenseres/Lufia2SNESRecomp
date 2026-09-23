include_guard(GLOBAL)

option(LUFIA2_ENABLE_DECOMP
    "Enable verified standalone-decomp replacements when available" ON)
set(LUFIA2_DECOMP_ROOT "" CACHE PATH
    "Optional Lufia2Decomp checkout; otherwise use lib/lufia2-decomp")
option(LUFIA2_DECOMP_REFERENCE_ONLY
    "Expose the decomp dependency without selecting native replacements" OFF)
option(LUFIA2_DECOMP_ALLOW_DRAFT_REPLACEMENTS
    "Allow explicitly opted-in draft replacements for runtime validation" ON)

function(lufia2_write_decomp_fallback_report path reason)
    get_filename_component(_report_dir "${path}" DIRECTORY)
    file(MAKE_DIRECTORY "${_report_dir}")
    file(WRITE "${path}"
        "Lufia II function source report\n"
        "================================\n\n"
        "Standalone decomp replacements used (0):\n"
        "- None\n\n"
        "Decomp integration status:\n"
        "- ${reason}\n\n"
        "All game functions use the generated static recomp/LLE path.\n")
endfunction()

function(lufia2_prepare_decomp)
    set(options)
    set(one_value_args CFG_DIR OUTPUT_DIR REPORT TEXT_REPORT OUT_CFG_DIR)
    cmake_parse_arguments(DECOMP "${options}" "${one_value_args}" "" ${ARGN})
    foreach(required CFG_DIR OUTPUT_DIR REPORT OUT_CFG_DIR)
        if(NOT DECOMP_${required})
            message(FATAL_ERROR "lufia2_prepare_decomp requires ${required}")
        endif()
    endforeach()

    set(_root "${LUFIA2_DECOMP_ROOT}")
    if(NOT _root AND EXISTS "${CMAKE_SOURCE_DIR}/lib/lufia2-decomp/CMakeLists.txt")
        set(_root "${CMAKE_SOURCE_DIR}/lib/lufia2-decomp")
    endif()

    if(NOT LUFIA2_ENABLE_DECOMP)
        message(STATUS "Lufia2 decomp integration OFF; using source cfg files")
        lufia2_write_decomp_fallback_report(
            "${DECOMP_TEXT_REPORT}" "disabled by LUFIA2_ENABLE_DECOMP=OFF")
        set(${DECOMP_OUT_CFG_DIR} "${DECOMP_CFG_DIR}" PARENT_SCOPE)
        set(LUFIA2_DECOMP_AVAILABLE FALSE PARENT_SCOPE)
        set(LUFIA2_DECOMP_TEXT_REPORT
            "${DECOMP_TEXT_REPORT}" PARENT_SCOPE)
        return()
    endif()
    if(NOT _root)
        message(STATUS
            "Lufia2 decomp checkout unavailable; using source cfg files. "
            "Set LUFIA2_DECOMP_ROOT or initialize lib/lufia2-decomp.")
        lufia2_write_decomp_fallback_report(
            "${DECOMP_TEXT_REPORT}" "checkout unavailable")
        set(${DECOMP_OUT_CFG_DIR} "${DECOMP_CFG_DIR}" PARENT_SCOPE)
        set(LUFIA2_DECOMP_AVAILABLE FALSE PARENT_SCOPE)
        set(LUFIA2_DECOMP_TEXT_REPORT
            "${DECOMP_TEXT_REPORT}" PARENT_SCOPE)
        return()
    endif()
    if(NOT EXISTS "${_root}/metadata/functions.toml" OR
       NOT EXISTS "${_root}/CMakeLists.txt")
        message(FATAL_ERROR "LUFIA2_DECOMP_ROOT is not a Lufia2Decomp tree: ${_root}")
    endif()

    get_filename_component(_root "${_root}" ABSOLUTE)
    if(NOT TARGET Lufia2::Decomp)
        add_subdirectory("${_root}" "${CMAKE_BINARY_DIR}/_deps/lufia2-decomp")
    endif()
    if(NOT TARGET Lufia2::Decomp)
        message(FATAL_ERROR "Lufia2Decomp did not define Lufia2::Decomp")
    endif()

    set(_args)
    if(LUFIA2_DECOMP_REFERENCE_ONLY)
        list(APPEND _args --reference-only)
    endif()
    if(LUFIA2_DECOMP_ALLOW_DRAFT_REPLACEMENTS)
        list(APPEND _args --allow-draft)
    endif()
    execute_process(
        COMMAND
            "${CMAKE_COMMAND}" -E env "PYTHONDONTWRITEBYTECODE=1"
            "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/scripts/decomp_manifest.py"
            --decomp-root "${_root}"
            --bindings "${CMAKE_SOURCE_DIR}/recomp/decomp_bindings.toml"
            --cfg-dir "${DECOMP_CFG_DIR}"
            --out-dir "${DECOMP_OUTPUT_DIR}"
            --report "${DECOMP_REPORT}"
            --text-report "${DECOMP_TEXT_REPORT}"
            ${_args}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE _result
        COMMAND_ECHO STDOUT)
    if(NOT _result EQUAL 0)
        message(FATAL_ERROR "Lufia2 decomp manifest generation failed")
    endif()

    set(${DECOMP_OUT_CFG_DIR} "${DECOMP_OUTPUT_DIR}" PARENT_SCOPE)
    set(LUFIA2_DECOMP_AVAILABLE TRUE PARENT_SCOPE)
    set(LUFIA2_DECOMP_RESOLVED_ROOT "${_root}" PARENT_SCOPE)
    set(LUFIA2_DECOMP_TEXT_REPORT "${DECOMP_TEXT_REPORT}" PARENT_SCOPE)
    message(STATUS "Lufia2 decomp source: ${_root}")
    if(LUFIA2_DECOMP_REFERENCE_ONLY)
        message(STATUS "Lufia2 decomp reference-only mode: replacements disabled")
    else()
        if(LUFIA2_DECOMP_ALLOW_DRAFT_REPLACEMENTS)
            message(STATUS "Lufia2 decomp draft validation: explicitly opted-in drafts enabled")
        endif()
        message(STATUS "Lufia2 decomp selection report: ${DECOMP_REPORT}")
    endif()
endfunction()

function(lufia2_add_decomp_bridge target snesrecomp_root)
    if(LUFIA2_DECOMP_TEXT_REPORT)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${LUFIA2_DECOMP_TEXT_REPORT}"
                "$<TARGET_FILE_DIR:${target}>/LUFIA2_FUNCTION_SOURCES.txt")
    endif()
    if(NOT LUFIA2_DECOMP_AVAILABLE)
        return()
    endif()
    set(_bridge_sources
        "${CMAKE_SOURCE_DIR}/src/decomp_bridge/player_update_bridge.c"
        "${CMAKE_SOURCE_DIR}/src/decomp_bridge/actor_bridge.c")
    target_sources(${target} PRIVATE ${_bridge_sources})
    source_group("Decomp Bridge" FILES ${_bridge_sources})
    target_include_directories(${target} PRIVATE
        "${CMAKE_SOURCE_DIR}/recomp"
        "${snesrecomp_root}/runner/src"
        "${snesrecomp_root}/runner/src/snes")
    target_link_libraries(${target} PRIVATE Lufia2::Decomp)
endfunction()
