#!/usr/bin/env python3
"""Scan asm/funcs/*.s for permanent-blocker patterns.

Reuses classify_func.detect_permanent_blockers and adds a few extra
detectors (cop0 ops, raw syscall instruction, wait, multi .word at the
end of body that looks like inline data). Reports a sorted list of
candidates that aren't already in known_blocked.txt.

Usage:
    python3 tools/scan_blocked.py [--all] [--csv tmp/blocked_candidates.csv]

--all: emit every match including ones already sequestered (for audit).
"""
from __future__ import annotations

import argparse
import csv
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
from classify_func import (
    load_asm_lines,
    detect_permanent_blockers,
    is_bios_trampoline,
    load_known_blocked,
)

ASM_FUNCS = ROOT / "asm" / "funcs"


# Extra detectors beyond classify_func's set. Most of these have been folded
# into classify_func.detect_permanent_blockers; what remains here is purely
# advisory (won't bump a function to permanently_blocked, just notes it).

def extra_blockers(insns) -> list[str]:
    tags: list[str] = []
    mnems = [m for _, m, _ in insns]
    text = "\n".join(ops for _, _, ops in insns)
    if "wait" in mnems:
        tags.append("wait_instruction")
    # Unusual: $k0/$k1 (kernel scratch regs) used as operands
    if "$k0" in text or "$k1" in text:
        tags.append("kernel_register")
    return tags


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--all", action="store_true",
                    help="Include functions already in known_blocked.txt")
    ap.add_argument("--csv", default="tmp/blocked_candidates.csv")
    args = ap.parse_args()

    known = load_known_blocked()
    files = sorted(ASM_FUNCS.glob("*.s"))
    print(f"# Scanning {len(files)} asm files for permanent-blocker patterns...")

    matches: list[dict] = []
    counts: dict[str, int] = {}
    for p in files:
        func = p.stem
        insns = load_asm_lines(func)
        if not insns:
            continue
        tags = detect_permanent_blockers(insns) + extra_blockers(insns)
        bios, bios_addr = is_bios_trampoline(insns)
        if bios:
            tags.append(f"bios_or_syscall:{bios_addr}")
        if not tags:
            continue
        if not args.all and func in known:
            continue
        for t in tags:
            root = t.split(":")[0]
            counts[root] = counts.get(root, 0) + 1
        matches.append({
            "func": func,
            "size": len(insns),
            "tags": ",".join(tags),
            "first_tag": tags[0],
            "already_known": func in known,
        })

    matches.sort(key=lambda r: (r["first_tag"], r["size"], r["func"]))
    out = ROOT / args.csv
    out.parent.mkdir(parents=True, exist_ok=True)
    with out.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=list(matches[0].keys()) if matches else
                                          ["func","size","tags","first_tag","already_known"])
        w.writeheader()
        w.writerows(matches)

    print(f"# Wrote {out.relative_to(ROOT)} ({len(matches)} candidates)")
    print()
    print("# Tag counts:")
    for tag, n in sorted(counts.items(), key=lambda x: -x[1]):
        print(f"  {tag:30s}  {n}")
    print()
    print("# Top 40 candidates by tag/size:")
    for r in matches[:40]:
        marker = " (known)" if r["already_known"] else ""
        print(f"  {r['func']:30s} {r['size']:>4d} insns  {r['first_tag']:30s}{marker}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
