function(setNebuliteMacros target)
    target_compile_definitions(${target} PRIVATE
        NEBULITE_USE_INVOKE_CONTAINER_STRINGMAP
    )
endfunction()