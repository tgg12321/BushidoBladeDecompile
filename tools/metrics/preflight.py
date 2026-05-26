"""tools/metrics/preflight.py — the deliberate, upfront metrics gate.

Per the project's call: it's worth being notified if metrics can't be recorded,
so a real run should be gated on Postgres being reachable. This is the ONE
intentional surface — it is silent on success and loud only when something is
actually wrong. (Contrast: the engine hot path is always silent; this gate runs
at the start of a run, not during one.)

    python tools/metrics/preflight.py            # silent + exit 0 if healthy
    python tools/metrics/preflight.py --status   # always print a short status

Exit codes:
    0  Postgres reachable (schema present, last-sync reported)
    1  Postgres unreachable / DSN wrong  -> a real run should NOT start
"""
from __future__ import annotations

import argparse
import sys

try:
    import psycopg
except ImportError:
    sys.exit("METRICS PREFLIGHT: psycopg not installed "
             "(python -m pip install \"psycopg[binary]\").")

DEFAULT_DSN = "host=127.0.0.1 port=5432 user=postgres dbname=bb2_metrics"


def main():
    ap = argparse.ArgumentParser(description="metrics Postgres preflight gate")
    ap.add_argument("--dsn", default=DEFAULT_DSN)
    ap.add_argument("--status", action="store_true",
                    help="print a status line even when healthy")
    a = ap.parse_args()

    try:
        with psycopg.connect(a.dsn, connect_timeout=5) as conn, conn.cursor() as cur:
            cur.execute("SELECT count(*) FROM engine_events")
            n_events = cur.fetchone()[0]
            cur.execute("SELECT count(*) FROM attempts")
            n_attempts = cur.fetchone()[0]
            cur.execute("SELECT value, ts FROM sync_meta WHERE key='last_sync'")
            row = cur.fetchone()
            last = row[1].isoformat(timespec="seconds") if row else "never"
    except Exception as e:
        print("METRICS PREFLIGHT FAILED — Postgres unreachable.\n"
              f"  DSN: {a.dsn}\n"
              f"  error: {e}\n"
              "  The engine event log keeps recording regardless; you just can't\n"
              "  query/sync until Postgres is back. Start it, then re-run sync.py.",
              file=sys.stderr)
        return 1

    if a.status:
        print(f"metrics OK - {n_events} events, {n_attempts} attempts, last sync {last}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
