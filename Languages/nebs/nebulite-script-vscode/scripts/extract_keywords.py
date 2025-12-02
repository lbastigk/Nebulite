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
DOC_DIRECTORY = "./doc/Commands.md"

# NEBS directory location
NEBS_DIRECTORY_LOCATION = "./Languages/nebs/nebulite-script-vscode/"

# Output files
OUTPUT_DIR = os.path.join(NEBS_DIRECTORY_LOCATION, "syntaxes")
OUTPUT_FILE = os.path.join(OUTPUT_DIR, "keywords.json")

def extract_keywords():
    keywords = set()

    # Check DOC_DIRECTORY for the list of commands
    # Any line starting with "| `"
    # contains the command name, e.g. | `MyCommand` | This is my command. |
    if not os.path.exists(DOC_DIRECTORY):
        raise FileNotFoundError(f"Documentation file not found: {DOC_DIRECTORY}")
    with open(DOC_DIRECTORY, "r") as f:
        for line in f:
            match = re.match(r'^\|\s*`([^`]+)`\s*\|', line)
            if match:
                command_name = match.group(1)
                # Split command name by spaces and add each part as a keyword
                for keyword in command_name.split():
                    keywords.add(keyword)

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

    # Write a json file
    with open(OUTPUT_FILE, "w") as f:
        json.dump(keywords, f, indent=2)

    # store as plain text for easier diffing
    with open(OUTPUT_FILE.replace(".json", ".txt"), "w") as f:
        for keyword in keywords:
            f.write(f"{keyword}\n")
    
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
    keywords = extract_keywords()
    save_keywords_to_file(keywords)
    update_tmlanguage_file(keywords)
    print(f"Extracted {len(keywords)} keywords and saved to {OUTPUT_FILE}")

if __name__ == "__main__":
    main()
