#!/bin/bash

# Check if clang-tidy is installed
if ! command -v clang-tidy &> /dev/null; then
    echo "Error: clang-tidy is not installed. Please install it to proceed."
    exit 1
fi

# Ensure we are in the correct directory, must have src and include directories
# Traverse up to the root directory of the project until we find the src and include directories
while [ ! -d "src" ] || [ ! -d "include" ]; do
    cd ..
    if [ "$(pwd)" == "/" ]; then
        echo "Error: Could not find the root directory of the project."
        exit 1
    fi
done

# Check if the .clang-tidy file exists in the root directory
if [ ! -f ".clang-tidy" ]; then
    echo "Error: .clang-tidy configuration file not found in the root directory."
    exit 1
fi

# Set paths for external libraries
external_include_list=(
    "./external/abseil/"
    "./external/imgui"
    "./external/imgui/backends"
    "./external/rapidjson/include"
    "./external/RmlUi/Backends"
    "./external/RmlUi/Include"
    "./external/SDL3/include"
    "./external/SDL3_image/include"
    "./external/SDL3_ttf/include"
    "./external/stb"
    "./external/tinyexpr"
)

external_includes=""
for include in "${external_include_list[@]}"; do
    external_includes="$external_includes -isystem $include"
done

# Match the project build definition so RmlUi's SDL backend headers preprocess correctly.
clang_tidy_define=-DRMLUI_SDL_VERSION_MAJOR=3

# Set the header filter to include .tpp files. Alternative for none: '^$'
clang_tidy_header_filter='.*\.tpp$'

# Check if --changed-files argument is provided
if [ "$1" == "--changed-files" ]; then
    # Get the list of changed files from git
    changed_files=$(git ls-files --modified)

    # Filter for C++ source and header files
    changed_files=$(echo "$changed_files" | grep -E '\.(cpp|hpp|h|tpp)$')
    # If there are no changed files, exit
    if [ -z "$changed_files" ]; then
        echo "No changed C++ files to lint."
        exit 0
    fi

    echo "Running clang-tidy on changed files only..."
    echo "Changed files:"
    echo "$changed_files"

    status=0
    for file in $changed_files; do
        if ! clang-tidy "$file" \
            -warnings-as-errors='*' \
            -header-filter="$clang_tidy_header_filter" \
            -config-file=./.clang-tidy \
            -p ./tmp/build_linux-debug \
            -- -std=c++26 "$clang_tidy_define" -I./include $external_includes
        then
            if [[ "$file" == *.tpp ]]; then
                echo "Warning: clang-tidy failed on $file, but ignoring because it's a .tpp file."
            else
                echo "Error: clang-tidy failed on $file"
                status=1
            fi
        fi
    done
    exit "$status"
else
    echo "Running clang-tidy on all files..."

    PARALLEL=0 # tpp-files likely need to be fixed for parallel execution

    # Check if xargs is installed
    if [ "$PARALLEL" -eq 1 ] && command -v xargs &> /dev/null; then
        find ./include ./src \( -name '*.hpp' -o -name '*.cpp' -o -name '*.tpp' \) -print0 |
        xargs -0 -P"$(nproc)" -I{} bash -c '
            clang-tidy "$1" \
            -warnings-as-errors='*' \
            -header-filter="'"$clang_tidy_header_filter"'" \
            -config-file=./.clang-tidy \
            -- -std=c++26 '"$clang_tidy_define"' -I./include '"$external_includes"' \
            2>&1
        ' _ {}
        status=$?
        exit "$status"
    # Check if gnu parallel is installed
    elif [ "$PARALLEL" -eq 1 ] && command -v parallel &> /dev/null; then
        find ./include ./src \( -name '*.hpp' -o -name '*.cpp' -o -name '*.tpp' \) |
        parallel --jobs "$(nproc)" --line-buffer 'clang-tidy {} -warnings-as-errors="*" -header-filter="'"$clang_tidy_header_filter"'" -config-file=./.clang-tidy -- -std=c++26 '"$clang_tidy_define"' -I./include '"$external_includes"
        status=$?
        exit "$status"
    else
        status=0
        while IFS= read -r -d '' file; do
            if ! clang-tidy "$file" \
                -warnings-as-errors='*' \
                -header-filter="$clang_tidy_header_filter" \
                -config-file=./.clang-tidy \
                -p ./tmp/build_linux-debug \
                -- -std=c++26 "$clang_tidy_define" -I./include $external_includes
            then
                # If file ends with .tpp, clang-tidy seems to fail ignore for now
                if [[ "$file" == *.tpp ]]; then
                    echo "Warning: clang-tidy failed on $file, but ignoring because it's a .tpp file."
                    continue
                else
                    echo "Error: clang-tidy failed on $file"
                    status=1
                fi
            fi
        done < <(find ./include ./src \( -name '*.hpp' -o -name '*.cpp' -o -name '*.tpp' \) -print0)
        exit "$status"
    fi
fi