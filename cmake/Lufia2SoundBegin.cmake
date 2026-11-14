include_guard(GLOBAL)

function(lufia2_sound_begin_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/system.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/system/sound_upload.c")
    set(_metadata "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}" OR NOT EXISTS "${_metadata}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_source}" _implementation)
    if(NOT _api MATCHES "Lufia2SoundBeginPayloadStep[(]" OR
       NOT _implementation MATCHES "Lufia2SoundBeginPayloadStep[(][^;{]*[{]")
        return()
    endif()
    file(READ "${_metadata}" _functions)
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2SendSoundPayload\"[^[]*"
        _contract "${_functions}")
    if(_contract MATCHES "address[ \t]*=[ \t]*\"80:9945\"" AND
       _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" AND
       _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        set(${output} TRUE PARENT_SCOPE)
    endif()
endfunction()
