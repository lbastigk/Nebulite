#!/bin/bash

ROOT_DIR="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"

$ROOT_DIR/Scripts/Validation/runTestsIfFilesChanged.sh
$ROOT_DIR/Scripts/Validation/clangCheck.sh --changed-files
