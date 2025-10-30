function(setup_common_sources)
    message(STATUS "Setting up common source files...")

    # Define common source files
    file(GLOB_RECURSE COMMON_SOURCES 
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
    )

    list(APPEND COMMON_SOURCES
        ${TINYEXPR_PATH}/tinyexpr.c
    )

    set(COMMON_SOURCES ${COMMON_SOURCES} PARENT_SCOPE)

    message(STATUS "Common source files set up successfully")
endfunction()