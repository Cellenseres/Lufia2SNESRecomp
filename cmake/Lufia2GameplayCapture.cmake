# Source copies only. Configured solely by the repository owner.
function(lufia2_capture_replace text_var old new)
    string(REPLACE "${old}" "" _removed "${${text_var}}")
    string(LENGTH "${${text_var}}" _before)
    string(LENGTH "${_removed}" _after)
    string(LENGTH "${old}" _length)
    math(EXPR _actual "${_before} - ${_after}")
    if(NOT _actual EQUAL _length)
        message(FATAL_ERROR "Gameplay capture pinned anchor mismatch: ${old}")
    endif()
    string(REPLACE "${old}" "${new}" _text "${${text_var}}")
    set(${text_var} "${_text}" PARENT_SCOPE)
endfunction()
function(lufia2_prepare_gameplay_capture sources_var core_root)
    set(_result)
    set(_count 0)
    foreach(_input IN LISTS ${sources_var})
        get_filename_component(_name "${_input}" NAME)
        if(_name MATCHES "^(interp_bridge|common_cpu_infra)\\.c$")
            file(READ "${_input}" _text)
            string(REPLACE "\r\n" "\n" _text "${_text}")
            if(_name STREQUAL "interp_bridge.c")
                lufia2_capture_replace(_text [=[        const uint32_t pc_before = ((uint32_t)in.k << 16) | in.pc;]=] [=[        const uint32_t pc_before = ((uint32_t)in.k << 16) | in.pc;
        L2_CAPTURE_PC(pc_before);]=])
                lufia2_capture_replace(_text [=[        int _cyc = interp816_runOpcode(&in);]=] [=[        const int _l2_capture_opcode = L2_CAPTURE_BEFORE(cpu, &in, pc_before, op);
        int _cyc = interp816_runOpcode(&in);]=])
                lufia2_capture_replace(_text [=[        if (auto_quiescent &&
            (progress_write_epoch]=] [=[        if (_l2_capture_opcode) L2_CAPTURE_AFTER(cpu, &in, pc_before, op);
        if (auto_quiescent &&
            (progress_write_epoch]=])
                lufia2_capture_replace(_text [=[            const int has_body  = cpu_dispatch_has_entry(cpu, target);]=] [=[            const int has_body  = cpu_dispatch_has_entry(cpu, target);
            if (_l2_capture_opcode) L2_CAPTURE_CALL(cpu, &in, pc_before, target, op, has_body, bounce_ok);]=])
                lufia2_capture_replace(_text [=[    return value;
}
/* Diagnostic env gates, read once.]=] [=[    L2_CAPTURE_BUS(adr, value, 0);
    return value;
}
/* Diagnostic env gates, read once.]=])
                lufia2_capture_replace(_text [=[    cpu_write8(cpu, (uint8)((adr >> 16) & 0xFF), (uint16)(adr & 0xFFFF), val);]=] [=[    cpu_write8(cpu, (uint8)((adr >> 16) & 0xFF), (uint16)(adr & 0xFFFF), val);
    L2_CAPTURE_BUS(adr, val, 1);]=])
                lufia2_capture_replace(_text [=[    *out = cpu_read16(cpu, (uint8)((adrl >> 16) & 0xFF), (uint16)(adrl & 0xFFFF));]=] [=[    *out = cpu_read16(cpu, (uint8)((adrl >> 16) & 0xFF), (uint16)(adrl & 0xFFFF));
    L2_CAPTURE_BUS(adrl, *out, 2);]=])
                lufia2_capture_replace(_text [=[    cpu_write16(cpu, (uint8)((adrl >> 16) & 0xFF), (uint16)(adrl & 0xFFFF), val);]=] [=[    cpu_write16(cpu, (uint8)((adrl >> 16) & 0xFF), (uint16)(adrl & 0xFFFF), val);
    L2_CAPTURE_BUS(adrl, val, 3);]=])
            endif()
            if(_name STREQUAL "common_cpu_infra.c")
                lufia2_capture_replace(_text [=[    longjmp(g_watchdog_jmp, 1);]=] [=[    L2CaptureAbort();
    longjmp(g_watchdog_jmp, 1);]=])
            endif()
            set(_out "${CMAKE_BINARY_DIR}/generated/gameplay-capture/${_name}")
            file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated/gameplay-capture")
            file(WRITE "${_out}" "#include \"lufia2_gameplay_capture.h\"\n${_text}")
            foreach(_prop IN ITEMS COMPILE_DEFINITIONS COMPILE_OPTIONS COMPILE_FLAGS
                    INCLUDE_DIRECTORIES LANGUAGE COMPILE_DEFINITIONS_RELEASE
                    COMPILE_DEFINITIONS_DEBUG COMPILE_DEFINITIONS_RELWITHDEBINFO
                    COMPILE_DEFINITIONS_MINSIZEREL)
                get_source_file_property(_value "${_input}" "${_prop}")
                if(NOT _value STREQUAL "NOTFOUND")
                    set_property(SOURCE "${_out}" PROPERTY "${_prop}" "${_value}")
                endif()
            endforeach()
            get_filename_component(_parent "${_input}" DIRECTORY)
            set_property(SOURCE "${_out}" APPEND PROPERTY INCLUDE_DIRECTORIES
                "${_parent};${core_root}/runner/src;${core_root}/runner/src/snes")
            list(APPEND _result "${_out}")
            math(EXPR _count "${_count} + 1")
        else()
            list(APPEND _result "${_input}")
        endif()
    endforeach()
    if(NOT _count EQUAL 2)
        message(FATAL_ERROR "Gameplay capture expected two runner units, got ${_count}")
    endif()
    set(${sources_var} "${_result}" PARENT_SCOPE)
endfunction()
