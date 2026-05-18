#!/usr/bin/env python3
"""For each asmfix queue function, measure initial diff after retire (without C body changes).

Iterates the active asmfix queue, retires the bridge, builds, captures
diff count via verify-c, restores the bridge, and ranks functions by
smallest initial diff. Use to find candidates where the existing C body
(stub or partial) is already close to matching.

Usage:
    python3 tools/probe_asmfix_initial_diffs.py [--limit N] [--src FILE]

Output: ranking of (func, initial_diffs, src) sorted ascending.
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
WORK_QUEUE = ROOT / "WORK_QUEUE.md"
ASMFIX = ROOT / "asmfix.txt"
ACTIVE_MARKER = ROOT / ".bb2_active_func"


def parse_queue() -> list[tuple[str, int, str]]:
    text = WORK_QUEUE.read_text(encoding="utf-8")
    rows = []
    in_section = False
    in_block = False
    for line in text.splitlines():
        if line.startswith("## "):
            in_section = "Asmfix Retirement Queue" in line
            continue
        if not in_section:
            continue
        if line.strip() == "```":
            in_block = not in_block
            continue
        if not in_block:
            continue
        m = re.match(r"^\s*\d+\s+(\S+)\s+(\d+)\s+\S+\s+(\S+)", line)
        if m:
            rows.append((m.group(1), int(m.group(2)), m.group(3)))
    return rows


def run(cmd: list[str], cwd: Path = ROOT) -> tuple[int, str]:
    p = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    return p.returncode, p.stdout + p.stderr


def retire(func: str) -> bool:
    asmfix_text = ASMFIX.read_text(encoding="utf-8")
    pattern = rf"^({re.escape(func)}: replace_with_asmfile .*)$"
    new = re.sub(pattern, r"# RETIRE: \1", asmfix_text, flags=re.MULTILINE, count=1)
    if new == asmfix_text:
        return False
    ASMFIX.write_text(new, encoding="utf-8")
    return True


def restore(func: str) -> None:
    asmfix_text = ASMFIX.read_text(encoding="utf-8")
    pattern = rf"^# RETIRE: ({re.escape(func)}: replace_with_asmfile .*)$"
    new = re.sub(pattern, r"\1", asmfix_text, flags=re.MULTILINE, count=1)
    ASMFIX.write_text(new, encoding="utf-8")


def measure_diff(func: str, src: str) -> int | None:
    src_obj = ROOT / "build" / "src" / src.replace(".c", ".o")
    if src_obj.exists():
        src_obj.unlink()
    rc, out = run(["bash", "tools/wsl.sh", "bash tools/dc.sh build"])
    if "matches!" in out and "MISMATCH" not in out:
        return 0  # Wait — if build matches, retire was no-op
    rc, out = run(["bash", "tools/wsl.sh", f"bash tools/dc.sh verify-c {func}"])
    m = re.search(r"(\d+) instruction\(s\) differ", out)
    if m:
        return int(m.group(1))
    if "MATCH (0 diffs" in out:
        return 0
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--limit", type=int, default=10, help="Max functions to probe")
    ap.add_argument("--src", help="Only probe functions in this src file")
    ap.add_argument("--max-size", type=int, default=300, help="Skip funcs bigger than this (insns)")
    args = ap.parse_args()
    if ACTIVE_MARKER.exists() and ACTIVE_MARKER.read_text().strip():
        print(f"ERROR: .bb2_active_func is set ('{ACTIVE_MARKER.read_text().strip()}'); clear first.")
        sys.exit(1)
    rows = parse_queue()
    if args.src:
        rows = [r for r in rows if args.src in r[2]]
    rows = [r for r in rows if r[1] <= args.max_size]
    rows = rows[: args.limit]
    print(f"Probing {len(rows)} functions...\n")
    results = []
    for i, (func, size, src) in enumerate(rows, 1):
        print(f"[{i}/{len(rows)}] {func} ({size} insns, {src})...", end=" ", flush=True)
        if not retire(func):
            print("could not retire (no asmfix entry)")
            continue
        try:
            diff = measure_diff(func, src)
            if diff is None:
                print("FAILED to measure (link error?)")
            else:
                print(f"{diff} diffs")
                results.append((diff, size, func, src))
        finally:
            restore(func)
            run(["bash", "tools/wsl.sh", "bash tools/dc.sh build"])
    print("\n=== Sorted by initial diff (smallest first) ===")
    print(f"{'diffs':>5}  {'size':>5}  {'func':<32}  src")
    print("-" * 80)
    for diff, size, func, src in sorted(results):
        print(f"{diff:>5}  {size:>5}  {func:<32}  {src}")


if __name__ == "__main__":
    main()
