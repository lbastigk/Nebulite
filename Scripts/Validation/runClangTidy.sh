#!/bin/bash

# Run clang-tidy on the project files
for file in {src/*,include/*}; do
    # check if path is a file
    if [ ! -f "$file" ]; then
        continue
    fi

    # Add any checks you want to exclude here
    EXCLUDED_WARNINGS="*,-llvmlibc-*"
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-cppcoreguidelines-avoid-do-while"    # Suppress warnings about do-while loops
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-misc-include-cleaner"                # Suppress warnings about include order
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-modernize-use-trailing-return-type"  # Suppress warnings about "use auto" instead of explicit return type
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-llvm-header-guard"                   # Suppress warnings about header guards, too many false positives
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-fuchsia-default-arguments-calls"     # Suppress warnings about default arguments in function calls, this is just stupid
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-cppcoreguidelines-pro-bounds-pointer-arithmetic" # Suppress warnings about pointer arithmetic, too many false positives
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-altera-id-dependent-backward-branch" # Suppress warnings about backward branches, not relevant for us
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-misc-non-private-member-variables-in-classes"    # Suppress warnings about non-private member variables in classes
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-altera-unroll-loops"                 # Suppress warnings about loop unrolling
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-modernize-use-nodiscard"             # Suppress warnings about missing [[nodiscard]] attributes
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-fuchsia-default-arguments-declarations"          # Suppress warnings about default arguments in function declarations
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-readability-avoid-const-params-in-decls"         # Suppress warnings about const parameters in declarations

    # To enable later on
    # 1.) c-arrays are still being used for FuncTree class. Later on we may refactor the entire program to use c++ containers instead of c-arrays.
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-cppcoreguidelines-avoid-c-arrays,-hicpp-avoid-c-arrays,-modernize-avoid-c-arrays"

    # Run clang-tidy with the specified checks and header filter
    clang-tidy "$file" \
        -p tmp/build_linux-debug \
        -header-filter='^/home/leo/Projects/Nebulite/(src|include)' \
        -checks="*,$EXCLUDED_WARNINGS"
done

