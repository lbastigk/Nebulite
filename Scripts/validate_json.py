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
    resources_dir = Path('./Resources')
    json_files = list(resources_dir.rglob('*.json')) + list(resources_dir.rglob('*.jsonc'))

    error_count = 0
    files_checked = 0
    references_checked = 0

    print("\n--------------------------------------")
    print("Scanning ./Resources for JSON and JSONC files...")
    print()

    for file_path in json_files:
        files_checked += 1
        # Validate JSON/JSONC syntax
        is_valid, error = validate_json_file(file_path)
        if not is_valid:
            print(f"❌ Invalid syntax in file: {file_path} - {error}")
            error_count += 1
            continue

        # Check internal paths
        paths = extract_paths(file_path)
        references_checked += len(paths)
        for path in paths:
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