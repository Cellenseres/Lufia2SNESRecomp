include_guard(GLOBAL)

function(lufia2_add_decomp_verifier)
    set(options)
    set(one_value_args SNESRECOMP_ROOT ROM)
    cmake_parse_arguments(VERIFY "${options}" "${one_value_args}" "" ${ARGN})

    if(VITA)
        return()
    endif()
    if(NOT VERIFY_SNESRECOMP_ROOT OR NOT VERIFY_ROM)
        message(FATAL_ERROR
            "lufia2_add_decomp_verifier requires SNESRECOMP_ROOT and ROM")
    endif()
    if(NOT TARGET Lufia2::Decomp)
        message(STATUS
            "Decomp verifier unavailable because Lufia2::Decomp is not present")
        return()
    endif()
    if(NOT EXISTS "${VERIFY_ROM}")
        message(STATUS
            "Decomp verifier unavailable because the supported ROM is absent")
        return()
    endif()

    add_executable(Lufia2DecompVerify EXCLUDE_FROM_ALL
        "${CMAKE_SOURCE_DIR}/tests/decomp_verify/snes_function_verify.c"
        "${CMAKE_SOURCE_DIR}/tests/decomp_verify/lufia2_bbf3_verify.c"
        "${VERIFY_SNESRECOMP_ROOT}/runner/src/snes/interp816.c")

    target_include_directories(Lufia2DecompVerify PRIVATE
        "${CMAKE_SOURCE_DIR}/tests/decomp_verify"
        "${VERIFY_SNESRECOMP_ROOT}/runner/src/snes")
    target_link_libraries(Lufia2DecompVerify PRIVATE Lufia2::Decomp)
    target_compile_features(Lufia2DecompVerify PRIVATE c_std_11)

    set(_report "${CMAKE_BINARY_DIR}/generated/DECOMP_VERIFY_REPORT.txt")
    add_custom_target(decomp-verify
        COMMAND "${CMAKE_COMMAND}" -E make_directory
            "${CMAKE_BINARY_DIR}/generated"
        COMMAND "$<TARGET_FILE:Lufia2DecompVerify>"
            "${VERIFY_ROM}"
            --report "${_report}"
        DEPENDS Lufia2DecompVerify
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Comparing native decomp semantics against original ROM code"
        VERBATIM)

    set_property(TARGET Lufia2DecompVerify PROPERTY FOLDER "Development")
    set_property(TARGET decomp-verify PROPERTY FOLDER "Development")
endfunction()
