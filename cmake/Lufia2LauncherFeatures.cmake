include_guard(GLOBAL)

# Keep fetched UI sources untouched.
function(lufia2_target_launcher_features target)
    set(_overlay
        "${CMAKE_BINARY_DIR}/generated/snesrecomp-platform/recomp-ui/common/backends/imgui/launcher_imgui.cpp")
    file(READ "${_overlay}" _source)
    set(_anchor "    if (hotkeys_p && !stacked_hotkeys) hotkeys_p->draw(m, &th);")
    set(_features [=[
    if (begin_panel("lufia2_features")) {
        eyebrow("LUFIA II RECOMP FEATURES");
        bool fix_sound_menu = Lufia2SoundMenuFixEnabled();
        if (ImGui::Checkbox("Fix Sound menu corruption (Level 0 bug)", &fix_sound_menu))
            Lufia2SoundMenuSetFixEnabled(fix_sound_menu);
        ImGui::TextWrapped(
            "Keeps left/right in CONFIG > MUSIC on Stereo and Mono. "
            "Prevents the original US game's party-data corruption.");
    }
    end_panel();
    ImGui::Spacing();
    if (hotkeys_p && !stacked_hotkeys) hotkeys_p->draw(m, &th);
]=])
    string(FIND "${_source}" "${_anchor}" _pos)
    if(_pos EQUAL -1)
        message(FATAL_ERROR "The pinned recomp-ui Settings context changed")
    endif()
    string(REPLACE "${_anchor}" "${_features}" _source "${_source}")
    set(_reset "            launcher_model_restore_defaults(m);")
    string(FIND "${_source}" "${_reset}" _pos)
    if(_pos EQUAL -1)
        message(FATAL_ERROR "The pinned recomp-ui Defaults context changed")
    endif()
    string(REPLACE "${_reset}"
        "${_reset}\n            Lufia2SoundMenuSetFixEnabled(true);"
        _source "${_source}")
    file(WRITE "${_overlay}"
        "#include \"patches/lufia2_sound_menu.h\"\n${_source}")
endfunction()
