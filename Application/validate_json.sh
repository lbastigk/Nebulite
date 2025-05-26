#!/bin/bash

echo
echo "--------------------------------------"
echo "Scanning ./Resources for JSON files..."
echo

# Find all JSON files recursively
json_files=$(find ./Resources -type f -name "*.json")

# Track error count
error_count=0

for file in $json_files; do
    # 1. Validate JSON syntax
    if ! jq empty "$file" 2>/dev/null; then
        echo "❌ Invalid JSON syntax in file: $file"
        ((error_count++))
        continue
    fi

    # 2. Extract relevant paths from string values
    paths=$(jq -r '.. | strings | select(test("^\\.\\/Resources\\/.*\\.(bmp|png|jpg|jpeg|gif|wav|mp3|ogg|json)$"))' "$file")

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
