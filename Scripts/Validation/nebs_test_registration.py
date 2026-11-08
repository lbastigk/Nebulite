#!/usr/bin/env python3
"""Check that test task files are directly registered in the test suite."""

import json
import re
import shlex
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from TestingSuite import strip_jsonc_comments


ROOT = Path(__file__).resolve().parents[2]
SUPPRESSION = re.compile(r"^\s*#NOLINT\(test-registration\)(?:\s.*)?$")


def load_json(path, jsonc=False):
    try:
        content = path.read_text(encoding="utf-8")
        return json.loads(strip_jsonc_comments(content) if jsonc else content)
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ValueError(f"{path}: {error}") from error


def registered_tasks(root):
    config_path = root / "Tools/tests.jsonc"
    config = load_json(config_path, jsonc=True)
    if not isinstance(config, dict) or not isinstance(config.get("tests"), list):
        raise ValueError(f"{config_path}: expected an object with a tests array")

    tasks = set()
    for entry_index, entry in enumerate(config["tests"], 1):
        source = f"{config_path}: tests entry {entry_index}"
        if isinstance(entry, str):
            manifest = root / entry
            entries = load_json(manifest)
            if not isinstance(entries, list):
                raise ValueError(f"{manifest}: expected an array of tests")
            source = str(manifest)
        else:
            entries = [entry]

        for index, test in enumerate(entries, 1):
            location = f"{source}: test {index}"
            if not isinstance(test, dict) or not isinstance(test.get("command"), str):
                raise ValueError(f"{location}: expected a string command")
            try:
                parts = shlex.split(test["command"])
            except ValueError as error:
                raise ValueError(f"{location}: invalid command: {error}") from error
            if not parts:
                raise ValueError(f"{location}: empty command")
            if parts[0] == "task":
                if len(parts) < 2:
                    raise ValueError(f"{location}: task command is missing its path")
                tasks.add((root / parts[1]).resolve())
    return tasks


def find_unregistered(root):
    tests_dir = root / "TaskFiles/Tests"
    if not tests_dir.is_dir():
        raise ValueError(f"{tests_dir}: test directory does not exist")
    registered = registered_tasks(root)
    missing = []
    for path in tests_dir.rglob("*.nebs"):
        if not path.is_file() or path.resolve() in registered:
            continue
        try:
            lines = path.read_text(encoding="utf-8").splitlines()
        except (OSError, UnicodeError) as error:
            raise ValueError(f"{path}: {error}") from error
        if not any(SUPPRESSION.fullmatch(line) for line in lines):
            missing.append(path.relative_to(root))
    return sorted(missing)


def main():
    try:
        missing = find_unregistered(ROOT)
    except ValueError as error:
        print(f"Error: {error}", file=sys.stderr)
        return 2
    if missing:
        print("Test files without a direct suite registration:")
        for path in missing:
            print(f"  {path}")
        print("Tip: Add a standalone #NOLINT(test-registration) comment to intentionally unregistered files.")
        return 1
    print("All .nebs files under TaskFiles/Tests are registered or exempted.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
