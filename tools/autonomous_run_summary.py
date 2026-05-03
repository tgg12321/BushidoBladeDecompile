#!/usr/bin/env python3
"""Summarize an autonomous run from `tmp/autonomous_run.log`.

Usage:
    python3 tools/autonomous_run_summary.py             # current/last run
    python3 tools/autonomous_run_summary.py --all       # all runs in log
    python3 tools/autonomous_run_summary.py --json      # machine-readable

Or via dc.sh: `bash tools/dc.sh run-summary [--all|--json]`.

What's surfaced:
  - Functions attempted / matched / stuck
  - Total wall-clock runtime + average per function
  - List of retros (commit SHA + one-line description)
  - List of stuck functions (with reason)
  - Number of new tools / recipes created (counted from retro summaries)
  - Token cost: NOT tracked here. Run `/cost` in Claude Code for the
    session total — this tool tracks function-level outcomes, not
    LLM internals.
"""
from __future__ import annotations

import argparse
import datetime as dt
import json
import subprocess
import sys
from pathlib import Path
from statistics import median

ROOT = Path(__file__).resolve().parent.parent
LOG_PATH = ROOT / "tmp" / "autonomous_run.log"


def parse_iso(s: str) -> dt.datetime:
    return dt.datetime.strptime(s, "%Y-%m-%dT%H:%M:%SZ").replace(tzinfo=dt.timezone.utc)


def load_events() -> list[dict]:
    if not LOG_PATH.exists():
        return []
    out = []
    for line in LOG_PATH.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line:
            continue
        try:
            out.append(json.loads(line))
        except json.JSONDecodeError:
            continue
    return out


def split_runs(events: list[dict]) -> list[list[dict]]:
    """Partition events into runs. A run starts at run_start and ends at
    run_end (or end of log)."""
    runs: list[list[dict]] = []
    current: list[dict] = []
    for ev in events:
        if ev.get("event") == "run_start":
            if current:
                runs.append(current)
            current = [ev]
        else:
            current.append(ev)
    if current:
        runs.append(current)
    return runs


def fmt_dur(seconds: int) -> str:
    if seconds < 60:
        return f"{seconds}s"
    m, s = divmod(seconds, 60)
    if m < 60:
        return f"{m}m{s:02d}s"
    h, m = divmod(m, 60)
    return f"{h}h{m:02d}m"


def summarize_run(events: list[dict]) -> dict:
    """Compute stats for one run's events."""
    summary = {
        "start_ts": None,
        "end_ts": None,
        "budget": 0,
        "started_at_commit": None,
        "ended_at_commit": None,
        "functions_attempted": 0,
        "matched": 0,
        "stuck": 0,
        "stuck_list": [],
        "match_durations": [],
        "retros": [],
        "stuck_durations": [],
        "match_list": [],
        "total_duration_sec": 0,
        "first_event_ts": None,
        "last_event_ts": None,
    }
    func_starts: dict[str, str] = {}

    for ev in events:
        ts = ev.get("ts")
        if summary["first_event_ts"] is None:
            summary["first_event_ts"] = ts
        summary["last_event_ts"] = ts

        if ev["event"] == "run_start":
            summary["start_ts"] = ts
            summary["budget"] = ev.get("budget", 0)
            summary["started_at_commit"] = ev.get("head_at_start")
        elif ev["event"] == "run_end":
            summary["end_ts"] = ts
            summary["ended_at_commit"] = ev.get("head_at_end")
        elif ev["event"] == "func_start":
            func_starts[ev["func"]] = ts
        elif ev["event"] == "func_end":
            summary["functions_attempted"] += 1
            duration = ev.get("duration_sec", 0)
            if not duration and ev["func"] in func_starts:
                try:
                    duration = int(
                        (parse_iso(ts) - parse_iso(func_starts[ev["func"]])).total_seconds()
                    )
                except Exception:
                    duration = 0
            if ev["status"] == "MATCHED":
                summary["matched"] += 1
                summary["match_list"].append({
                    "func": ev["func"],
                    "commit": ev.get("commit_sha", ""),
                    "duration_sec": duration,
                    "recipe": ev.get("recipe", ""),
                })
                summary["match_durations"].append(duration)
                if ev.get("retro_sha"):
                    summary["retros"].append({
                        "func": ev["func"],
                        "retro_sha": ev["retro_sha"],
                        "summary": ev.get("retro_summary", ""),
                    })
            elif ev["status"] == "STUCK":
                summary["stuck"] += 1
                summary["stuck_list"].append({
                    "func": ev["func"],
                    "reason": ev.get("stuck_reason", ""),
                    "duration_sec": duration,
                })
                summary["stuck_durations"].append(duration)

    # Total runtime: prefer run_start → run_end; fall back to first/last event.
    if summary["start_ts"] and summary["end_ts"]:
        summary["total_duration_sec"] = int(
            (parse_iso(summary["end_ts"]) - parse_iso(summary["start_ts"])).total_seconds()
        )
    elif summary["first_event_ts"] and summary["last_event_ts"]:
        summary["total_duration_sec"] = int(
            (parse_iso(summary["last_event_ts"]) - parse_iso(summary["first_event_ts"])).total_seconds()
        )

    return summary


def count_new_tools_from_retros(retros: list[dict]) -> int:
    n = 0
    for r in retros:
        s = r.get("summary", "").lower()
        if "tools/" in s or "new tool" in s or "wired as" in s:
            n += 1
    return n


def commits_in_run(start_sha: str | None, end_sha: str | None) -> list[str]:
    """List commits between start and end (exclusive of start)."""
    if not start_sha or not end_sha or start_sha == end_sha:
        return []
    try:
        out = subprocess.check_output(
            ["git", "log", "--format=%h %s", f"{start_sha}..{end_sha}"],
            cwd=str(ROOT), text=True,
        )
        return [line for line in out.splitlines() if line.strip()]
    except subprocess.CalledProcessError:
        return []


def print_run(summary: dict, idx: int = 0, total: int = 1) -> None:
    print(f"=== Autonomous run "
          f"{idx + 1}/{total} "
          f"({summary.get('start_ts', '?')} -> {summary.get('end_ts', 'in-progress')}) ===")
    print()
    print(f"  Budget:          {summary['budget'] or 'open-ended'}")
    print(f"  Attempted:       {summary['functions_attempted']}")
    print(f"  Matched:         {summary['matched']}")
    print(f"  Stuck:           {summary['stuck']}")
    if summary["functions_attempted"] > 0:
        rate = 100 * summary["matched"] / summary["functions_attempted"]
        print(f"  Success rate:    {rate:.1f}%")
    if summary["total_duration_sec"]:
        print(f"  Wall-clock:      {fmt_dur(summary['total_duration_sec'])}")
    if summary["match_durations"]:
        avg = sum(summary["match_durations"]) // len(summary["match_durations"])
        med = int(median(summary["match_durations"]))
        print(f"  Per-match:       avg {fmt_dur(avg)}, median {fmt_dur(med)}, "
              f"max {fmt_dur(max(summary['match_durations']))}")
    n_tools = count_new_tools_from_retros(summary["retros"])
    print(f"  Retros logged:   {len(summary['retros'])} "
          f"(of which {n_tools} created new tools/recipes)")

    if summary["match_list"]:
        print()
        print("  Matched functions:")
        for m in summary["match_list"]:
            recipe = m["recipe"] or "(no recipe)"
            commit = m["commit"][:8] if m["commit"] else "?"
            print(f"    {m['func']:<32s}  {fmt_dur(m['duration_sec']):>8s}  "
                  f"{commit}  {recipe}")

    if summary["retros"]:
        print()
        print("  Retros:")
        for r in summary["retros"]:
            sha = r["retro_sha"][:8] if r["retro_sha"] else "?"
            sumry = r["summary"] or "(no summary)"
            print(f"    {r['func']:<32s}  {sha}  {sumry}")

    if summary["stuck_list"]:
        print()
        print("  Stuck (need user attention):")
        for s in summary["stuck_list"]:
            reason = s["reason"] or "(no reason given)"
            print(f"    {s['func']:<32s}  {reason}")

    # Sanity check via git
    extra_commits = commits_in_run(summary.get("started_at_commit"),
                                   summary.get("ended_at_commit"))
    if extra_commits:
        print()
        print(f"  Commits during run ({len(extra_commits)}):")
        for c in extra_commits[:20]:
            print(f"    {c}")
        if len(extra_commits) > 20:
            print(f"    ... and {len(extra_commits) - 20} more")

    print()
    print("Notes:")
    print("  - Token costs not tracked here; run /cost in Claude Code for session totals.")
    if summary["stuck"]:
        print("  - Stuck functions need user-driven `dc.sh release` (typed-name confirm) "
              "or new tooling before they can come off the active marker.")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--all", action="store_true",
                    help="Show all runs in the log (default: just the last run)")
    ap.add_argument("--json", action="store_true",
                    help="Emit machine-readable JSON instead of human-readable text")
    ap.add_argument("--check-open", action="store_true",
                    help="Quiet exit 0 if no unclosed run; print one-line warning + exit 0 "
                         "if the last run has no run_end event. Used by `dc.sh start`.")
    args = ap.parse_args()

    events = load_events()
    if not events:
        if args.check_open:
            return 0
        print("No autonomous run log found. Did you call `dc.sh run-log run-start ...`?")
        return 0

    runs = split_runs(events)
    summaries = [summarize_run(r) for r in runs]

    if args.check_open:
        if summaries and summaries[-1]["start_ts"] and not summaries[-1]["end_ts"]:
            last = summaries[-1]
            attempted = last["functions_attempted"]
            matched = last["matched"]
            print(f"  WARNING: autonomous run is unclosed "
                  f"(started {last['start_ts']}, "
                  f"{matched}/{attempted} matched, no run_end logged).")
            print(f"           Inspect with `dc.sh run-summary` or close with "
                  f"`dc.sh run-log run-end --note 'closed manually'`.")
        return 0

    if not args.all:
        summaries = summaries[-1:]

    if args.json:
        print(json.dumps(summaries, indent=2))
        return 0

    for i, s in enumerate(summaries):
        print_run(s, i, len(summaries))
        if i < len(summaries) - 1:
            print()
            print("=" * 60)
            print()
    return 0


if __name__ == "__main__":
    sys.exit(main())
