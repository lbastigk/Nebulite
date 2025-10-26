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
    else()
        set_target_properties(Nebulite PROPERTIES OUTPUT_NAME "Nebulite")
    endif()
endfunction()