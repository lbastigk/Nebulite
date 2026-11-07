#!/bin/bash

ROOT_DIR="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"

if ! $ROOT_DIR/Scripts/Validation/clangCheck.sh --changed-files ; then
    echo "Error: Clang check failed. Aborting commit."
    exit 1
fi

if ! $ROOT_DIR/Scripts/Validation/segmented_string_view_argument_validator.py ; then
    echo "Error: A transformation function isn't consuming all of its supported arguments. Aborting commit."
    exit 1
fi

if ! $ROOT_DIR/Scripts/Validation/runTestsIfFilesChanged.sh ; then
    echo "Error: Tests failed. Aborting commit."
    exit 1
fi


