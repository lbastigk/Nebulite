#!/bin/bash

# Set the workspace folder (this is just a placeholder)
WORKSPACE_FOLDER="\${workspaceFolder}"

# Find all .cpp files, excluding those in the _LIBS directory
find . -name '*.cpp' ! -path '*/_LIBS/*' | while read -r file; do
    # Replace the leading ./ with ${workspaceFolder}
    formatted_file="${WORKSPACE_FOLDER}/${file#./}"
    echo "\"${formatted_file}\","
done
