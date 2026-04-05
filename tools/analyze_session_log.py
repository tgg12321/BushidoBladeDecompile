#!/usr/bin/env python3
"""Analyze session logs for agent efficiency auditing.

Usage:
    python3 tools/analyze_session_log.py                          # all functions
    python3 tools/analyze_session_log.py func_80034200            # one function
    python3 tools/analyze_session_log.py --summary                # ranked summary table
"""
import argparse
import os
import re
import sys
from collections import Counter
from datetime import datetime
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PERMUTER_DIR = ROOT / "permuter"


def parse_log(log_path):
    events = []
    try:
        with open(log_path) as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                parts = line.split("|", 2)
                if len(parts) < 3:
                    continue
                ts_str, event, data = parts
                try:
                    ts = datetime.strptime(ts_str, "%Y-%m-%dT%H:%M:%SZ")
                except ValueError:
                    ts = None
                events.append({"ts": ts, "event": event, "data": data})
    except FileNotFoundError:
        pass
    return events


def analyze(func_name, events, verbose=True):
    if not events:
        return None

    scores = []
    reads = []
    edits = []
    sessions = []
    compile_fails = 0

    for e in events:
        ev = e["event"]
        d = e["data"]
        if ev == "SCORE":
            m = re.search(r"score=(\d+)", d)
            if m:
                scores.append(int(m.group(1)))
        elif ev == "COMPILE_FAIL":
            compile_fails += 1
        elif ev == "READ":
            reads.append(d)
        elif ev == "EDIT":
            edits.append(d)
        elif ev == "SESSION_START":
            sessions.append({"start": e["ts"], "data": d})
        elif ev == "SESSION_END":
            if sessions:
                sessions[-1]["end"] = e["ts"]
                sessions[-1]["end_data"] = d

    if not scores:
        return None

    # Score progression
    score_deltas = []
    for i in range(1, len(scores)):
        score_deltas.append(scores[i] - scores[i - 1])

    wasted_attempts = sum(1 for d in score_deltas if d >= 0)  # no improvement
    best_score = min(scores)
    final_score = scores[-1]
    n_attempts = len(scores)

    # File read analysis
    read_counts = Counter(reads)
    re_reads = {f: c for f, c in read_counts.items() if c > 1}

    # Duration
    duration_min = None
    if sessions and sessions[-1].get("end") and sessions[0]["start"]:
        delta = sessions[-1]["end"] - sessions[0]["start"]
        duration_min = delta.total_seconds() / 60

    result = {
        "func": func_name,
        "attempts": n_attempts,
        "scores": scores,
        "best": best_score,
        "final": final_score,
        "wasted_attempts": wasted_attempts,
        "waste_pct": round(100 * wasted_attempts / max(n_attempts, 1)),
        "compile_fails": compile_fails,
        "reads": len(reads),
        "re_reads": re_reads,
        "edits": len(edits),
        "duration_min": duration_min,
    }

    if verbose:
        print(f"\n{'='*60}")
        print(f"Function: {func_name}")
        print(f"  Attempts:        {n_attempts}")
        print(f"  Score trail:     {' → '.join(str(s) for s in scores)}")
        print(f"  Best score:      {best_score}  (final: {final_score})")
        print(f"  Wasted attempts: {wasted_attempts}/{n_attempts} ({result['waste_pct']}%) — no improvement")
        print(f"  Compile fails:   {compile_fails}")
        print(f"  File reads:      {len(reads)}")
        if re_reads:
            print(f"  Re-reads (waste):")
            for f, c in sorted(re_reads.items(), key=lambda x: -x[1]):
                print(f"    {c}x  {f}")
        print(f"  Edits logged:    {len(edits)}")
        if duration_min is not None:
            print(f"  Duration:        {duration_min:.1f} min")

        # Score stall detection
        stall_start = None
        max_stall = 0
        cur_stall = 0
        for d in score_deltas:
            if d >= 0:
                cur_stall += 1
                max_stall = max(max_stall, cur_stall)
            else:
                cur_stall = 0
        if max_stall >= 3:
            print(f"  ⚠ Longest stall: {max_stall} attempts with no improvement")

    return result


def summary_table(results):
    results = [r for r in results if r]
    if not results:
        print("No session logs found.")
        return

    results.sort(key=lambda r: r["attempts"], reverse=True)
    print(f"\n{'Function':<30} {'Attempts':>8} {'Best':>6} {'Final':>6} {'Waste%':>7} {'Fails':>6} {'Re-reads':>9}")
    print("-" * 80)
    for r in results:
        rr = len(r["re_reads"])
        print(f"{r['func']:<30} {r['attempts']:>8} {r['best']:>6} {r['final']:>6} {r['waste_pct']:>6}% {r['compile_fails']:>6} {rr:>9}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("func_name", nargs="?", help="Function name (omit for all)")
    parser.add_argument("--summary", action="store_true", help="Show ranked summary table")
    args = parser.parse_args()

    if args.func_name:
        log = PERMUTER_DIR / args.func_name / "session_log.txt"
        events = parse_log(log)
        if not events:
            print(f"No session log found for {args.func_name}")
            sys.exit(1)
        analyze(args.func_name, events, verbose=True)
    else:
        results = []
        for func_dir in sorted(PERMUTER_DIR.iterdir()):
            if not func_dir.is_dir():
                continue
            log = func_dir / "session_log.txt"
            events = parse_log(log)
            if events:
                r = analyze(func_dir.name, events, verbose=not args.summary)
                results.append(r)

        if args.summary:
            summary_table(results)
        elif not results:
            print("No session logs found in permuter/")


if __name__ == "__main__":
    main()
