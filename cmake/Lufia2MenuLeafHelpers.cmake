include_guard(GLOBAL)

function(lufia2_menu_contracts_available output)
    set(${output} FALSE PARENT_SCOPE)
    set(_metadata_file "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    if(NOT EXISTS "${_metadata_file}")
        return()
    endif()
    file(READ "${_metadata_file}" _metadata)
    foreach(_item IN LISTS ARGN)
        string(REPLACE "|" ";" _fields "${_item}")
        list(GET _fields 0 _address)
        list(GET _fields 1 _name)
        list(GET _fields 2 _stem)
        set(_area "menu")
        set(_header_name "menu")
        set(_mode "M1X0")
        list(LENGTH _fields _field_count)
        if(_field_count GREATER 3)
            list(GET _fields 3 _area)
            list(GET _fields 4 _header_name)
            list(GET _fields 5 _mode)
        endif()
        set(_header "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/${_header_name}.h")
        if(NOT EXISTS "${_header}")
            return()
        endif()
        file(READ "${_header}" _api)
        set(_source "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/${_area}/${_stem}.c")
        if(NOT EXISTS "${_source}")
            return()
        endif()
        file(READ "${_source}" _implementation)
        string(REGEX MATCH
            "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"${_name}\"[^[]*"
            _contract "${_metadata}")
        if(NOT _api MATCHES "${_name}[(]" OR
           NOT _implementation MATCHES "${_name}[(]" OR
           NOT _contract MATCHES "address[ \t]*=[ \t]*\"${_address}\"" OR
           NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"" OR
           NOT _contract MATCHES "source[ \t]*=[ \t]*\"src/${_area}/${_stem}[.]c\"" OR
           NOT _contract MATCHES "entry_mx[ \t]*=[ \t]*\"${_mode}\"" OR
           NOT _contract MATCHES "exit_mx[ \t]*=[ \t]*\"${_mode}\"")
            return()
        endif()
    endforeach()
    set(${output} TRUE PARENT_SCOPE)
endfunction()

function(lufia2_menu_leaves_available output)
    lufia2_menu_contracts_available(_available
        "80:8AF7|Lufia2MenuResolvePointerOffset|menu_string"
        "82:F4D7|Lufia2MenuCycleNameEntrySelection|name_entry")
    set(${output} ${_available} PARENT_SCOPE)
endfunction()

function(lufia2_fixed_cursor_available output)
    lufia2_menu_contracts_available(_available
        "82:A00F|Lufia2MenuCreateFixedCursorPair|menu_cursor_callers")
    set(${output} ${_available} PARENT_SCOPE)
endfunction()

function(lufia2_post_return_leaves_available output)
    lufia2_menu_contracts_available(_available
        "82:F46C|Lufia2SaveBackupMenuSelection|save_menu_build|menu|menu|M1X0"
        "82:F6BC|Lufia2PartyClearBaseStats|capsule_stat_callers|party|party|M0X0"
        "85:EB5F|Lufia2BattleCheckScratchByte|battle_exit|battle|battle|M1X0")
    set(${output} ${_available} PARENT_SCOPE)
endfunction()

function(lufia2_tile_spell_available output)
    lufia2_menu_contracts_available(_available
        "83:9D57|Lufia2CaveReplaceLowerTileIndex|cave_grid|cave|ancient_cave|M0X0"
        "81:F3E4|Lufia2SpellRecordByteA|item_records|item|item|M1X0")
    set(${output} ${_available} PARENT_SCOPE)
endfunction()

function(lufia2_raster_edge_available output)
    lufia2_menu_contracts_available(_available
        "83:FEC4|Lufia2RasterizeEdge|field_raster_edge|field|raster_edge|M1X0")
    set(${output} ${_available} PARENT_SCOPE)
endfunction()
