include_guard(GLOBAL)

function(lufia2_battle_effect_cleanup_steps_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle_effect_slot_steps.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_effect_engine.c")
    set(_metadata_path "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}" OR
       NOT EXISTS "${_metadata_path}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_source}" _implementation)
    foreach(_name IN ITEMS Lufia2BattleEffectClearBytesStep)
        if(NOT _api MATCHES "${_name}[(]" OR
           NOT _implementation MATCHES "${_name}[(][^;{]*[{]")
            return()
        endif()
    endforeach()
    file(READ "${_metadata_path}" _metadata)
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2BattlePlayEffect\"[^[]*"
        _contract "${_metadata}")
    if(_contract MATCHES "address[ \t]*=[ \t]*\"81:895E\"" AND
       _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" AND
       _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()

function(lufia2_battle_effect_clear_spans_available output)
    set(${output} FALSE PARENT_SCOPE)
    lufia2_battle_effect_cleanup_steps_available(_steps)
    if(NOT _steps)
        return()
    endif()
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle_effect_slot_steps.h" _api)
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_effect_engine.c" _implementation)
    if(_api MATCHES "Lufia2BattleEffectClearByteSpan[(]" AND
       _api MATCHES "LUFIA2_BATTLE_EFFECT_CLEAR_SPAN_MAX_BYTES[ \t]*=[ \t]*32u" AND
       _implementation MATCHES "Lufia2BattleEffectClearByteSpan[(][^;{]*[{]")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()
