include_guard(GLOBAL)

function(lufia2_nmi_tilemap_loads_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/nmi_tilemap_steps.h")
    set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/system/nmi_uploads.c")
    set(_metadata "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_header}" OR NOT EXISTS "${_source}" OR NOT EXISTS "${_metadata}")
        return()
    endif()
    file(READ "${_header}" _api)
    file(READ "${_source}" _body)
    if(NOT _api MATCHES "#define LUFIA2_NMI_TILEMAP_LOAD_STEPS 1([\r\n]|$)" OR
       NOT _api MATCHES "Lufia2NmiPrepareTilemapBlock[(]" OR
       NOT _body MATCHES "Lufia2NmiPrepareTilemapBlock[(][^;{]*[{]" OR
       NOT _body MATCHES "case 0x808802u:" OR
       NOT _body MATCHES "case 0x808811u:" OR
       NOT _body MATCHES "case 0x808820u:")
        return()
    endif()
    file(READ "${_metadata}" _functions)
    string(REGEX MATCH
        "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"Lufia2NmiTilemapUploads\"[^[]*"
        _contract "${_functions}")
    if(NOT _contract MATCHES "address[ \t]*=[ \t]*\"80:87FC\"" OR
       NOT _contract MATCHES "entry_mx[ \t]*=[ \t]*\"M1X0\"" OR
       NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"")
        return()
    endif()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
