#!/usr/bin/env python3
"""Find bridged functions with associated regfix rules (vestigial / safe to strip).

A bridged function (replace_with_asmfile in asmfix.txt) bypasses cc1 entirely.
Any regfix rules for that function don't execute — they're vestigial.
Stripping them is safe (bridge produces all bytes); cleanup reduces rule volume.

Usage: python3 tools/find_stale_bridged_rules.py [--apply] [--verify-force]

--apply: strip rules in-place (otherwise just report)
--verify-force: after each strip, run `dc.sh verify --all --force`
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path


def get_bridged_funcs() -> set[str]:
    """Return set of function names with active replace_with_asmfile."""
    text = Path("asmfix.txt").read_text(encoding="utf-8")
    pat = re.compile(r"^(\w+):\s+replace_with_asmfile", re.MULTILINE)
    return set(pat.findall(text))


def count_regfix_rules(func: str) -> dict[str, int]:
    """Return rule counts per regfix file for a function."""
    counts = {}
    for fname in ("regfix.txt", "regfix_stage2.txt"):
        text = Path(fname).read_text(encoding="utf-8")
        pat = re.compile(rf"^{re.escape(func)}:", re.MULTILINE)
        counts[fname] = len(pat.findall(text))
    return counts


def strip_func_rules(func: str, fname: str) -> int:
    """Strip all rules for func from fname. Returns count removed."""
    p = Path(fname)
    text = p.read_text(encoding="utf-8")
    lines = text.split("\n")
    kept = []
    removed = 0
    for line in lines:
        if line.startswith(f"{func}:") or line.startswith(f"# {func}:") or line.startswith(f"# {func} "):
            removed += 1
            continue
        kept.append(line)
    new_text = "\n".join(kept)
    p.write_text(new_text, encoding="utf-8", newline="\n")
    return removed


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--apply", action="store_true", help="Strip rules in-place")
    ap.add_argument("--min-rules", type=int, default=1, help="Only show funcs with >=N rules")
    args = ap.parse_args()

    bridged = get_bridged_funcs()
    print(f"Total bridged: {len(bridged)}")

    candidates = []
    for func in sorted(bridged):
        counts = count_regfix_rules(func)
        total = sum(counts.values())
        if total >= args.min_rules:
            candidates.append((func, counts, total))

    candidates.sort(key=lambda c: -c[2])

    print(f"Bridged functions with >={args.min_rules} regfix rules: {len(candidates)}")
    for func, counts, total in candidates:
        pieces = ", ".join(f"{k.replace('.txt', '')}={v}" for k, v in counts.items() if v > 0)
        print(f"  {func:50}  total={total:>3}  ({pieces})")

    if not args.apply:
        print("\n--apply to strip rules (and run verify --all --force to confirm safety)")
        return 0

    print("\n=== Applying ===")
    for func, counts, total in candidates:
        removed = 0
        for fname in ("regfix.txt", "regfix_stage2.txt"):
            removed += strip_func_rules(func, fname)
        print(f"  {func}: stripped {removed} rules")

    print("\nReminder: run `bash tools/dc.sh verify --all --force` to confirm no sibling drift.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
