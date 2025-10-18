#==============================================================================
# Python Testing Suite for Nebulite
#==============================================================================

import json
import subprocess
import sys
import os
import fnmatch
import glob
import shutil
from typing import List, Dict, Any, Union

#==============================================================================
# Utility Functions
#==============================================================================

def load_tests_config(path: str) -> Dict[str, Any]:
    """Load the test configuration from a JSON file."""
    with open(path, 'r') as f:
        content = f.read()
    print(f"Loaded file {path} with {len(content.splitlines())} lines")
    config = json.loads(content)
    
    # Process test entries - expand file links
    if 'tests' in config:
        expanded_tests = []
        base_dir = os.path.dirname(path)
        
        for test_entry in config['tests']:
            if isinstance(test_entry, str):
                # It's a file link - load the external test file
                # Use absolute path if provided, otherwise relative to current working directory
                if os.path.isabs(test_entry):
                    test_file_path = test_entry
                else:
                    test_file_path = test_entry  # Relative to project root (current working directory)
                try:
                    print(f"Loading external test file: {test_file_path}")
                    with open(test_file_path, 'r') as test_file:
                        external_tests = json.load(test_file)
                    
                    if isinstance(external_tests, list):
                        # It's an array of tests
                        expanded_tests.extend(external_tests)
                        print(f"  Loaded {len(external_tests)} tests from {test_file_path}")
                    else:
                        print(f"  Warning: {test_file_path} does not contain a JSON array of tests")
                        
                except FileNotFoundError:
                    print(f"  Error: Test file not found: {test_file_path}")
                except json.JSONDecodeError as e:
                    print(f"  Error: Invalid JSON in test file {test_file_path}: {e}")
            else:
                # It's a regular test object
                expanded_tests.append(test_entry)
        
        config['tests'] = expanded_tests
        print(f"Total tests loaded: {len(expanded_tests)}")
    
    return config

def apply_ignore_filters(output: List[str], ignore_patterns: List[str]) -> List[str]:
    """Filter out lines from output that match any of the ignore patterns."""
    filtered_output = []
    for line in output:
        if not any(fnmatch.fnmatch(line, pattern) for pattern in ignore_patterns):
            filtered_output.append(line)
    return filtered_output

def reset_coverage_data():
    """Reset coverage data by removing existing .gcda files."""
    print("Resetting coverage data...")
    for root, dirs, files in os.walk('.'):
        for file in files:
            if file.endswith('.gcda'):
                os.remove(os.path.join(root, file))

def collect_coverage_data(coverage_dir: str = "tmp/coverage_data"):
    """Collect coverage data files to a specified directory."""
    if os.path.exists(coverage_dir):
        shutil.rmtree(coverage_dir)
    os.makedirs(coverage_dir, exist_ok=True)
    
    # Find all .gcda and .gcno files - look specifically in build directory
    gcda_files = []
    gcno_files = []
    
    build_coverage_dir = ".build/coverage"
    search_dirs = [build_coverage_dir] if os.path.exists(build_coverage_dir) else ['.']
    
    for search_dir in search_dirs:
        for root, dirs, files in os.walk(search_dir):
            # Skip tinyexpr external library to avoid missing .gcno errors
            if 'tinyexpr' in root:
                continue
                
            for file in files:
                if file.endswith('.gcda'):
                    gcda_files.append(os.path.join(root, file))
                elif file.endswith('.gcno'):
                    gcno_files.append(os.path.join(root, file))
    
    print(f"Found {len(gcda_files)} .gcda files and {len(gcno_files)} .gcno files")
    
    # Copy files to coverage directory while preserving structure
    for file_path in gcda_files + gcno_files:
        rel_path = os.path.relpath(file_path)
        dest_path = os.path.join(coverage_dir, rel_path)
        os.makedirs(os.path.dirname(dest_path), exist_ok=True)
        shutil.copy2(file_path, dest_path)
    
    return len(gcda_files) > 0

def generate_coverage_report(coverage_dir: str = "tmp/coverage_data", output_dir: str = "tmp/coverage_report", auto_open: bool = True):
    """Generate HTML coverage report using lcov."""
    if not os.path.exists(coverage_dir):
        print("No coverage data found!")
        return False
    
    print("Generating coverage report...")
    
    # Create output directory
    os.makedirs(output_dir, exist_ok=True)
    
    # Run lcov to capture coverage data
    lcov_file = os.path.join(output_dir, "coverage.info")
    
    try:
        # Capture coverage data
        subprocess.run([
            "lcov", "--capture", 
            "--directory", coverage_dir,
            "--output-file", lcov_file,
            "--ignore-errors", "gcov,negative"  # Ignore missing .gcno files and negative counts
        ], check=True, capture_output=True)
        
        # Filter out unwanted files (external dependencies, tests, etc.)
        filtered_lcov = os.path.join(output_dir, "coverage_filtered.info")
        subprocess.run([
            "lcov", "--remove", lcov_file,
            "*/external/*", "*/.build/*", "*/usr/include/*", "*/usr/local/*",
            "--output-file", filtered_lcov,
            "--ignore-errors", "unused"
        ], check=True, capture_output=True)
        
        # Generate HTML report
        subprocess.run([
            "genhtml", filtered_lcov,
            "--output-directory", os.path.join(output_dir, "html"),
            "--title", "Nebulite Code Coverage Report",
            "--num-spaces", "4",
            "--sort", "--function-coverage", "--branch-coverage"
        ], check=True, capture_output=True)
        
        print(f"Coverage report generated in {output_dir}/html/")
        print(f"Open {output_dir}/html/index.html to view the report")
        
        # Automatically open the coverage report if requested
        if auto_open:
            try:
                import webbrowser
                report_path = os.path.abspath(f"{output_dir}/html/index.html")
                print(f"🚀 Opening coverage report in browser...")
                webbrowser.open(f"file://{report_path}")
            except Exception as e:
                print(f"Note: Could not automatically open browser: {e}")
        
        # Print summary
        try:
            result = subprocess.run(["lcov", "--summary", filtered_lcov], 
                                  capture_output=True, text=True, check=True)
            print("\nCoverage Summary:")
            print(result.stdout)
        except subprocess.CalledProcessError:
            pass
        
        return True
        
    except subprocess.CalledProcessError as e:
        print(f"Error generating coverage report: {e}")
        return False
    except FileNotFoundError:
        print("lcov/genhtml not found. Please install lcov package.")
        print("On Ubuntu/Debian: sudo apt install lcov")
        print("On Fedora: sudo dnf install lcov")
        return False

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
    if output['exit_code'] != 0 and not expected.get('allow_nonzero_exit', False) and not expected.get('ignore_exit', False):
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

def run_testsuite(config: Dict[str, Any], stop_on_fail: bool = False, verbose: bool = False, enable_coverage: bool = False, auto_open_coverage: bool = True):
    """Run the complete test suite."""
    timeout = config.get('timeout', 40)
    binaries = config.get('binaries', [])
    tests = config.get('tests', [])
    ignore_lines = config.get('ignore_lines', {})

    # Filter binaries for coverage if enabled
    if enable_coverage:
        # Replace regular binaries with coverage versions
        coverage_binaries = []
        for binary in binaries:
            if './bin/Nebulite --headless' in binary:
                coverage_binaries.append('./bin/Nebulite_Coverage --headless')
            elif './bin/Nebulite_Debug --headless' in binary:
                # For coverage, we only need one instrumented binary
                pass  # Skip debug binary for coverage
            else:
                # Skip Windows binaries for coverage (they don't have coverage support)
                pass
        
        if not coverage_binaries:
            # Fallback: add coverage binary if none found
            coverage_binaries = ['./bin/Nebulite_Coverage --headless']
        
        binaries = coverage_binaries
        print("Running tests with coverage enabled")
        print(f"Using coverage binaries: {binaries}")
        reset_coverage_data()

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
    
    # Generate coverage report if enabled
    if enable_coverage:
        print("\n============== Coverage Report ==============")
        if collect_coverage_data():
            if generate_coverage_report(auto_open=auto_open_coverage):
                # Auto-open message is handled within generate_coverage_report
                pass
        else:
            print("No coverage data found. Make sure you built with coverage enabled.")
        print("=============================================\n")

#==============================================================================
# Main Entry Point
#==============================================================================

def main():
    import argparse
    parser = argparse.ArgumentParser(description="Nebulite Test Suite")
    parser.add_argument('-c', '--config', default='Tools/tests.json', help='Path to tests.json')
    parser.add_argument('-s', '--stop', action='store_true', help='Stop on first failure')
    parser.add_argument('-v', '--verbose', action='store_true', help='Verbose output')
    parser.add_argument('--coverage', action='store_true', help='Enable code coverage analysis')
    parser.add_argument('--no-open', action='store_true', help='Don\'t automatically open coverage report')
    args = parser.parse_args()

    config = load_tests_config(args.config)
    run_testsuite(config, stop_on_fail=args.stop, verbose=args.verbose, enable_coverage=args.coverage, auto_open_coverage=not args.no_open)

if __name__ == "__main__":
    main()