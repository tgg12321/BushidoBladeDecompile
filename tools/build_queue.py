#!/usr/bin/env python3
"""Generate a ranked work queue for overnight matching sessions.

Reads tmp/batch_attempt.csv (the classify output), filters to tractable
candidates, and emits a sorted candidate list with priority signals so an
agent can pick the most likely-to-match candidate first.

Priority signals (lower = better):
  1. Size bucket (smallest first -- shorter diffs, faster iteration)
  2. Has prior NEAR_MISS data (we know it's close)
  3. No structural blockers in the asm
  4. Has a clear recipe fingerprint (gen_regfix-style diff)

Output: tmp/work_queue.txt with one func per line, plus a CSV with all
the metadata.
"""
from __future__ import annotations

import csv
import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_FUNCS = ROOT / "asm" / "funcs"
PERMUTER = ROOT / "permuter"


def asm_size(name: str) -> int:
    p = ASM_FUNCS / f"{name}.s"
    if not p.exists():
        return 0
    return sum(1 for line in p.read_text(encoding="utf-8").splitlines()
               if "/* " in line and " */" in line)


def has_permuter_dir(name: str) -> bool:
    return (PERMUTER / name / "base.c").exists()


def load_known_blocked() -> set[str]:
    """Read known_blocked.txt -> set of func names. Defensive against
    missing/malformed rows. Used as a belt-and-suspenders filter even when
    the CSV's recommendation field already says permanently_blocked."""
    p = ROOT / "known_blocked.txt"
    out: set[str] = set()
    if not p.exists():
        return out
    for raw in p.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        first = line.split()[0] if line.split() else ""
        if first:
            out.add(first)
    return out


def main():
    csv_path = ROOT / "tmp" / "batch_attempt.csv"
    if not csv_path.exists():
        print(f"ERROR: {csv_path} missing -- run batch_attempt --classify-only first",
              file=sys.stderr)
        return 1

    rows = list(csv.DictReader(open(csv_path)))
    blocked = load_known_blocked()

    def is_tractable(rec: str) -> bool:
        if rec in ("easy_attempt", "standard"):
            return True
        # Defensive: anything starting with "permanently_blocked",
        # "bios_or_syscall", "psyq_stdlib", or that names a func in
        # known_blocked.txt is filtered out below.
        return False

    # Filter: tractable recommendation AND not in known_blocked.txt
    tract = [r for r in rows
             if is_tractable(r.get("recommendation", ""))
             and r.get("func") not in blocked]
    skipped_blocked = sum(1 for r in rows if r.get("func") in blocked)
    if skipped_blocked:
        print(f"# Filtered {skipped_blocked} permanently-blocked functions "
              f"(known_blocked.txt)", file=sys.stderr)

    # Annotate
    enriched = []
    for r in tract:
        sz = int(r.get("size_insns", 0) or 0)
        enriched.append({
            "func": r["func"],
            "src": r.get("src", ""),
            "rec": r["recommendation"],
            "size": sz,
            "has_perm": has_permuter_dir(r["func"]),
        })

    # Sort: smallest first, then by recommendation (easy_attempt before standard)
    rec_order = {"easy_attempt": 0, "standard": 1}
    enriched.sort(key=lambda r: (r["size"], rec_order.get(r["rec"], 9), r["func"]))

    # Write queue + CSV
    out_txt = ROOT / "tmp" / "work_queue.txt"
    out_csv = ROOT / "tmp" / "work_queue.csv"
    with out_txt.open("w", encoding="utf-8", newline="\n") as f:
        f.write(f"# Overnight work queue, {len(enriched)} candidates\n")
        f.write("# Sorted: small-first, easy_attempt before standard\n")
        f.write("# Format: <func>  <size>  <rec>  <src>\n\n")
        for r in enriched:
            f.write(f"{r['func']:30s}  {r['size']:4d}  {r['rec']:14s}  "
                    f"{r['src'].replace('src/','')}\n")
    with out_csv.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=["func", "size", "rec", "src", "has_perm"])
        w.writeheader()
        w.writerows(enriched)

    # Print top 30 + bucket summary
    print(f"# Wrote {out_txt.relative_to(ROOT)} ({len(enriched)} candidates)")
    print()
    print("# Size bucket summary:")
    buckets = [(0, 15), (16, 30), (31, 60), (61, 120), (121, 250), (251, 9999)]
    for lo, hi in buckets:
        n = sum(1 for r in enriched if lo <= r["size"] <= hi)
        print(f"  {lo:>4}-{hi:<4d}  {n:>4d}")

    print()
    print(f"# First 30 candidates (smallest, highest match probability):")
    for r in enriched[:30]:
        src = r["src"].replace("src/", "")
        print(f"  {r['func']:30s} {r['size']:>4d} insns  {r['rec']:14s} {src}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
