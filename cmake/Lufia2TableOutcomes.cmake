include_guard(GLOBAL)

function(lufia2_table_outcomes_available output target)
    set(${output} FALSE PARENT_SCOPE)
    if(NOT target STREQUAL "Lufia2Recomp" OR LUFIA2_DECOMP_REFERENCE_ONLY)
        return()
    endif()
    set(_names Lufia2SpecialMenuPlaySong Lufia2CardTableRunSelectedRound Lufia2CardTableChooseRoundMode Lufia2CardTablePayRound Lufia2CardTableMarkPairedLines Lufia2CardTableUpdateRoundOutcome Lufia2CardTableEvaluateMatches Lufia2CardTableRestoreTilemap Lufia2CardTableStopAlternateColumns Lufia2CardTablePayAlternateRound Lufia2CardTableMarkAlternatePairedLines Lufia2CardTableCalculateAlternatePayout Lufia2CardTableChooseAlternateLines Lufia2CardTableTryPayStake)
    set(_headers special_menu.h card_table.h card_table.h card_table.h card_table.h card_table.h card_table.h card_table.h card_table.h card_table.h card_table.h card_table.h card_table.h card_table.h)
    set(_addresses "84:A60C" "84:DEB9" "84:DEDC" "84:E334" "84:E3F6" "84:E416" "84:E489" "84:F01A" "84:F4C6" "84:F53C" "84:F589" "84:F5A5" "84:F648" "84:F74E")
    set(_bridges Lufia2DecompBridge_84A60C Lufia2DecompBridge_84DEB9 Lufia2DecompBridge_84DEDC Lufia2DecompBridge_84E334 Lufia2DecompBridge_84E3F6 Lufia2DecompBridge_84E416 Lufia2DecompBridge_84E489 Lufia2DecompBridge_84F01A Lufia2DecompBridge_84F4C6 Lufia2DecompBridge_84F53C Lufia2DecompBridge_84F589 Lufia2DecompBridge_84F5A5 Lufia2DecompBridge_84F648 Lufia2DecompBridge_84F74E)
    set(_metadata_path "${LUFIA2_DECOMP_RESOLVED_ROOT}/metadata/functions.toml")
    set(_bindings_path "${CMAKE_SOURCE_DIR}/recomp/decomp_bindings.toml")
    if(NOT EXISTS "${_metadata_path}" OR NOT EXISTS "${_bindings_path}")
        return()
    endif()
    file(READ "${_metadata_path}" _metadata)
    file(READ "${_bindings_path}" _bindings)
    foreach(_index RANGE 0 13)
        list(GET _names ${_index} _name)
        list(GET _headers ${_index} _header)
        list(GET _addresses ${_index} _address)
        list(GET _bridges ${_index} _bridge)
        set(_path "${LUFIA2_DECOMP_RESOLVED_ROOT}/include/lufia2/${_header}")
        if(NOT EXISTS "${_path}")
            return()
        endif()
        file(READ "${_path}" _api)
        string(REGEX MATCH
            "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"${_name}\"[^[]*"
            _contract "${_metadata}")
        string(REGEX MATCH
            "\\[\\[binding\\]\\][^[]*address[ \t]*=[ \t]*\"${_address}\"[^[]*"
            _binding "${_bindings}")
        if(NOT _api MATCHES "${_name}[(]" OR
           NOT _contract MATCHES "address[ \t]*=[ \t]*\"${_address}\"" OR
           NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"" OR
           NOT _binding MATCHES "bridge[ \t]*=[ \t]*\"${_bridge}\"")
            return()
        endif()
    endforeach()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
