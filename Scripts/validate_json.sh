#!/bin/bash

echo
echo "--------------------------------------"
echo "Scanning ./Resources for JSONC files..."
echo

# Find all JSONC files recursively
json_files=$(find ./Resources -type f -name "*.jsonc")

# Track error count
error_count=0

# Function to strip JSONC comments and convert to valid JSON
strip_jsonc_comments() {
    local file="$1"
    # More robust comment removal for JSONC
    # 1. Remove single-line comments (// ...)
    # 2. Remove multi-line comments (/* ... */) - both single and multi-line
    # 3. Preserve strings that might contain // or /* */
    python3 -c "
import re
import sys

def strip_jsonc_comments(content):
    # This is a simplified approach - a full parser would be more robust
    # Remove single-line comments
    content = re.sub(r'//.*?$', '', content, flags=re.MULTILINE)
    # Remove multi-line comments
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    return content

try:
    with open('$file', 'r') as f:
        content = f.read()
    print(strip_jsonc_comments(content))
except Exception as e:
    print(f'Error processing file: {e}', file=sys.stderr)
    sys.exit(1)
"
}

for file in $json_files; do
    # 1. Validate JSONC syntax by stripping comments and validating as JSON
    if ! strip_jsonc_comments "$file" | jq empty 2>/dev/null; then
        echo "❌ Invalid JSONC syntax in file: $file"
        ((error_count++))
        continue
    fi

    # 2. Extract relevant paths from string values (strip comments first)
    paths=$(strip_jsonc_comments "$file" | jq -r '.. | strings | select(test("^\\.\\/Resources\\/.*\\.(bmp|png|jpg|jpeg|gif|wav|mp3|ogg|jsonc)$"))')

    # 2. Extract relevant paths from string values (strip comments first)
    paths=$(strip_jsonc_comments "$file" | jq -r '.. | strings | select(test("^\\.\\/Resources\\/.*\\.(bmp|png|jpg|jpeg|gif|wav|mp3|ogg|jsonc)$"))')

    while IFS= read -r path; do
        # Skip empty or whitespace-only lines
        [[ -z "${path// }" ]] && continue

        if [ ! -f "$path" ]; then
            echo "❌ Missing file: $path (referenced in $file)"
            ((error_count++))
        fi
    done <<< "$paths"
done

echo
echo "Finished checking. Errors found: $error_count"
echo

# Exit with non-zero code if errors were found
if [ $error_count -gt 0 ]; then
    exit 1
else
    exit 0
fi
