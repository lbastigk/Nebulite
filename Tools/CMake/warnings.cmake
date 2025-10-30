# Compiler warnings configuration
# This file contains compiler-specific warning configurations

message(STATUS "Loading compiler warnings configuration...")

# Function to configure warnings for a target
function(configure_warnings target_name)
    message(STATUS "Configuring warnings for target: ${target_name}")

    # Only show warnings in non-release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        message(STATUS "Release build detected, skipping warning configuration for target: ${target_name}")
        return()
    endif()

    # Common base (both compilers)
    set(BASE_WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
        -Wnon-virtual-dtor
        -Woverloaded-virtual
        -Wcast-align
        -Wmissing-field-initializers
        -Wformat=2
        -Wundef
    )
    
    ########################################
    # GCC
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target_name} PRIVATE ${BASE_WARNINGS}
            -Wnull-dereference
            -Wdouble-promotion
            -Wfloat-equal
            -Wshadow
        )
        message(STATUS "Applied GCC-specific warnings")
    ########################################
    # Clang
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${target_name} PRIVATE ${BASE_WARNINGS}
            # Clang-only checks
            -Wnull-dereference
            -Wdouble-promotion
            -Wfloat-equal
            -Wextra-semi
            -Wshadow
        )
        message(STATUS "Applied Clang warnings")
    ########################################
    # MSVC
    elseif(MSVC)
        # MSVC-specific warnings
        target_compile_options(${target_name} PRIVATE
            /W4                        # High warning level
            /WX-                       # Don't treat warnings as errors (can be changed)
        )
        message(STATUS "Applied MSVC warnings")
    ########################################
    # Unknown compiler
    else()
        message(STATUS "Unknown compiler, no specific warnings applied")
    endif()
endfunction()

message(STATUS "Compiler warnings configuration loaded successfully")