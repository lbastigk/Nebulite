function(setup_binary_settings)
    message(STATUS "Setting up binary output settings...")

    # Only add debug info in Debug builds
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g)
    endif()

    # Coverage build configuration
    if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
        add_compile_options(-g --coverage -fprofile-arcs -ftest-coverage)
        add_link_options(--coverage)
        message(STATUS "Coverage build enabled")
    endif()

    add_executable(Nebulite
        ${CMAKE_SOURCE_DIR}/src/main.cpp
        ${COMMON_SOURCES}
    )
    target_compile_options(Nebulite PRIVATE -Wno-system-headers) # Suppress warnings from system headers

    ############################################################
    # Configure binary output directory and naming
    set_target_properties(Nebulite PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin
    )

    # Set output name based on build type
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_target_properties(Nebulite PROPERTIES OUTPUT_NAME "Nebulite_Debug")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Coverage")
        set_target_properties(Nebulite PROPERTIES OUTPUT_NAME "Nebulite_Coverage")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Profiling")
        set_target_properties(Nebulite PROPERTIES OUTPUT_NAME "Nebulite_Profiling")
    else()
        set_target_properties(Nebulite PROPERTIES OUTPUT_NAME "Nebulite")
    endif()
endfunction()

function(lto_fix)
    # Ensure LTO-aware archiver/randlib are chosen early and disable IPO for vendored builds
    if((CMAKE_C_COMPILER MATCHES ".*x86_64-w64-mingw32.*") OR (CMAKE_CXX_COMPILER MATCHES ".*x86_64-w64-mingw32.*"))
        find_program(GCC_AR x86_64-w64-mingw32-gcc-ar)
        find_program(GCC_RANLIB x86_64-w64-mingw32-gcc-ranlib)
        find_program(GCC_NM x86_64-w64-mingw32-gcc-nm)

        if(GCC_AR AND GCC_RANLIB)
            set(CMAKE_AR    "${GCC_AR}"    CACHE STRING "Archiver" FORCE)
            set(CMAKE_RANLIB "${GCC_RANLIB}" CACHE STRING "Ranlib" FORCE)
            if(GCC_NM)
                set(CMAKE_NM "${GCC_NM}" CACHE STRING "NM" FORCE)
            endif()
            message(STATUS "Using LTO-aware archiver: ${CMAKE_AR}, ${CMAKE_RANLIB}")
        else()
            # Best-effort fallback to common wrapper names (may be on PATH)
            set(CMAKE_AR    "x86_64-w64-mingw32-gcc-ar"    CACHE STRING "Archiver" FORCE)
            set(CMAKE_RANLIB "x86_64-w64-mingw32-gcc-ranlib" CACHE STRING "Ranlib" FORCE)
            message(WARNING "Could not find gcc-ar/gcc-ranlib via find_program; forcing wrapper names")
        endif()

        # Avoid producing LTO objects for vendored external libraries
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF CACHE BOOL "Disable IPO/LTO for vendored builds" FORCE)
    endif()
endfunction()