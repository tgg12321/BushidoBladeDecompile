#!/usr/bin/env python3
"""Dry-run the NON-BUILD half of `engine queue done`'s completion gate.

`queue done` refuses a function that still carries any cheat, then verifies the
full-build SHA1 against the oracle. This reports the cheap half — every cheat
predicate the gate checks — WITHOUT a build, so a candidate can be checked in
seconds instead of a full link.

This is a briefing aid, not a gate. All-clear here means only that the cheat
predicates pass; the oracle SHA1 remains the sole proof of a match, and the
semantic `cheat-reviewer` layer stays mandatory for completion-class commits.

Usage (WSL, venv active, from the repo root):
  python3 tools/gate_precheck.py <func> <stem>

Example:
  python3 tools/gate_precheck.py func_8003F388 config
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

from engine import cheats, inlineasm, queue  # noqa: E402


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Report the cheat predicates `queue done` checks.")
    ap.add_argument("func", help="function to check")
    ap.add_argument("stem", help="src/<stem>.c containing it")
    args = ap.parse_args()

    print("rules            :", queue._rule_count(args.func))
    print("prologue_fix     :", cheats.func_prologue_count(args.func))
    print("maspsx gates     :", cheats.maspsx_gate_entries(args.func))
    print("canonical-listed :", args.func in cheats.canonical_asm_funcs())
    print("cheat constructs :",
          inlineasm.file_func_cheat_asm_count(args.stem, args.func))
    return 0


if __name__ == "__main__":
    sys.exit(main())
