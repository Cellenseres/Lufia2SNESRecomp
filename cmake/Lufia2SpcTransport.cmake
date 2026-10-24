# Consumer-only instrumentation of the reviewed semantic SPC implementation.
# The standalone library is unchanged. Source drift requires another audit.
function(lufia2_spc_transport_overlay target)
    if(LUFIA2_DECOMP_RESOLVED_ROOT)
        set(_input "${LUFIA2_DECOMP_RESOLVED_ROOT}/src/spc")
    else()
        set(_input "${CMAKE_SOURCE_DIR}/lib/lufia2-decomp/src/spc")
    endif()
    set(_output "${CMAKE_BINARY_DIR}/generated/spc-transport/spc")
    file(MAKE_DIRECTORY "${_output}")
    set(_expected
    spc_driver.c 635010a717414ae0f453b8f0f45a3064cf6eca8476763e3b36153c1e6f10145e
    spc_driver_internal.h 0f894f345143c2c59267275e932a903451085bb461ca21ba51c44c71537ba7a0
    spc_effects.c 14ee8630d62f5d7cea001a878abbff53bbb3905304f89d3632a786e00f4f99aa
    spc_mixer.c 189d09d4d1bb1c742f982b6d183dbc01a794f04712f86381ffadde098684efa4
    spc_music.c b2502bf69fa3925f4397a57a57f762f662b4c7161139326b6fd634f329ce59c8
    spc_ram.h 7865822979fc3b6c07aab4c1238b9fd8e4ef284bfe7c232cef172145f10b950d
    spc_voice.c d5708275d198705af423d8434234f0372652ebc12d321e6275b04255ab0a6f58
    )
    list(LENGTH _expected _length)
    math(EXPR _last "${_length} - 1")
    foreach(_i RANGE 0 ${_last} 2)
        math(EXPR _j "${_i} + 1")
        list(GET _expected ${_i} _name)
        list(GET _expected ${_j} _hash)
        file(READ "${_input}/${_name}" _text)
        string(REPLACE "\r\n" "\n" _text "${_text}")
        string(SHA256 _actual "${_text}")
        if(NOT _actual STREQUAL _hash)
            message(FATAL_ERROR "Review SPC transport access contract for ${_name}: ${_actual}")
        endif()
        if(_name STREQUAL "spc_driver_internal.h")
            string(REPLACE "static inline SpcRam *SpcRamOf" "#define LUFIA2_SPC_TRANSPORT_OBSERVED 0x00010002u\nextern void Lufia2SpcHostAramAccess(const Lufia2SpcDriver *, uint16_t, uint32_t, int);\n\nstatic inline SpcRam *SpcRamOf" _text "${_text}")
            string(REPLACE "    return &SpcRamOf(driver)->layout.vars;" "    Lufia2SpcHostAramAccess(driver, 0, sizeof(SpcVariables), 2);\n    return &SpcRamOf(driver)->layout.vars;" _text "${_text}")
            string(REPLACE "    return &SpcRamOf(driver)->layout.tracks;" "    Lufia2SpcHostAramAccess(driver, SPC_TRACKS_ADDRESS, sizeof(SpcTracks), 1);\n    return &SpcRamOf(driver)->layout.tracks;" _text "${_text}")
            string(REPLACE "    return &SpcRamOf(driver)->layout.settings;" "    Lufia2SpcHostAramAccess(driver, SPC_SETTINGS_ADDRESS, sizeof(SpcSettings), 1);\n    return &SpcRamOf(driver)->layout.settings;" _text "${_text}")
            string(REPLACE "    return driver->aram[address];" "    Lufia2SpcHostAramAccess(driver, address, 1, 0);\n    return driver->aram[address];" _text "${_text}")
            string(REPLACE "    driver->aram[address] = value;" "    Lufia2SpcHostAramAccess(driver, address, 1, 1);\n    driver->aram[address] = value;" _text "${_text}")
        elseif(_name STREQUAL "spc_driver.c")
            string(REPLACE "        SpcWordSet(&entry->start, start);" "        Lufia2SpcHostAramAccess(driver, (uint16_t)(SPC_DIRECTORY_ADDRESS + (slot >> 2) * sizeof(*entry)), sizeof(*entry), 1);\n        SpcWordSet(&entry->start, start);" _text "${_text}")
        endif()
        if(_name MATCHES "[.]c$")
            string(APPEND _text "\n#ifndef LUFIA2_SPC_TRANSPORT_OBSERVED\n#error SPC transport must include its instrumented header\n#endif\n")
            if(_name STREQUAL "spc_driver.c")
                string(APPEND _text "uint32_t Lufia2SpcTransportContract(void) { return LUFIA2_SPC_TRANSPORT_OBSERVED; }\n")
                # A non-final ReceiveBlock cannot reach another queued step:
                # stable input ports permit at most one two-byte pair. Its only
                # direct variable write is cursor.hi at an index wrap; byte
                # payload writes retain their existing explicit observers.
                string(APPEND _text [=[
uint32_t Lufia2SpcTransportTransferVars(const Lufia2SpcDriver *driver,
    uint8_t command, uint8_t check, uint16_t *dirty_byte) {
    *dirty_byte = UINT16_MAX;
    if (driver->wait != LUFIA2_SPC_WAIT_BLOCK_DATA || driver->substep != 1u ||
        !driver->step_count || driver->step_count > LUFIA2_SPC_MAX_STEPS ||
        (driver->steps[0] != STEP_RECEIVE_AT_TRANSFER_CURSOR &&
         driver->steps[0] != STEP_RECEIVE_AT_UPLOAD_CURSOR) || check == PROTOCOL_END)
        return 0;
    if (command == (driver->counter & 0x7fu) && driver->counter >= 254u) {
        const size_t cursor = driver->steps[0] == STEP_RECEIVE_AT_TRANSFER_CURSOR
            ? offsetof(SpcVariables, scratch) + 0x0cu
            : offsetof(SpcVariables, upload_cursor);
        *dirty_byte = (uint16_t)(cursor + 1u);
    }
    return 1;
}
]=])
            endif()
        endif()
        file(WRITE "${_output}/${_name}" "${_text}")
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_input}/${_name}")
    endforeach()
    # Compile the instrumented SPC objects into this consumer. The semantic
    # archive retains its original objects for standalone users and tools;
    # its SPC members are not extracted when the consumer supplies them.
    set(_consumer_sources "")
    foreach(_name IN ITEMS spc_driver spc_music spc_effects spc_voice spc_mixer)
        set(_source "${_output}/${_name}.c")
        list(APPEND _consumer_sources "${_source}")
        set_source_files_properties("${_source}" PROPERTIES
            INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/generated/spc-transport")
    endforeach()
    target_sources(${target} PRIVATE ${_consumer_sources})
endfunction()
