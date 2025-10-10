#==============================================================================
# Python Testing Suite for Nebulite
#==============================================================================

import json
import subprocess
import sys
import os
import fnmatch
from typing import List, Dict, Any, Union

#==============================================================================
# Utility Functions
#==============================================================================

def load_tests_config(path: str) -> Dict[str, Any]:
    """Load the test configuration from a JSON file."""
    with open(path, 'r') as f:
        content = f.read()
    print(f"Loaded file {path} with {len(content.splitlines())} lines")
    return json.loads(content)

def apply_ignore_filters(output: List[str], ignore_patterns: List[str]) -> List[str]:
    """Filter out lines from output that match any of the ignore patterns."""
    filtered_output = []
    for line in output:
        if not any(fnmatch.fnmatch(line, pattern) for pattern in ignore_patterns):
            filtered_output.append(line)
    return filtered_output

#==============================================================================
# Test Runner
#==============================================================================

def run_command(cmd: str, timeout: int) -> Dict[str, Union[List[str], int]]:
    """Run a command and capture stdout and stderr as lists of lines."""
    try:
        # Split command into list to avoid shell=True security issue
        import shlex
        cmd_parts = shlex.split(cmd)
        
        # Separate environment variables from the actual command
        env_vars = {}
        cmd_list = []
        
        for part in cmd_parts:
            if '=' in part and not cmd_list:  # Environment variables come before the command
                key, value = part.split('=', 1)
                env_vars[key] = value
            else:
                cmd_list.append(part)
        
        # Merge with current environment
        import os
        env = os.environ.copy()
        env.update(env_vars)
        
        result = subprocess.run(cmd_list, env=env, capture_output=True, text=True, timeout=timeout)
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

def filter_command_output(output: Dict[str, Union[List[str], int]], ignore_lines: Dict[str, List[str]]) -> Dict[str, Union[List[str], int]]:
    """Apply ignore filters to command output."""
    if isinstance(output['cout'], list):
        output['cout'] = apply_ignore_filters(output['cout'], ignore_lines.get('cout', []))
    if isinstance(output['cerr'], list):
        output['cerr'] = apply_ignore_filters(output['cerr'], ignore_lines.get('cerr', []))
    return output

def validate_test_result(output: Dict[str, Union[List[str], int]], expected: Dict[str, Any], verbose: bool = False) -> bool:
    """Validate if test output matches expected results."""
    passed = True
    
    # Compare expected stdout
    if 'cout' in expected:
        if expected["cout"] is not None and output['cout'] != expected['cout']:
            passed = False
            if verbose:
                print(f"  ✗ cout mismatch\n")
                print(f"    Expected : {expected['cout']}")
                print(f"    Got      : {output['cout']}")
    
    # Compare expected stderr
    if 'cerr' in expected:
        if expected["cerr"] is not None and output['cerr'] != expected['cerr']:
            passed = False
            if verbose:
                print(f"  ✗ cerr mismatch\n    Expected: {expected['cerr']}\n    Got: {output['cerr']}")
    
    # Check exit code
    if output['exit_code'] != 0 and not expected.get('allow_nonzero_exit', False):
        passed = False
        if verbose:
            print(f"  ✗ Nonzero exit code: {output['exit_code']}")
    
    return passed

def run_single_test(binary: str, test: Dict[str, Any], timeout: int, ignore_lines: Dict[str, List[str]], verbose: bool = False) -> Dict[str, Any]:
    """Run a single test and return the result."""
    cmd = f"{binary} {test['command']}"
    expected = test.get('expected', {})
    
    print(f"Test: {cmd}")
    output = run_command(cmd, timeout)
    output = filter_command_output(output, ignore_lines)
    
    passed = validate_test_result(output, expected, verbose)
    
    if passed:
        print("  ✓ Passed")
    else:
        print("  ✗ Failed")
    
    return {
        'cmd': cmd,
        'passed': passed,
        'binary': binary,
        'result': 'PASS' if passed else 'FAIL'
    }

def print_test_summary(passed_tests: int, failed_tests: int, total_tests: int):
    """Print the test results summary."""
    print("\n============== Test Summary ==============")
    print(f"Tests passed: {passed_tests}")
    print(f"Tests failed: {failed_tests}")
    print(f"Total tests:  {total_tests}")
    print("==========================================\n")

def run_testsuite(config: Dict[str, Any], stop_on_fail: bool = False, verbose: bool = False):
    """Run the complete test suite."""
    timeout = config.get('timeout', 40)
    binaries = config.get('binaries', [])
    tests = config.get('tests', [])
    ignore_lines = config.get('ignore_lines', {})

    total_tests = 0
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
            total_tests += 1
            test_result = run_single_test(binary, test, timeout, ignore_lines, verbose)
            
            if test_result['passed']:
                passed_tests += 1
            else:
                failed_tests += 1
                if stop_on_fail:
                    print("Stopping on first failure as requested.")
                    results.append(test_result)
                    break
            
            results.append(test_result)
        
        if stop_on_fail and failed_tests > 0:
            break

    print_test_summary(passed_tests, failed_tests, total_tests)

#==============================================================================
# Main Entry Point
#==============================================================================

def main():
    import argparse
    parser = argparse.ArgumentParser(description="Nebulite Test Suite")
    parser.add_argument('-c', '--config', default='Tools/tests.json', help='Path to tests.json')
    parser.add_argument('-s', '--stop', action='store_true', help='Stop on first failure')
    parser.add_argument('-v', '--verbose', action='store_true', help='Verbose output')
    args = parser.parse_args()

    config = load_tests_config(args.config)
    run_testsuite(config, stop_on_fail=args.stop, verbose=args.verbose)

if __name__ == "__main__":
    main()