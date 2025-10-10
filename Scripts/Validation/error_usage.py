#!/usr/bin/env python3

# Script to validate error usage in the Nebulite codebase.

import re
import os
import sys
import argparse
from pathlib import Path
from typing import Set, List, Tuple, Dict

ERROR_DEF_FILE = "include/Constants/ErrorTypes.hpp"
ERROR_USAGE_PATHS = ["include", "src"]

def extract_error_definitions(file_path: str) -> Set[str]:
    """
    Extract all error names from ErrorTypes.hpp.
    Looks for patterns like: 'static inline Error <ErrorName>(){'
    """
    error_names = set()
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
            
        # Pattern to match: static inline Error ERROR_NAME(){
        pattern = r'static\s+inline\s+Error\s+(\w+)\s*\(\s*\)\s*\{'
        matches = re.finditer(pattern, content)
        
        for match in matches:
            error_name = match.group(1)
            error_names.add(error_name)
            
    except FileNotFoundError:
        print(f"Error: Could not find {file_path}")
        sys.exit(1)
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
        sys.exit(1)
        
    return error_names

def find_error_usages(search_paths: List[str], error_names: Set[str]) -> Dict[str, List[Tuple[str, int]]]:
    """
    Search for error usages in the specified paths.
    Looks for patterns like: ErrorTable::<something>::<ErrorName>()
    Returns: dict mapping error_name -> list of (file_path, usage_count) tuples
    """
    usage_map = {error: [] for error in error_names}
    
    for search_path in search_paths:
        if not os.path.exists(search_path):
            print(f"Warning: Path {search_path} does not exist")
            continue
            
        for root, dirs, files in os.walk(search_path):
            for file in files:
                if file.endswith(('.hpp', '.cpp', '.h', '.c')):
                    file_path = os.path.join(root, file)
                    check_file_for_errors(file_path, error_names, usage_map)
                    
    return usage_map

def check_file_for_errors(file_path: str, error_names: Set[str], usage_map: Dict[str, List[Tuple[str, int]]]):
    """
    Check a single file for error usages.
    Updates usage_map with (file_path, count) tuples.
    """
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
            
        for error_name in error_names:
            # Pattern to match: ErrorTable::<something>::<ErrorName>()
            # Also match: Constants::ErrorTable::<something>::<ErrorName>()
            # Also match: Nebulite::Constants::ErrorTable::<something>::<ErrorName>()
            patterns = [
                rf'ErrorTable::\w+::{error_name}\s*\(\s*\)',
                rf'Constants::ErrorTable::\w+::{error_name}\s*\(\s*\)',
                rf'Nebulite::Constants::ErrorTable::\w+::{error_name}\s*\(\s*\)',
                rf'ErrorTable::{error_name}\s*\(\s*\)',  # Direct access
                rf'Constants::ErrorTable::{error_name}\s*\(\s*\)',
                rf'Nebulite::Constants::ErrorTable::{error_name}\s*\(\s*\)'
            ]
            
            total_count = 0
            for pattern in patterns:
                matches = re.findall(pattern, content)
                total_count += len(matches)
            
            if total_count > 0:
                usage_map[error_name].append((file_path, total_count))
                    
    except Exception as e:
        print(f"Warning: Could not read {file_path}: {e}")

def main():
    """Main function to validate error usage."""
    parser = argparse.ArgumentParser(description='Validate error usage in Nebulite codebase')
    parser.add_argument('-v', '--verbose', action='store_true', 
                       help='Show detailed usage count for each error')
    args = parser.parse_args()
    
    print("Validating error usage in Nebulite codebase...")
    print(f"Error definitions file: {ERROR_DEF_FILE}")
    print(f"Search paths: {ERROR_USAGE_PATHS}")
    print("-" * 60)
    print()
    
    # Extract error definitions
    error_names = extract_error_definitions(ERROR_DEF_FILE)
    print(f"[INFO] Found {len(error_names)} error definitions")
    if args.verbose:
        for error in sorted(error_names):
            print(f"  - {error}")
    print()
    
    # Find usages
    usage_map = find_error_usages(ERROR_USAGE_PATHS, error_names)
    
    # Analyze results
    unused_errors = []
    used_errors = []
    
    for error_name, usages in usage_map.items():
        if not usages:
            unused_errors.append(error_name)
        else:
            # Calculate total usage count
            total_uses = sum(count for _, count in usages)
            used_errors.append((error_name, usages, total_uses))
    
    # Sort used errors by total usage count (descending)
    used_errors.sort(key=lambda x: x[2], reverse=True)
    
    if unused_errors:
        print("[WARN] UNUSED ERRORS:")
        for error in sorted(unused_errors):
            print(f"  - {error}")
        print()
    
    if used_errors:
        if args.verbose:
            print("[INFO] USED ERRORS:")
            print("(Sorted by usage count, most used first)")
            for error_name, usages, total_count in used_errors:
                print(f"  - {error_name} (used {total_count} time(s) in {len(usages)} file(s))")
                for file_path, count in usages:
                    print(f"    → {file_path}: {count} usage(s)")
        print()
    
    # Show statistics
    if used_errors:
        total_usages = sum(total_count for _, _, total_count in used_errors)
        avg_usage = total_usages / len(used_errors)
        most_used = used_errors[0]
        print(f"[INFO] STATISTICS:")
        print(f"  Total error usages: {total_usages}")
        print(f"  Used errors: {len(used_errors)}")
        print(f"  Unused errors: {len(unused_errors)}")
        print(f"  Average usages per error: {avg_usage:.1f}")
        print(f"  Most used error: {most_used[0]} ({most_used[2]} usages)")
        print()

    # Exit with error code if unused errors found
    if unused_errors:
        print(f"[INFO] {len(unused_errors)} unused error(s)")
        return 1
    else:
        print("[INFO] All errors are being used!")
        return 0

if __name__ == "__main__":
    sys.exit(main())

