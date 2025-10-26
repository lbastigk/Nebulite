function(initialize_git_submodules)
    message(STATUS "Updating git submodules...")
    execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE GIT_SUBMOD_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )

    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(WARNING "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}")
    else()
        message(STATUS "Git submodules updated successfully")
    endif()
endfunction()