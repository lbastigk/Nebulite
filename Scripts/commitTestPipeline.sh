#!/bin/bash

ROOT_DIR="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"

if ! $ROOT_DIR/Scripts/Validation/clangCheck.sh --changed-files ; then
    echo "Error: Clang check failed. Aborting commit."
    exit 1
fi

if ! $ROOT_DIR/Scripts/Validation/runTestsIfFilesChanged.sh ; then
    echo "Error: Tests failed. Aborting commit."
    exit 1
fi


