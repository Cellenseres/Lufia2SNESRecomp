include_guard(GLOBAL)

# The pinned generic config reader quite correctly rejects game-owned keys.
# Lufia consumes these three values in main.c; a generated source overlay only
# acknowledges their names so the generic pass does not print false errors.
function(lufia2_target_config_overlay target snesrecomp_root)
    if(NOT TARGET "${target}")
        message(FATAL_ERROR "Unknown config-overlay target: ${target}")
    endif()

    set(_upstream
        "${snesrecomp_root}/runner/src/desktop/mmx_config.c")
    if(NOT EXISTS "${_upstream}")
        message(FATAL_ERROR "Missing snesrecomp config source: ${_upstream}")
    endif()

    file(READ "${_upstream}" _source)
    set(_old [=[
    } else if (StringEqualsNoCase(key, "Shader")) {
      g_config.shader = *value ? value : NULL;
      return true;
    }
]=])
    set(_new [=[
    } else if (StringEqualsNoCase(key, "Shader")) {
      g_config.shader = *value ? value : NULL;
      return true;
    } else if (StringEqualsNoCase(key, "VisualPreset") ||
               StringEqualsNoCase(key, "HDMode7") ||
               StringEqualsNoCase(key, "HDMode7Perspective")) {
      /* Game-owned visual settings are parsed by LoadVisualConfig(). */
      return true;
    }
]=])
    string(FIND "${_source}" "${_old}" _anchor)
    if(_anchor EQUAL -1)
        message(FATAL_ERROR
            "The pinned mmx_config.c Graphics context changed")
    endif()
    string(REPLACE "${_old}" "${_new}" _patched "${_source}")

    set(_overlay_dir "${CMAKE_BINARY_DIR}/generated/lufia2-config")
    set(_overlay "${_overlay_dir}/mmx_config.c")
    file(MAKE_DIRECTORY "${_overlay_dir}")
    file(WRITE "${_overlay}" "${_patched}")
    set_source_files_properties("${_overlay}" PROPERTIES
        INCLUDE_DIRECTORIES
            "${snesrecomp_root}/runner/src/desktop;${snesrecomp_root}/runner/src")

    target_sources("${target}" PRIVATE "${_overlay}")
    target_include_directories("${target}" PRIVATE
        "${snesrecomp_root}/runner/src/desktop")
endfunction()
