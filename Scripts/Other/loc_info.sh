#!/bin/bash

# Run script from project root
if [ "$(basename "$PWD")" != "Nebulite" ]; then
    echo "Please run this script from the Nebulite project root directory."
    exit 1
fi


# Inform about lines of code:
# Argument 1: directory list
# Argument 2: label
function print_loc_for_dirs {
    DIRS=$1
    LABEL=$2

    CLOC_SETTINGS="--force-lang-def=./Tools/cloc_lang_define.txt $DIRS"
    echo "Lines of code for $LABEL:"
    #cloc $CLOC_SETTINGS 2>/dev/null # Unused, too detailed
    echo ""
    cloc $CLOC_SETTINGS --csv 2>/dev/null | tail -1 | awk -F',' '{total=$3+$4+$5; print "Total lines (including comments and blanks): " total}' 2>/dev/null
    echo ""
}

# Custom lang define, counting:
print_loc_for_dirs "./src ./include" "Source Code (C, C++)"
print_loc_for_dirs "./Scripts" "Scripts (Python, Bash)"
print_loc_for_dirs "./Tools/Tests/ ./Tools/tests.jsonc ./TaskFiles/" "Tests (JSON, JSONC, nebs)"
print_loc_for_dirs "./Resources" "Resources (JSON, JSOC)"