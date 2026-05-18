#!/usr/bin/env python3
"""Scan asmfix queue for tractable functions: STUB c-body + small size + no penalty tags.

Output: sorted list of candidate (func, size, src, tags) — easiest first.
A "stub" body has 0-2 lines of meaningful code (just `(void)argN;` placeholders).
"""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = ROOT / "src"
WORK_QUEUE = ROOT / "WORK_QUEUE.md"

STUB_BODY_RE = re.compile(
    r"^\s*(?:/\*[^*]*\*/|\(void\)\s*\w+\s*;)\s*$"
)


def find_function_body(c_file: Path, func: str) -> int | None:
    """Return number of meaningful lines in function body, or None if not found."""
    if not c_file.exists():
        return None
    text = c_file.read_text(encoding="utf-8", errors="replace")
    # Find function signature line: looks like `<rettype> <func>(...)`
    sig_re = re.compile(
        rf"^(?:[\w\s\*]+\s+){re.escape(func)}\s*\([^;]*\)\s*\{{",
        re.MULTILINE,
    )
    m = sig_re.search(text)
    if not m:
        return None
    body_start = m.end()
    # Walk braces to find end
    depth = 1
    i = body_start
    while i < len(text) and depth > 0:
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
        i += 1
    if depth != 0:
        return None
    body = text[body_start:i - 1]
    # Count meaningful lines (not blank, not pure comment, not pure `(void)X;`)
    meaningful = 0
    in_comment = False
    for line in body.splitlines():
        line = line.strip()
        if not line:
            continue
        if in_comment:
            if "*/" in line:
                in_comment = False
            continue
        if line.startswith("/*"):
            if "*/" not in line:
                in_comment = True
            continue
        if line.startswith("//"):
            continue
        # Skip (void)arg<N>; placeholders
        if STUB_BODY_RE.match(line):
            continue
        meaningful += 1
    return meaningful


def parse_queue() -> list[tuple[str, int, str, str, str]]:
    """Yield (func, size, rec, src, tags) tuples from the Asmfix Retirement Queue."""
    text = WORK_QUEUE.read_text(encoding="utf-8")
    rows = []
    in_section = False
    in_block = False
    for line in text.splitlines():
        if line.startswith("## "):
            in_section = "Asmfix Retirement Queue" in line
            continue
        if not in_section:
            continue
        if line.strip() == "```":
            in_block = not in_block
            continue
        if not in_block:
            continue
        # Format: <#>  <func>  <size>  <rec>  <src>  [tags]
        m = re.match(
            r"^\s*(\d+)\s+(\S+)\s+(\d+)\s+(\S+)\s+(\S+)(?:\s+\[([^\]]+)\])?",
            line,
        )
        if m:
            rows.append((m.group(2), int(m.group(3)), m.group(4), m.group(5), m.group(6) or ""))
    return rows


def main():
    rows = parse_queue()
    candidates = []
    for func, size, rec, src, tags in rows:
        # Locate C body
        c_file = SRC_DIR / src
        meaningful = find_function_body(c_file, func)
        if meaningful is None:
            continue
        candidates.append((meaningful, size, func, src, rec, tags))
    # Sort: smallest size first (less cascade surface), then by stub-ness
    candidates.sort(key=lambda x: (x[1], x[0]))
    print(f"{'lines':>5}  {'size':>5}  {'func':<32}  {'rec':<22}  {'src':<24}  tags")
    print("-" * 110)
    for lines, size, func, src, rec, tags in candidates[:50]:
        print(f"{lines:>5}  {size:>5}  {func:<32}  {rec:<22}  {src:<24}  {tags}")


if __name__ == "__main__":
    main()
