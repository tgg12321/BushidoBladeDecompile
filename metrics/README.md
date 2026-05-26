# Decomp metrics

Long-term metrics for decomp runs: which function was worked, by what agent
(model), how long, how many tokens, the outcome, archived-log pointers, and
links to the techniques/findings involved.

## Architecture (two layers, firewalled)

```
engine command ──► engine/metrics.py.record_event()  (best-effort, SILENT)
                          │  appends one line
                          ▼
              metrics/events.jsonl   ◄── committed, append-only SOURCE OF TRUTH
                          │
   tools/metrics/sync.py  │  (offline, on demand; also reads session transcripts)
                          ▼
              Postgres  bb2_metrics   ◄── derived, disposable QUERY LAYER
                          │
   tools/metrics/report.py / psql     ◄── ask questions
```

**The firewall.** The engine hot path only ever appends one line to
`events.jsonl`, inside a blanket `try/except` that swallows everything and
prints nothing. It has no Postgres dependency. So a metrics fault — bad disk,
PG down, anything — is invisible to the agent and never perturbs an engine
command's output. Postgres being down is *query staleness*, never data loss:
re-run `sync.py` when it's back and `events.jsonl` replays in full.

The one deliberate surface is **`preflight.py`** — run it at the start of a real
run to be *notified* if PG is unreachable (silent on success). Per project call,
gating a run on metrics being recordable is worth that one upfront check.

## Files

| File | Role |
|---|---|
| `metrics/events.jsonl` | committed, append-only event log (the source of truth) |
| `engine/metrics.py` | the silent recorder the engine hot path calls |
| `tools/metrics/schema.sql` | Postgres DDL (idempotent) |
| `tools/metrics/sync.py` | offline loader: events.jsonl + transcripts → Postgres |
| `tools/metrics/report.py` | canned queries (`--list` for sections) |
| `tools/metrics/preflight.py` | PG-reachability gate (silent on success) |

## Usage

```powershell
# start-of-run gate (Windows side; silent if healthy)
python tools/metrics/preflight.py --status

# refresh the database from the log + transcripts
python tools/metrics/sync.py

# query
python tools/metrics/report.py                 # all sections
python tools/metrics/report.py cost_per_match   # one section
python tools/metrics/report.py --sql "SELECT ..."
```

`sync.py` is fully idempotent — run it as often as you like; it rebuilds the
derived tables from the durable sources each time.

## Schema (Postgres `bb2_metrics`)

- **engine_events** — one row per instrumented engine command (raw ingest).
- **agent_runs** — a session or subagent, rolled up from its transcript:
  model, tokens (input/output/cache), wall-clock, est. cost.
- **attempts** — the headline unit: a function worked by one agent in one
  stretch — start→final score, outcome, attributed tokens/cost, commit.
- **techniques** — dual fingerprint: `slug` (a `.claude/rules` / `memory`
  entry) when promoted, else a `content_hash` of a diff signature.
- **attempt_techniques**, **attempt_logs** — links + transcript-slice pointers.

## Attribution & known v1 heuristics (tunable, not load-bearing)

- **event → agent run**: explicit `session_id` → transcript content-correlation
  (the engine command nearest in time) → innermost active-window run → owning
  session → NULL. Content-correlation makes per-subagent attribution robust even
  when experiment subagents run concurrently.
- **token slicing per attempt**: messages of the attributed run whose timestamps
  fall in the attempt window. Approximate when one agent multitasks functions.
- **technique → attempt linking**: by commit-time proximity to the attempt
  window (±12 h). Coarse until real attempts accumulate; refine in `sync.py`.
- **est_cost_usd**: `PRICES` in `sync.py` are *estimates* — token counts are
  ground truth. Edit the table to match current pricing.

Tighten any of these in `tools/metrics/sync.py`; the raw events and transcripts
are never discarded, so re-syncing applies new logic to all history.
