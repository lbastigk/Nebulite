#!/usr/bin/env python3
"""
Extract Keywords Script for Nebulite NEBS Language

This script extracts keywords from Domain Module header files to generate
syntax highlighting for the NEBS language in VS Code.

Usage:
    Run this script from the project root directory:
    $ python Languages/nebs/nebulite-script-vscode/scripts/extract_keywords.py

The script will:
1. Scan all .cpp files in ./src/DomainModule/
2. Look for lines matching: const std::string *_name = "function name";
3. Extract and split the function names into individual keywords
4. Save keywords to ./Languages/nebs/nebulite-script-vscode/syntaxes/keywords.json
5. Update ./Languages/nebs/nebulite-script-vscode/syntaxes/nebs.tmLanguage.json

Author: Generated for Nebulite project
"""

import os
import re
import json

# Directory containing the header files
INCLUDE_DIR = "./src/DomainModule"

# Regex to match const std::string variables ending with _name
# Matches patterns like: const std::string ClassName::function_name = "value";
CONST_STRING_NAME_REGEX = r'^\s*const\s+std::string\s+\w+::\w+_name\s*=\s*"([^"]+)"'

# NEBS directory location
NEBS_DIRECTORY_LOCATION = "./Languages/nebs/nebulite-script-vscode/"

# Output files
OUTPUT_DIR = os.path.join(NEBS_DIRECTORY_LOCATION, "syntaxes")
OUTPUT_FILE = os.path.join(OUTPUT_DIR, "keywords.json")

def extract_keywords():
    keywords = set()

    # Check if we're running from project root
    if not os.path.exists(INCLUDE_DIR):
        print(f"ERROR: Directory {INCLUDE_DIR} does not exist!")
        print("This script should be run from the project root directory.")
        print("Expected structure:")
        print("  ./include/DomainModule/")
        print("  ./Languages/nebs/nebulite-script-vscode/")
        return sorted(keywords)
    
    print(f"Looking for headers in: {os.path.abspath(INCLUDE_DIR)}")
    print(f"NEBS directory: {os.path.abspath(NEBS_DIRECTORY_LOCATION)}")
    print(f"Output directory: {os.path.abspath(OUTPUT_DIR)}")

    # Walk through the DomainModule directory
    for root, _, files in os.walk(INCLUDE_DIR):
        for file in files:
            if file.endswith(".cpp"):
                file_path = os.path.join(root, file)
                print(f"Processing {file_path}")
                try:
                    with open(file_path, "r", encoding='utf-8') as f:
                        for line_num, line in enumerate(f, 1):
                            match = re.match(CONST_STRING_NAME_REGEX, line)
                            if match:
                                function_name = match.group(1)
                                print(f"Found function name at {file}:{line_num}: '{function_name}'")
                                # Split by whitespace and add each word as a keyword
                                for keyword in function_name.split():
                                    if keyword not in keywords:
                                        keywords.add(keyword)
                                        print(f"  Added keyword: '{keyword}'")
                except Exception as e:
                    print(f"Error processing {file_path}: {e}")

    print(f"Total keywords found: {len(keywords)}")
    return sorted(keywords)

def save_keywords_to_file(keywords):
    # Check if JSON array is empty
    # Throw exception
    if not keywords:
        raise ValueError('No keywords found. Aborting...')

    # Create the syntaxes directory if it doesn't exist
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)
        print(f"Created directory: {OUTPUT_DIR}")

    # Write
    with open(OUTPUT_FILE, "w") as f:
        json.dump(keywords, f, indent=2)
    
    print(f"Saved {len(keywords)} keywords to: {OUTPUT_FILE}")

def update_tmlanguage_file(keywords):
    tmlanguage_path = os.path.join(OUTPUT_DIR, "nebs.tmLanguage.json")

    # Check if tmLanguage file exists
    if not os.path.exists(tmlanguage_path):
        print(f"WARNING: {tmlanguage_path} does not exist. Skipping tmLanguage update.")
        print("You can manually create this file or update it with the keywords from keywords.json")
        return

    try:
        # Load the existing tmLanguage file
        with open(tmlanguage_path, "r") as f:
            tmlanguage = json.load(f)

        # Sort keywords by length in descending order
        keywords = sorted(keywords, key=len, reverse=True)

        # Update the patterns array with the new keywords regex
        keywords_regex = "\\b(" + "|".join(keywords) + ")\\b"
        for pattern in tmlanguage["patterns"]:
            if pattern.get("name") == "keyword.control.nebs":
                pattern["match"] = keywords_regex
                break
        else:
            # If the pattern doesn't exist, add it
            tmlanguage["patterns"].append({
                "name": "keyword.control.nebs",
                "match": keywords_regex
            })

        # Save the updated tmLanguage file
        with open(tmlanguage_path, "w") as f:
            json.dump(tmlanguage, f, indent=2)
        
        print(f"Updated {tmlanguage_path} with {len(keywords)} keywords")
        
    except Exception as e:
        print(f"ERROR updating tmLanguage file: {e}")
        print("Keywords were still saved to keywords.json")

def main():
    # TODO: Use command documentation markdown file instead of parsing source files directly.
    # for now, we exit directly and tell the user this version is deprecated.
    system.exit("This script is deprecated as it does not work with the new naming conventions. Please send a pull request to update it to use the command documentation markdown file instead.")

    keywords = extract_keywords()
    save_keywords_to_file(keywords)
    update_tmlanguage_file(keywords)
    print(f"Extracted {len(keywords)} keywords and saved to {OUTPUT_FILE}")

if __name__ == "__main__":
    main()
