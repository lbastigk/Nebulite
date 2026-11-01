#!/usr/bin/env python3
"""Find accepted SegmentedStringView arguments that are never consumed.

This is intentionally conservative. It recognizes direct literal accesses such as
args.at(1), args.tryGetAs<T>(2), args[3], and args.recombineSubspan(1), together
with simple guards such as `if (args.size() > 5) return false;`.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path
from typing import Iterable

try:
    from tree_sitter import Language, Parser
    import tree_sitter_cpp
except ImportError as error:
    print(
        "error: install the analyzer dependencies with "
        "`python3 -m pip install tree-sitter tree-sitter-cpp`",
        file=sys.stderr,
    )
    raise SystemExit(2) from error


SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx"}
TYPE_MARKER = "SegmentedStringView"
SIZE_GUARD = re.compile(
    r"\bargs\s*\.\s*size\s*\(\s*\)\s*(?P<operator>>=|>|<=|<|==)\s*(?P<value>\d+)"
)
INDEX_ACCESS = re.compile(
    r"\bargs\s*(?:\.\s*(?:at|tryGetAs)\s*(?:<[^;(){}]*>)?\s*\(\s*"
    r"(?P<method_index>\d+)\s*\)|\[\s*(?P<subscript_index>\d+)\s*\])"
)
RECOMBINE_ACCESS = re.compile(
    r"\bargs\s*\.\s*recombineSubspan\s*\(\s*(?P<index>\d+)\s*\)"
)


def make_parser() -> Parser:
    """Create a parser across supported tree-sitter Python API versions."""
    language = Language(tree_sitter_cpp.language())
    try:
        return Parser(language)
    except TypeError:
        parser = Parser()
        parser.language = language
        return parser


def descendants(node) -> Iterable:
    for child in node.children:
        yield child
        yield from descendants(child)


def node_text(node, source: bytes) -> str:
    return source[node.start_byte:node.end_byte].decode("utf-8", errors="replace")


def function_definitions(root):
    return (
        node for node in descendants(root)
        if node.type == "function_definition"
    )


def parameter_name(parameters, source: bytes) -> str | None:
    for node in descendants(parameters):
        if node.type == "parameter_declaration":
            text = node_text(node, source)
            if TYPE_MARKER not in text:
                continue
            match = re.search(r"\b([A-Za-z_]\w*)\s*&?\s*(?:,|$)", text)
            if match:
                return match.group(1)
    return None


def find_node(node, node_type: str):
    if node.type == node_type:
        return node
    for child in node.children:
        result = find_node(child, node_type)
        if result is not None:
            return result
    return None


def find_upper_bound(body_text: str) -> int | None:
    """Return the largest permitted args.size(), if a simple upper guard exists."""
    maximum: int | None = None
    for match in SIZE_GUARD.finditer(body_text):
        operator = match.group("operator")
        value = int(match.group("value"))
        if operator == ">":
            candidate = value
        elif operator == ">=":
            candidate = value - 1
        elif operator == "==":
            candidate = value
        else:
            continue
        maximum = candidate if maximum is None else min(maximum, candidate)
    return maximum


def analyze_function(
    function,
    source: bytes,
    verbose: bool = False,
) -> list[tuple[int, str]]:
    parameters = find_node(function, "parameter_list")
    body = find_node(function, "compound_statement")
    if parameters is None or body is None:
        return []

    args_name = parameter_name(parameters, source)
    if args_name != "args":
        return []

    if verbose:
        function_name = node_text(function.child_by_field_name("declarator"), source)
        print(f"\tChecking {function_name}")

    body_text = node_text(body, source)
    maximum = find_upper_bound(body_text)
    if maximum is None:
        return []

    used: set[int] = set()
    remainder_start: int | None = None
    for match in INDEX_ACCESS.finditer(body_text):
        index = match.group("method_index") or match.group("subscript_index")
        used.add(int(index))
    for match in RECOMBINE_ACCESS.finditer(body_text):
        index = int(match.group("index"))
        remainder_start = (
            index if remainder_start is None else min(remainder_start, index)
        )

    function_name = node_text(function.child_by_field_name("declarator"), source)
    findings = []
    for index in range(1, max(1, maximum)):
        if index not in used and (
            remainder_start is None or index < remainder_start
        ):
            line = source[:function.start_byte].count(b"\n") + 1
            findings.append(
                (line, f"{function_name}: argument index {index} is accepted but never consumed")
            )
    return findings


def source_files(paths: list[Path]) -> Iterable[Path]:
    for path in paths:
        if path.is_file() and path.suffix in SOURCE_SUFFIXES:
            yield path
        elif path.is_dir():
            yield from (
                child for child in path.rglob("*")
                if child.is_file() and child.suffix in SOURCE_SUFFIXES
            )


def analyze_file(
    path: Path,
    parser: Parser,
    verbose: bool = False,
) -> list[tuple[Path, int, str]]:
    source = path.read_bytes()
    tree = parser.parse(source)
    findings = []
    for function in function_definitions(tree.root_node):
        for line, message in analyze_function(function, source, verbose):
            findings.append((path, line, message))
    return findings


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "paths",
        nargs="*",
        type=Path,
        help="C/C++ files or directories to scan (default: src/Nebulite)",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="Print each scanned source file",
    )
    args = parser.parse_args()

    tree_parser = make_parser()
    paths = args.paths or [Path("src/Nebulite")]
    findings = []
    for path in source_files(paths):
        if args.verbose:
            print(f"Scanning {path}")
        findings.extend(analyze_file(path, tree_parser, args.verbose))
    for path, line, message in findings:
        print(f"{path}:{line}: warning: {message}")
    return 1 if findings else 0


if __name__ == "__main__":
    raise SystemExit(main())
