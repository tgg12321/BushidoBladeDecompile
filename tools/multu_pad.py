#!/usr/bin/env python3
"""multu/mflo pad pass: insert nops to enforce a fixed cycle gap.

The original game's `mult`/`multu` instructions are followed by exactly
2 instructions before the corresponding `mflo`/`mfhi`. GCC 2.7.2's
scheduler doesn't always preserve this gap (the R3000 has hardware
interlocks, so 0 gap is functionally fine). To match the original byte
sequence we must restore the 2-cycle gap.

Pipeline placement (in Makefile):
    cpp | cc1 | prologue_fix | maspsx | MULTU_PAD | regfix | ...

This pass operates on assembly TEXT — it parses the maspsx output, finds
multu/mult instructions, locates the next mflo/mfhi, and inserts nops if
the gap is < 2 instructions. Nops are inserted JUST BEFORE the mflo so
any useful work GCC scheduled stays where it landed.

Activation list: multu_pad_funcs.txt — one function name per line.
Functions not in the list pass through unchanged.

Usage:
    python3 tools/multu_pad.py --funcs multu_pad_funcs.txt < in.s > out.s
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

# Match instructions of interest. Whitespace-tolerant.
MULTU_RE = re.compile(r"^\s*mult[u]?\b")
MFLO_RE = re.compile(r"^\s*mf(lo|hi)\b")
FUNC_LABEL_RE = re.compile(r"^(\w+):\s*$")
END_RE = re.compile(r"^\s*\.end\s+(\w+)")


def is_instruction(line: str) -> bool:
    """Same predicate maspsx/regfix use: not directive, not label, not comment, not blank."""
    s = line.strip()
    if not s:
        return False
    if s.startswith(".") or s.startswith("#"):
        return False
    if s.endswith(":"):
        return False
    if any(s.startswith(d) for d in ("gcc2_compiled",)):
        return False
    return True


def load_funcs(path: Path) -> set[str]:
    out: set[str] = set()
    if not path.exists():
        return out
    for line in path.read_text(encoding="utf-8").splitlines():
        s = line.strip()
        if not s or s.startswith("#"):
            continue
        out.add(s)
    return out


def pad_stream(text: str, padded_funcs: set[str], gap: int = 2) -> str:
    """Insert nops to enforce `gap` instructions between each multu/mult and the
    next mflo/mfhi within functions listed in `padded_funcs`. Returns the
    rewritten asm text."""
    lines = text.splitlines(keepends=True)
    n = len(lines)

    # First pass: find pad-points. For each mflo/mfhi inside a target function
    # whose preceding multu/mult has too few intervening instructions, record
    # how many nops to inject just before the mflo.
    pad_before: dict[int, int] = {}
    in_func: str | None = None
    last_multu_line: int | None = None
    insns_since_multu: int = 0

    for i, raw in enumerate(lines):
        s = raw.strip()
        # Function entry / exit. Use full-line label match so we don't snag
        # branch labels like `.L8007FA80:`.
        m = FUNC_LABEL_RE.match(s)
        if m:
            name = m.group(1)
            in_func = name if name in padded_funcs else None
            last_multu_line = None
            insns_since_multu = 0
            continue
        m = END_RE.match(s)
        if m:
            in_func = None
            last_multu_line = None
            insns_since_multu = 0
            continue

        if not in_func:
            continue
        if not is_instruction(raw):
            continue

        if MULTU_RE.match(s):
            # New multu. If a prior multu hasn't been consumed by an mflo
            # yet, its result is overwritten — no padding needed for that one.
            last_multu_line = i
            insns_since_multu = 0
        elif MFLO_RE.match(s):
            if last_multu_line is not None:
                if insns_since_multu < gap:
                    pad_before[i] = gap - insns_since_multu
                last_multu_line = None
                insns_since_multu = 0
        else:
            if last_multu_line is not None:
                insns_since_multu += 1

    if not pad_before:
        return text

    # Second pass: emit lines, inserting nops where needed.
    out_parts: list[str] = []
    for i, raw in enumerate(lines):
        if i in pad_before:
            for _ in range(pad_before[i]):
                out_parts.append("\tnop\n")
        out_parts.append(raw)
    return "".join(out_parts)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--funcs", required=True,
                    help="Path to file listing function names that need multu/mflo padding")
    ap.add_argument("--gap", type=int, default=2,
                    help="Required instructions between multu/mult and following mflo/mfhi (default: 2)")
    args = ap.parse_args()

    padded = load_funcs(Path(args.funcs))
    if not padded:
        # No-op pass: just stream through.
        sys.stdout.write(sys.stdin.read())
        return 0

    text = sys.stdin.read()
    sys.stdout.write(pad_stream(text, padded, args.gap))
    return 0


if __name__ == "__main__":
    sys.exit(main())
