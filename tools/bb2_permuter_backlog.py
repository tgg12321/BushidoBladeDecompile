#!/usr/bin/env python3
"""
bb2_permuter_backlog.py -- apply the BB2 targeted permuter to backlog
functions (already-decompiled C with regfix rules), aiming to find pure-C
replacements that match without regfix.

Workflow per function:
  1. Set up permuter/<func>/ via `dc.sh inline-setup` (gets target.s,
     target.o, m2c-generated base.c)
  2. Prebake target.s arg-pin chain into base.c
  3. Run bb2_permuter with BB2_PERMUTER_HEAVY=1 for time-budget seconds
  4. If a score=0 candidate is found, save it to backlog_results/<func>/

Use --top-N to run on the N functions with fewest regfix rules. Use
--func <name> to target a specific function.
"""
from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path
from typing import List, Dict, Tuple


def read_text_lf(path: Path) -> str:
    return path.read_bytes().decode("utf-8", errors="ignore")


def write_text_lf(path: Path, content: str) -> None:
    path.write_bytes(content.encode("utf-8"))


def find_backlog_candidates(regfix_path: Path, max_rules: int = 3) -> List[Tuple[str, int]]:
    """Read regfix.txt, return [(func, rule_count)] for functions with
    1..max_rules rules, sorted by ascending rule count.
    """
    from collections import Counter
    counter: Counter = Counter()
    with regfix_path.open(encoding="utf-8", errors="ignore") as f:
        for line in f:
            m = re.match(r"^([a-zA-Z_]\w+):", line)
            if m:
                counter[m.group(1)] += 1
    return sorted(
        [(f, c) for f, c in counter.items() if 1 <= c <= max_rules],
        key=lambda x: x[1],
    )


def run_backlog(
    root: Path,
    func: str,
    time_seconds: int = 180,
    workers: int = 6,
) -> Dict:
    """Set up, prebake, and run the targeted permuter on one backlog
    function. Returns a result dict with status, best_score, etc.
    """
    print(f"\n{'=' * 70}")
    print(f"=== {func}")
    print(f"{'=' * 70}")

    wsl_root = str(root).replace("C:", "/mnt/c").replace("\\", "/")
    permuter_dir = root / "permuter" / func
    base_c = permuter_dir / "base.c"

    # Setup (skip if dir already exists from a prior run)
    if not base_c.exists():
        print(f"[1/3] dc.sh inline-setup {func}...")
        result = subprocess.run(
            ["wsl", "bash", "-c", f'cd "{wsl_root}" && bash tools/dc.sh inline-setup {func} 2>&1'],
            capture_output=True,
            text=True,
            timeout=120,
        )
        if not base_c.exists():
            return {
                "func": func,
                "status": "setup-failed",
                "error": (result.stderr or result.stdout).splitlines()[-3:],
            }
    else:
        print(f"[1/3] base.c exists; skipping setup")

    # Prebake
    print(f"[2/3] prebake...")
    prebake_cmd = (
        f'cd "{wsl_root}" && source .venv/bin/activate && '
        f'python3 tools/bb2_permuter_regression.py prebake {func} '
        f'--base-path permuter/{func}/base.c '
        f'--target-path permuter/{func}/target.s 2>&1'
    )
    result = subprocess.run(
        ["wsl", "bash", "-c", prebake_cmd],
        capture_output=True,
        text=True,
        timeout=60,
    )
    print(result.stdout)
    if result.returncode != 0:
        # Prebake errors are non-fatal; the permuter still runs with
        # target-aware weighting (Phase 2 v2)
        print("  (prebake skipped; running with Phase 2 weighting only)")

    # Run permuter
    print(f"[3/3] bb2_permuter {time_seconds}s -j{workers}...")
    # Clear previous outputs
    for d in permuter_dir.glob("output-*/"):
        if d.is_dir():
            shutil.rmtree(d, ignore_errors=True)

    cmd = (
        f'cd "{wsl_root}" && source .venv/bin/activate && '
        f'BB2_PERMUTER_HEAVY=1 timeout {time_seconds} python3 tools/bb2_permuter.py '
        f'permuter/{func} --stop-on-zero --best-only -j {workers} 2>&1'
    )
    log_path = root / "backlog_results" / func / "permuter.log"
    log_path.parent.mkdir(parents=True, exist_ok=True)
    result = subprocess.run(
        ["wsl", "bash", "-c", cmd],
        capture_output=True,
        text=True,
        timeout=time_seconds + 60,
    )
    write_text_lf(log_path, result.stdout)

    # Parse the log for the best score
    scores = re.findall(r"score = (\d+)", result.stdout)
    iters = re.findall(r"iteration (\d+)", result.stdout)
    best_score = min(int(s) for s in scores) if scores else None
    max_iters = max(int(i) for i in iters) if iters else 0
    is_match = best_score == 0

    # If we found a match, copy the candidate
    match_src = None
    if is_match:
        for d in sorted(permuter_dir.glob("output-0-*/")):
            src_c = d / "source.c"
            if src_c.exists():
                match_src = root / "backlog_results" / func / "source.c"
                match_src.write_bytes(src_c.read_bytes())
                break

    record = {
        "func": func,
        "status": "matched" if is_match else "improved" if best_score and best_score < 1000 else "no-progress",
        "best_score": best_score,
        "iterations": max_iters,
        "match_source": str(match_src) if match_src else None,
    }
    write_text_lf(
        root / "backlog_results" / func / "result.json",
        json.dumps(record, indent=2),
    )
    print(f"  Result: best={best_score} iters={max_iters} status={record['status']}")
    return record


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--root", default=".", help="Project root")
    p.add_argument("--func", help="Run on a specific function")
    p.add_argument("--top-n", type=int, default=5, help="Run on N smallest-regfix functions")
    p.add_argument("--max-rules", type=int, default=3, help="Only consider funcs with <=N rules")
    p.add_argument("--time-seconds", type=int, default=180)
    p.add_argument("--workers", type=int, default=6)
    args = p.parse_args()
    root = Path(args.root).resolve()

    funcs: List[str]
    if args.func:
        funcs = [args.func]
    else:
        candidates = find_backlog_candidates(root / "regfix.txt", max_rules=args.max_rules)
        funcs = [f for f, _ in candidates[:args.top_n]]

    results = []
    for func in funcs:
        rec = run_backlog(root, func, args.time_seconds, args.workers)
        results.append(rec)

    # Aggregate
    print(f"\n{'=' * 70}")
    print(f"=== Aggregate")
    print(f"{'=' * 70}")
    matched = [r for r in results if r["status"] == "matched"]
    print(f"  Matched (score=0): {len(matched)} / {len(results)}")
    for r in matched:
        print(f"    {r['func']}  (iters={r['iterations']})")
    print()
    write_text_lf(
        root / "backlog_results" / "summary.json",
        json.dumps(results, indent=2),
    )


if __name__ == "__main__":
    main()
