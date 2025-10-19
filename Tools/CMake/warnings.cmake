# Compiler warnings configuration
# This file contains compiler-specific warning configurations

message(STATUS "Loading compiler warnings configuration...")

# Function to configure warnings for a target
function(configure_warnings target_name)
    message(STATUS "Configuring warnings for target: ${target_name}")
    
    # Enable unused function detection (only for GCC/Clang)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target_name} PRIVATE
            -Wunused-function          # Warn about unused static functions
            -Wunused-variable          # Warn about unused variables
            -Wunused-parameter         # Warn about unused function parameters
            -Wunreachable-code         # Warn about unreachable code
            -Wall                      # Enable most common warnings
            -Wextra                    # Enable extra warnings
            # -Wpedantic               # Enable pedantic warnings (disabled due to __int128 conflicts in abseil)
        )
        
        # GCC-specific warnings
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${target_name} PRIVATE
                -Wunused-but-set-variable  # Warn about variables that are set but never used
            )
            message(STATUS "Applied GCC-specific warnings")
        endif()
        
        # Clang-specific warnings
        if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options(${target_name} PRIVATE
                -Wdead-code               # Warn about dead code (Clang only)
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