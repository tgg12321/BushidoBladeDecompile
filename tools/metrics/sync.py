"""tools/metrics/sync.py — offline loader: events.jsonl + transcripts -> Postgres.

This is the SIDECAR. The engine never imports it. It runs on the Windows side
(where the Claude Code transcripts live) on demand:

    python tools/metrics/sync.py

It is fully idempotent — derived tables are rebuilt every run from the durable
sources (the committed metrics/events.jsonl + the session transcripts), so you
can run it as often as you like. Postgres being down is the only failure mode,
and it surfaces as a clear connection error (you WANT to know — see preflight).

What it builds:
  engine_events  <- metrics/events.jsonl            (append-only, dedup by line hash)
  agent_runs     <- transcripts (session + subagents) rolled up: model/tokens/time
  attempts       <- engine_events grouped by (func, attributed run, time-gap)
  techniques     <- git commits touching .claude/rules|memory (slug) or regfix/
                    asmfix/src (content-hash)  [dual fingerprint]
  attempt_*      <- links: techniques used and transcript-slice log pointers

Attribution (event -> agent run) prefers, in order: a subagent whose transcript
issued that exact engine command nearest in time (content correlation, robust
even for concurrent experiment subagents); then the innermost run whose active
window contains the event; then the owning session; else NULL.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
import sys
from datetime import datetime, timedelta, timezone
from pathlib import Path

try:
    import psycopg
    from psycopg.types.json import Json
except ImportError:
    sys.exit("psycopg not installed. Run:  python -m pip install \"psycopg[binary]\"")

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_EVENTS = ROOT / "metrics" / "events.jsonl"
DEFAULT_EXPERIMENTS = ROOT / "metrics" / "experiments.jsonl"
DEFAULT_DSN = "host=127.0.0.1 port=5432 user=postgres dbname=bb2_metrics"

# Claude Code mangles the project cwd into its transcript dir name by replacing
# every non-alphanumeric char with '-'.
_MANGLED = re.sub(r"[^A-Za-z0-9]", "-", str(ROOT))
DEFAULT_TRANSCRIPTS = Path.home() / ".claude" / "projects" / _MANGLED

# Rough USD per 1,000,000 tokens. ESTIMATES — edit to match current pricing.
# Token counts in the DB are ground truth; est_cost_usd is a convenience column.
PRICES = {
    "opus":   {"in": 15.0, "out": 75.0, "cache_write": 18.75, "cache_read": 1.50},
    "sonnet": {"in":  3.0, "out": 15.0, "cache_write":  3.75, "cache_read": 0.30},
    "haiku":  {"in":  1.0, "out":  5.0, "cache_write":  1.25, "cache_read": 0.10},
}

ATTEMPT_GAP = timedelta(hours=6)   # events on a func >6h apart = separate attempts
WINDOW_SLACK = timedelta(minutes=2)


def _ts(s):
    if not s:
        return None
    try:
        return datetime.fromisoformat(str(s).replace("Z", "+00:00"))
    except Exception:
        return None


def _price_tier(model):
    m = (model or "").lower()
    for tier in ("opus", "sonnet", "haiku"):
        if tier in m:
            return tier
    return "opus"


def _cost(model, intok, outtok, cr, cc):
    p = PRICES[_price_tier(model)]
    return round((intok * p["in"] + outtok * p["out"]
                  + cr * p["cache_read"] + cc * p["cache_write"]) / 1e6, 6)


# ---------------------------------------------------------------------------
# schema
# ---------------------------------------------------------------------------
def ensure_schema(conn):
    sql = (Path(__file__).parent / "schema.sql").read_text(encoding="utf-8")
    # strip line comments, split on ';' — the DDL is all simple statements
    body = "\n".join(l for l in sql.splitlines() if not l.lstrip().startswith("--"))
    with conn.cursor() as cur:
        for stmt in body.split(";"):
            if stmt.strip():
                cur.execute(stmt)
    conn.commit()


# ---------------------------------------------------------------------------
# engine_events ingest
# ---------------------------------------------------------------------------
def ingest_events(conn, events_path):
    if not events_path.exists():
        print(f"  events log not found: {events_path} (0 events)")
        return
    n_seen = n_new = 0
    with conn.cursor() as cur, events_path.open(encoding="utf-8") as fh:
        for raw in fh:
            raw = raw.strip()
            if not raw:
                continue
            n_seen += 1
            line_hash = hashlib.sha1(raw.encode("utf-8")).hexdigest()
            try:
                e = json.loads(raw)
            except Exception:
                continue
            cur.execute("""
                INSERT INTO engine_events
                  (ts, command, func, file, session_id, git_commit, branch, cwd,
                   pid, ppid, exit_code, score, verdict, sha1, ok, asm_insns,
                   target_insns, build_insns, total_dropped, extra, payload, line_hash)
                VALUES (%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)
                ON CONFLICT (line_hash) DO NOTHING
            """, (
                _ts(e.get("ts")), e.get("command"), e.get("func"), e.get("file"),
                e.get("session_id"), e.get("git_commit"), e.get("branch"), e.get("cwd"),
                e.get("pid"), e.get("ppid"), e.get("exit_code"), e.get("score"),
                e.get("verdict"), e.get("sha1"), e.get("ok"), e.get("asm_insns"),
                e.get("target_insns"), e.get("build_insns"), e.get("total_dropped"),
                Json(e.get("extra")), Json(e.get("payload")), line_hash,
            ))
            n_new += cur.rowcount
    conn.commit()
    print(f"  engine_events: {n_seen} in log, {n_new} new")


# ---------------------------------------------------------------------------
# experiments ingest (authoritative per-run records from experiment.py)
# ---------------------------------------------------------------------------
_EXP_COLS = ("ts", "func", "file", "model", "effort", "session_id", "budget_usd",
             "minutes", "matched", "start_score", "final_score", "full_sha1_match",
             "total_cost_usd", "input_tokens", "output_tokens", "cache_read_tokens",
             "cache_creation_tokens", "num_turns", "duration_s", "wall_s",
             "terminal_reason", "git_commit", "notes")


def ingest_experiments(conn, path):
    if not path.exists():
        print(f"  experiments: none ({path.name} absent)")
        return
    n_seen = n_new = 0
    with conn.cursor() as cur, path.open(encoding="utf-8") as fh:
        for raw in fh:
            raw = raw.strip()
            if not raw:
                continue
            n_seen += 1
            run_key = hashlib.sha1(raw.encode("utf-8")).hexdigest()
            try:
                e = json.loads(raw)
            except Exception:
                continue
            vals = [_ts(e["ts"]) if "ts" in e else None]
            vals += [e.get(c) for c in _EXP_COLS[1:]]
            cur.execute(f"""
                INSERT INTO experiments ({','.join(_EXP_COLS)}, run_key)
                VALUES ({','.join(['%s'] * len(_EXP_COLS))}, %s)
                ON CONFLICT (run_key) DO NOTHING
            """, (*vals, run_key))
            n_new += cur.rowcount
    conn.commit()
    print(f"  experiments: {n_seen} in log, {n_new} new")


def correct_run_costs(conn):
    """Override agent_runs.est_cost_usd with the authoritative experiment cost
    where a run is a measured experiment (run_id == experiment session_id)."""
    with conn.cursor() as cur:
        cur.execute("""
            UPDATE agent_runs r SET est_cost_usd = e.total_cost_usd
            FROM experiments e
            WHERE e.session_id = r.run_id AND e.total_cost_usd IS NOT NULL
        """)
    conn.commit()


# ---------------------------------------------------------------------------
# transcripts -> agent_runs
# ---------------------------------------------------------------------------
def parse_transcript(path, is_subagent, parent_run_id):
    """Roll one .jsonl transcript into a run dict (+ per-message timeline + markers)."""
    run = {
        "run_id": path.stem, "parent_run_id": parent_run_id,
        "is_subagent": is_subagent, "model": None, "branch": None, "cwd": None,
        "started_at": None, "ended_at": None, "api_ms": 0,
        "in": 0, "out": 0, "cr": 0, "cc": 0, "messages": 0,
        "transcript_path": str(path),
        "timeline": [],   # [(ts, in, out, cr, cc)] for attempt-level slicing
        "markers": [],    # [(ts, command_text)] engine.cli invocations for correlation
    }
    try:
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    except Exception:
        return None
    for raw in lines:
        if not raw.strip():
            continue
        try:
            o = json.loads(raw)
        except Exception:
            continue
        ts = _ts(o.get("timestamp"))
        if ts:
            run["started_at"] = ts if run["started_at"] is None else min(run["started_at"], ts)
            run["ended_at"] = ts if run["ended_at"] is None else max(run["ended_at"], ts)
        if o.get("gitBranch"):
            run["branch"] = o["gitBranch"]
        if o.get("cwd"):
            run["cwd"] = o["cwd"]
        msg = o.get("message") or {}
        if o.get("type") == "assistant" and isinstance(msg, dict):
            if msg.get("model"):
                run["model"] = msg["model"]
            u = msg.get("usage") or {}
            it = u.get("input_tokens") or 0
            ot = u.get("output_tokens") or 0
            cr = u.get("cache_read_input_tokens") or 0
            cc = u.get("cache_creation_input_tokens") or 0
            run["in"] += it; run["out"] += ot; run["cr"] += cr; run["cc"] += cc
            run["messages"] += 1
            run["api_ms"] += o.get("durationMs") or 0
            if ts:
                run["timeline"].append((ts, it, ot, cr, cc))
            # engine.cli markers for content correlation
            for c in (msg.get("content") or []):
                if isinstance(c, dict) and c.get("type") == "tool_use":
                    cmd = (c.get("input") or {}).get("command") or ""
                    if "engine.cli" in cmd or "engine.cli" in str(c.get("input")):
                        run["markers"].append((ts, cmd))
    return run


def parse_all_transcripts(tdir):
    runs = []
    if not tdir.exists():
        print(f"  transcripts dir not found: {tdir}")
        return runs
    for path in sorted(tdir.glob("*.jsonl")):
        r = parse_transcript(path, is_subagent=False, parent_run_id=None)
        if r:
            runs.append(r)
        sub = tdir / path.stem / "subagents"
        if sub.is_dir():
            for sp in sorted(sub.glob("*.jsonl")):
                sr = parse_transcript(sp, is_subagent=True, parent_run_id=path.stem)
                if sr:
                    runs.append(sr)
    return runs


def upsert_runs(conn, runs):
    with conn.cursor() as cur:
        for r in runs:
            wall = None
            if r["started_at"] and r["ended_at"]:
                wall = (r["ended_at"] - r["started_at"]).total_seconds()
            cost = _cost(r["model"], r["in"], r["out"], r["cr"], r["cc"])
            cur.execute("""
                INSERT INTO agent_runs
                  (run_id, parent_run_id, is_subagent, model, branch, cwd,
                   started_at, ended_at, wall_clock_s, api_duration_s,
                   input_tokens, output_tokens, cache_read_tokens,
                   cache_creation_tokens, message_count, est_cost_usd, transcript_path)
                VALUES (%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)
            """, (
                r["run_id"], r["parent_run_id"], r["is_subagent"], r["model"],
                r["branch"], r["cwd"], r["started_at"], r["ended_at"], wall,
                round(r["api_ms"] / 1000.0, 3), r["in"], r["out"], r["cr"], r["cc"],
                r["messages"], cost, r["transcript_path"],
            ))
    conn.commit()
    print(f"  agent_runs: {len(runs)} ({sum(1 for r in runs if r['is_subagent'])} subagents)")


# ---------------------------------------------------------------------------
# attribution: event -> run
# ---------------------------------------------------------------------------
def attribute_event(ev, runs):
    """Pick the run that issued this engine event. Returns run dict or None."""
    ts = ev["ts"]
    if ts is None:
        return None
    # 1. explicit session_id stamp (rare, but authoritative when present)
    if ev.get("session_id"):
        for r in runs:
            if r["run_id"] == ev["session_id"]:
                return r
    cmd, func = ev.get("command"), ev.get("func")
    # 2. content correlation: a transcript marker invoking this command+func
    best, best_dt = None, None
    for r in runs:
        for mts, text in r["markers"]:
            if mts is None or not text:
                continue
            if cmd and cmd in text and (not func or func in text):
                dt = abs((mts - ts).total_seconds())
                if best_dt is None or dt < best_dt:
                    best, best_dt = r, dt
    if best is not None and best_dt is not None and best_dt < 1800:
        return best
    # 3. innermost run whose active window contains the event (subagents first)
    contains = [r for r in runs
                if r["started_at"] and r["ended_at"]
                and r["started_at"] - WINDOW_SLACK <= ts <= r["ended_at"] + WINDOW_SLACK]
    if contains:
        contains.sort(key=lambda r: (not r["is_subagent"],
                                     (r["ended_at"] - r["started_at"])))
        return contains[0]
    return None


# ---------------------------------------------------------------------------
# attempts
# ---------------------------------------------------------------------------
def build_attempts(conn, runs):
    runs_by_id = {r["run_id"]: r for r in runs}
    with conn.cursor() as cur:
        cur.execute("""
            SELECT ts, command, func, file, session_id, git_commit, score,
                   verdict, ok, sha1
            FROM engine_events WHERE func IS NOT NULL ORDER BY func, ts
        """)
        rows = cur.fetchall()

    # group by func, split on time gaps / run change
    events = [dict(ts=r[0], command=r[1], func=r[2], file=r[3], session_id=r[4],
                   git_commit=r[5], score=r[6], verdict=r[7], ok=r[8], sha1=r[9])
              for r in rows]
    for e in events:
        run = attribute_event(e, runs)
        e["run_id"] = run["run_id"] if run else None

    groups, cur_group = [], []
    for e in events:
        if cur_group:
            prev = cur_group[-1]
            same = (e["func"] == prev["func"] and e["run_id"] == prev["run_id"]
                    and e["ts"] and prev["ts"] and (e["ts"] - prev["ts"]) <= ATTEMPT_GAP)
            if not same:
                groups.append(cur_group); cur_group = []
        cur_group.append(e)
    if cur_group:
        groups.append(cur_group)

    n = 0
    with conn.cursor() as cur:
        for g in groups:
            func = g[0]["func"]
            run_id = g[0]["run_id"]
            run = runs_by_id.get(run_id)
            started = min(e["ts"] for e in g if e["ts"])
            ended = max(e["ts"] for e in g if e["ts"])
            scores = [e["score"] for e in g if e["score"] is not None]
            start_score = scores[0] if scores else None
            final_score = scores[-1] if scores else None
            file = next((e["file"] for e in g if e["file"]), None)
            verdicts = [e["verdict"] for e in g if e["verdict"]]
            matched = any(e["command"] == "retire" and e["ok"] for e in g) \
                or (final_score == 0)
            commit_sha = next((e["git_commit"] for e in reversed(g)
                               if e["command"] == "retire" and e["ok"]), None) \
                or g[-1]["git_commit"]
            if matched:
                outcome = "matched"
            elif any(v and v.startswith("ASM") for v in verdicts):
                outcome = "canonical-asm"
            elif final_score is not None and final_score > 0:
                outcome = "stalled"
            else:
                outcome = "in-progress"

            # token slice = run messages whose ts falls in the attempt window
            in_tok = out_tok = 0
            model = run["model"] if run else None
            if run:
                for (mts, it, ot, cr, cc) in run["timeline"]:
                    if started - WINDOW_SLACK <= mts <= ended + WINDOW_SLACK:
                        in_tok += it; out_tok += ot
            cost = _cost(model, in_tok, out_tok, 0, 0)
            wall = (ended - started).total_seconds() if started and ended else None

            cur.execute("""
                INSERT INTO attempts
                  (func, file, run_id, model, started_at, ended_at, wall_clock_s,
                   start_score, final_score, outcome, matched, commit_sha,
                   input_tokens, output_tokens, est_cost_usd, n_events)
                VALUES (%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)
                RETURNING attempt_id
            """, (func, file, run_id, model, started, ended, wall,
                  start_score, final_score, outcome, matched, commit_sha,
                  in_tok, out_tok, cost, len(g)))
            attempt_id = cur.fetchone()[0]
            # log pointer: the run transcript slice covering this attempt
            if run:
                cur.execute("""
                    INSERT INTO attempt_logs (attempt_id, kind, transcript_path)
                    VALUES (%s, 'session-slice', %s)
                """, (attempt_id, run["transcript_path"]))
            n += 1
    conn.commit()
    print(f"  attempts: {n}")
    return n


# ---------------------------------------------------------------------------
# techniques (dual fingerprint: slug from rules/memory, content-hash from diffs)
# ---------------------------------------------------------------------------
def _git(args):
    try:
        out = subprocess.run(["git", "-C", str(ROOT)] + args,
                             capture_output=True, text=True, timeout=60)
        return out.stdout
    except Exception:
        return ""


def sync_techniques(conn):
    # walk commit history with changed files; cheap enough for this repo
    log = _git(["log", "--no-merges", "--name-status",
                "--format=__C__%x09%H%x09%cI%x09%s"])
    if not log:
        print("  techniques: git unavailable, skipped")
        return
    commits = []
    cur_commit = None
    for line in log.splitlines():
        if line.startswith("__C__\t"):
            _, sha, cdate, subj = line.split("\t", 3)
            cur_commit = {"sha": sha, "ts": _ts(cdate), "subj": subj, "files": []}
            commits.append(cur_commit)
        elif cur_commit and line.strip():
            parts = line.split("\t")
            if len(parts) >= 2:
                cur_commit["files"].append((parts[0], parts[1]))

    with conn.cursor() as cur:
        # fetch attempts windows for linking
        cur.execute("SELECT attempt_id, started_at, ended_at FROM attempts")
        attempts = cur.fetchall()

        def link(technique_id, ts, relation):
            if ts is None:
                return
            for aid, a0, a1 in attempts:
                if a0 and a1 and a0 - timedelta(hours=12) <= ts <= a1 + timedelta(hours=12):
                    cur.execute("""INSERT INTO attempt_techniques
                        (attempt_id, technique_id, relation) VALUES (%s,%s,%s)
                        ON CONFLICT DO NOTHING""", (aid, technique_id, relation))

        n_slug = n_hash = 0
        for c in commits:
            rule_touched = False
            for status, fpath in c["files"]:
                m = re.search(r"(?:\.claude/rules|memory/.*?)/([a-z0-9-]+)\.md$", fpath)
                if m:
                    rule_touched = True
                    slug = m.group(1)
                    if slug in ("MEMORY",):
                        continue
                    kind = "rule" if ".claude/rules" in fpath else "memory"
                    relation = "discovered" if status.startswith("A") else "applied"
                    cur.execute("""
                        INSERT INTO techniques (slug, kind, title, rule_path,
                                                first_seen_ts, first_seen_commit)
                        VALUES (%s,%s,%s,%s,%s,%s)
                        ON CONFLICT (slug) DO UPDATE SET
                          rule_path = COALESCE(techniques.rule_path, EXCLUDED.rule_path)
                        RETURNING technique_id
                    """, (slug, kind, slug.replace("-", " "), fpath, c["ts"], c["sha"]))
                    tid = cur.fetchone()[0]
                    n_slug += 1
                    link(tid, c["ts"], relation)
            # content-hash technique: a codegen/cheat change not tied to a rule write-up
            if not rule_touched:
                touched = [f for _, f in c["files"]]
                if any(f in ("regfix.txt", "asmfix.txt") or f.startswith("src/")
                       for f in touched):
                    sig = c["subj"] + "|" + "|".join(sorted(Path(f).name for f in touched))
                    chash = hashlib.sha1(sig.encode("utf-8")).hexdigest()[:16]
                    cur.execute("""
                        INSERT INTO techniques (content_hash, kind, title,
                                                first_seen_ts, first_seen_commit)
                        VALUES (%s,'pattern',%s,%s,%s)
                        ON CONFLICT (content_hash) DO NOTHING
                        RETURNING technique_id
                    """, (chash, c["subj"][:120], c["ts"], c["sha"]))
                    row = cur.fetchone()
                    if row:
                        n_hash += 1
                        link(row[0], c["ts"], "applied")
    conn.commit()
    print(f"  techniques: {n_slug} slug-links, {n_hash} new content-hash patterns")


# ---------------------------------------------------------------------------
def main():
    ap = argparse.ArgumentParser(description="sync decomp metrics into Postgres")
    ap.add_argument("--dsn", default=DEFAULT_DSN)
    ap.add_argument("--events", default=str(DEFAULT_EVENTS))
    ap.add_argument("--experiments", default=str(DEFAULT_EXPERIMENTS))
    ap.add_argument("--transcripts", default=str(DEFAULT_TRANSCRIPTS))
    a = ap.parse_args()

    print(f"sync -> {a.dsn}")
    try:
        conn = psycopg.connect(a.dsn, connect_timeout=5)
    except Exception as e:
        sys.exit(f"FATAL: cannot reach Postgres ({e}).\n"
                 f"  Start it or fix the DSN. The event log is intact; rerun sync later.")
    with conn:
        ensure_schema(conn)
        with conn.cursor() as cur:
            cur.execute("TRUNCATE attempt_logs, attempt_techniques, attempts, "
                        "agent_runs, techniques RESTART IDENTITY CASCADE")
        conn.commit()
        ingest_events(conn, Path(a.events))
        ingest_experiments(conn, Path(a.experiments))
        runs = parse_all_transcripts(Path(a.transcripts))
        upsert_runs(conn, runs)
        correct_run_costs(conn)
        build_attempts(conn, runs)
        sync_techniques(conn)
        with conn.cursor() as cur:
            cur.execute("INSERT INTO sync_meta (key, value, ts) VALUES "
                        "('last_sync', %s, now()) ON CONFLICT (key) DO UPDATE "
                        "SET value = EXCLUDED.value, ts = now()",
                        (datetime.now(timezone.utc).isoformat(),))
        conn.commit()
    print("sync complete.")


if __name__ == "__main__":
    main()
