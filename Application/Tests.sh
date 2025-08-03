#!/bin/bash
echo "    _   ____________  __  ____    ________________   ____________________________";
echo "   / | / / ____/ __ )/ / / / /   /  _/_  __/ ____/  /_  __/ ____/ ___/_  __/ ___/";
echo "  /  |/ / __/ / __  / / / / /    / /  / / / __/      / / / __/  \__ \ / /  \__ \ ";
echo " / /|  / /___/ /_/ / /_/ / /____/ /  / / / /___     / / / /___ ___/ // /  ___/ / ";
echo "/_/ |_/_____/_____/\____/_____/___/ /_/ /_____/____/_/ /_____//____//_/  /____/  ";
echo "                                             /_____/                             ";
echo ""
if [ "$EUID" -eq 0 ]; then
  echo "This script should NOT be run as root or with sudo. Please run as a regular user."
  exit 1
fi

#####################################################
# Various declarations and initializations
echoerr() { echo "$@" 1>&2; }
START_DIR=$(pwd)

# Results
declare -A test_results
declare -i total_tests=0
declare -i passed_tests=0
declare -i failed_tests=0

# Settings
TIMEOUT="${NEBULITE_TEST_TIMEOUT:-20s}"
VERBOSE="${NEBULITE_TEST_VERBOSE:-false}"
STOP_ON_FAIL="${NEBULITE_TEST_STOP_ON_FAIL:-false}"

#####################################################
# List of test input files

# TODO: All tests headless, checking for success through print-global and grep?
# tests --output--> matches expected output
# e.g. echo 1234 --output--> 1234
#
# Also, see if the current structure allows for passing arguments that are NOT part of Nebulite:
# so that the '| grep' isn't interpreted as part of the Nebulite command!
tests=(
    "help"                                                              # Simple Executable call: cout of all help commands
    "echo 1234"                                                         # Simple Executable call: custom echo command in Nebulite GlobalSpaceTree
    "set-fps 60 ; wait 30 ; exit"                                       # set-fps forces initialization of renderer
    "task TaskFiles/Tests/Short/snapshot.txt"                           # Test snapshot functionality
    "task TaskFiles/Tests/Short/echo_global_value.txt"
    "task TaskFiles/Tests/Short/print_time.txt"
    "task TaskFiles/Tests/Short/init_print_global.txt"
    "task TaskFiles/Tests/Short/Object_spawn_and_state_printing.txt"
    "task TaskFiles/Tests/Short/Object_out_of_bounds.txt"              # Check if Renderer correctly handles objects going out of bounds
    "task TaskFiles/Tests/Short/gravity_classic_some_frames.txt"       # Classic Solar system with 2 Planets and 1 Sun
    "task TaskFiles/Tests/Short/Pong_for_5s.txt"                       # Running pong, allowing for complex invoke tests
)
#expected_outputs=()

# Test binaries in order of importance
binaries=(
    "./bin/Nebulite_Debug|Linux Debug"
    "./bin/Nebulite|Linux Release"
    "wine ./bin/Nebulite_Debug.exe|Windows Debug"
    "wine ./bin/Nebulite.exe|Windows Release"
)

export WINEDEBUG=-all,err # Set WINEDEBUG to skip all non-critical errors in Wine tests


#####################################################
# Helper function to run provided tests
run_tests() {
    local binary="$1"
    local label="$2"

    encountered_error=false
    
    for args in "${tests[@]}"; do
        echo ""
        echo "Testing: $args"
        
        # Create unique error log for this test
        local error_log="errors_${label// /_}_$$.log"
        
        # Run test with timeout
        if [[ "$VERBOSE" == "true" ]]; then
            # Verbose mode: show stdout, capture stderr
            if ! timeout "$TIMEOUT" $binary $args 2>"$error_log"; then
                local exit_code=$?
                echoerr "  ✗ Test failed with exit code $exit_code"
                if [[ -s "$error_log" ]]; then
                    echoerr "    Error output:"
                    sed 's/^/    /' "$error_log" >&2
                fi
                test_results[$label]="FAIL: Last Fail: '$args' (exit $exit_code)"
                rm -f "$error_log"
                encountered_error=true
            fi
        else
            # Non-verbose mode: suppress stdout, capture stderr
            if ! timeout "$TIMEOUT" $binary $args 1>/dev/null 2>"$error_log"; then
                local exit_code=$?
                echoerr "  ✗ Test failed with exit code $exit_code"
                if [[ -s "$error_log" ]]; then
                    echoerr "    Error output:"
                    sed 's/^/    /' "$error_log" >&2
                fi
                test_results[$label]="FAIL: Last Fail: '$args' (exit $exit_code)"
                rm -f "$error_log"
                encountered_error=true
            fi
        fi
        
        # Check for unexpected stderr output
        if [[ -s "$error_log" ]]; then
            echoerr " Unexpected stderr output:"
            sed 's/^/    /' "$error_log" >&2
            encountered_error=true
        fi

        rm -f "$error_log"

        # Counting logic and feedback
        ((total_tests++))
        if $encountered_error; then
            ((failed_tests++))  
            echo "  ✗ Failed"

            # Stop on first failure if requested
            if [[ "$STOP_ON_FAIL" == "true" ]]; then
                echoerr "Stopping on first failure as requested"
                return 1
            fi
        else
            ((passed_tests++))
            echo "  ✓ Passed"
        fi        
    done

    echo ""
    echo ""
    
    # Let main know if any test failed
    if $encountered_error; then
        return 1
    fi
    return 0
}

#####################################################
# Function to check binary functionality and run tests
check_binary() {
    local binary="$1"
    local label="$2"

    #####################################################
    # Check if basic binary functionality works

    # - $binary help should print text
    ((total_tests++))
    if ! timeout "$TIMEOUT" $binary help 2>/dev/null | grep -q "Help for Nebulite"; then
        ((failed_tests++))
        echoerr "Help command failed for $label"
        test_results[$label]="FAIL: Last Fail: unable to run help command"
        return 1
    fi
    ((passed_tests++))

    # - $binary echo 1234 should print 1234
    ((total_tests++))
    if ! timeout "$TIMEOUT" $binary echo 1234 2>/dev/null | grep -q "1234"; then
        ((failed_tests++))
        echoerr "Echo command failed for $label"
        test_results[$label]="FAIL: Last Fail: unable to run echo command"
        return 1
    fi
    ((passed_tests++))

    # - $binary error <anything> should print error message and return non-zero exit code
    ((total_tests++))
    if ! output=$(timeout "$TIMEOUT" $binary error "      "); then     # Cant supress error output, so we print whitespace
        ((failed_tests++))
        echoerr "Error command failed for $label"
        test_results[$label]="FAIL: Last Fail: unable to run error command"
        return 1
    fi
    ((passed_tests++))

    return 0
}

# Help function to display usage
usage() {
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  -v, --verbose     Enable verbose output"
    echo "  -s, --stop        Stop on first failure"
    echo "  -t, --timeout SEC Set timeout (default: 10s)"
    echo "  -h, --help        Show this help"
    echo ""
    echo ""
    echo "Examples:"
    echo "  $0                        # Run with defaults"
    echo "  $0 -v                     # Run with verbose output"
    echo "  $0 -t 30s -s              # 30 second timeout, stop on fail"
    echo "  $0 --timeout 5s --verbose # 5 second timeout, verbose"
}

# Main execution
main() {
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -v|--verbose)
                VERBOSE="true"
                shift
                ;;
            -s|--stop)
                STOP_ON_FAIL="true"
                shift
                ;;
            -t|--timeout)
                TIMEOUT="$2"
                shift 2
                ;;
            -h|--help)
                usage
                exit 0
                ;;
            *)
                echo "Unknown option: $1"
                usage
                exit 1
                ;;
        esac
    done

    echo "Starting Nebulite test suite..."
    echo "Configuration: TIMEOUT=$TIMEOUT, VERBOSE=$VERBOSE, STOP_ON_FAIL=$STOP_ON_FAIL"
    echo ""
    
    for binary_info in "${binaries[@]}"; do
        IFS='|' read -r binary label <<< "$binary_info"

        echo ""
        echo ""
        echo "=============================="
        echo "Running tests for: $label"
        echo "Using binary: $binary"
        echo "=============================="
        
        # First check if binary is functional
        if ! check_binary "$binary" "$label"; then
            echoerr "Basic tests failed for $label"
            
            # Stop on first failure if requested
            if [[ "$STOP_ON_FAIL" == "true" ]]; then
                echoerr "Stopping on first failure as requested"
                break
            fi
            continue  # Skip to next binary
        fi
        
        # Now run the full test suite
        if ! run_tests "$binary" "$label"; then
            echoerr "Test suite failed for $label"
        else
            test_results[$label]="PASS"
            echo "✓ All tests passed for $label"
        fi
    done
    
    print_summary
    return $(( failed_tests > 0 ? 1 : 0 ))
}

print_summary() {
    echo ""
    echo "============== Test Summary =============="
    echo ""
    echo ""
    printf "%-20s | %-50s\n" "Binary" "Result"
    echo "$(printf '%.0s-' {1..75})"
    
    for label in "Linux Debug" "Linux Release" "Windows Debug" "Windows Release"; do
        local result="${test_results[$label]:-NOT RUN}"
        local status_icon="✗"
        [[ "$result" == "PASS" ]] && status_icon="✓"
        [[ "$result" == "NOT RUN" ]] && status_icon="⚬"
        
        printf "%-20s | %s %-47s\n" "$label" "$status_icon" "$result"
    done
    echo ""
    echo ""
    echo "=========================================="
    
    echo ""
    echo "Tests passed: $passed_tests"
    echo "Tests failed: $failed_tests"
    echo "Total tests:  $total_tests"
}

# Run main function
main "$@"
exit $?