#!/usr/bin/env bash

set -uo pipefail

count=0

while true; do
    ((count++))

    printf "Run %d ... " "$count"

    if ! ./bin/Nebulite --headless task TaskFiles/Debugging/debug.nebs > /dev/null; then
        echo "FAILED on iteration $count"
        exit 1
    else
        echo "OK"
    fi
done