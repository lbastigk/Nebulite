#!/bin/bash

# Run clang-tidy on the project files
for file in {src/*,include/*}; do
    # check if path is a file
    if [ ! -f "$file" ]; then
        continue
    fi

    # Add any checks you want to exclude here
    EXCLUDED_WARNINGS="*,-llvmlibc-*"
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-cppcoreguidelines-avoid-do-while"    # do-while loops are fine
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-modernize-use-trailing-return-type"  # return types should be explicit
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-llvm-header-guard"                   # Suppress warnings about header guards, too many false positives
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-fuchsia-default-arguments-calls"     # Suppress warnings about default arguments in function calls, this is just stupid
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-cppcoreguidelines-pro-bounds-pointer-arithmetic" # Suppress warnings about pointer arithmetic, too many false positives
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-altera-id-dependent-backward-branch" # Suppress warnings about backward branches, not relevant for us
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-misc-non-private-member-variables-in-classes"    # Suppress warnings about non-private member variables in classes
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-altera-unroll-loops"                 # Loop unrolling may help later on, but for now we suppress the warnings
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-modernize-use-nodiscard"             # Suppress warnings about missing [[nodiscard]] attributes
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-fuchsia-default-arguments-declarations"          # Default arguments in function declarations are fine
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-readability-avoid-const-params-in-decls"         # Suppress warnings about const parameters in declarations
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-readability-identifier-length"       # Allow for short variable names like i, j, x, y, it etc.
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-cppcoreguidelines-pro-bounds-constant-array-index"    # Ignoring array index out of bounds warnings for now
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-fuchsia-overloaded-operator"         # We allow overloaded operators in our codebase for Capture cout/cerr streams using <<
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-boost*"                              # Suppress all boost related warnings

    # To enable later on
    # 1.) c-arrays are still being used for FuncTree class. Later on we may refactor the entire program to use c++ containers instead of c-arrays.
    EXCLUDED_WARNINGS="$EXCLUDED_WARNINGS,-cppcoreguidelines-avoid-c-arrays,-hicpp-avoid-c-arrays,-modernize-avoid-c-arrays"

    # Run clang-tidy with the specified checks and header filter
    clang-tidy "$file" \
        -p tmp/build_linux-debug \
        -header-filter='^/home/leo/Projects/Nebulite/(src|include)' \
        -checks="*,$EXCLUDED_WARNINGS"
done

