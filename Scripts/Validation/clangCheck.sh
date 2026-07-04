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

run_clang_tidy_from_stdin() {
    # Read a null-delimited list of files from stdin, run clang-tidy on each,
    # accumulate 'warnings treated as errors' into the global total_treated,
    # and preserve existing .tpp special-case handling and status propagation.

    status=0
    total_treated=0

    # Files that cause issues like hangs or crashes that will be skipped during the clang-tidy run
    # all entries must start with './'
    known_offenders=(
        "./src/Module/Domain/Renderer/Audio.cpp"
    )

    while IFS= read -r -d '' file; do
        # Ensure the file starts with './' to avoid issues with relative paths
        if [[ "$file" != ./* ]]; then
            file="./$file"
        fi

        # Check if the file is in the known offenders list
        if [[ " ${known_offenders[*]} " == *" $file "* ]]; then
            echo "Skipping known offender: $file"
            echo "If you believe this file should be checked, please remove it from the known offenders list in the script and see if it works."
            continue
        fi

        echo "Running clang-tidy on $file"

        tmpfile=$(mktemp)
        clang-tidy "$file" \
            -warnings-as-errors='*' \
            -header-filter="$clang_tidy_header_filter" \
            -config-file=./.clang-tidy \
            -p ./tmp/build_linux-debug \
            -- -std=c++26 "$clang_tidy_define" -I./include $external_includes \
            >"$tmpfile" 2>&1
        clang_rc=$?

        # print filtered logs for the user
        grep -Ev '^([0-9]+ warnings generated\.|Suppressed [0-9]+ warnings .*|Use -header-filter=.*)' "$tmpfile" || true

        # extract 'warnings treated as errors' from raw output and accumulate
        output=$(grep -Eo '[0-9]+ warnings? treated as errors?' "$tmpfile")
        file_treated=$(echo "$output" | grep -Eo '[0-9]+' | awk '{s+=$1} END{print s+0}')

        # Only add to total_treated if file_treated is greater than 0
        if [ "$file_treated" -gt 0 ]; then
            total_treated=$((total_treated + file_treated))
        fi

        rm -f "$tmpfile"

        if [ "$clang_rc" -ne 0 ]; then
            # If file ends with .tpp, clang-tidy seems to fail ignore for now
            if [[ "$file" == *.tpp ]]; then
                echo "Warning: clang-tidy failed on $file, but ignoring because it's a .tpp file."
                continue
            else
                echo "Error: clang-tidy failed on $file"
                status=1
            fi
        fi
    done

    echo ""
    echo "Analysis complete."
    echo ""
    echo "Total warnings treated as errors: $total_treated"
    exit "$status"
}

# Check if --changed-files argument is provided
if [ "$1" == "--changed-files" ]; then
    # Get the list of changed files from git
    changed_files=$(git ls-files --modified)
    #changed_files=$(git diff --cached --name-only) # somehow this won't work with clion, the integrated git gui doesn't stage the files before running checks

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

    # Convert newline-delimited changed_files into null-delimited entries and run helper
    while IFS= read -r file; do
        printf '%s\0' "$file"
    done <<<"$changed_files" | run_clang_tidy_from_stdin
else
    echo "Running clang-tidy on all files..."
    find ./include ./src \( -name '*.hpp' -o -name '*.cpp' -o -name '*.tpp' \) -print0 | run_clang_tidy_from_stdin
fi
