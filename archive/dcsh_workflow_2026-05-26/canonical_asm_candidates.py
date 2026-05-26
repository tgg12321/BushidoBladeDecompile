#!/usr/bin/env python3
"""List functions that may warrant canonical-asm authorization.

Per the user's "hard confirmation" rule for canonical-asm authorization,
this tool prepares a review-ready list:

1. Cheats that resist pure-C decompilation after multiple attempts
2. Functions with heavy regfix volume indicating cc1 cannot match naturally
3. Functions explicitly flagged as STRONG by scan_hand_coded.py

Outputs a report the user can review on return to decide which (if any)
warrant being added to inline_asm_canonical.txt.

Usage: python3 tools/canonical_asm_candidates.py [--memory-search]
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path


def get_strong_hand_coded() -> list[str]:
    """Return list of functions flagged as STRONG by scan_hand_coded."""
    try:
        r = subprocess.run(
            ["python3", "tools/scan_hand_coded.py", "--all"],
            capture_output=True, text=True,
        )
        strong = []
        current_func = None
        for line in r.stdout.split("\n"):
            m = re.match(r"\s*HAND_CODED: tier=STRONG\s+score=(\d+)/(\d+)\s+\((\w+),", line)
            if m:
                strong.append((m.group(3), int(m.group(1)), int(m.group(2))))
        return strong
    except Exception as e:
        print(f"scan_hand_coded failed: {e}", file=sys.stderr)
        return []


def get_heavy_cheats() -> list[tuple[str, int, str]]:
    """Return functions with ≥30 total regfix rules + cheat indicators."""
    # Wildcards from audit
    r = subprocess.run(
        ["python3", "tools/audit_asm_cheats.py", "--all"],
        capture_output=True, text=True,
    )
    wildcards = {}
    for line in r.stdout.split("\n"):
        m = re.match(r"\s+(\w+)\s+wildcard_count=(\d+)", line)
        if m:
            wildcards[m.group(1)] = int(m.group(2))

    # Cumulative rule counts
    rule_counts = {}
    for fname in ("regfix.txt", "regfix_stage2.txt"):
        text = Path(fname).read_text(encoding="utf-8")
        for line in text.split("\n"):
            m = re.match(r"^(\w+):\s+", line)
            if m:
                rule_counts[m.group(1)] = rule_counts.get(m.group(1), 0) + 1

    heavy = []
    for func, count in rule_counts.items():
        if count < 30:
            continue
        cheat_info = ""
        if func in wildcards:
            cheat_info = f"  wildcards={wildcards[func]}"
        heavy.append((func, count, cheat_info))
    heavy.sort(key=lambda x: -x[1])
    return heavy


def main():
    ap = argparse.ArgumentParser()
    args = ap.parse_args()

    print("=" * 70)
    print("CANONICAL-ASM CANDIDATES — USER REVIEW REQUIRED")
    print("=" * 70)
    print()
    print("Per the user's directive: 'If a function is identified as having")
    print("canonical asm, it can be included as authorized asm, but only after")
    print("hard confirmation.' Agents cannot self-authorize. This list is for")
    print("the user's review on return.")
    print()

    print("--- Tier 1: STRONG hand-coded scanner verdict ---")
    print("(Per feedback_hand_coded_asm_recognition.md — multiple GCC-impossible signals)")
    strong = get_strong_hand_coded()
    if strong:
        for func, score, total in strong:
            print(f"  {func:50}  signals={score}/{total}")
    else:
        print("  (none currently)")
    print()

    print("--- Tier 2: Heavy regfix volume (≥30 rules) ---")
    print("(Heavy cheat/workaround usage suggests cc1 cannot match naturally)")
    heavy = get_heavy_cheats()
    for func, count, cheat in heavy[:20]:
        print(f"  {func:50}  rules={count:>3}{cheat}")
    if len(heavy) > 20:
        print(f"  ... and {len(heavy) - 20} more")
    print()

    print("--- Recommendation ---")
    print("  Tier 1 candidates: confirm via memory & scan_hand_coded; add to")
    print("    inline_asm_canonical.txt with one-line evidence note.")
    print("  Tier 2 candidates: investigate per function — may be retirable with")
    print("    new tooling (per documented intractability) OR may warrant canonical-asm")
    print("    authorization if user agrees the cost of pure-C exceeds the value.")
    print()
    print("To authorize a function as canonical-asm:")
    print("  1. Append `<func_name>  # <evidence>` to inline_asm_canonical.txt")
    print("  2. Replace C body with file-scope __asm__(glabel ...) pattern (see")
    print("     decomp-next skill §2.5.c)")
    print("  3. Build + verify; commit")

    return 0


if __name__ == "__main__":
    sys.exit(main())
