#!/usr/bin/env python3
"""agent_status.py -- snapshot of all running parallel agents' progress.

Reads tmp/parallel_logs/<func>.log files (written by tools/agent_log.sh
from each worker subagent) and prints a per-agent summary so the
coordinator can drop in at any time and see what's happening.

Run via: bash tools/dc.sh agent-status

Output format (one block per agent):
  [STATUS] func_name        Nm ago   event1 -> event2 -> event3 -> event4
    last: <event details>

STATUS is one of:
  [DONE]  -- last event was 'matched' or details contained MATCHED
  [STUCK] -- last event was 'stuck'
  [STALL] -- no events for >5 min (might need a check)
  [work]  -- actively logging within last 5 min
"""
from __future__ import annotations

import sys
import time
from pathlib import Path

LOG_DIR = Path("tmp/parallel_logs")


def fmt_age(seconds: float) -> str:
    if seconds < 60:
        return f"{int(seconds)}s"
    if seconds < 3600:
        return f"{int(seconds // 60)}m"
    return f"{int(seconds // 3600)}h{int((seconds % 3600) // 60)}m"


def parse_event_line(line: str) -> tuple[str, str, str, str]:
    """Returns (timestamp, worker_id, event, details). Tab-separated."""
    parts = line.rstrip().split("\t", 3)
    while len(parts) < 4:
        parts.append("")
    return parts[0], parts[1], parts[2], parts[3]


def classify_status(last_event: str, last_details: str, age_sec: float) -> str:
    if "matched" in last_event.lower() or "MATCHED" in last_details:
        return "[DONE]"
    if "stuck" in last_event.lower() or "STUCK" in last_details:
        return "[STUCK]"
    if "error" in last_event.lower():
        return "[ERR] "
    if age_sec > 300:  # 5 min
        return "[STALL]"
    return "[work] "


def main() -> int:
    if not LOG_DIR.exists():
        print(f"No log dir {LOG_DIR} -- no parallel run active.")
        return 0
    logs = sorted(LOG_DIR.glob("*.log"))
    if not logs:
        print(f"No logs in {LOG_DIR} -- no parallel run active.")
        return 0

    now = time.time()
    print("=== Parallel run status ===")
    print(f"  ({len(logs)} agent log{'s' if len(logs) != 1 else ''} in {LOG_DIR})")
    print()

    for log in logs:
        func = log.stem
        try:
            mtime = log.stat().st_mtime
        except OSError:
            mtime = now
        age_sec = now - mtime

        try:
            text = log.read_text(encoding="utf-8", errors="replace")
        except OSError:
            print(f"  ?     {func:32s} (could not read log)")
            continue

        lines = [l for l in text.splitlines() if l.strip()]
        if not lines:
            print(f"  ?     {func:32s} {fmt_age(age_sec):>6s} ago  (empty log)")
            continue

        # Recent event chain (last 6, names only)
        chain_events = [parse_event_line(l)[2] for l in lines[-6:]]
        chain = " -> ".join(e for e in chain_events if e)

        # Last event details (full)
        last_ts, last_wid, last_event, last_details = parse_event_line(lines[-1])

        status = classify_status(last_event, last_details, age_sec)
        wid_str = f"({last_wid})" if last_wid and last_wid != "?" else ""

        print(f"  {status} {func:24s} {fmt_age(age_sec):>5s} ago  {wid_str}")
        print(f"         chain: {chain}")
        if last_details:
            details_short = last_details[:120] + "..." if len(last_details) > 120 else last_details
            print(f"         last:  {last_event}  {details_short}")
        else:
            print(f"         last:  {last_event}")
        print()

    return 0


if __name__ == "__main__":
    sys.exit(main())
