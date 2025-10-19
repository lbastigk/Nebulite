#!/bin/bash

# Run script from project root
if [ "$(basename "$PWD")" != "Nebulite" ]; then
    echo "Please run this script from the Nebulite project root directory."
    exit 1
fi

# Inform about lines of code:

# Custom lang define, counting:
# - custom .nebs files
# - c++ source files
# - c++ header files
# - .sh and .py scripts
# - custom external for SDL compilation
CLOC_SETTINGS="--force-lang-def=./Tools/cloc_lang_define.txt \
    Resources/ \
    TaskFiles/ \
    src/ \
    include/ \
    Scripts/ \
    external/SDL_Crossplatform_Local/Scripts/ \
    external/SDL_Crossplatform_Local/src/"
echo ""
echo ""
echo "Lines of code for Project + Tests:"
cloc $CLOC_SETTINGS 2>/dev/null
echo ""
cloc $CLOC_SETTINGS --csv 2>/dev/null | tail -1 | awk -F',' '{total=$3+$4+$5; print "Total lines (including comments and blanks): " total}' 2>/dev/null
echo ""