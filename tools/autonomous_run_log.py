#!/usr/bin/env python3
"""Append-only log for autonomous-mode runs.

The coordinator appends events as it loops; `autonomous_run_summary.py`
reads the log to produce a stats report. Logs are JSON Lines for
easy machine-readability + grep-friendliness.

Usage:
    python3 tools/autonomous_run_log.py run-start --budget 10 [--note '...']
    python3 tools/autonomous_run_log.py func-start <func>
    python3 tools/autonomous_run_log.py func-end <func> <status> [--duration N] [--commit SHA] [--retro SHA] [--recipe "..."] [--retro-summary "..."] [--stuck-reason "..."] [--note "..."]
    python3 tools/autonomous_run_log.py run-end

Or via dc.sh:
    bash tools/dc.sh run-log <subcommand> ...

The log lives at `tmp/autonomous_run.log` (jsonl, gitignored). Every
event is timestamped. `run-start` records the starting commit; the
coordinator can later compute "commits made during this run" by
git-log range.
"""
from __future__ import annotations

import argparse
import datetime as dt
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LOG_PATH = ROOT / "tmp" / "autonomous_run.log"


def now_iso() -> str:
    return dt.datetime.now(dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def head_sha() -> str | None:
    try:
        return subprocess.check_output(
            ["git", "rev-parse", "HEAD"], cwd=str(ROOT), text=True
        ).strip()
    except subprocess.CalledProcessError:
        return None


def append(event: dict) -> None:
    LOG_PATH.parent.mkdir(parents=True, exist_ok=True)
    with LOG_PATH.open("a", encoding="utf-8", newline="\n") as f:
        f.write(json.dumps(event) + "\n")


def cmd_run_start(args) -> int:
    event = {
        "ts": now_iso(),
        "event": "run_start",
        "budget": args.budget,
        "head_at_start": head_sha(),
        "note": args.note or "",
    }
    append(event)
    print(f"run_start logged (budget={args.budget})")
    return 0


def cmd_func_start(args) -> int:
    event = {
        "ts": now_iso(),
        "event": "func_start",
        "func": args.func,
    }
    append(event)
    return 0


def cmd_func_end(args) -> int:
    event = {
        "ts": now_iso(),
        "event": "func_end",
        "func": args.func,
        "status": args.status,  # MATCHED | STUCK
        "duration_sec": args.duration,
        "commit_sha": args.commit or "",
        "retro_sha": args.retro or "",
        "recipe": args.recipe or "",
        "retro_summary": args.retro_summary or "",
        "stuck_reason": args.stuck_reason or "",
        "note": args.note or "",
    }
    append(event)
    return 0


def cmd_run_end(args) -> int:
    event = {
        "ts": now_iso(),
        "event": "run_end",
        "head_at_end": head_sha(),
        "note": args.note or "",
    }
    append(event)
    return 0


def main() -> int:
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("run-start")
    p.add_argument("--budget", type=int, default=0,
                   help="Number of functions the coordinator plans to attempt (0 = open-ended)")
    p.add_argument("--note", default="")
    p.set_defaults(fn=cmd_run_start)

    p = sub.add_parser("func-start")
    p.add_argument("func")
    p.set_defaults(fn=cmd_func_start)

    p = sub.add_parser("func-end")
    p.add_argument("func")
    p.add_argument("status", choices=["MATCHED", "STUCK"])
    p.add_argument("--duration", type=int, default=0,
                   help="Wall-clock duration in seconds")
    p.add_argument("--commit", default="",
                   help="SHA of the match commit")
    p.add_argument("--retro", default="",
                   help="SHA of the retro commit (if any)")
    p.add_argument("--recipe", default="",
                   help="One-line recipe summary")
    p.add_argument("--retro-summary", default="",
                   help="One-line summary of what the retro added")
    p.add_argument("--stuck-reason", default="",
                   help="If status=STUCK, brief reason")
    p.add_argument("--note", default="")
    p.set_defaults(fn=cmd_func_end)

    p = sub.add_parser("run-end")
    p.add_argument("--note", default="")
    p.set_defaults(fn=cmd_run_end)

    args = ap.parse_args()
    return args.fn(args)


if __name__ == "__main__":
    sys.exit(main())
