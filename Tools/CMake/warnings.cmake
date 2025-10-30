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
    
    ########################################
    # GCC
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target_name} PRIVATE
            -Wall                      # Enable most common warnings
            -Wextra                    # Enable extra warnings
            -Wpedantic                 # Enable pedantic warnings
            -Wno-unused-parameter      # Explicitly disable unused parameter warnings due to too many false positives with argc/argv signatures that may be unused
        )
        message(STATUS "Applied GCC-specific warnings")
    ########################################
    # Clang
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${target_name} PRIVATE
            -Wall                           # Enable most common warnings
            -Wextra                         # Enable extra warnings
            -Wpedantic                      # Enable pedantic warnings
            -Wno-unused-parameter           # Explicitly disable unused parameter warnings due to too many false positives with argc/argv signatures that may be unused
            -Wconversion                    # Warn on implicit type conversions that may alter a value
            -Wshadow                        # Warn on variable shadowing
            -Wold-style-cast                # Warn on old-style casts
            -Wnon-virtual-dtor              # Warn if a class with virtual functions is not properly destroyed
            -Wnull-dereference              # Warn on null pointer dereferences
            -Wdouble-promotion              # Warn on double promotion
            -Wformat=2                      # Enable format string warnings
            -Wcast-align                    # Warn on pointer casts that may change alignment
            -Woverloaded-virtual            # Warn on overloaded virtual functions
            -Wmissing-field-initializers    # Warn on missing initializers for struct fields
            -Wfloat-equal                   # Warn on floating-point comparisons
            -Wextra-semi                    # Warn on extra semicolons
            -Wundef                         # Warn on undefined macros
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