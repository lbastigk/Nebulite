# Compiler warnings configuration
message(STATUS "[COMPILER] Loading compiler warnings configuration...")

function(configure_warnings target_name)
    message(STATUS "[COMPILER] Configuring warnings for target: ${target_name}")

    # Find source files under ./src and ./include
    file(GLOB_RECURSE _restricted_sources CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/src/*.c
        ${CMAKE_SOURCE_DIR}/src/*.cc
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/src/*.cxx
        ${CMAKE_SOURCE_DIR}/include/*.c
        ${CMAKE_SOURCE_DIR}/include/*.cc
        ${CMAKE_SOURCE_DIR}/include/*.cpp
        ${CMAKE_SOURCE_DIR}/include/*.cxx
    )

    # Collect external include directories and prepare -isystem flags so headers under external are treated as system headers.
    file(GLOB _external_include_dirs CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/external/*/include
    )
    set(_external_include_flags "")
    if(_external_include_dirs)
        foreach(_inc IN LISTS _external_include_dirs)
            # Use absolute path, prefix with -isystem
            list(APPEND _external_include_flags "-isystem${_inc}")
        endforeach()
    endif()

    ########################################
    # GCC
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(SRC_COMPILE_OPTIONS
            -Wall
            -Wextra
            -Wpedantic
            -Wno-unused-parameter
            -Wcast-align
            -Wmissing-field-initializers
            -Wformat=2
            -Wundef
            -Wnull-dereference
            -Wdouble-promotion
            -Wfloat-equal
            -Wshadow
        )

        # Append external include flags if any
        if(_external_include_flags)
            list(APPEND SRC_COMPILE_OPTIONS ${_external_include_flags})
        endif()

        list(LENGTH _restricted_sources _src_count)
        list(LENGTH SRC_COMPILE_OPTIONS _opt_count)
        if(_src_count GREATER 0 AND _opt_count GREATER 0)
            # Apply per-file to avoid set_source_files_properties being called with an empty/invalid file list.
            foreach(_src IN LISTS _restricted_sources)
                set_source_files_properties(${_src} PROPERTIES COMPILE_OPTIONS "${SRC_COMPILE_OPTIONS}")
            endforeach()
            message(STATUS "[COMPILER] Applied GCC-specific warnings to ${CMAKE_SOURCE_DIR}/src and ${CMAKE_SOURCE_DIR}/include")
        else()
            if(NOT _src_count GREATER 0)
                message(STATUS "[COMPILER] No sources found to apply GCC warnings")
            else()
                message(STATUS "[COMPILER] No GCC compile options configured, skipping per-file apply")
            endif()
        endif()

    ########################################
    # Clang
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(SRC_COMPILE_OPTIONS
                -Weverything
                -Wno-c++98-compat
                -Wno-c++98-compat-pedantic
                -Wno-ctad-maybe-unsupported
                -Wno-padded
        )

        # Suppress unsafe-buffer-usage warnings coming from some external headers (clang)
        list(APPEND SRC_COMPILE_OPTIONS -Wno-unsafe-buffer-usage)

        # Append external include flags so external headers are system headers
        if(_external_include_flags)
            list(APPEND SRC_COMPILE_OPTIONS ${_external_include_flags})
        endif()

        list(LENGTH _restricted_sources _src_count)
        list(LENGTH SRC_COMPILE_OPTIONS _opt_count)
        if(_src_count GREATER 0 AND _opt_count GREATER 0)
            # Apply per-file to avoid set_source_files_properties being called with an empty/invalid file list.
            foreach(_src IN LISTS _restricted_sources)
                set_source_files_properties(${_src} PROPERTIES COMPILE_OPTIONS "${SRC_COMPILE_OPTIONS}")
            endforeach()
            message(STATUS "[COMPILER] Applied Clang warnings to ${CMAKE_SOURCE_DIR}/src and ${CMAKE_SOURCE_DIR}/include")
        else()
            if(NOT _src_count GREATER 0)
                message(STATUS "[COMPILER] No sources found to apply Clang warnings")
            else()
                message(STATUS "[COMPILER] No Clang compile options configured, skipping per-file apply")
            endif()
        endif()

    ########################################
    # MSVC
    elseif(MSVC)
        target_compile_options(${target_name} PRIVATE
            /W4
            /WX-
        )
        message(STATUS "[COMPILER] Applied MSVC warnings to target: ${target_name}")
    else()
        message(STATUS "[COMPILER] Unknown compiler, no specific warnings applied")
    endif()
endfunction()


