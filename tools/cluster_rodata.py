#!/usr/bin/env python3
"""Classify rodata blocks into clusters and rank pilot candidates.

Phase 1 of the rodata-cleanup project (docs/rodata-cleanup-project.md).

Reads memory/project/rodata_block_inventory.csv (produced by
tools/audit_rodata_blocks.py) and emits memory/project/rodata_clusters.csv —
one row per linked rodata block, with cluster classification:

  - kind: single-function / single-file / multi-file / trivial
  - readiness: all-matched / partial / all-stub / blocked
  - pilot rank: numeric difficulty estimate (lower = easier)

Also prints a ranked summary of pilot candidates for §5 Phase 1b.
"""

from __future__ import annotations

import csv
import sys
from collections import defaultdict
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
INVENTORY = REPO / "memory" / "project" / "rodata_block_inventory.csv"
OUT_CSV = REPO / "memory" / "project" / "rodata_clusters.csv"

# Trivial blocks (no symbols, just padding) that remain in bb2.ld.
# 101C.rodata_c_pre.s and 101C.rodata_text1a_DB8.s were retired 2026-06-09.
TRIVIAL_BLOCKS = {
    "101C.rodata_post.s",
}


def classify_cluster(rows: list[dict]) -> dict:
    """Reduce a list of inventory rows for one block into a cluster record."""
    symbols: dict[str, dict] = {}
    owners: set[str] = set()
    files: set[str] = set()
    status_counts: dict[str, int] = defaultdict(int)
    queue_counts: dict[str, int] = defaultdict(int)

    for r in rows:
        sym = r["symbol"]
        if sym not in symbols:
            symbols[sym] = {
                "address": r["address"],
                "type": r["type"],
                "size_bytes": int(r["size_bytes"]) if r["size_bytes"].isdigit() else 0,
            }
        if r["owner_func"] != "(none)":
            owners.add(r["owner_func"])
            if r["owner_file"] and r["owner_file"] != "?":
                files.add(r["owner_file"])
        status_counts[r["owner_status"]] += 1
        if r["queue_status"] not in ("-", ""):
            queue_counts[r["queue_status"]] += 1

    n_owners = len(owners)
    n_files = len(files)
    bytes_total = sum(s["size_bytes"] for s in symbols.values())
    jtbl_count = sum(1 for s in symbols.values() if s["type"] == "jtbl")
    string_count = sum(1 for s in symbols.values() if s["type"] == "string")
    data_count = sum(1 for s in symbols.values() if s["type"] == "data")

    # Kind classification
    if n_owners == 0:
        kind = "all-orphan"
    elif n_owners == 1:
        kind = "single-function"
    elif n_files <= 1:
        kind = "single-file"
    else:
        kind = "multi-file"

    # Readiness: are all owners already byte-emitting from C?
    # 'matched' = completed (compiled from C) OR active/parked-not-stub
    # 'blocked' = stub or orphan or has any stubs
    n_stubs = status_counts.get("stub", 0)
    n_completed = status_counts.get("completed", 0)
    n_active = status_counts.get("active", 0)
    n_parked = status_counts.get("parked", 0)
    n_authorize = status_counts.get("authorize", 0)
    n_orphan = status_counts.get("orphan", 0)

    if n_stubs == 0 and n_orphan == 0 and (n_active + n_parked + n_authorize) == 0:
        readiness = "all-matched"  # only completed owners
    elif n_stubs == 0 and n_orphan == 0:
        readiness = "matched-with-incomplete"  # some active/parked, no stubs
    elif n_stubs > 0 and n_completed > 0:
        readiness = "partial"
    elif n_stubs > 0 and n_completed == 0:
        readiness = "all-stub"
    else:
        readiness = "unknown"

    # Pilot rank (lower = easier). Heuristic:
    #   base 1000
    #   - kind: single-function -10000, single-file -5000, multi-file 0, all-orphan +5000
    #   - readiness: all-matched -2000, matched-with-incomplete -500, partial 0, all-stub +500, unknown +1000
    #   - byte size (lighter = easier): + bytes_total / 100
    #   - jtbl count (jtbls harder than strings): + jtbl_count * 50
    #   - n_owners (more owners harder): + n_owners * 30
    #   - n_files (cross-file harder): + n_files * 100
    rank = 1000
    rank += {"single-function": -10000, "single-file": -5000, "multi-file": 0, "all-orphan": 5000}[kind]
    rank += {
        "all-matched": -2000,
        "matched-with-incomplete": -500,
        "partial": 0,
        "all-stub": 500,
        "unknown": 1000,
    }[readiness]
    rank += bytes_total // 100
    rank += jtbl_count * 50
    rank += n_owners * 30
    rank += n_files * 100

    return {
        "n_symbols": len(symbols),
        "n_jtbls": jtbl_count,
        "n_strings": string_count,
        "n_data": data_count,
        "bytes": bytes_total,
        "n_owners": n_owners,
        "n_files": n_files,
        "files": ",".join(sorted(files)) if files else "",
        "owners": ",".join(sorted(owners)) if owners else "",
        "kind": kind,
        "readiness": readiness,
        "n_stubs": n_stubs,
        "n_completed": n_completed,
        "n_active": n_active,
        "n_parked": n_parked,
        "n_authorize": n_authorize,
        "n_orphan": n_orphan,
        "pilot_rank": rank,
    }


def main() -> int:
    if not INVENTORY.exists():
        print(
            f"ERR: {INVENTORY} not found — run tools/audit_rodata_blocks.py first",
            file=sys.stderr,
        )
        return 2

    rows = list(csv.DictReader(INVENTORY.open(encoding="utf-8")))
    by_block: dict[str, list[dict]] = defaultdict(list)
    for r in rows:
        by_block[r["block_file"]].append(r)

    cluster_rows = []
    for block_file, block_rows in by_block.items():
        c = classify_cluster(block_rows)
        c["block_file"] = block_file
        cluster_rows.append(c)
    # Also emit trivial blocks (those with no inventory rows)
    for tb in TRIVIAL_BLOCKS:
        if tb not in by_block:
            cluster_rows.append(
                {
                    "block_file": tb,
                    "n_symbols": 0,
                    "n_jtbls": 0,
                    "n_strings": 0,
                    "n_data": 0,
                    "bytes": 0,
                    "n_owners": 0,
                    "n_files": 0,
                    "files": "",
                    "owners": "",
                    "kind": "trivial",
                    "readiness": "deletion-ready",
                    "n_stubs": 0,
                    "n_completed": 0,
                    "n_active": 0,
                    "n_parked": 0,
                    "n_authorize": 0,
                    "n_orphan": 0,
                    "pilot_rank": -20000,  # easiest of all
                }
            )

    cluster_rows.sort(key=lambda c: c["pilot_rank"])

    fieldnames = [
        "pilot_rank",
        "block_file",
        "kind",
        "readiness",
        "n_symbols",
        "n_jtbls",
        "n_strings",
        "n_data",
        "bytes",
        "n_owners",
        "n_files",
        "files",
        "owners",
        "n_stubs",
        "n_completed",
        "n_active",
        "n_parked",
        "n_authorize",
        "n_orphan",
    ]
    with OUT_CSV.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=fieldnames)
        w.writeheader()
        w.writerows(cluster_rows)
    print(f"wrote {len(cluster_rows)} cluster rows to {OUT_CSV.relative_to(REPO)}", file=sys.stderr)

    # Human-readable summary
    print()
    print(
        f"{'rank':>6} {'block':<40} {'kind':<16} {'readiness':<24} "
        f"{'syms':>4} {'jtb':>3} {'bytes':>6} {'own':>3} {'fil':>3}  "
        f"{'stub':>4} {'comp':>4} {'rest':>4}"
    )
    print("-" * 132)
    for c in cluster_rows:
        rest = c["n_active"] + c["n_parked"] + c["n_authorize"] + c["n_orphan"]
        print(
            f"{c['pilot_rank']:>6} {c['block_file']:<40} {c['kind']:<16} {c['readiness']:<24} "
            f"{c['n_symbols']:>4} {c['n_jtbls']:>3} {c['bytes']:>6} {c['n_owners']:>3} {c['n_files']:>3}  "
            f"{c['n_stubs']:>4} {c['n_completed']:>4} {rest:>4}"
        )

    print()
    print("=== PILOT CANDIDATES (easiest first) ===")
    print()
    for i, c in enumerate(cluster_rows[:5], 1):
        print(f"  #{i}  {c['block_file']}  ({c['kind']}, {c['readiness']})")
        print(f"      {c['n_symbols']} syms, {c['bytes']} bytes, {c['n_owners']} owners across {c['n_files']} file(s)")
        if c['owners']:
            owners_short = c['owners'][:80] + ('...' if len(c['owners']) > 80 else '')
            print(f"      owners: {owners_short}")
        print()

    return 0


if __name__ == "__main__":
    sys.exit(main())
