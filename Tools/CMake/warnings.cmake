# Compiler warnings configuration
# This file contains compiler-specific warning configurations

message(STATUS "[COMPILER] Loading compiler warnings configuration...")

# Function to configure warnings for a target
function(configure_warnings target_name)
    message(STATUS "[COMPILER] Configuring warnings for target: ${target_name}")

    # Uncomment to only show warnings in non-release builds
    #if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    #    message(STATUS "[COMPILER] Release build detected, skipping warning configuration for target: ${target_name}")
    #    return()
    #endif()

    # Common base (both compilers)
    set(BASE_WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
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
        message(STATUS "[COMPILER] Applied GCC-specific warnings")
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

        # TODO: Make sure this doesn't flag any stdlib or external code issues
        option(ENABLE_CLANG_TIDY "Run clang-tidy as part of build when using Clang (can be slow)" OFF)
        if(ENABLE_CLANG_TIDY)
            find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy-14 clang-tidy-15)
            if(CLANG_TIDY_EXE)
                # Attach clang-tidy per-target so every target that calls
                # configure_warnings gets the tidy invocation.
                set_target_properties(${target_name} PROPERTIES
                    CXX_CLANG_TIDY
                    "${CLANG_TIDY_EXE};-checks=*,-llvmlibc-*;-header-filter=^${CMAKE_SOURCE_DIR}/(src|include)"
                )
                # Print the property so it's easy to confirm during configure
                get_target_property(_ct_prop ${target_name} CXX_CLANG_TIDY)
                message(STATUS "[COMPILER] CXX_CLANG_TIDY for ${target_name}: ${_ct_prop}")
            else()
                message(WARNING "[COMPILER] ENABLE_CLANG_TIDY is ON but clang-tidy executable not found.")
            endif()
        endif()

        message(STATUS "[COMPILER] Applied Clang warnings")
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
        message(STATUS "[COMPILER] Unknown compiler, no specific warnings applied")
    endif()
endfunction()

message(STATUS "[COMPILER] Compiler warnings configuration loaded successfully")