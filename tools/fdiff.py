#!/usr/bin/env python3
"""Diff one function between two ASSEMBLY TEXT streams, normalizing `.L` labels.

The text-level counterpart to `objdiff.py` / `pairdiff.py` (which compare linked
objects): it extracts the named function from each file, renumbers `.L` labels
into first-appearance order so cosmetic label drift does not register, and
prints a unified diff. Directives other than `.L` labels are dropped, so only
instructions are compared.

Scope: both inputs must be TOOLCHAIN-EMITTED assembly — cc1 output, or any
later pipeline stage (prologue_fix / maspsx / regfix / asmfix). It is the tool
for "which stage changed this instruction". splat's `asm/funcs/*.s` is NOT a
valid input: it prints hex operands and ABI register names against the
toolchain's raw `$N`, so every line would differ. Compare against splat's
target at the OBJECT level with `objdiff.py` / `pairdiff.py` instead.

Usage (WSL, from the repo root):
  python3 tools/fdiff.py <ours.s> <target.s> <func>

Example:
  python3 tools/fdiff.py tmp/before.s tmp/after.s func_8003F388
"""
from __future__ import annotations

import argparse
import difflib
import re
import sys


def extract(path: str, func: str) -> list[str]:
    """The body lines of `func` — from its label to the next top-level label
    (or a `.size` / `.end` directive)."""
    out: list[str] = []
    inside = False
    with open(path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            s = line.strip()
            if not inside:
                if re.match(r"^(glabel\s+%s|%s:)\s*$"
                            % (re.escape(func), re.escape(func)), s):
                    inside = True
                continue
            if re.match(r"^[A-Za-z_][A-Za-z0-9_]*:\s*$", s) and not s.startswith(".L"):
                break
            if s.startswith(".size") or s.startswith(".end"):
                break
            out.append(s)
    return out


def norm(lines: list[str]) -> list[str]:
    """Drop comments/directives (keeping `.L` labels) and renumber `.L` labels
    into first-appearance order."""
    labels: dict[str, str] = {}
    res: list[str] = []
    for s in lines:
        if not s or s.startswith("#") or s.startswith("."):
            if not s.startswith(".L"):
                continue
        for lab in re.findall(r"\.L\d+", s):
            if lab not in labels:
                labels[lab] = ".L%03d" % len(labels)
        s = re.sub(r"\.L\d+", lambda m: labels[m.group(0)], s)
        res.append(s)
    return res


def main() -> int:
    ap = argparse.ArgumentParser(description="Diff one function across two .s files.")
    ap.add_argument("ours", help="assembly file with our codegen")
    ap.add_argument("target", help="assembly file with the reference codegen")
    ap.add_argument("func", help="function to extract from both")
    args = ap.parse_args()

    a = norm(extract(args.ours, args.func))
    b = norm(extract(args.target, args.func))
    print("ours=%d target=%d" % (len(a), len(b)))
    for line in difflib.unified_diff(a, b, "ours", "target", lineterm="", n=4):
        print(line)
    return 0


if __name__ == "__main__":
    sys.exit(main())
