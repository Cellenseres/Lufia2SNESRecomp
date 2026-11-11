include_guard(GLOBAL)

function(lufia2_battle_row_spans_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/battle.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/battle/battle_frame_rows.c")
    set(_metadata "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}" OR NOT EXISTS "${_metadata}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_source}" _implementation)
    file(READ "${_metadata}" _functions)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${_header}" "${_source}" "${_metadata}")
    if(NOT _api MATCHES "#define LUFIA2_BATTLE_ROW_SPAN_API 1" OR
       NOT _api MATCHES "Lufia2BattleFillFrameRowSpan[(]" OR
       NOT _implementation MATCHES "Lufia2BattleFillFrameRowSpan[(][^;{]*[{]")
        return()
    endif()
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2BattleFrameRow\"[^[]*"
        _contract "${_functions}")
    if(_contract MATCHES "address[ \t]*=[ \t]*\"81:E542\"" AND
       _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M0X0\"" AND
       _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()
