"""tools/metrics/rotate.py — move completed months out of metrics/events.jsonl.

events.jsonl is append-only telemetry; uncapped it crossed 90 MB (GitHub hard-rejects
files > 100 MB). This moves every event from a month BEFORE the current UTC month into
metrics/history/events-YYYY-MM.jsonl.gz (appending to an existing file), and leaves
the current month in events.jsonl. Lines are copied byte-for-byte, so sync.py's
line-hash dedup still recognises them. Readers that only need recent events
(grindlib attest_floor reads the tail) are unaffected.

Usage:  python tools/metrics/rotate.py [--dry-run]
Run it at a quiet moment (no engine command mid-append) and commit the result.
"""
from __future__ import annotations

import argparse
import datetime as dt
import gzip
import json
import os
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EVENTS = ROOT / "metrics" / "events.jsonl"
HISTORY = ROOT / "metrics" / "history"


def month_of(raw: bytes) -> str | None:
    try:
        ts = json.loads(raw).get("ts") or ""
    except ValueError:
        return None
    return ts[:7] if len(ts) >= 7 else None


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args()

    current = dt.datetime.now(dt.timezone.utc).strftime("%Y-%m")
    size_before = EVENTS.stat().st_size
    with EVENTS.open("rb") as f:
        lines = f.readlines()
    old: dict[str, list[bytes]] = defaultdict(list)
    keep: list[bytes] = []
    for raw in lines:
        m = month_of(raw)
        # unparseable lines stay in the live log rather than being lost
        if m and m < current:
            old[m].append(raw if raw.endswith(b"\n") else raw + b"\n")
        else:
            keep.append(raw)

    for m in sorted(old):
        print(f"{m}: {len(old[m])} events -> metrics/history/events-{m}.jsonl.gz")
    print(f"keep in events.jsonl: {len(keep)} events (month >= {current})")
    if a.dry_run or not old:
        return 0

    # Write the trimmed live log first, then swap it in only if nothing was
    # appended meanwhile; the history files are written after the swap.
    tmp = EVENTS.with_suffix(".jsonl.tmp")
    with tmp.open("wb") as f:
        f.writelines(keep)
    if EVENTS.stat().st_size != size_before:
        tmp.unlink()
        sys.exit("events.jsonl grew during rotation; nothing changed. Re-run rotate.py.")
    os.replace(tmp, EVENTS)

    HISTORY.mkdir(parents=True, exist_ok=True)
    for m, rows in sorted(old.items()):
        with gzip.open(HISTORY / f"events-{m}.jsonl.gz", "ab") as gz:
            gz.writelines(rows)
    return 0


if __name__ == "__main__":
    sys.exit(main())
