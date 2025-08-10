#!/usr/bin/env python3
"""
Script to convert $(self.*), $(other.*), $(global.*), and $(.*)
syntax to {self.*}, {other.*}, {global.*}, and {.*} respectively
in all JSONC files.
"""

import os
import re
import argparse
from pathlib import Path


def convert_syntax(content):
    """
    Convert $(self.*), $(other.*), $(global.*), and $(.*)
    to {self.*}, {other.*}, {global.*}, and {.*} respectively.
    """
    # Pattern to match $(...) expressions
    # This handles nested parentheses correctly
    def replace_dollar_expr(match):
        inner_content = match.group(1)
        return '{' + inner_content + '}'
    
    # Replace $(content) with {content}
    # Use a more sophisticated approach to handle nested parentheses
    pattern = r'\$\(([^()]*(?:\([^()]*\)[^()]*)*)\)'
    
    # Keep replacing until no more matches (to handle nested cases)
    prev_content = ""
    while prev_content != content:
        prev_content = content
        content = re.sub(pattern, replace_dollar_expr, content)
    
    return content


def process_file(file_path, dry_run=False):
    """Process a single JSONC file."""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            original_content = f.read()
        
        converted_content = convert_syntax(original_content)
        
        if original_content != converted_content:
            print(f"Converting: {file_path}")
            
            if not dry_run:
                with open(file_path, 'w', encoding='utf-8') as f:
                    f.write(converted_content)
                print(f"  ✓ Updated")
            else:
                print(f"  ✓ Would be updated (dry run)")
            
            return True
        else:
            print(f"No changes: {file_path}")
            return False
            
    except Exception as e:
        print(f"Error processing {file_path}: {e}")
        return False


def find_jsonc_files(root_dir):
    """Find all JSONC files in the directory tree."""
    jsonc_files = []
    for root, dirs, files in os.walk(root_dir):
        for file in files:
            if file.endswith('.jsonc') or file.endswith('.json'):
                jsonc_files.append(os.path.join(root, file))
    return jsonc_files


def main():
    parser = argparse.ArgumentParser(description='Convert $(expr) to {expr} in JSONC files')
    parser.add_argument('directory', nargs='?', default='.', 
                       help='Directory to search for JSONC files (default: current directory)')
    parser.add_argument('--dry-run', action='store_true',
                       help='Show what would be changed without making changes')
    parser.add_argument('--recursive', '-r', action='store_true', default=True,
                       help='Search recursively (default: True)')
    
    args = parser.parse_args()
    
    if not os.path.exists(args.directory):
        print(f"Error: Directory '{args.directory}' does not exist")
        return 1
    
    print(f"Searching for JSONC files in: {args.directory}")
    if args.dry_run:
        print("DRY RUN MODE - No files will be modified")
    print()
    
    jsonc_files = find_jsonc_files(args.directory)
    
    if not jsonc_files:
        print("No JSONC files found.")
        return 0
    
    print(f"Found {len(jsonc_files)} JSONC files")
    print()
    
    modified_count = 0
    for file_path in jsonc_files:
        if process_file(file_path, args.dry_run):
            modified_count += 1
    
    print()
    print(f"Summary: {modified_count} files {'would be' if args.dry_run else 'were'} modified out of {len(jsonc_files)} total files")
    
    return 0


if __name__ == '__main__':
    exit(main())
