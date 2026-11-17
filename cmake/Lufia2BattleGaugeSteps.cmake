include_guard(GLOBAL)

function(lufia2_battle_gauge_steps_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle_gauge_steps.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_gauges.c")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}")
        return()
    endif()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_header}" "${_source}")
    file(READ "${_header}" _api)
    file(READ "${_source}" _body)
    if(NOT _api MATCHES "#define[ \t]+LUFIA2_BATTLE_GAUGE_STEPS[ \t]+1([ \t\r\n]|$)"
       OR NOT _api MATCHES "#define[ \t]+LUFIA2_GAUGE_BEGIN_OPCODES[ \t]+4u([ \t\r\n]|$)"
       OR NOT _api MATCHES "#define[ \t]+LUFIA2_GAUGE_READ_OPCODES[ \t]+2u([ \t\r\n]|$)"
       OR NOT _api MATCHES "#define[ \t]+LUFIA2_GAUGE_SCALE_OPCODES[ \t]+5u([ \t\r\n]|$)"
       OR NOT _api MATCHES "#define[ \t]+LUFIA2_GAUGE_PART_MAX_OPCODES[ \t]+12u([ \t\r\n]|$)"
       OR NOT _api MATCHES "Lufia2BattleBeginGaugeStep[(]"
       OR NOT _body MATCHES "Lufia2ExecutionResult[ \r\n]+Lufia2BattleBeginGaugeStep[(]"
       OR NOT _api MATCHES "Lufia2BattlePrepareGaugeScaleStep[(]"
       OR NOT _body MATCHES "Lufia2ExecutionResult[ \r\n]+Lufia2BattlePrepareGaugeScaleStep[(]"
       OR NOT _api MATCHES "Lufia2BattleGaugePartStep[(]"
       OR NOT _body MATCHES "Lufia2ExecutionResult[ \r\n]+Lufia2BattleGaugePartStep[(]")
        return()
    endif()
    set(_metadata "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_metadata}")
        return()
    endif()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_metadata}")
    file(READ "${_metadata}" _contracts)
    foreach(_pair IN ITEMS
            "Lufia2BattleFillStatusGauge|81:E308"
            "Lufia2BattleStatusGauge|81:E2AF")
        string(REPLACE "|" ";" _parts "${_pair}")
        list(GET _parts 0 _name)
        list(GET _parts 1 _address)
        string(REGEX MATCH
            "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"${_name}\"[^[]*"
            _contract "${_contracts}")
        if(NOT _contract MATCHES "address[ \t]*=[ \t]*\"${_address}\"" OR
           NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
            return()
        endif()
    endforeach()
    set(_build "${LUFIA2_DECOMP_RESOLVED_ROOT}/CMakeLists.txt")
    if(NOT EXISTS "${_build}")
        return()
    endif()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_build}")
    file(READ "${_build}" _sources)
    if(NOT _sources MATCHES "src/battle/battle_gauges[.]c")
        return()
    endif()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
