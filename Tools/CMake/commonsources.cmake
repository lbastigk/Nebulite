function(setup_common_sources TARGET)
    message(STATUS "Setting up common source files for target ${TARGET}...")

    file(GLOB_RECURSE COMMON_SOURCES CONFIGURE_DEPENDS
            "${CMAKE_SOURCE_DIR}/src/*.cpp"
    )

    list(APPEND COMMON_SOURCES "${TINYEXPR_PATH}/tinyexpr.c")

    if(TARGET)
        if(TARGET ${TARGET})
            target_sources(${TARGET} PRIVATE ${COMMON_SOURCES})
        else()
            message(WARNING "Target ${TARGET} does not exist; sources will be returned in COMMON_SOURCES")
        endif()
    endif()

    set(COMMON_SOURCES ${COMMON_SOURCES} PARENT_SCOPE)
    message(STATUS "Common source files set up successfully")
endfunction()
