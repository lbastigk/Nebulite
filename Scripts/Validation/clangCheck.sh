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
    ["./include/Nebulite/Interaction/Execution/FuncTreeArgumentCompletion.tpp"]="./include/Nebulite/Interaction/Execution/FuncTree.hpp"
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

organize_files(){
    # Keep track of seen files to avoid processing the same file multiple times
    # This is due to the fact that any .tpp file will be replaced with its corresponding .hpp file, which may already be in the list.
    declare -A seen

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

        # Print the file to stdout as a null-delimited entry for further processing
        printf '%s\0' "$file"
    done
}

run_clang_tidy_from_stdin() {
    # Read a null-delimited list of files from stdin, run clang-tidy on each,
    # accumulate 'warnings treated as errors' into the global total_treated,
    # and preserve existing .tpp special-case handling and status propagation.
    status=0
    total_warnings=0
    total_errors=0

    # Process each file from the null-delimited input
    while IFS= read -r -d '' file; do
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
        warning_count=$(grep -Eo '[0-9]+ warnings? treated as errors?' "$tmpfile" | grep -Eo '[0-9]+' | awk '{s+=$1} END{print s+0}')
        if [ "$warning_count" -gt 0 ]; then
            total_warnings=$((total_warnings + warning_count))
        fi

        # Extract error count: "x warnings? and y errors? generated." and accumulate
        error_count=$(grep -Eo '[0-9]+ warnings? and [0-9]+ errors? generated\.' "$tmpfile" | grep -Eo '[0-9]+ errors?' | grep -Eo '[0-9]+' | awk '{s+=$1} END{print s+0}')
        if [ "$error_count" -gt 0 ]; then
            total_errors=$((total_errors + error_count))
        fi

        # Check the return code of clang-tidy and set the status accordingly
        if [ "$clang_rc" -ne 0 ]; then
            #>&2 echo "clang-tidy found $file_treated issues in file: $file"
            status=1
        fi
        rm -f "$tmpfile"
    done

    # TODO: Print error count as well
    # TODO: add machine-readable output option for parallel processing and CI integration

    echo ""
    echo "Analysis complete."
    echo ""
    echo "Total warnings treated as errors: $total_warnings"
    echo "Total errors found: $total_errors"
    exit "$status"
}

###################################################################
# Main script logic

# Check if --changed-files argument is provided
tmpfile=$(mktemp)
if [ "$1" == "--changed-files" ]; then
    {
        git diff --name-only
        git diff --cached --name-only
    } | sort -u | grep -E '\.(cpp|hpp|h|tpp)$' | tr '\n' '\0' | organize_files >"$tmpfile"
else
    echo ""
    echo "Running clang-tidy on all files..."
    find ./include ./src \( -name '*.hpp' -o -name '*.cpp' -o -name '*.tpp' \) -print0 | organize_files >"$tmpfile"
fi

if [ ! -s "$tmpfile" ]; then
    echo ""
    echo "No valid changed C++ files to lint after filtering known offenders and .tpp handling."
    exit 0
fi

# Run clang-tidy on the organized list of changed files
echo ""
cat "$tmpfile" | run_clang_tidy_from_stdin
result=$?
rm -f "$tmpfile"
exit "$result"
