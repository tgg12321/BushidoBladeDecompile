#!/usr/bin/env python3
"""Produce a final summary report from tmp/batch_attempt.csv.

Outputs:
  - Headline: total tractable, auto-MATCHED, NEAR_MISS, HARD, ERROR
  - NEAR_MISS leaderboard (sorted by score; lowest = closest to a match)
  - File-by-file breakdown
  - Size-bucketed breakdown (small / medium / large functions)

Usage:
    python3 tools/batch_report.py
    python3 tools/batch_report.py --csv tmp/batch_attempt.csv
"""
from __future__ import annotations

import argparse
import csv
import sys
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_FUNCS = ROOT / "asm" / "funcs"


def asm_size(name: str) -> int:
    p = ASM_FUNCS / f"{name}.s"
    if not p.exists():
        return 0
    return sum(1 for line in p.read_text(encoding="utf-8").splitlines()
               if "/* " in line and " */" in line)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--csv", default="tmp/batch_attempt.csv")
    args = ap.parse_args()

    rows = list(csv.DictReader(open(args.csv)))
    if not rows:
        print("(empty CSV)")
        return 1

    total = len(rows)
    counts = Counter(r["result"] for r in rows)

    print(f"=== batch_report: {args.csv} ({total} rows) ===")
    print()
    for k in ("MATCHED", "NEAR_MISS", "HARD", "ERROR", "SKIPPED",
              "NOT_FOUND", "PROC_TIMEOUT", "NO_OUTPUT"):
        n = counts.get(k, 0)
        if n:
            pct = 100 * n / total
            print(f"  {k:14s} {n:>4d}  ({pct:>5.1f}%)")

    # MATCHED leaderboard
    matched = [r for r in rows if r["result"] == "MATCHED"]
    if matched:
        print()
        print("MATCHED:")
        for r in matched:
            print(f"  {r['func']:30s}  via {r['stage']:<10s}  ({r.get('elapsed','?')}s)")

    # NEAR_MISS leaderboard
    near = []
    for r in rows:
        if r["result"] != "NEAR_MISS":
            continue
        try:
            sc = int(r["score"])
        except (ValueError, KeyError):
            continue
        near.append((sc, r["func"], r.get("src", "")))
    near.sort()
    if near:
        print()
        print("NEAR_MISS (lowest score = closest to match):")
        for sc, fn, src in near[:20]:
            srcname = src.replace("src/", "").replace("src\\", "")
            print(f"  {sc:>5d}  {fn:30s}  {srcname}")
        if len(near) > 20:
            print(f"  ... +{len(near)-20} more")

    # By size bucket
    print()
    print("By size bucket:")
    buckets = [(0, 30), (31, 60), (61, 120), (121, 250), (251, 9999)]
    bucket_counts: dict[str, Counter] = {}
    for lo, hi in buckets:
        key = f"{lo:>4}-{hi:<4}"
        bucket_counts[key] = Counter()

    for r in rows:
        s = asm_size(r["func"])
        for lo, hi in buckets:
            if lo <= s <= hi:
                key = f"{lo:>4}-{hi:<4}"
                bucket_counts[key][r["result"]] += 1
                break

    headers = ["MATCHED", "NEAR_MISS", "HARD", "ERROR"]
    print(f"  {'size':<10} " + " ".join(f"{h:>10s}" for h in headers))
    for bucket, c in bucket_counts.items():
        row_str = f"  {bucket:<10}"
        for h in headers:
            n = c.get(h, 0)
            row_str += f" {n:>10d}"
        print(row_str)

    # By source file
    print()
    print("By source file:")
    by_src: dict[str, Counter] = defaultdict(Counter)
    for r in rows:
        src = r.get("src", "?").replace("src/", "").replace("src\\", "")
        by_src[src][r["result"]] += 1
    for src in sorted(by_src):
        c = by_src[src]
        n_total = sum(c.values())
        cells = " ".join(f"{c.get(h,0):>4d}" for h in ("MATCHED","NEAR_MISS","HARD","ERROR"))
        print(f"  {src:<22}  total={n_total:>4d}  M/NM/H/E= {cells}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
