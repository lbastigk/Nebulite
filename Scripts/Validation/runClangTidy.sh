#!/bin/bash

# Run clang-tidy on the project files
for file in {src/*,include/*}; do
    # check if path is a file
    if [ ! -f "$file" ]; then
        continue
    fi

    # Exclude llvmlibc checks (they produce a lot of noisy warnings about
    # libc++/libc implementation details). Adjust as needed.
    clang-tidy "$file" \
        -p tmp/build_linux-debug \
        -header-filter='^/home/leo/Projects/Nebulite/(src|include)' \
        -checks='*,-llvmlibc-*'
    # -checks='-*,readability-make-member-function-const'
done

