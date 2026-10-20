include_guard(GLOBAL)

function(lufia2_scene_rebuild_spans_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/scene_rebuild.h")
    if(NOT EXISTS "${_header}")
        return()
    endif()
    file(READ "${_header}" _api)
    if(NOT _api MATCHES "Lufia2ActorRebuildScenePhase[(]")
        return()
    endif()
    file(READ "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml" _metadata)
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2ActorRebuildSceneState\"[^[]*"
        _contract "${_metadata}")
    if(NOT _contract MATCHES "address[ \t]*=[ \t]*\"83:A82E\"" OR
       NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        return()
    endif()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
