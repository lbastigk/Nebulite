# This file is used to move a key inside a json document from one location to another.
# Does so for every document in a specified collection.
# Example: python ./Scripts/Restructuring/MoveKey.py --source ./Resources/ --from MyOldKey --to My.New.Key

import json
import os
# use jsonc to support comments in json files
import json5
import argparse
from typing import Any, Dict

def move_key(data: Dict[str, Any], from_key: str, to_key: str) -> None:
    """Move a key in a nested dictionary from one location to another."""
    keys_from = from_key.split('.')
    keys_to = to_key.split('.')

    # Navigate to the source key
    current = data
    for key in keys_from[:-1]:
        current = current.get(key, {})
    value = current.pop(keys_from[-1], None)

    if value is None:
        return  # Key not found, nothing to move

    # Navigate to the destination key
    current = data
    for key in keys_to[:-1]:
        if key not in current:
            current[key] = {}
        current = current[key]
    
    # Set the value at the new location
    current[keys_to[-1]] = value


def process_files(source_dir: str, from_key: str, to_key: str) -> None:
    """Recursively process all JSON and JSONC files in the directory."""
    for root, _, files in os.walk(source_dir):
        for filename in files:
            if filename.endswith(('.json', '.jsonc')):
                file_path = os.path.join(root, filename)

                with open(file_path, 'r', encoding='utf-8') as f:
                    data = json5.load(f)

                move_key(data, from_key, to_key)

                # Write STRICT JSON without trailing commas
                with open(file_path, 'w', encoding='utf-8') as f:
                    json.dump(data, f, indent=4, ensure_ascii=False)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Move a key in JSON documents from one location to another.')
    parser.add_argument('--source', required=True, help='Source directory containing JSON files.')
    parser.add_argument('--from', dest='from_key', required=True, help='The key to move (dot-separated for nested keys).')
    parser.add_argument('--to', dest='to_key', required=True, help='The destination key (dot-separated for nested keys).')

    args = parser.parse_args()
    process_files(args.source, args.from_key, args.to_key)
