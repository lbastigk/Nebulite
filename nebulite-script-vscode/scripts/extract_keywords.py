import os
import re
import json

# Directory containing the header files
INCLUDE_DIR = "../include"

# Regex to match bindFunction lines
BIND_FUNCTION_REGEX = r"^\s*bindFunction\([^,]+,\s*\"([a-zA-Z0-9\-]+)\""

# Output file for the keywords
OUTPUT_FILE = "./syntaxes/keywords.json"

def extract_keywords():
    keywords = set()

    # Walk through the include directory
    for root, _, files in os.walk(INCLUDE_DIR):
        for file in files:
            if file.endswith(".h"):
                file_path = os.path.join(root, file)
                print(f"Processing {file_path}")
                with open(file_path, "r") as f:
                    for line in f:
                        match = re.match(BIND_FUNCTION_REGEX, line)
                        if match:
                            keywords.add(match.group(1))

    return sorted(keywords)

def save_keywords_to_file(keywords):
    with open(OUTPUT_FILE, "w") as f:
        json.dump(keywords, f, indent=2)

def update_tmlanguage_file(keywords):
    tmlanguage_path = "./syntaxes/nebs.tmLanguage.json"

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

def main():
    keywords = extract_keywords()
    save_keywords_to_file(keywords)
    update_tmlanguage_file(keywords)
    print(f"Extracted {len(keywords)} keywords and saved to {OUTPUT_FILE}")

if __name__ == "__main__":
    main()
