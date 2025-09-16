# This script is meant as a replacement for Tests.sh
# Instead of modifying this script with new tests, 
# the plan is to implement a tests.json file:
# tests.json:
# - timeout:int
# - tests:array:
#   - command
#   - expected:
#     - cout:array:string/none
#     - cerr:array:string/none
# - binaries:array:string

# For the expected output, an array of strings is used
# to mimic the line-by-line nature of the output.

#==============================================================================
# Python Testing Suite for Nebulite
#==============================================================================

import json
import subprocess
import sys
import os
from typing import List, Dict, Any

#==============================================================================
# Utility Functions
#==============================================================================

def load_tests_config(path: str) -> Dict[str, Any]:
    """Load the test configuration from a JSON file."""
    with open(path, 'r') as f:
        content = f.read()
    print(f"Loaded file {path} with {len(content.splitlines())} lines")
    return json.loads(content)

#==============================================================================
# Test Runner
#==============================================================================

def run_command(cmd: str, timeout: int) -> Dict[str, List[str]]:
    """Run a command and capture stdout and stderr as lists of lines."""
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=timeout)
        return {
            'cout': result.stdout.splitlines(),
            'cerr': result.stderr.splitlines(),
            'exit_code': result.returncode
        }
    except subprocess.TimeoutExpired:
        return {
            'cout': [],
            'cerr': [f'Timeout after {timeout}s'],
            'exit_code': -1
        }

#==============================================================================
# Test Suite Logic
#==============================================================================

def run_testsuite(config: Dict[str, Any], stop_on_fail: bool = False, verbose: bool = False):
    timeout  = config.get('timeout', 40)
    binaries = config.get('binaries', [])
    tests    = config.get('tests', [])

    total_tests  = 0
    passed_tests = 0
    failed_tests = 0
    results = []

    if len(binaries) == 0:
        print("Warning: No binaries specified!")
    else:
        print(f"Testing {len(binaries)} Binaries")

    for binary in binaries:
        print(f"\n==============================")
        print(f"Testing binary: {binary}")
        print(f"==============================\n")
        for test in tests:
            cmd = f"{binary} {test['command']}"
            expected = test.get('expected', {})
            total_tests += 1
            print(f"Test: {cmd}")
            output = run_command(cmd, timeout)
            passed = True
            # Compare expected output
            if 'cout' in expected:
                if expected["cout"] == None:
                    passed = True
                elif output['cout'] != expected['cout']:
                    passed = False
                    if verbose:
                        print(f"  ✗ cout mismatch\n    Expected: {expected['cout']}\n    Got: {output['cout']}")
            if 'cerr' in expected:
                if expected["cerr"] == None:
                    passed = True
                elif output['cerr'] != expected['cerr']:
                    passed = False
                    if verbose:
                        print(f"  ✗ cerr mismatch\n    Expected: {expected['cerr']}\n    Got: {output['cerr']}")
            if output['exit_code'] != 0 and not expected.get('allow_nonzero_exit', False):
                passed = False
                if verbose:
                    print(f"  ✗ Nonzero exit code: {output['exit_code']}")
            if passed:
                passed_tests += 1
                print("  ✓ Passed")
            else:
                failed_tests += 1
                print("  ✗ Failed")
                if stop_on_fail:
                    print("Stopping on first failure as requested.")
                    results.append({'binary': binary, 'test': cmd, 'result': 'FAIL'})
                    break
            results.append({'binary': binary, 'test': cmd, 'result': 'PASS' if passed else 'FAIL'})
        if stop_on_fail and failed_tests > 0:
            break

    # Summary
    print("\n============== Test Summary ==============")
    print(f"Tests passed: {passed_tests}")
    print(f"Tests failed: {failed_tests}")
    print(f"Total tests:  {total_tests}")
    print("==========================================\n")
    #for r in results:
    #    print(f"Test: {r['test']}\n\t-> {r['result']}")

#==============================================================================
# Main Entry Point
#==============================================================================

def main():
    import argparse
    parser = argparse.ArgumentParser(description="Nebulite Test Suite")
    parser.add_argument('-c', '--config', default='tests.json', help='Path to tests.json')
    parser.add_argument('-s', '--stop', action='store_true', help='Stop on first failure')
    parser.add_argument('-v', '--verbose', action='store_true', help='Verbose output')
    args = parser.parse_args()

    config = load_tests_config(args.config)
    run_testsuite(config, stop_on_fail=args.stop, verbose=args.verbose)

if __name__ == "__main__":
    main()