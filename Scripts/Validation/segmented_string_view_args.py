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
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Iterable, Match

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
EMPTY_CHECK = re.compile(r"\bargs\s*\.\s*empty\s*\(\s*\)")
MINIMUM_GUARD = re.compile(
    r"\bargs\s*\.\s*size\s*\(\s*\)\s*(?:<|<=)\s*(?P<value>\d+)"
)
RECOMBINE_ALL_ACCESS = re.compile(r"\bargs\s*\.\s*recombine\s*\(\s*\)")
PASSTHROUGH_ACCESS = re.compile(r"(?:\(|,)\s*args\s*(?:,|\))")


@dataclass(frozen=True)
class IndexRange:
    """Inclusive index range; None as end means unbounded."""

    start: int
    end: int | None = None

    def intersect(self, other: "IndexRange") -> "IndexRange | None":
        start = max(self.start, other.start)
        if self.end is None:
            end = other.end
        elif other.end is None:
            end = self.end
        else:
            end = min(self.end, other.end)
        return None if end is not None and start > end else IndexRange(start, end)

    def contains(self, index: int) -> bool:
        return index >= self.start and (self.end is None or index <= self.end)

    def __str__(self) -> str:
        return f"{self.start}.." if self.end is None else f"{self.start}..{self.end}"


def exact(index: int) -> IndexRange:
    return IndexRange(index, index)


def from_index(index: int) -> IndexRange:
    return IndexRange(index)


def span(start: int, count: int) -> IndexRange:
    return IndexRange(start, start + count - 1)


Pattern = tuple[
    str,
    re.Pattern[str],
    Callable[[Match[str]], IndexRange | None],
]


# Each tuple is: (description, regex, coverage builder).
# The builders turn regex captures into logical argument-index coverage.
GUARD_PATTERNS: list[Pattern] = [
    (
        "args.size() != N",
        re.compile(
            r"\bargs\s*\.\s*size\s*\(\s*\)\s*!=\s*(?P<value>\d+)"
        ),
        lambda match: IndexRange(1, int(match.group("value")) - 1),
    ),
    (
        "args.size() < N",
        re.compile(
            r"\bargs\s*\.\s*size\s*\(\s*\)\s*<\s*(?P<value>\d+)"
        ),
        lambda match: IndexRange(int(match.group("value")) - 1, None),
    ),
    (
        "args.size() > N",
        re.compile(
            r"\bargs\s*\.\s*size\s*\(\s*\)\s*>\s*(?P<value>\d+)"
        ),
        lambda match: IndexRange(1, int(match.group("value")) - 1),
    ),
]

ACCESS_PATTERNS: list[Pattern] = [
    (
        "args[<i>]",
        re.compile(r"\bargs\s*\[\s*(?P<index>\d+)\s*\]"),
        lambda match: exact(int(match.group("index"))),
    ),
    (
        "args.at(<i>) / args.tryGetAs(<i>)",
        re.compile(
            r"\bargs\s*\.\s*(?:at|tryGetAs)"
            r"(?:<[^;(){}]*>)?\s*\(\s*(?P<index>\d+)\s*\)"
        ),
        lambda match: exact(int(match.group("index"))),
    ),
    (
        "args.recombineSubspan(<i>)",
        re.compile(
            r"\bargs\s*\.\s*recombineSubspan"
            r"\(\s*(?P<index>\d+)\s*\)"
        ),
        lambda match: from_index(int(match.group("index"))),
    ),
    (
        "args.copySubspan(<target>, <i>)",
        re.compile(
            r"\bargs\s*\.\s*copySubspan"
            r"\([^,]+,\s*(?P<index>\d+)\s*\)"
        ),
        lambda match: from_index(int(match.group("index"))),
    ),
    (
        "args.subspan(<i>)",
        re.compile(
            r"\bargs\s*\.\s*subspan"
            r"\(\s*(?P<index>\d+)\s*\)"
        ),
        lambda match: from_index(int(match.group("index"))),
    ),
    (
        "args.subspan(<i>, <j>)",
        re.compile(
            r"\bargs\s*\.\s*subspan"
            r"\(\s*(?P<index>\d+)\s*,\s*(?P<count>\d+)\s*\)"
        ),
        lambda match: span(
            int(match.group("index")),
            int(match.group("count")),
        ),
    ),
    (
        "args | std::views::drop(<i>)",
        re.compile(
            r"\bargs\s*\|\s*std::views::drop"
            r"\(\s*(?P<index>\d+)\s*\)"
        ),
        lambda match: from_index(max(1, int(match.group("index")))),
    ),
]

# Compatibility aliases used by the control-flow diagnostics below.
INDEX_ACCESS = ACCESS_PATTERNS[0][1]
RECOMBINE_ACCESS = ACCESS_PATTERNS[2][1]
COPY_SUBSPAN_ACCESS = ACCESS_PATTERNS[3][1]
SUBSPAN_ACCESS = ACCESS_PATTERNS[4][1]
DROP_ACCESS = ACCESS_PATTERNS[6][1]


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
    coverage = guard_range(body_text)
    return None if coverage is None else coverage.end


def guard_range(body_text: str) -> IndexRange | None:
    """Combine recognized rejecting size guards into an accepted index range."""
    accepted = IndexRange(1, None)
    found = False
    for _, pattern, builder in GUARD_PATTERNS:
        for match in pattern.finditer(body_text):
            found = True
            candidate = builder(match)
            accepted = accepted.intersect(candidate)
            if accepted is None:
                return None
    return accepted if found else None


def access_ranges(body_text: str) -> tuple[list[tuple[str, IndexRange]], bool]:
    """Return declarative access coverage and whether all indexes are consumed."""
    ranges: list[tuple[str, IndexRange]] = []
    for name, pattern, builder in ACCESS_PATTERNS:
        for match in pattern.finditer(body_text):
            ranges.append((name, builder(match)))

    if RECOMBINE_ALL_ACCESS.search(body_text):
        ranges.append(("recombine", IndexRange(1, None)))
    if PASSTHROUGH_ACCESS.search(body_text):
        ranges.append(("pass-through", IndexRange(1, None)))
    return ranges, any(access.end is None for _, access in ranges)


def covered(index: int, ranges: list[tuple[str, IndexRange]]) -> bool:
    return any(access.contains(index) for _, access in ranges)


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

    pattern_accesses, _ = access_ranges(body_text)
    pattern_guard = guard_range(body_text)
    invalid_drop = next(
        (access for name, access in pattern_accesses if name == "invalid drop"),
        None,
    )
    if invalid_drop is not None:
        line = source[:function.start_byte].count(b"\n") + 1
        return [
            (
                line,
                f"{function_name}: drop() discards user argument indexes "
                f"{invalid_drop}",
            )
        ], None
    if pattern_accesses and (
        pattern_guard is not None
        or any(access.end is None for _, access in pattern_accesses)
    ):
        if pattern_guard is None:
            if verbose:
                print("\t\tAccess pattern consumes all user indexes")
            return [], None
        maximum = pattern_guard.end
        if maximum is None:
            missing = "unbounded"
            uncovered = []
            if not any(access.end is None for _, access in pattern_accesses):
                line = source[:function.start_byte].count(b"\n") + 1
                return [
                    (
                        line,
                        f"{function_name}: guard accepts an unbounded argument "
                        "range, but access patterns are bounded",
                    )
                ], None
        else:
            uncovered = [
                index for index in range(pattern_guard.start, maximum + 1)
                if not covered(index, pattern_accesses)
            ]
            missing = str(uncovered or "none")
        if verbose:
            print(f"\t\tGuard coverage: {pattern_guard}")
            print(
                "\t\tAccess coverage: "
                + ", ".join(f"{name} {access}" for name, access in pattern_accesses)
            )
            print(f"\t\tUncovered accepted indexes: {missing}")
        if uncovered:
            line = source[:function.start_byte].count(b"\n") + 1
            return [
                (
                    line,
                    f"{function_name}: accepted argument indexes "
                    f"{uncovered} are not covered by an access pattern",
                )
            ], None
        return [], None

    if pattern_accesses:
        if pattern_guard is None:
            if verbose:
                print("\t\tNo guard coverage; access patterns are insufficient to prove all indexes")
            return [
                (
                    source[:function.start_byte].count(b"\n") + 1,
                    f"{function_name}: no argument-count guard covers the access patterns",
                )
            ], None
        return [], None

    if re.search(r"\bargs\s*\.\s*size\s*\(\s*\)\s*!=\s*1", body_text):
        if verbose:
            print("\t\tExact args.size() == 1 guard; no user indexes are accepted")
        return [], None
    if pattern_guard is not None:
        if verbose:
            print(f"\t\tGuard coverage: {pattern_guard}; no access patterns required")
        return [], None

    if verbose:
        print("\t\tNo recognized guard or access pattern; skipping")
    return [], function_name


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
