#!/bin/bash

###################################################################
# Settings/Constants/etc.

# Set paths for external libraries
external_include_list=(
    "./external/abseil/"
    "./external/imgui"
    "./external/imgui/backends"
    "./external/rapidjson/include"
    "./external/RmlUi/Backends"
    "./external/RmlUi/Include"
    "./external/RmlUi/Source"
    "./external/SDL3/include"
    "./external/SDL3_image/include"
    "./external/SDL3_ttf/include"
    "./external/stb"
    "./external/tinyexpr"
)

# Files that cause issues like hangs or crashes that will be skipped during the clang-tidy run
# all entries must start with './'
known_offenders=(
    "./src/Module/Domain/Renderer/Audio.cpp"
)

# Map of known .tpp -> .hpp overrides.
# Keys must be the .tpp path (starting with './'), values the .hpp path to use instead.
# Add entries when a .tpp's corresponding .hpp is located elsewhere or has a different name.
declare -A tpp_overrides=(
    # Example:
    # ["./include/Utility/IO/Capture.tpp"]="./include/Utility/IO/Capture.hpp"
    ["./include/Interaction/Execution/FuncTreeArgumentCompletion.tpp"]="./include/Interaction/Execution/FuncTree.hpp"
)

# Required defines
clang_tidy_define=-DRMLUI_SDL_VERSION_MAJOR=3

###################################################################
# Setup

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

# Check clang-tidy file
echo "Checking clang-tidy configuration..."
clang-tidy --verify-config -config-file=.clang-tidy
if [ $? -ne 0 ]; then
    echo "Error: clang-tidy configuration is invalid. Please check the .clang-tidy file."
    exit 1
fi

# Turn list of external includes into a single string for clang-tidy
external_include_args=()
for include in "${external_include_list[@]}"; do
    external_include_args+=(-isystem "$include")
done

# Set the header filter to include .tpp files. Alternative for none: '^$'
clang_tidy_header_filter='.*\.tpp$'

###################################################################
# Functions

run_clang_tidy_from_stdin() {
    # Read a null-delimited list of files from stdin, run clang-tidy on each,
    # accumulate 'warnings treated as errors' into the global total_treated,
    # and preserve existing .tpp special-case handling and status propagation.

    status=0
    total_treated=0

    # Keep track of seen files to avoid processing the same file multiple times
    # This is due to the fact that any .tpp file will be replaced with its corresponding .hpp file, which may already be in the list.
    declare -A seen

    # Process each file from the null-delimited input
    while IFS= read -r -d '' file; do
        # If the file contains a whitespace, this could cause all sorts of issues
        if [[ "$file" =~ [[:space:]] ]]; then
            >&2 echo "Error: File path contains whitespace: $file"
            exit 1
        fi

        # Ensure the file starts with './' to avoid issues with relative paths
        if [[ "$file" != ./* ]]; then
            file="./$file"
        fi

        # If the file is a .tpp file, check the corresponding .hpp file instead, which should include the .tpp file.
        if [[ "$file" == *.tpp ]]; then
            # Use override mapping if present
            if [[ -n "${tpp_overrides["$file"]+x}" ]]; then
                file="${tpp_overrides["$file"]}"

                # Check if the override file exists
                if [ ! -f "$file" ]; then
                    >&2 echo "Error: Specified override file for $file does not exist: ${tpp_overrides["$file"]}"
                    exit 1
                fi
            else
                # Try simple replacement of .tpp with .hpp
                corresponding_hpp="${file%.tpp}.hpp"
                if [ -f "$corresponding_hpp" ]; then
                  file="$corresponding_hpp"
                else
                  # No corresponding hpp found and no override provided
                  >&2 echo "Error: No corresponding .hpp file not found for $file. Please add an override in this script if the .hpp file is located elsewhere or has a different name."
                  exit 1
                fi
            fi
        fi

        # Skip the file if it has already been processed
        if [[ -n "${seen["$file"]}" ]]; then
            continue
        fi
        seen["$file"]=1

        # Check if the file is in the known offenders list
        if [[ " ${known_offenders[*]} " == *" $file "* ]]; then
            >&2 echo "Skipping known offender: $file"
            >&2 echo "If you believe this file should be checked, please remove it from the known offenders list in the script and see if it works."
            continue
        fi

        # Run test
        echo "Running clang-tidy on $file"
        tmpfile=$(mktemp)
        clang-tidy "$file" \
            -warnings-as-errors='*' \
            -header-filter="$clang_tidy_header_filter" \
            -config-file=./.clang-tidy \
            -p ./tmp/build_linux-debug \
            -- -std=c++26 "$clang_tidy_define" \
            -I./include \
            ${external_include_args[@]} \
            >"$tmpfile" 2>&1
        clang_rc=$?

        # print filtered logs for the user
        grep -Ev '^([0-9]+ warnings generated\.|Suppressed [0-9]+ warnings .*|Use -header-filter=.*)' "$tmpfile" || true

        # extract 'xyz warnings treated as errors' from raw output and accumulate
        output=$(grep -Eo '[0-9]+ warnings? treated as errors?' "$tmpfile")
        file_treated=$(echo "$output" | grep -Eo '[0-9]+' | awk '{s+=$1} END{print s+0}')
        rm -f "$tmpfile"
        if [ "$file_treated" -gt 0 ]; then
            total_treated=$((total_treated + file_treated))
        fi

        # Check the return code of clang-tidy and set the status accordingly
        if [ "$clang_rc" -ne 0 ]; then
            #>&2 echo "clang-tidy found $file_treated issues in file: $file"
            status=1
        fi
    done

    echo ""
    echo "Analysis complete."
    echo ""
    echo "Total warnings treated as errors: $total_treated"
    exit "$status"
}

###################################################################
# Main script logic

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
