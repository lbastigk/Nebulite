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
# These functions use named/spanned arguments whose consumed indexes cannot be
# inferred from the positional-argument analysis below.
KNOWN_EXCEPTIONS = {
    "Fft::applyTransferFunctionFrequencyDomain("
    "Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc"
    ")",
}
SIZE_GUARD = re.compile(
    r"\bargs\s*\.\s*size\s*\(\s*\)\s*"
    r"(?P<operator>!=|>=|>|<=|<|==)\s*(?P<value>\d+)"
)
MINIMUM_GUARD = re.compile(
    r"\bargs\s*\.\s*size\s*\(\s*\)\s*(?P<operator><|<=)\s*(?P<value>\d+)"
)
EMPTY_CHECK = re.compile(r"\bargs\s*\.\s*empty\s*\(\s*\)")
INDEX_ACCESS = re.compile(
    r"\bargs\s*(?:\.\s*(?:at|tryGetAs)\s*(?:<[^;(){}]*>)?\s*\(\s*"
    r"(?P<method_index>\d+)\s*\)|\[\s*(?P<subscript_index>\d+)\s*\])"
)
RECOMBINE_ACCESS = re.compile(
    r"\bargs\s*\.\s*recombineSubspan\s*\(\s*(?P<index>\d+)\s*\)"
)
COPY_SUBSPAN_ACCESS = re.compile(
    r"\bargs\s*\.\s*copySubspan\s*\([^,]+,\s*(?P<index>\d+)\s*\)"
)
RECOMBINE_ALL_ACCESS = re.compile(r"\bargs\s*\.\s*recombine\s*\(\s*\)")
SUBSPAN_ACCESS = re.compile(
    r"\bargs\s*\.\s*subspan\s*\(\s*(?P<index>\d+)\s*\)"
)
DROP_ACCESS = re.compile(
    r"\bargs\s*\|\s*std::views::drop\s*\(\s*(?P<index>\d+)\s*\)"
)
PASSTHROUGH_ACCESS = re.compile(r"(?:\(|,)\s*args\s*(?:,|\))")


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
        elif operator in {"==", "!="}:
            candidate = value
        else:
            continue
        maximum = candidate if maximum is None else min(maximum, candidate)
    return maximum


def analyze_function(
    function,
    source: bytes,
    verbose: bool = False,
) -> tuple[list[tuple[int, str]], str | None]:
    parameters = find_node(function, "parameter_list")
    body = find_node(function, "compound_statement")
    if parameters is None or body is None:
        return [], None

    args_name = parameter_name(parameters, source)
    if args_name != "args":
        return [], None

    function_name = node_text(function.child_by_field_name("declarator"), source)
    if verbose:
        print(f"\tChecking {function_name}")

    if function_name in KNOWN_EXCEPTIONS:
        if verbose:
            print("\t\tKnown exception; named/spanned arguments require manual analysis")
        return [], None

    body_text = node_text(body, source)
    if EMPTY_CHECK.search(body_text):
        line = source[:function.start_byte].count(b"\n") + 1
        function_name = node_text(function.child_by_field_name("declarator"), source)
        return [
            (
                line,
                f"{function_name}: args.empty() is always false; "
                "args[0] contains the transformation name",
            )
        ], None

    passed_through = bool(PASSTHROUGH_ACCESS.search(body_text))
    if passed_through:
        if verbose:
            print("\t\tPassed as a function argument; all indexes considered consumed")
        return [], None

    if RECOMBINE_ALL_ACCESS.search(body_text):
        if verbose:
            print("\t\tRecombined all arguments; all indexes are consumed")
        return [], None

    recombine_match = RECOMBINE_ACCESS.search(body_text)
    if recombine_match and int(recombine_match.group("index")) == 1:
        if verbose:
            print(
                "\t\tRecombined from index 1; reserved index 0 is discarded "
                "and all user indexes are consumed"
            )
        return [], None

    subspan_match = SUBSPAN_ACCESS.search(body_text)
    if subspan_match and int(subspan_match.group("index")) == 1:
        if verbose:
            print(
                "\t\tIterates subspan from index 1; reserved index 0 is discarded "
                "and all user indexes are consumed"
            )
        return [], None
    if subspan_match and int(subspan_match.group("index")) == 0:
        if verbose:
            print("\t\tIterates subspan from index 0; all indexes are consumed")
        return [], None

    drop_match = DROP_ACCESS.search(body_text)
    if drop_match:
        drop_count = int(drop_match.group("index"))
        if drop_count <= 1:
            if verbose:
                print(
                    f"\t\tIterates args after dropping {drop_count} index; "
                    "all user indexes are consumed"
                )
            return [], None

        function_name = node_text(function.child_by_field_name("declarator"), source)
        discarded = f"1..{drop_count - 1}"
        if verbose:
            print(
                f"\t\tDrops indexes 0..{drop_count - 1}; "
                f"user indexes {discarded} are discarded"
            )
        line = source[:function.start_byte].count(b"\n") + 1
        return [
            (
                line,
                f"{function_name}: args.drop({drop_count}) discards "
                f"user argument indexes {discarded}",
            )
        ], None

    used_indexes = {
        int(match.group("method_index") or match.group("subscript_index"))
        for match in INDEX_ACCESS.finditer(body_text)
    }
    remainder_match = RECOMBINE_ACCESS.search(body_text)
    remainder_name = "recombineSubspan"
    if remainder_match is None:
        remainder_match = COPY_SUBSPAN_ACCESS.search(body_text)
        remainder_name = "copySubspan"
    if remainder_match:
        remainder_start = int(remainder_match.group("index"))
        missing_indexes = [
            index for index in range(1, remainder_start)
            if index not in used_indexes
        ]
        if missing_indexes:
            function_name = node_text(
                function.child_by_field_name("declarator"),
                source,
            )
            line = source[:function.start_byte].count(b"\n") + 1
            return [
                (
                    line,
                    f"{function_name}: argument indexes "
                    f"{missing_indexes} are accepted but never consumed before "
                    f"{remainder_name}({remainder_start})",
                )
            ], None
        if verbose:
            print(
                f"\t\tDirectly consumed indexes below {remainder_start}: "
                f"{sorted(index for index in used_indexes if index < remainder_start) or 'none'}"
            )
            print(
                f"\t\t{remainder_name} from index {remainder_start}; "
                "all later indexes are consumed"
            )
        return [], None

    maximum = find_upper_bound(body_text)
    if maximum is None:
        minimum_match = MINIMUM_GUARD.search(body_text)
        used_indexes = sorted(used_indexes)
        if minimum_match and used_indexes:
            minimum = int(minimum_match.group("value"))
            first_unused = max(used_indexes) + 1
            function_name = node_text(
                function.child_by_field_name("declarator"),
                source,
            )
            if verbose:
                print(
                    "\t\tNo maximum argument count; "
                    f"directly consumed indexes: {used_indexes}"
                )
                print(
                    "\t\tArguments from index "
                    f"{first_unused} onward are accepted but never consumed"
                )
            line = source[:function.start_byte].count(b"\n") + 1
            return [
                (
                    line,
                    f"{function_name}: no maximum argument count; "
                    f"argument index {first_unused} and later arguments are accepted "
                    "but never consumed",
                )
            ], None
        if verbose:
            if re.search(r"\bargs\s*\.\s*size\s*\(\s*\)\s*!=\s*1", body_text):
                print(
                    "\t\tExact args.size() == 1 guard; "
                    "reserved index 0 is the only argument"
                )
            else:
                print("\t\tNo simple maximum argument count found; skipping")
        return [], node_text(function.child_by_field_name("declarator"), source)

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
    for match in SUBSPAN_ACCESS.finditer(body_text):
        index = int(match.group("index"))
        remainder_start = (
            index if remainder_start is None else min(remainder_start, index)
        )

    accepted = set(range(1, max(1, maximum)))
    consumed = {
        index for index in accepted
        if index in used
        or (remainder_start is not None and index >= remainder_start)
    }
    unused = sorted(accepted - consumed)
    if verbose:
        print(f"\t\tAccepted argument indexes: 1..{maximum - 1}")
        print(
            "\t\tDirectly consumed indexes: "
            f"{sorted(index for index in used if index in accepted) or 'none'}"
        )
        print(
            "\t\tConsumed remainder/subspan from index: "
            f"{remainder_start if remainder_start is not None else 'none'}"
        )
        print(f"\t\tUnused accepted indexes: {unused or 'none'}")

    findings = []
    for index in unused:
        if index not in used:
            line = source[:function.start_byte].count(b"\n") + 1
            findings.append(
                (line, f"{function_name}: argument index {index} is accepted but never consumed")
            )
    return findings, None


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
) -> tuple[list[tuple[Path, int, str]], list[str]]:
    source = path.read_bytes()
    tree = parser.parse(source)
    findings = []
    skipped_functions = []
    for function in function_definitions(tree.root_node):
        function_findings, skipped = analyze_function(function, source, verbose)
        if skipped is not None:
            skipped_functions.append(skipped)
        for line, message in function_findings:
            findings.append((path, line, message))
    return findings, skipped_functions


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
    skipped_functions = []
    for path in source_files(paths):
        if args.verbose:
            print(f"Scanning {path}")
        file_findings, file_skipped_functions = analyze_file(
            path,
            tree_parser,
            args.verbose,
        )
        findings.extend(file_findings)
        skipped_functions.extend(
            f"{path}: {function}" for function in file_skipped_functions
        )
    if args.verbose:
        print("")
        print("Analysis complete.")
        print("")
    for path, line, message in findings:
        print(f"{path}:{line}: warning: {message}")
    print(f"Skipped functions: {len(skipped_functions)}")
    for function in skipped_functions:
        print(f"\t{function}")
    return 1 if findings else 0


if __name__ == "__main__":
    raise SystemExit(main())
