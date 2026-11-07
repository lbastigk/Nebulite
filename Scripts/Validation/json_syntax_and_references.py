#!/usr/bin/env python3

import argparse
import os
import re
import sys
import json
from pathlib import Path

def strip_jsonc_comments(content):
    """Remove comments from JSONC content."""
    # Remove single-line comments
    content = re.sub(r'//.*?$', '', content, flags=re.MULTILINE)
    # Remove multi-line comments
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    return content

def validate_json_file(file_path):
    """Validate JSON or JSONC file syntax."""
    try:
        with open(file_path, 'r') as f:
            content = f.read()
        # Strip comments for JSONC files
        if file_path.suffix == '.jsonc':
            content = strip_jsonc_comments(content)
        # Parse JSON to validate
        json.loads(content)
        return True, None
    except Exception as e:
        return False, str(e)

def extract_paths(file_path):
    """Extract paths from JSON or JSONC file."""
    try:
        with open(file_path, 'r') as f:
            content = f.read()
        # Strip comments for JSONC files
        if file_path.suffix == '.jsonc':
            content = strip_jsonc_comments(content)
        # Parse JSON
        data = json.loads(content)
        # Extract paths from string values
        paths = []
        def find_paths(obj):
            if isinstance(obj, str) and re.match(r'^\.\/Resources\/.*\.(bmp|png|jpg|jpeg|gif|wav|mp3|ogg|jsonc)$', obj):
                paths.append(obj)
            elif isinstance(obj, dict):
                for value in obj.values():
                    find_paths(value)
            elif isinstance(obj, list):
                for item in obj:
                    find_paths(item)
        find_paths(data)
        return paths
    except Exception as e:
        return []

def main():
    parser = argparse.ArgumentParser(description="Validate resource JSON files and references.")
    parser.add_argument('-v', '--verbose', action='store_true', help="Show discovery and validation progress.")
    args = parser.parse_args()

    def progress(message):
        if args.verbose:
            print(message, flush=True)

    resources_dir = Path('./Resources')
    print("\n--------------------------------------", flush=True)
    print("Scanning ./Resources for JSON and JSONC files...", flush=True)

    json_files = []
    jsonc_files = []
    for directory, dirnames, filenames in os.walk(resources_dir):
        progress(f"Scanning directory: {directory}")
        for filename in dirnames + filenames:
            file_path = Path(directory) / filename
            if file_path.suffix == '.json':
                json_files.append(file_path)
            elif file_path.suffix == '.jsonc':
                jsonc_files.append(file_path)
    json_files.extend(jsonc_files)
    progress(f"Discovery complete: {len(json_files)} files found.")

    error_count = 0
    files_checked = 0
    references_checked = 0

    for file_path in json_files:
        files_checked += 1
        progress(f"[{files_checked}/{len(json_files)}] Validating: {file_path}")
        # Validate JSON/JSONC syntax
        is_valid, error = validate_json_file(file_path)
        if not is_valid:
            print(f"❌ Invalid syntax in file: {file_path} - {error}")
            error_count += 1
            continue

        # Check internal paths
        progress(f"  Extracting references: {file_path}")
        paths = extract_paths(file_path)
        references_checked += len(paths)
        progress(f"  Found {len(paths)} references.")
        for path in paths:
            progress(f"  Checking reference: {path}")
            if not Path(path).is_file():
                print(f"❌ Missing file: {path} (referenced in {file_path})")
                error_count += 1

    print()
    print("Validation Summary:")
    print("+--------------------+-------+")
    print("| Metric             | Count |")
    print("+--------------------+-------+")
    print(f"| Files Checked      | {files_checked:5} |")
    print(f"| References Checked | {references_checked:5} |")
    print(f"| Errors Found       | {error_count:5} |")
    print("+--------------------+-------+")
    print()

    # Exit with non-zero code if errors were found
    sys.exit(1 if error_count > 0 else 0)

if __name__ == '__main__':
    main()