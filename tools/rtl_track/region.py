#!/usr/bin/env python3
"""Print a window of one function's RTL around every line matching a pattern.

The follow-up to `find.py`: once a pass is known to be where an insn appears or
dies, this shows the surrounding RTL in that pass so the transformation itself
is readable.

Usage (WSL, from the repo root):
  python3 tools/rtl_track/region.py <tag> <dump> <func> <regex> [-B N] [-A N]

  dump   a pass file inside tmp/rtl/<tag>, e.g. in.i.combine

Example:
  python3 tools/rtl_track/region.py cfg in.i.combine func_8003F388 'mult' -B 6 -A 14
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(Path(__file__).resolve().parent))

from find import function_region  # noqa: E402


def main() -> int:
    ap = argparse.ArgumentParser(description="Window one function's RTL.")
    ap.add_argument("tag", help="dump dir tmp/rtl/<tag> produced by dump.py")
    ap.add_argument("dump", help="pass file name, e.g. in.i.combine")
    ap.add_argument("func", help="function whose region to search")
    ap.add_argument("pattern", help="regex matched against each RTL line")
    ap.add_argument("-B", "--before", type=int, default=6,
                    help="lines of context before a match (default: 6)")
    ap.add_argument("-A", "--after", type=int, default=14,
                    help="lines of context after a match (default: 14)")
    args = ap.parse_args()

    path = ROOT / "tmp" / "rtl" / args.tag / args.dump
    if not path.exists():
        raise SystemExit("no such dump: %s — run dump.py first" % path)

    region = function_region(path.read_text(errors="replace"), args.func)
    if region is None:
        raise SystemExit("function %s not found in %s" % (args.func, args.dump))

    lines = region.splitlines()
    rx = re.compile(args.pattern)
    for i, line in enumerate(lines):
        if not rx.search(line):
            continue
        print("---- match at %d ----" % i)
        for j in range(max(0, i - args.before), min(len(lines), i + args.after)):
            print("%5d %s" % (j, lines[j]))
        print()
    return 0


if __name__ == "__main__":
    sys.exit(main())
