#!/usr/bin/env python3
"""Track an instruction class across every RTL pass of one function.

Reads the dumps produced by `dump.py`, extracts the requested function's region
from each, and reports how many lines match a regex — in GCC 2.7.2's pass order.
The pass where the count drops to zero is the pass that removed the insn; the
pass where it first appears is the pass that created it.

Usage (WSL, from the repo root):
  python3 tools/rtl_track/find.py <tag> <func> <regex> [--limit N]

Example:
  python3 tools/rtl_track/find.py cfg func_8003F388 'set .*reg.*mult'
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent

# GCC 2.7.2's pass order. Any dump not listed here is appended (alphabetically)
# so a differently-configured cc1 still reports everything it wrote.
PASS_ORDER = [
    "in.i.rtl", "in.i.jump", "in.i.cse", "in.i.loop", "in.i.cse2", "in.i.flow",
    "in.i.combine", "in.i.sched", "in.i.lreg", "in.i.greg", "in.i.jump2",
    "in.i.sched2", "in.i.dbr",
]


def function_region(text: str, func: str) -> str | None:
    """The `;; Function <func>` section of a dump, or the whole text for
    single-function dumps (which carry no header)."""
    for part in re.split(r"\n(?=;; Function )", text):
        head = (part.splitlines() or [""])[0]
        if head.strip() == ";; Function %s" % func:
            return part
    return text if func in text else None


def main() -> int:
    ap = argparse.ArgumentParser(description="Track a pattern across RTL passes.")
    ap.add_argument("tag", help="dump dir tmp/rtl/<tag> produced by dump.py")
    ap.add_argument("func", help="function whose region to search")
    ap.add_argument("pattern", help="regex matched against each RTL line")
    ap.add_argument("--limit", type=int, default=12,
                    help="max matching lines printed per pass (default: 12)")
    args = ap.parse_args()

    rx = re.compile(args.pattern)
    dump_dir = ROOT / "tmp" / "rtl" / args.tag
    if not dump_dir.is_dir():
        raise SystemExit("no dumps at %s — run dump.py first" % dump_dir)

    present = {p.name for p in dump_dir.iterdir() if p.name.startswith("in.i.")}
    names = [n for n in PASS_ORDER if n in present]
    names += sorted(present - set(PASS_ORDER))

    for name in names:
        region = function_region((dump_dir / name).read_text(errors="replace"),
                                 args.func)
        if region is None:
            print("%-14s  <function not found>" % name)
            continue
        hits = [ln.strip() for ln in region.splitlines() if rx.search(ln)]
        print("%-14s  %d hit(s)" % (name, len(hits)))
        for hit in hits[:args.limit]:
            print("                 %s" % hit[:150])
    return 0


if __name__ == "__main__":
    sys.exit(main())
