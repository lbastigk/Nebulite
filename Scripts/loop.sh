#!/bin/bash

index=1

if [ $# -eq 0 ]; then
    echo "No command provided. Please provide a command to loop."
    exit 1
fi

while true; do
    prefix=$(printf "iteration %05i: " "$index")

    # Run command and prefix every output line
    if ! "$@" 2>&1 | sed "s/^/$prefix/"; then
        echo "${prefix}command failed with exit code $?"
        exit 1
    fi

    ((index++))
done