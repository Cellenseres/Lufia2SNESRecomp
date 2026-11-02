include_guard(GLOBAL)

function(lufia2_fixed_message_waits_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_action_followups.c")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_source}" _implementation)
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml" _metadata)
    foreach(_item IN ITEMS
        "81:AD96|Lufia2BattleRepeatFixedMessage"
        "81:AEEA|Lufia2BattleDisplayFixedMessage")
        string(REPLACE "|" ";" _fields "${_item}")
        list(GET _fields 0 _address)
        list(GET _fields 1 _name)
        string(REGEX MATCH
            "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"${_name}\"[^[]*"
            _contract "${_metadata}")
        if(NOT _api MATCHES "${_name}[(]" OR
           NOT _implementation MATCHES "${_name}[(]" OR
           NOT _contract MATCHES "address[ \t]*=[ \t]*\"${_address}\"" OR
           NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"" OR
           NOT _contract MATCHES "source[ \t]*=[ \t]*\"src/battle/battle_action_followups[.]c\"" OR
           NOT _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" OR
           NOT _contract MATCHES "exit_mx[ \t]*=[ \t]*\"M1X0\"")
            return()
        endif()
    endforeach()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
