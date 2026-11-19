include_guard(GLOBAL)

include("${CMAKE_SOURCE_DIR}/lib/snesrecomp-platform/cmake/SnesRecompSourceContract.cmake")
function(lufia2_prepare_spc_bootstrap sources_var core_root)
    set(_sources "${${sources_var}}")
    set(_input "${CMAKE_BINARY_DIR}/generated/apu-program/apu.c")
    list(FIND _sources "${_input}" _index)
    if(_index EQUAL -1)
        message(FATAL_ERROR "SPC bootstrap requires the reviewed APU program overlay")
    endif()

    file(READ "${_input}" _text)
    _snesrecomp_apu_program_edit(_text
        "static bool apu_program_cycle(Apu *apu);"
        "static bool apu_program_cycle(Apu *apu);\nstatic unsigned RunNativeSpcTransfer(Apu *apu);")
    _snesrecomp_apu_program_edit(_text
        "    apu->cpuCyclesLeft = apu_program_take_over(apu) ? 1 : spc_runOpcode(apu->spc);"
        [=[    if (apu_program_take_over(apu)) {
      apu->cpuCyclesLeft = 1;
    } else {
      const unsigned cycles = RunNativeSpcTransfer(apu);
      apu->cpuCyclesLeft = cycles ? cycles : spc_runOpcode(apu->spc);
    }]=])
    string(APPEND _text "\n#include \"${CMAKE_SOURCE_DIR}/src/lufia2_spc_bootstrap.inc\"\n")

    set(_output "${CMAKE_BINARY_DIR}/generated/lufia2-spc-bootstrap/apu.c")
    file(CONFIGURE OUTPUT "${_output}" CONTENT "${_text}" @ONLY)
    snesrecomp_platform_copy_source_contract("${_input}" "${_output}")
    get_source_file_property(_includes "${_input}" INCLUDE_DIRECTORIES)
    if(NOT _includes STREQUAL "NOTFOUND")
        set_source_files_properties("${_output}" PROPERTIES INCLUDE_DIRECTORIES "${_includes}")
    endif()
    list(REMOVE_AT _sources ${_index})
    list(INSERT _sources ${_index} "${_output}")

    set(_spc_input "${core_root}/runner/src/snes/spc.c")
    list(FIND _sources "${_spc_input}" _spc_index)
    if(_spc_index EQUAL -1)
        message(FATAL_ERROR "SPC resume requires the pinned SPC core")
    endif()
    file(READ "${_spc_input}" _spc_text)
    string(REPLACE "\r\n" "\n" _spc_text "${_spc_text}")
    string(SHA256 _spc_hash "${_spc_text}")
    if(NOT _spc_hash STREQUAL "f4bafd70c3b32b6c0b605f3d8c0b79b621091a19ca1648a7a56f68d116768cb9")
        message(FATAL_ERROR "Review SPC resume against the changed SPC core: ${_spc_hash}")
    endif()

    set(_dispatch "static void spc_doOpcode(Spc* spc, uint8_t opcode) {")
    string(FIND "${_spc_text}" "${_dispatch}" _dispatch_at)
    string(SUBSTRING "${_spc_text}" ${_dispatch_at} -1 _specialized_dispatch)
    string(REPLACE "${_dispatch}"
        "static LUFIA2_SPC_INLINE void SpcResumeDoOpcode(Spc *spc, uint8_t opcode) {"
        _specialized_dispatch "${_specialized_dispatch}")
    string(APPEND _spc_text [=[

#if defined(_MSC_VER)
#define LUFIA2_SPC_INLINE __forceinline
#else
#define LUFIA2_SPC_INLINE inline __attribute__((always_inline))
#endif
]=])
    string(APPEND _spc_text "${_specialized_dispatch}\n#include \"${CMAKE_SOURCE_DIR}/src/lufia2_spc_resume.inc\"\n")
    set(_spc_output "${CMAKE_BINARY_DIR}/generated/lufia2-spc-bootstrap/spc.c")
    file(CONFIGURE OUTPUT "${_spc_output}" CONTENT "${_spc_text}" @ONLY)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_spc_input}")
    snesrecomp_platform_copy_source_contract("${_spc_input}" "${_spc_output}")
    get_source_file_property(_spc_includes "${_spc_input}" INCLUDE_DIRECTORIES)
    if(NOT _spc_includes STREQUAL "NOTFOUND")
        set_source_files_properties("${_spc_output}" PROPERTIES INCLUDE_DIRECTORIES "${_spc_includes}")
    endif()
    set_property(SOURCE "${_spc_output}" APPEND PROPERTY INCLUDE_DIRECTORIES "${core_root}/runner/src/snes")
    list(REMOVE_AT _sources ${_spc_index})
    list(INSERT _sources ${_spc_index} "${_spc_output}")

    set(${sources_var} "${_sources}" PARENT_SCOPE)
endfunction()
