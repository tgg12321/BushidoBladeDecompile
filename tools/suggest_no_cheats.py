#!/usr/bin/env python3
"""Apply regfix-suggest output, filtering out lost_codegen / wildcard cheats.

Workflow:
1. Run `dc.sh regfix-suggest <func>` to get candidate rules
2. Filter out lost_codegen patterns (`addu Rd, $0, $0`, `addu Rd, Rs, $0`)
3. Filter out wildcard substs (`subst ".*"`)
4. Append filtered rules to regfix.txt
5. Build + verify

Usage: python3 tools/suggest_no_cheats.py <func>
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

LOST_CODEGEN_INSERT = re.compile(
    r'insert(_after|_before)?\s+"addu\s+\$\w+,\s*\$\w+,\s*\$\w+"'
)
WILDCARD_SUBST = re.compile(r'subst\s+"\.\*"')


def is_cheat(line: str) -> bool:
    """Return True if line is a known cheat pattern."""
    if line.startswith("#"):
        return False  # comments not cheats
    if WILDCARD_SUBST.search(line):
        return True
    m = LOST_CODEGEN_INSERT.search(line)
    if m:
        # Check if this matches lost_codegen specifically: $0 in any position
        text = m.group()
        zero_pattern = re.compile(r'\$(0|zero)')
        if len(zero_pattern.findall(text)) >= 1:
            return True
    return False


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("func")
    ap.add_argument("--apply", action="store_true", help="Append filtered rules to regfix.txt")
    ap.add_argument("--max-rules", type=int, default=100)
    args = ap.parse_args()

    # Run regfix-suggest (preview only)
    result = subprocess.run(
        ["bash", "tools/dc.sh", "regfix-suggest", args.func, "--max-rules", str(args.max_rules)],
        capture_output=True, text=True,
    )
    if result.returncode != 0:
        print(f"regfix-suggest failed: {result.stderr}", file=sys.stderr)
        return 1

    legit_rules = []
    cheat_rules = []
    for line in result.stdout.split("\n"):
        if not line.startswith(f"{args.func}:"):
            continue
        if is_cheat(line):
            cheat_rules.append(line)
        else:
            legit_rules.append(line)

    print(f"Total suggested: {len(legit_rules) + len(cheat_rules)}")
    print(f"  Legitimate (will apply): {len(legit_rules)}")
    print(f"  Cheats (skipped): {len(cheat_rules)}")
    print()
    if cheat_rules:
        print("Skipped cheats:")
        for r in cheat_rules:
            print(f"  {r}")
        print()

    if not args.apply:
        print(f"\nLegitimate rules (preview):")
        for r in legit_rules:
            print(f"  {r}")
        return 0

    # Append legit rules to regfix.txt
    regfix = Path("regfix.txt")
    text = regfix.read_text(encoding="utf-8")
    if not text.endswith("\n"):
        text += "\n"
    text += "\n# (suggest_no_cheats.py)\n"
    for r in legit_rules:
        text += r + "\n"
    regfix.write_text(text, encoding="utf-8", newline="\n")
    print(f"Appended {len(legit_rules)} legitimate rules to regfix.txt")
    return 0


if __name__ == "__main__":
    sys.exit(main())
