#!/usr/bin/env python3
"""Classify lost_codegen cheats by retirement difficulty.

For each function with lost_codegen insert rules, analyzes:
1. Is it bridged? (bridged = stale rules, easy retire)
2. Does the C source have natural place for inline_move_aliasing? (medium)
3. Is it a register-cascade where insert + many renames work together? (hard)

Output: prioritized list for future cheat-cleanup sessions.

Usage: python3 tools/classify_lost_codegen.py [--func FUNC]
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path


LOST_CODEGEN_RE = re.compile(
    r'^(\w+):\s+(insert|insert_after|insert_before)\s+"(addu\s+\$\w+,\s*\$\w+,\s*\$\w+)"\s+@\s+(\d+)\s*$',
    re.MULTILINE,
)


def is_bridged(func: str) -> bool:
    """Check if func has replace_with_asmfile in asmfix.txt."""
    asmfix = Path("asmfix.txt").read_text(encoding="utf-8")
    pat = re.compile(rf"^{re.escape(func)}:\s+replace_with_asmfile", re.MULTILINE)
    return bool(pat.search(asmfix))


def count_rules(func: str) -> tuple[int, int]:
    """Return (regfix_count, regfix_stage2_count) for func."""
    counts = []
    for fname in ("regfix.txt", "regfix_stage2.txt"):
        text = Path(fname).read_text(encoding="utf-8")
        pat = re.compile(rf"^{re.escape(func)}:", re.MULTILINE)
        counts.append(len(pat.findall(text)))
    return tuple(counts)


def find_lost_codegen() -> dict[str, list[tuple[str, int]]]:
    """Return {func: [(insn_template, idx), ...]} for all lost_codegen inserts."""
    cheats = {}
    for fname in ("regfix.txt", "regfix_stage2.txt"):
        text = Path(fname).read_text(encoding="utf-8")
        for m in LOST_CODEGEN_RE.finditer(text):
            func = m.group(1)
            insn = m.group(3)
            idx = int(m.group(4))
            # Filter for lost_codegen pattern specifically
            if re.search(r'\$\w+,\s*\$(0|zero),\s*\$\w+|\$\w+,\s*\$\w+,\s*\$(0|zero)', insn):
                cheats.setdefault(func, []).append((insn, idx))
    return cheats


def classify(func: str, inserts: list[tuple[str, int]]) -> dict:
    """Classify a single function."""
    bridged = is_bridged(func)
    rf_count, rfs2_count = count_rules(func)
    total = rf_count + rfs2_count

    return {
        "func": func,
        "bridged": bridged,
        "insert_count": len(inserts),
        "total_rules": total,
        "regfix_count": rf_count,
        "regfix_stage2_count": rfs2_count,
        "category": _categorize(bridged, len(inserts), total),
    }


def _categorize(bridged: bool, n_inserts: int, total: int) -> str:
    if bridged:
        return "bridged-vestigial"  # easy: strip rules
    if total <= 5:
        return "minimal-cascade"  # try inline_move_aliasing
    if total <= 12:
        return "moderate-cascade"  # tricky source rewrite
    return "heavy-cascade"  # documented intractable per memory


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--func", help="Single function only")
    args = ap.parse_args()

    cheats = find_lost_codegen()

    if args.func:
        if args.func not in cheats:
            print(f"No lost_codegen cheats for {args.func}")
            return 0
        cheats = {args.func: cheats[args.func]}

    classified = [classify(func, inserts) for func, inserts in cheats.items()]
    classified.sort(key=lambda c: (c["category"], c["total_rules"]))

    by_cat = {}
    for c in classified:
        by_cat.setdefault(c["category"], []).append(c)

    print(f"=== {len(classified)} functions with lost_codegen cheats ===")
    for cat in ["bridged-vestigial", "minimal-cascade", "moderate-cascade", "heavy-cascade"]:
        items = by_cat.get(cat, [])
        print(f"\n{cat}: {len(items)} functions")
        for c in items:
            br = " [BRIDGED]" if c["bridged"] else ""
            print(f"  {c['func']:50}  rules={c['total_rules']:>3} inserts={c['insert_count']}{br}")

    print()
    print("Recommended retirement order:")
    print("  1. bridged-vestigial: strip all rules (verify --all --force)")
    print("  2. minimal-cascade: try inline_move_aliasing with %N placeholders")
    print("  3. moderate-cascade: source rewrite + small swap rules")
    print("  4. heavy-cascade: needs regfix-position-shifter or canonical-asm")
    return 0


if __name__ == "__main__":
    sys.exit(main())
