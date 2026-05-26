#!/usr/bin/env python3
"""permute_adaptive.py -- run permuter with budget scaled to diff penalty.

Audit found workers running fixed 10-30min permuter regardless of how
much structural diff existed. Most score plateaus happen in the first
5min when diff is small; longer runs only help when ins+del is high.

Budget rule:
  0 ins/del        -> 0 (skip permuter; do regfix swap instead)
  1-2 ins/del      -> 90s
  3-5 ins/del      -> 5 min
  6-10 ins/del     -> 15 min
  >10 ins/del      -> 30 min

Usage:
  python3 tools/permute_adaptive.py <func>      # run with adaptive budget
  python3 tools/permute_adaptive.py <func> --dry-run   # print budget only

Falls back to permute_capped.py with the chosen budget.
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def run(cmd: list[str]) -> str:
    return subprocess.check_output(cmd, cwd=ROOT, text=True, errors="replace")


def get_penalty_breakdown(func: str) -> dict[str, int] | None:
    """Run dc.sh diff and parse the penalty list. Returns dict with
    ins/del/reord/reg counts, or None if diff doesn't run."""
    try:
        out = subprocess.check_output(
            ["bash", "tools/dc.sh", "diff", func],
            cwd=ROOT, text=True, errors="replace", stderr=subprocess.DEVNULL
        )
    except subprocess.CalledProcessError:
        return None
    counts = {"ins": 0, "del": 0, "reord": 0, "reg": 0}
    for line in out.splitlines():
        m = re.search(r"(insertions?|deletions?|reorderings?|register)\s*:\s*(\d+)", line, re.I)
        if m:
            kind = m.group(1).lower()
            n = int(m.group(2))
            if kind.startswith("ins"):
                counts["ins"] = n
            elif kind.startswith("del"):
                counts["del"] = n
            elif kind.startswith("reord"):
                counts["reord"] = n
            elif kind.startswith("reg"):
                counts["reg"] = n
    return counts


def choose_budget(ins: int, dele: int) -> int:
    """Return permuter budget in seconds."""
    structural = ins + dele
    if structural == 0:
        return 0
    if structural <= 2:
        return 90
    if structural <= 5:
        return 300  # 5 min
    if structural <= 10:
        return 900  # 15 min
    return 1800  # 30 min


def main() -> int:
    args = sys.argv[1:]
    dry = "--dry-run" in args
    args = [a for a in args if a != "--dry-run"]
    if len(args) != 1:
        print(f"Usage: {sys.argv[0]} <func> [--dry-run]", file=sys.stderr)
        return 1
    func = args[0]

    counts = get_penalty_breakdown(func)
    if counts is None:
        print(f"[permute-adaptive] could not get diff for {func}; using default 90s", file=sys.stderr)
        budget = 90
    else:
        budget = choose_budget(counts["ins"], counts["del"])
        print(f"[permute-adaptive] {func} diff: ins={counts['ins']} del={counts['del']} "
              f"reord={counts['reord']} reg={counts['reg']}")
        if budget == 0:
            print(f"[permute-adaptive] structural diff is 0 (only register diffs); "
                  f"SKIP permuter, use regfix swap rules instead.")
            return 0
        print(f"[permute-adaptive] budget = {budget}s ({budget // 60}min)")

    if dry:
        return 0

    # Delegate to permute_capped.py
    cmd = ["python3", "tools/permute_capped.py", func, "--max-time", str(budget)]
    print(f"[permute-adaptive] running: {' '.join(cmd)}")
    rc = subprocess.call(cmd, cwd=ROOT)
    return rc


if __name__ == "__main__":
    sys.exit(main())
