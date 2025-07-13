#!/bin/bash
set -e

echoerr() { echo "$@" 1>&2; }

START_DIR=$(pwd)
declare -A test_results

check_binary() {
    local binary="$1"
    local label="$2"

    echo ""
    echo ""
    echo "=============================="
    echo "Running tests for: $label"
    echo "Using binary: $binary"
    echo "=============================="

    # List of test input files
    local tests=(
        "help"
        "echo 1234"
        "task TaskFiles/Tests/crash_test.txt"
        "task TaskFiles/Simulations/gravity_classic.txt"
        "task TaskFiles/Tests/obj_oob.txt"
    )

    for args in "${tests[@]}"; do
        echo ""
        echo "Running test args: $args"

        # Run the test, redirect stderr to log file
        $binary $args 2>errors.log
        local exit_code=$?

        if [ $exit_code -ne 0 ]; then
            echoerr "Binary exited with non-zero status: $exit_code for test args: $args"
            cat errors.log
            test_results[$label]="FAIL: '$args' exited $exit_code"
            return 1
        fi

        if [ -s errors.log ]; then
            echoerr "Errors found in errors.log for test args: $args"
            
            # Format and print each line with tab-indentation
            while IFS= read -r line; do
                printf "\t\t\t%s\n" "$line"
            done < errors.log

            # Build multiline string for test_results
            local formatted_errors=""
            while IFS= read -r line; do
                formatted_errors+=$'\t\t\t> '"$line"$'\n'

            done < errors.log

            test_results[$label]=$'FAIL: stderr in '"'$args:'"$'\n'"$formatted_errors"

            return 1
        fi
    done

    echo "All tests passed for $label"
    test_results[$label]="PASS"
    return 0
}

#############################################
# Run tests
t="30s"

# Run Linux tests with timeout
check_binary "timeout $t ./bin/Nebulite_Debug" "Linux Debug"   || echoerr "Linux Release tests failed"
check_binary "timeout $t ./bin/Nebulite"       "Linux Release" || echoerr "Linux Release tests failed"

# Run Windows tests with timeout
check_binary "timeout $t wine ./bin/Nebulite_Debug.exe" "Windows Debug"   || echoerr "Windows Debug tests failed"
check_binary "timeout $t wine ./bin/Nebulite.exe"       "Windows Release" || echoerr "Windows Release tests failed"

cd "$START_DIR"

#############################################
# Summary
echo ""
echo "========== Test Summary =========="
for label in "Linux Debug" "Linux Release" "Windows Debug" "Windows Release"; do
    printf "%-20s : %s\n" "$label" "${test_results[$label]:-NOT RUN}"
done
echo ""
echo "If there are any failed tests, please make sure no taskFiles run longer than the specified timeout of $t under normal circumstances."
