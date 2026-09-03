# MSU-1 test target. Include after the platform subdirectory is added.
# The pack module is compiled straight in, like lufia2_task04_profile_test;
# the test supplies its own host-boot functions and needs no libraries.

set(LUFIA2_MSU_PLATFORM_DIR "${CMAKE_SOURCE_DIR}/lib/snesrecomp-platform")

add_executable(lufia2_msu_pack_test
    tests/test_msu_pack.c
    "${LUFIA2_MSU_PLATFORM_DIR}/src/msu_pack.c")
target_include_directories(lufia2_msu_pack_test PRIVATE
    "${LUFIA2_MSU_PLATFORM_DIR}/include")
add_test(NAME lufia2_msu_pack_test
    COMMAND lufia2_msu_pack_test "${CMAKE_CURRENT_BINARY_DIR}")
