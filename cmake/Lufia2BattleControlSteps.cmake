include_guard(GLOBAL)

function(lufia2_battle_control_steps_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle_control_steps.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_control_steps.c")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}")
        return()
    endif()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_header}" "${_source}")
    file(READ "${_header}" _api)
    file(READ "${_source}" _body)
    if(NOT _api MATCHES "#define[ \t]+LUFIA2_BATTLE_CONTROL_STEPS[ \t]+1([ \t\r\n]|$)" OR
       NOT _api MATCHES "Lufia2BattleControlStep[(]" OR
       NOT _body MATCHES "Lufia2ExecutionResult[ \r\n]+Lufia2BattleControlStep[(]")
        return()
    endif()
    set(_metadata "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_metadata}")
        return()
    endif()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_metadata}")
    file(READ "${_metadata}" _contracts)
    foreach(_pair IN ITEMS
            "Lufia2BattleNmiUploads|85:8DC5"
            "Lufia2BattleRippleWords|85:AA3D"
            "Lufia2BattleQueueTilePatchAt4400|85:9C64"
            "Lufia2BattleQueueMinimalAuxiliaryTilemap|85:9CC0"
            "Lufia2BattleResultWindowWait|81:DE9E")
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
    if(NOT _sources MATCHES "src/battle/battle_control_steps[.]c")
        return()
    endif()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
