"""tools/metrics/report.py — canned queries over the bb2_metrics database.

    python tools/metrics/report.py              # print every section
    python tools/metrics/report.py models       # one section by name
    python tools/metrics/report.py --list        # list section names
    python tools/metrics/report.py --sql "..."   # ad-hoc query

Read-only. Run `sync.py` first to refresh the data.
"""
from __future__ import annotations

import argparse
import sys

try:
    import psycopg
except ImportError:
    sys.exit("psycopg not installed. Run:  python -m pip install \"psycopg[binary]\"")

DEFAULT_DSN = "host=127.0.0.1 port=5432 user=postgres dbname=bb2_metrics"

SECTIONS = {
    "models": ("Per-model usage & matches (the cheap-vs-expensive ledger)", """
        SELECT a.model,
               count(*)                                    AS attempts,
               count(*) FILTER (WHERE a.matched)           AS matches,
               round(100.0 * count(*) FILTER (WHERE a.matched)
                     / NULLIF(count(*),0), 1)              AS match_pct,
               round(avg(a.wall_clock_s) FILTER (WHERE a.matched)/60, 1) AS avg_min_per_match,
               sum(a.input_tokens+a.output_tokens)         AS tokens,
               round(sum(a.est_cost_usd)::numeric, 2)      AS est_usd
        FROM attempts a
        GROUP BY a.model ORDER BY matches DESC NULLS LAST
    """),
    "cost_per_match": ("Cost & effort per MATCHED function, by model", """
        SELECT model,
               count(*)                                 AS matches,
               round(avg(input_tokens+output_tokens))   AS avg_tokens,
               round(avg(est_cost_usd)::numeric, 3)     AS avg_usd,
               round(avg(wall_clock_s)/60, 1)           AS avg_minutes
        FROM attempts WHERE matched GROUP BY model ORDER BY avg_usd
    """),
    "outcomes": ("Outcome distribution by model", """
        SELECT model, outcome, count(*) AS n
        FROM attempts GROUP BY model, outcome ORDER BY model, n DESC
    """),
    "functions": ("Per-function attempt history (most-worked first)", """
        SELECT func,
               count(*)                          AS attempts,
               bool_or(matched)                  AS ever_matched,
               min(start_score)                  AS first_score,
               min(final_score)                  AS best_score,
               round(sum(est_cost_usd)::numeric,3) AS total_usd
        FROM attempts GROUP BY func ORDER BY attempts DESC, func LIMIT 40
    """),
    "techniques": ("Most-linked techniques/findings", """
        SELECT COALESCE(t.slug, t.content_hash) AS technique,
               t.kind,
               count(at.attempt_id)             AS used_in_attempts
        FROM techniques t
        LEFT JOIN attempt_techniques at ON at.technique_id = t.technique_id
        GROUP BY technique, t.kind
        HAVING count(at.attempt_id) > 0
        ORDER BY used_in_attempts DESC LIMIT 25
    """),
    "recent": ("20 most recent attempts", """
        SELECT started_at, func, model, outcome, final_score,
               input_tokens+output_tokens AS tokens, round(est_cost_usd::numeric,3) AS usd
        FROM attempts ORDER BY started_at DESC NULLS LAST LIMIT 20
    """),
    "runs": ("Agent runs rolled up by model", """
        SELECT model,
               count(*)                              AS runs,
               count(*) FILTER (WHERE is_subagent)   AS subagents,
               sum(input_tokens)                     AS in_tok,
               sum(output_tokens)                    AS out_tok,
               round(sum(est_cost_usd)::numeric, 2)  AS est_usd
        FROM agent_runs GROUP BY model ORDER BY est_usd DESC NULLS LAST
    """),
}


def _print_table(cur, title):
    cols = [d.name for d in cur.description]
    rows = cur.fetchall()
    widths = [len(c) for c in cols]
    srows = []
    for r in rows:
        cells = ["" if v is None else str(v) for v in r]
        srows.append(cells)
        widths = [max(w, len(c)) for w, c in zip(widths, cells)]
    print(f"\n== {title} ==")
    print("  ".join(c.ljust(w) for c, w in zip(cols, widths)))
    print("  ".join("-" * w for w in widths))
    for cells in srows:
        print("  ".join(c.ljust(w) for c, w in zip(cells, widths)))
    if not srows:
        print("(no rows — run sync.py, or no data yet)")


def main():
    ap = argparse.ArgumentParser(description="report on bb2_metrics")
    ap.add_argument("section", nargs="?", help="one of the named sections (default: all)")
    ap.add_argument("--dsn", default=DEFAULT_DSN)
    ap.add_argument("--list", action="store_true", help="list section names")
    ap.add_argument("--sql", help="run an ad-hoc read-only query")
    a = ap.parse_args()

    if a.list:
        for name, (desc, _) in SECTIONS.items():
            print(f"  {name:16} {desc}")
        return

    with psycopg.connect(a.dsn, connect_timeout=5) as conn, conn.cursor() as cur:
        if a.sql:
            cur.execute(a.sql)
            _print_table(cur, "ad-hoc")
            return
        names = [a.section] if a.section else list(SECTIONS)
        for name in names:
            if name not in SECTIONS:
                sys.exit(f"unknown section '{name}'. --list to see them.")
            desc, sql = SECTIONS[name]
            cur.execute(sql)
            _print_table(cur, desc)


if __name__ == "__main__":
    main()
