#!/usr/bin/env python3
"""SessionStart hook — surface a DOWN metrics Postgres, nothing more.

Design contract (mirrors engine/metrics.py's non-interference rule):
  * SILENT on success — prints nothing, so nothing is added to the session
    context when Postgres is up (the normal case).
  * On failure prints ONE clean line so a decomp run knows its metrics won't be
    queryable until PG is back. Never blocks the session.
  * stdlib only (a socket probe) — NO psycopg import, so this hook can't fail
    for a dependency reason and start spewing. Any error is swallowed.

The hard "refuse to start" gate for batch/experiment runs lives in
tools/metrics/preflight.py (a deeper psycopg check); this hook is the gentle,
always-safe interactive notifier.
"""
import socket
import sys

HOST, PORT = "127.0.0.1", 5432


def main() -> int:
    try:
        try:
            sys.stdin.read()  # drain the hook payload; we don't need it
        except Exception:
            pass
        socket.create_connection((HOST, PORT), timeout=1.0).close()
        # Postgres reachable -> stay silent
    except Exception:
        try:
            print(f"[metrics] Postgres unreachable on {HOST}:{PORT}. Engine events "
                  f"still log to metrics/events.jsonl; run "
                  f"`python tools/metrics/sync.py` once Postgres is back to query them.")
        except Exception:
            pass
    return 0


if __name__ == "__main__":
    sys.exit(main())
