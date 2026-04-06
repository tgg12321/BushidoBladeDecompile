#!/usr/bin/env python3
"""Score a candidate .o against a target .o using the permuter's scorer.

Usage: python3 tools/score_match.py <target.o> <candidate.o> [--debug] [--quiet]

Prints the score (integer). Exit 0 if score==0, exit 1 otherwise, exit 2 on error.
With --debug, prints colored side-by-side diff and penalty breakdown.
With --quiet, prints only the numeric score.
"""
import sys
import os
import re
import subprocess

def main():
    args = sys.argv[1:]
    debug = "--debug" in args
    quiet = "--quiet" in args
    args = [a for a in args if not a.startswith("--")]

    if len(args) < 2:
        print("Usage: python3 tools/score_match.py <target.o> <candidate.o> [--debug] [--quiet]", file=sys.stderr)
        sys.exit(2)

    target_o = args[0]
    cand_o = args[1]

    if not os.path.exists(target_o):
        print(f"ERROR: {target_o} not found", file=sys.stderr)
        sys.exit(2)
    if not os.path.exists(cand_o):
        print(f"ERROR: {cand_o} not found", file=sys.stderr)
        sys.exit(2)

    # Validate .text sections are non-empty (catches silently dropped GTE instructions)
    for label, path in [("target", target_o), ("candidate", cand_o)]:
        try:
            r = subprocess.run(
                ["mipsel-linux-gnu-objdump", "-h", path],
                capture_output=True, text=True, timeout=5)
            m = re.search(r'\.text\s+(\w+)', r.stdout)
            if not m or m.group(1) == "00000000":
                print(f"ERROR: {label} {path} has empty .text section!", file=sys.stderr)
                print(f"  Score would be meaningless (comparing empty objects).", file=sys.stderr)
                sys.exit(2)
        except Exception as e:
            print(f"WARNING: Could not validate {label} .text section: {e}", file=sys.stderr)

    # Add decomp-permuter to path
    script_dir = os.path.dirname(os.path.abspath(__file__))
    root = script_dir
    while not os.path.exists(os.path.join(root, "Makefile")):
        parent = os.path.dirname(root)
        if parent == root:
            print("ERROR: Cannot find project root", file=sys.stderr)
            sys.exit(2)
        root = parent

    permuter_path = os.path.join(root, "tools", "decomp-permuter")
    if permuter_path not in sys.path:
        sys.path.insert(0, permuter_path)

    from src.scorer import Scorer

    scorer = Scorer(
        target_o,
        stack_differences=False,
        algorithm="difflib",
        debug_mode=debug,
        ign_branch_targets=True,
        objdump_command="mipsel-linux-gnu-objdump -drz -m mips:4300",
    )

    score, hash_val = scorer.score(cand_o)

    if quiet:
        print(score)
    else:
        print(f"Score: {score}")

    sys.exit(0 if score == 0 else 1)

if __name__ == "__main__":
    main()
