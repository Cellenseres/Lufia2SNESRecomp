include_guard(GLOBAL)

include("${CMAKE_SOURCE_DIR}/lib/snesrecomp-platform/cmake/SnesRecompSourceContract.cmake")
function(lufia2_prepare_spc_bootstrap sources_var)
    set(_sources "${${sources_var}}")
    set(_input "${CMAKE_BINARY_DIR}/generated/apu-program/apu.c")
    list(FIND _sources "${_input}" _index)
    if(_index EQUAL -1)
        message(FATAL_ERROR "SPC bootstrap requires the reviewed APU program overlay")
    endif()

    file(READ "${_input}" _text)
    _snesrecomp_apu_program_edit(_text
        "static bool apu_program_cycle(Apu *apu);"
        "static bool apu_program_cycle(Apu *apu);\nstatic unsigned RunSpcBootstrap(Apu *apu);")
    _snesrecomp_apu_program_edit(_text
        "    apu->cpuCyclesLeft = apu_program_take_over(apu) ? 1 : spc_runOpcode(apu->spc);"
        [=[    if (apu_program_take_over(apu)) {
      apu->cpuCyclesLeft = 1;
    } else {
      const unsigned cycles = RunSpcBootstrap(apu);
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
    set(${sources_var} "${_sources}" PARENT_SCOPE)
endfunction()
