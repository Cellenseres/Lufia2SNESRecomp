include_guard(GLOBAL)

function(lufia2_interrupt_entries_available output target)
    set(${output} FALSE PARENT_SCOPE)
    if(NOT target STREQUAL "Lufia2Recomp" OR LUFIA2_DECOMP_REFERENCE_ONLY)
        return()
    endif()
    set(_root "${LUFIA2_DECOMP_RESOLVED_ROOT}")
    set(_bindings_path "${CMAKE_SOURCE_DIR}/recomp/decomp_bindings.toml")
    if(NOT EXISTS "${_root}/include/lufia2/nmi_calls.h" OR
       NOT EXISTS "${_root}/include/lufia2/system.h" OR
       NOT EXISTS "${_root}/metadata/functions.toml" OR
       NOT EXISTS "${_root}/src/system/nmi.c" OR
       NOT EXISTS "${_bindings_path}")
        return()
    endif()
    file(READ "${_root}/include/lufia2/system.h" _system_api)
    file(READ "${_root}/include/lufia2/nmi_calls.h" _calls_api)
    if(NOT _system_api MATCHES "Lufia2NmiSaveInterruptFrame[(]" OR
       NOT _calls_api MATCHES "Lufia2NmiReturnEmptyInterrupt[(]")
        return()
    endif()
    file(READ "${_root}/src/system/nmi.c" _implementation)
    if(NOT _implementation MATCHES "Lufia2NmiReturnEmptyInterrupt[(][^}]*cpu->program_bank != 0u && cpu->program_bank != 0x80u")
        return()
    endif()
    file(READ "${_root}/metadata/functions.toml" _metadata)
    file(READ "${_bindings_path}" _bindings)
    set(_names Lufia2NmiSaveInterruptFrame Lufia2NmiReturnEmptyInterrupt)
    set(_addresses "80:862D" "80:86C0")
    set(_bridges Lufia2DecompBridge_80862D Lufia2DecompBridge_8086C0)
    foreach(_index RANGE 0 1)
        list(GET _names ${_index} _name)
        list(GET _addresses ${_index} _address)
        list(GET _bridges ${_index} _bridge)
        string(REGEX MATCH
            "\\[\\[function\\]\\][^[]*name[ \t]*=[ \t]*\"${_name}\"[^[]*"
            _contract "${_metadata}")
        string(REGEX MATCH
            "\\[\\[binding\\]\\][^[]*address[ \t]*=[ \t]*\"${_address}\"[^[]*"
            _binding "${_bindings}")
        if(NOT _contract MATCHES "address[ \t]*=[ \t]*\"${_address}\"" OR
           NOT _contract MATCHES "status[ \t]*=[ \t]*\"verified\"" OR
           NOT _binding MATCHES "bridge[ \t]*=[ \t]*\"${_bridge}\"")
            return()
        endif()
    endforeach()
    set(${output} TRUE PARENT_SCOPE)
endfunction()
