include(CheckCXXCompilerFlag)

function(optimize_rapidjson target)
    check_cxx_compiler_flag("-msse4.2" HAS_SSE4_2)

    if(HAS_SSE4_2)
        target_compile_definitions(${target} PRIVATE RAPIDJSON_SSE42)
        target_compile_options(${target} PRIVATE -msse4.2)
        message(STATUS "Enabling SSE4.2 optimizations for RapidJSON")
    else()
        message(STATUS "SSE4.2 not supported by the compiler, skipping RapidJSON optimizations")
    endif()
endfunction()

