include_guard(GLOBAL)

function(lufia2_compose_native_call_owner output source)
    if(source MATCHES "native_call_sdk_context[.]inc|native_call_sdk_child[.]inc")
        message(FATAL_ERROR "Native call owner context was already composed")
    endif()
    string(PREPEND source
        "#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/native_call_owner.h\"\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/native_call_sdk_context.inc\"\n")
    string(APPEND source
        "\n#include \"${CMAKE_SOURCE_DIR}/src/decomp_bridge/native_call_sdk_child.inc\"\n")
    set(${output} "${source}" PARENT_SCOPE)
endfunction()
