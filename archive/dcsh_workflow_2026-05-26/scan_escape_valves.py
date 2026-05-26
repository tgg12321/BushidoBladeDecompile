#!/usr/bin/env python3
"""Scan `git diff` output on stdin for escape-valve additions.

Used by tools/hooks/escape_valve_guard.sh to detect commits that add
patterns requiring the .bb2_attempts/<func>.jsonl gate.

Detects:
  1. New INLINE_MOVE_ALIASING: comment blocks in src/*.c
  2. New `func: replace_with_asmfile "..."` lines in asmfix.txt
     (uncommented additions only; commented-out is fine — that RETIRES)
  3. New function-name lines in inline_asm_canonical.txt

Output: one `<func>\t<kind>` line per detection (deduplicated), where
<kind> is one of INLINE_MOVE_ALIASING / asmfix_bridge /
inline_asm_canonical. Functions with unresolvable names emit
<unknown-from-diff> for the agent to refine.

For INLINE_MOVE_ALIASING: the function name is the next signature
defined AFTER the comment block (since the comment goes ABOVE the
function definition). We scan the staged file on disk to resolve.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Heuristic regex for a C function DEFINITION line at column 0 (not a
# declaration). Distinguished from declarations by requiring either `{`
# on the same line after `)`, or end-of-line (brace on next line). A
# trailing `;` would be a declaration and is excluded.
FUNC_DEF_RE = re.compile(
    r'^\s*(?:static\s+)?[a-zA-Z_][\w*]*\s+\**\s*([a-zA-Z_]\w*)\s*\([^;{]*\)\s*(?:\{|$)'
)
INLINE_ALIASING_DIFF_RE = re.compile(r'^\+.*\bINLINE_MOVE_ALIASING:')
ASMFIX_BRIDGE_ADD_RE = re.compile(r'^\+\s*([a-zA-Z_]\w*):\s*replace_with_asmfile')
CANONICAL_FUNC_RE = re.compile(r'^\+\s*([a-zA-Z_][\w]*)\s*$')


def next_function_after(file_path: Path, marker_phrase: str) -> str | None:
    """Read file_path, find the marker_phrase, return the name of the
    next C function defined after it. Returns None if unresolvable."""
    if not file_path.exists():
        return None
    try:
        lines = file_path.read_text(encoding="utf-8", errors="ignore").splitlines()
    except OSError:
        return None
    seen_marker = False
    for line in lines:
        if not seen_marker:
            if marker_phrase in line:
                seen_marker = True
            continue
        # Look for a function definition after the marker.
        m = FUNC_DEF_RE.match(line)
        if m:
            return m.group(1)
    return None


def main() -> int:
    diff = sys.stdin.read()
    if not diff.strip():
        return 0

    current_file: str | None = None
    hits: list[tuple[str, str]] = []
    pending_aliasing: list[str] = []  # files that had INLINE_MOVE_ALIASING added

    for line in diff.splitlines():
        if line.startswith("diff --git"):
            current_file = None
        elif line.startswith("+++ b/"):
            current_file = line[6:]
        elif current_file is None:
            continue
        elif current_file.startswith("src/") and current_file.endswith(".c"):
            if INLINE_ALIASING_DIFF_RE.match(line):
                if current_file not in pending_aliasing:
                    pending_aliasing.append(current_file)
        elif current_file == "asmfix.txt":
            m = ASMFIX_BRIDGE_ADD_RE.match(line)
            if m:
                hits.append((m.group(1), "asmfix_bridge"))
        elif current_file == "inline_asm_canonical.txt":
            m = CANONICAL_FUNC_RE.match(line)
            if m and not m.group(1).startswith("#"):
                hits.append((m.group(1), "inline_asm_canonical"))

    # Resolve INLINE_MOVE_ALIASING hits by reading the file on disk and
    # finding the function defined AFTER the marker comment.
    for fp in pending_aliasing:
        func = next_function_after(ROOT / fp, "INLINE_MOVE_ALIASING:")
        hits.append((func or "<unknown-from-diff>", "INLINE_MOVE_ALIASING"))

    # Deduplicate while preserving order.
    seen: set[tuple[str, str]] = set()
    for func, kind in hits:
        if (func, kind) in seen:
            continue
        seen.add((func, kind))
        print(f"{func}\t{kind}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
