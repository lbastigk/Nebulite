import re
import sys
import json
from pathlib import Path

# Directories to search
SEARCH_DIRS = [
    Path('../include'),
]
# File patterns to match
FILE_PREFIXES = ("JTE_", "GTE_", "RTE_")
FILE_SUFFIX = ".h"

# Regex to match bindFunction lines and extract the quoted function name
BIND_REGEX = re.compile(r'bindFunction\([^,]+,\s*"([^"]+)"')

def find_function_names():
    function_names = set()
    for search_dir in SEARCH_DIRS:
        for header in search_dir.glob("*.h"):
            if not header.name.startswith(FILE_PREFIXES):
                continue
            with open(header, 'r', encoding='utf-8', errors='ignore') as f:
                for line in f:
                    match = BIND_REGEX.search(line)
                    if match:
                        function_names.add(match.group(1))
    return sorted(function_names)

def escape_for_regex(s):
    # Escape regex special chars, then double-escape backslashes for JSON
    s = re.sub(r'([\\.^$|?*+()\[\]{{}}-])', r'\\\\\1', s)
    s = s.replace('\\b', '\\\\b')
    return s

def main():
    names = find_function_names()
    # Sort by length descending, then alphabetically for stability
    names.sort(key=lambda n: (-len(n), n))
    tm_path = Path(__file__).parent / "syntaxes" / "nebulite-script.tmLanguage.json"

    # Read the tmLanguage file as JSON
    with open(tm_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    # Find the keyword patterns array
    repo = data.get("repository", {})
    keyword = repo.get("keyword", {})
    # Generate new patterns
    new_patterns = []
    for name in names:
        escaped = escape_for_regex(name)
        new_patterns.append({
            "name": "keyword.control.nebs",
            "match": f"\\b{escaped}\\b"
        })

    # Replace the patterns array
    keyword["patterns"] = new_patterns
    repo["keyword"] = keyword
    data["repository"] = repo

    # Write back as pretty JSON
    with open(tm_path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, ensure_ascii=False)
        f.write("\n")
    print(f"Updated {tm_path} with {len(names)} patterns.")

if __name__ == "__main__":
    main()
