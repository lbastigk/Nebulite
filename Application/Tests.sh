#!/bin/bash

check_binary() {
    local args="$1"
    echo "Running: ./bin/Nebulite $args"

    # Run binary and redirect stderr to a log file
    ./bin/Nebulite $args >/dev/null 2>errors.log
    local exit_code=$?

    if [ $exit_code -ne 0 ]; then
        echo "Binary exited with non-zero status: $exit_code"
        cat errors.log
        echo "Test failed for: $args"
        exit 1
    fi

    if [ -s errors.log ]; then
        echo "Errors found in errors.log for: $args"
        cat errors.log
        echo "Test failed for: $args"
        exit 1
    fi

    echo "Test passed for: $args"
    echo ""
}

# List of test input files
tests=(
    "task TaskFiles/crash_test.txt"
    "task TaskFiles/gravity_classic.txt"
    "task TaskFiles/benchmark_gravity.txt"
    "task TaskFiles/obj_oob.txt"
)

for test in "${tests[@]}"; do
    check_binary "$test"
done



echo "All tests passed."
exit 0
