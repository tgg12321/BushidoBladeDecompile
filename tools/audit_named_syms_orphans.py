#!/usr/bin/env python3
"""Detect and (optionally) strip orphan `<name> = <addr>;` entries in
named_syms.txt that shadow real C/asm symbols in build/src/*.o.

Background: while a function is `INCLUDE_ASM`-shaped, named_syms.txt
carries `<name> = 0x<addr>;` so the linker knows where to find the
symbol (it's defined by the .s file's `glabel`). When the function
gets decompiled to pure C, the .o now exports a real text-section
symbol at the same address -- and the named_syms entry becomes a
shadow assignment.

Two consequences:
  1. The linker map records `<name> = 0x<addr>` instead of the real
     symbol entry, which breaks `dc.sh verify <name>` lookups (the
     primary symptom the user-facing tools care about).
  2. Future grep/audit passes see two definitions and have to decide
     which is canonical.

This tool reads build/*.o files via nm, finds which names are now
defined in .text by real .o symbols, and flags / strips the matching
named_syms.txt lines.

Usage:
    python3 tools/audit_named_syms_orphans.py            # report only
    python3 tools/audit_named_syms_orphans.py --apply    # rewrite file
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
NAMED_SYMS = ROOT / "named_syms.txt"
BUILD_SRC = ROOT / "build" / "src"

# `<name> = 0x<addr>;` -- the form we audit.
ASSIGN_RE = re.compile(r"^\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)\s*;\s*$")


def collect_o_text_symbols() -> set[str]:
    """Return the set of names defined as text-section symbols by any
    .o file under build/src/. We treat any global `T`/`t`/`W` symbol
    as a real definition that would shadow a named_syms.txt assignment.
    """
    if not BUILD_SRC.is_dir():
        return set()
    names: set[str] = set()
    for o in BUILD_SRC.glob("*.o"):
        try:
            out = subprocess.run(
                ["mipsel-linux-gnu-nm", "--defined-only", str(o)],
                capture_output=True, text=True, check=True, timeout=30,
            ).stdout
        except (subprocess.CalledProcessError, FileNotFoundError,
                subprocess.TimeoutExpired):
            continue
        for line in out.splitlines():
            # Format: "0000abcd T name" — we want T/t/W/V (text/weak).
            parts = line.split()
            if len(parts) < 3:
                continue
            kind = parts[1]
            if kind in ("T", "t", "W", "V"):
                names.add(parts[2])
    return names


def audit(apply: bool) -> int:
    if not NAMED_SYMS.exists():
        print(f"(no {NAMED_SYMS}; nothing to audit)")
        return 0
    o_syms = collect_o_text_symbols()
    if not o_syms:
        print("(no symbols found in build/src/*.o -- run `make` first)",
              file=sys.stderr)
        return 0

    lines = NAMED_SYMS.read_text(encoding="utf-8").splitlines(keepends=True)
    kept: list[str] = []
    orphans: list[str] = []
    for line in lines:
        m = ASSIGN_RE.match(line)
        if m and m.group(1) in o_syms:
            orphans.append(m.group(1))
            continue
        kept.append(line)

    if not orphans:
        print("named_syms.txt: no orphans (all assignments still reference asm-only symbols)")
        return 0

    print(f"named_syms.txt: {len(orphans)} orphan assignment(s) shadow real .o symbols:")
    for name in orphans:
        print(f"  - {name}")
    if apply:
        NAMED_SYMS.write_text("".join(kept), encoding="utf-8")
        print(f"\nStripped {len(orphans)} orphan line(s) from {NAMED_SYMS.name}.")
    else:
        print(f"\n(re-run with --apply to strip)")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--apply", action="store_true",
                    help="Rewrite named_syms.txt with orphans removed")
    args = ap.parse_args()
    return audit(args.apply)


if __name__ == "__main__":
    sys.exit(main())
