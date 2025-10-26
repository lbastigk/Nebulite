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
    
    # Enable unused function detection (only for GCC/Clang)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target_name} PRIVATE
            -Wno-system-headers        # Suppress warnings from system headers
            -Wunused-function          # Warn about unused static functions
            -Wunused-variable          # Warn about unused variables
            -Wunreachable-code         # Warn about unreachable code
            -Wall                      # Enable most common warnings
            -Wextra                    # Enable extra warnings
            -Wno-unused-parameter      # Explicitly disable unused parameter warnings (overrides -Wall. Disabled due to too many false positives with argc/argv signatures that may be unused)
            # -Wpedantic                 # Enable pedantic warnings (disabled due to __int128 conflicts in abseil)
        )
        
        # GCC-specific warnings
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            # Warn, that GCC is not recommended for this project. Wait a few seconds before continuing.
            message("GCC is not recommended for this project. Consider using Clang for better code analysis.")

            target_compile_options(${target_name} PRIVATE
                -Wunused-but-set-variable  # Warn about variables that are set but never used
            )
            message(STATUS "Applied GCC-specific warnings")
        endif()
        
        # Clang-specific warnings
        if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options(${target_name} PRIVATE
                -Wconversion
                -Wshadow
                -Wold-style-cast
                -Wnon-virtual-dtor
                -Wnull-dereference
                -Wdouble-promotion
                -Wformat=2
                -Wcast-align
                -Woverloaded-virtual
                -Wuseless-cast
                -Wmissing-field-initializers
                -Wfloat-equal
                -Wextra-semi
                -Wundef
            )
            message(STATUS "Applied Clang-specific warnings")
        endif()
        
        message(STATUS "Applied common GCC/Clang warnings")
    elseif(MSVC)
        # MSVC-specific warnings
        target_compile_options(${target_name} PRIVATE
            /W4                        # High warning level
            /WX-                       # Don't treat warnings as errors (can be changed)
        )
        message(STATUS "Applied MSVC warnings")
    else()
        message(STATUS "Unknown compiler, no specific warnings applied")
    endif()
endfunction()

message(STATUS "Compiler warnings configuration loaded successfully")