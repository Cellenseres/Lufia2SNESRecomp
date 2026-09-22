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

    add_executable(Lufia2DecompBridgeVerify EXCLUDE_FROM_ALL
        "${CMAKE_SOURCE_DIR}/tests/decomp_verify/snes_function_verify.c"
        "${CMAKE_SOURCE_DIR}/tests/decomp_verify/lufia2_bbf3_bridge_verify.c"
        "${CMAKE_SOURCE_DIR}/src/decomp_bridge/player_update_bridge.c"
        "${VERIFY_SNESRECOMP_ROOT}/runner/src/snes/interp816.c")

    target_include_directories(Lufia2DecompBridgeVerify PRIVATE
        "${CMAKE_SOURCE_DIR}/tests/decomp_verify"
        "${VERIFY_SNESRECOMP_ROOT}/runner/src"
        "${VERIFY_SNESRECOMP_ROOT}/runner/src/snes")
    target_link_libraries(Lufia2DecompBridgeVerify PRIVATE Lufia2::Decomp)
    target_compile_features(Lufia2DecompBridgeVerify PRIVATE c_std_11)


    add_executable(Lufia2ActorDispatchVerify EXCLUDE_FROM_ALL
        "${CMAKE_SOURCE_DIR}/tests/decomp_verify/snes_function_verify.c"
        "${CMAKE_SOURCE_DIR}/tests/decomp_verify/lufia2_actor_dispatch_verify.c"
        "${VERIFY_SNESRECOMP_ROOT}/runner/src/snes/interp816.c")

    target_include_directories(Lufia2ActorDispatchVerify PRIVATE
        "${CMAKE_SOURCE_DIR}/tests/decomp_verify"
        "${VERIFY_SNESRECOMP_ROOT}/runner/src/snes")
    target_link_libraries(Lufia2ActorDispatchVerify PRIVATE Lufia2::Decomp)
    target_compile_features(Lufia2ActorDispatchVerify PRIVATE c_std_11)

    set(_report "${CMAKE_BINARY_DIR}/generated/DECOMP_VERIFY_REPORT.txt")
    set(_bridge_report
        "${CMAKE_BINARY_DIR}/generated/DECOMP_BRIDGE_VERIFY_REPORT.txt")
    add_custom_target(decomp-verify
        COMMAND "${CMAKE_COMMAND}" -E make_directory
            "${CMAKE_BINARY_DIR}/generated"
        COMMAND "$<TARGET_FILE:Lufia2DecompVerify>"
            "${VERIFY_ROM}"
            --report "${_report}"
        COMMAND "$<TARGET_FILE:Lufia2DecompBridgeVerify>"
            "${VERIFY_ROM}"
            --report "${_bridge_report}"
        COMMAND "$<TARGET_FILE:Lufia2ActorDispatchVerify>"
            "${VERIFY_ROM}"
        DEPENDS Lufia2DecompVerify Lufia2DecompBridgeVerify
            Lufia2ActorDispatchVerify
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Comparing native decomp semantics against original ROM code"
        VERBATIM)

    set_property(TARGET Lufia2DecompVerify PROPERTY FOLDER "Development")
    set_property(TARGET Lufia2DecompBridgeVerify PROPERTY FOLDER "Development")
    set_property(TARGET Lufia2ActorDispatchVerify PROPERTY FOLDER "Development")
    set_property(TARGET decomp-verify PROPERTY FOLDER "Development")
endfunction()
