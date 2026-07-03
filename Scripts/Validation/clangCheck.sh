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

# Check if --changed-files argument is provided
if [ "$1" == "--changed-files" ]; then
    # Get the list of changed files from git
    changed_files=$(git ls-files --modified)

    # Filter for C++ source and header files
    changed_files=$(echo "$changed_files" | grep -E '\.(cpp|hpp|h)$')
    # If there are no changed files, exit
    if [ -z "$changed_files" ]; then
        echo "No changed C++ files to lint."
        exit 0
    fi

    echo "Running clang-tidy on changed files only..."
    echo "Changed files:"
    echo "$changed_files"
    clang-tidy -config-file=./.clang-tidy $changed_files -- -std=c++26 -I./include
else
  echo "Running clang-tidy on all files..."
  clang-tidy -config-file=./.clang-tidy ./src/* ./include/* -- -std=c++26 -I./include
fi