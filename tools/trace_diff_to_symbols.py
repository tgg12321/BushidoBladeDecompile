#!/usr/bin/env python3
"""Trace binary diff ranges back to map symbols and object sections."""

from __future__ import annotations

import argparse
from pathlib import Path

from func_tooling import (
    BUILD_MAP,
    EXE_HEADER,
    LOAD_ADDR,
    ORIGINAL_EXE,
    PROJECT_ROOT,
    find_nearest_map_symbols,
    find_sections_containing_addr,
    find_symbol_span,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--built",
        default=str(PROJECT_ROOT / "build" / "bb2.exe"),
        help="Built executable to inspect",
    )
    parser.add_argument(
        "--original",
        default=str(ORIGINAL_EXE),
        help="Original executable to compare against",
    )
    parser.add_argument(
        "--limit",
        type=int,
        default=8,
        help="Maximum number of diff ranges to print",
    )
    return parser.parse_args()


def load_bytes(path: Path) -> bytes:
    if not path.exists():
        raise SystemExit(f"Missing file: {path}")
    return path.read_bytes()


def collect_diff_ranges(expected: bytes, actual: bytes) -> list[tuple[int, int]]:
    ranges: list[tuple[int, int]] = []
    max_len = max(len(expected), len(actual))
    start: int | None = None
    for offset in range(max_len):
        exp_byte = expected[offset] if offset < len(expected) else None
        act_byte = actual[offset] if offset < len(actual) else None
        if exp_byte != act_byte:
            if start is None:
                start = offset
        elif start is not None:
            ranges.append((start, offset))
            start = None
    if start is not None:
        ranges.append((start, max_len))
    return ranges


def format_addr(file_offset: int) -> str:
    if file_offset < EXE_HEADER:
        return f"file+0x{file_offset:X} (header)"
    vram = LOAD_ADDR + (file_offset - EXE_HEADER)
    return f"file+0x{file_offset:X} / vram 0x{vram:08X}"


def print_range(start: int, end: int) -> None:
    print(f"{format_addr(start)} .. {format_addr(end - 1)} ({end - start} bytes)")
    if start < EXE_HEADER:
        print("  before load address; no symbol mapping")
        return

    vram_start = LOAD_ADDR + (start - EXE_HEADER)
    vram_end = LOAD_ADDR + (end - EXE_HEADER)

    sections = []
    for probe in (vram_start, max(vram_start, vram_end - 1)):
        for section in find_sections_containing_addr(probe):
            if section not in sections:
                sections.append(section)
    if sections:
        for section in sections:
            print(
                "  section "
                f"{section.section} {section.obj_path} "
                f"[0x{section.addr:08X}-0x{section.end:08X})"
            )
    else:
        print("  section unknown in build map")

    span = find_symbol_span(vram_start)
    if span is not None:
        current, next_symbol = span
        if next_symbol is None:
            print(f"  symbol {current[1]} @ 0x{current[0]:08X} (last known symbol)")
        else:
            print(
                f"  symbol {current[1]} @ 0x{current[0]:08X} "
                f"through 0x{next_symbol[0]:08X} ({next_symbol[1]})"
            )
    else:
        print("  symbol before first mapped symbol")

    neighbors = find_nearest_map_symbols(vram_start, before=2, after=2)
    if neighbors:
        print("  nearby symbols:")
        for addr, name in neighbors:
            marker = "<=" if addr <= vram_start else "=>"
            print(f"    {marker} 0x{addr:08X} {name}")


def main() -> int:
    args = parse_args()
    built_path = Path(args.built)
    original_path = Path(args.original)

    if not BUILD_MAP.exists():
        raise SystemExit(f"Missing build map: {BUILD_MAP}")

    expected = load_bytes(original_path)
    actual = load_bytes(built_path)
    diff_ranges = collect_diff_ranges(expected, actual)

    if not diff_ranges:
        print("No differences found.")
        return 0

    print(
        f"{len(diff_ranges)} diff range(s) between {built_path.name} and {original_path.name}."
    )
    if len(expected) != len(actual):
        print(f"Size mismatch: built={len(actual)} original={len(expected)} bytes")

    for idx, (start, end) in enumerate(diff_ranges[: args.limit], start=1):
        print()
        print(f"[{idx}] ", end="")
        print_range(start, end)

    if len(diff_ranges) > args.limit:
        print()
        print(f"... {len(diff_ranges) - args.limit} additional range(s) omitted")

    return 1


if __name__ == "__main__":
    raise SystemExit(main())
