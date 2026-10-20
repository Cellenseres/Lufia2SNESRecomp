function(lufia2_target_shrine_graphics target)
    set(patch "${CMAKE_SOURCE_DIR}/patches/data/shrine_graphics.bps")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${patch}")
    file(READ "${patch}" patch_bytes HEX)
    string(REGEX REPLACE "(..)" "0x\\1," patch_bytes "${patch_bytes}")
    set(header "${CMAKE_CURRENT_BINARY_DIR}/generated/lufia2_shrine_patch.h")
    file(CONFIGURE OUTPUT "${header}"
        CONTENT "static const uint8_t kLufia2ShrinePatch[] = {@patch_bytes@};\n"
        @ONLY)
    target_sources(${target} PRIVATE "${CMAKE_SOURCE_DIR}/patches/lufia2_shrine_graphics.c")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
endfunction()
