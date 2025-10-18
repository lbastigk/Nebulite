# Code Coverage for Nebulite

This document explains how to use the code coverage features added to the Nebulite testing suite.

## Prerequisites

You need to install `lcov` for coverage report generation:

```bash
# Ubuntu/Debian
sudo apt install lcov

# Fedora
sudo dnf install lcov

# Arch Linux
sudo pacman -S lcov
```

## Quick Start

### Method 1: Using VS Code Tasks (Recommended)

1. Open the Command Palette (`Ctrl+Shift+P`)
2. Type "Tasks: Run Task"
3. Select `[COVERAGE FULL]` for complete coverage workflow

This will:
- Build Nebulite with coverage instrumentation
- Run all tests with coverage data collection
- Generate an HTML coverage report

### Method 2: Command Line

```bash
# 1. Build with coverage
./build.sh -coverage

# 2. Run tests with coverage
python Scripts/TestingSuite.py --coverage --verbose

# 3. View the generated report
# The report will be in coverage_report/html/index.html
```

### Method 3: Step by Step

```bash
# Build with coverage
./build.sh -coverage

# Run tests (coverage data is collected automatically)
python Scripts/TestingSuite.py --coverage

# Generate report manually
python Scripts/generate_coverage_report.py
```

## Available VS Code Tasks

- **`[COVERAGE BUILD]`**: Build Nebulite with coverage instrumentation
- **`[COVERAGE TEST]`**: Run tests with coverage data collection
- **`[COVERAGE REPORT]`**: Generate HTML report from existing coverage data
- **`[COVERAGE FULL]`**: Complete workflow (build + test + report)

## Command Line Options

### TestingSuite.py Coverage Options

```bash
python Scripts/TestingSuite.py --coverage [options]

Options:
  --coverage       Enable code coverage analysis
  --verbose        Show detailed test output
  --stop          Stop on first test failure
  --config FILE   Use custom test configuration
```

### Coverage Report Generator

```bash
python Scripts/generate_coverage_report.py [options]

Options:
  --clean                Clean all coverage data files
  --collect-only         Only collect coverage files, don't generate report
  --output DIR           Output directory for reports (default: tmp/coverage_report)
  --coverage-data DIR    Directory to collect coverage data (default: tmp/coverage_data)
  --project-name NAME    Project name for reports (default: Nebulite)
```

## Understanding Coverage Reports

The HTML coverage report shows:

- **Line Coverage**: Percentage of code lines executed
- **Function Coverage**: Percentage of functions called  
- **Branch Coverage**: Percentage of conditional branches taken

### Coverage Levels

- **High (85-100%)**: Green - Good coverage
- **Medium (50-84%)**: Yellow - Acceptable coverage
- **Low (0-49%)**: Red - Needs improvement

## Files and Directories

### Generated Files

- `tmp/coverage_data/`: Collected coverage data files (.gcda, .gcno)
- `tmp/coverage_report/`: Generated coverage reports
- `tmp/coverage_report/html/`: HTML coverage report (open index.html)
- `tmp/coverage_report/coverage.info`: Raw LCOV data
- `tmp/coverage_report/coverage_filtered.info`: Filtered LCOV data

### Coverage Binary

- `bin/Nebulite_Coverage`: Instrumented binary for coverage analysis

## Coverage Workflow Details

1. **Build Phase**: Compile with `-fprofile-arcs -ftest-coverage --coverage`
2. **Test Phase**: Run tests using the coverage-instrumented binary
3. **Collection Phase**: Gather .gcda files generated during test execution
4. **Report Phase**: Use lcov/genhtml to create HTML reports

## Troubleshooting

### No Coverage Data Found

```bash
# Check if coverage binary exists
ls -la bin/Nebulite_Coverage

# Rebuild with coverage if missing
./build.sh -coverage

# Check for .gcda files after running tests
find . -name "*.gcda" | head -5
```

### lcov Not Found

```bash
# Install lcov
sudo apt install lcov  # Ubuntu/Debian
sudo dnf install lcov  # Fedora
```

### Low Coverage on External Libraries

This is normal - external libraries (in `external/`) are filtered out of coverage reports to focus on your project code.

## Integration with CI/CD

For automated testing with coverage:

```bash
#!/bin/bash
# Build with coverage
./build.sh -coverage

# Run tests with coverage
python Scripts/TestingSuite.py --coverage --stop

# Generate report
python Scripts/generate_coverage_report.py

# Coverage data is now in coverage_report/html/
```

## Advanced Usage

### Custom Coverage Filters

Edit `Scripts/generate_coverage_report.py` to modify which files are included/excluded:

```python
# In generate_coverage_report function, modify the lcov --remove command:
subprocess.run([
    "lcov", "--remove", lcov_file,
    "*/external/*",     # Exclude external dependencies
    "*/build/*",        # Exclude build artifacts  
    "*/test/*",         # Exclude test files
    "*/custom_path/*",  # Add your custom exclusions
    "--output-file", filtered_lcov
])
```

### Coverage Thresholds

You can check coverage thresholds by parsing the lcov summary:

```bash
# Get line coverage percentage
python Scripts/generate_coverage_report.py 2>&1 | grep "lines\.\.\.\.\.\." | awk '{print $2}'
```