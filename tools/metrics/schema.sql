-- bb2_metrics — analytics layer for the decomp loop.
--
-- This schema is a DERIVED MIRROR. The durable source of truth is the
-- committed, append-only metrics/events.jsonl (engine events) plus the Claude
-- Code session transcripts. tools/metrics/sync.py rebuilds these tables from
-- those sources, so the whole database is disposable/recreatable. Postgres
-- being down is query-staleness, never data loss.
--
-- Apply:  psql -U postgres -d bb2_metrics -f tools/metrics/schema.sql

-- ---------------------------------------------------------------------------
-- engine_events — one row per instrumented engine CLI command (raw ingest)
-- ---------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS engine_events (
    event_id      BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    ts            TIMESTAMPTZ NOT NULL,
    command       TEXT NOT NULL,              -- sandbox|canonical|diagnose|retire|verify-oracle|build
    func          TEXT,                       -- function worked (NULL for whole-build cmds)
    file          TEXT,
    session_id    TEXT,                       -- best-effort (often NULL across the WSL boundary)
    git_commit    TEXT,
    branch        TEXT,
    cwd           TEXT,
    pid           INTEGER,
    ppid          INTEGER,
    exit_code     INTEGER,
    score         INTEGER,                    -- sandbox / diagnose distance
    verdict       TEXT,                       -- canonical / diagnose verdict
    sha1          TEXT,                        -- retire / build resulting SHA1
    ok            BOOLEAN,
    asm_insns     INTEGER,
    target_insns  INTEGER,
    build_insns   INTEGER,
    total_dropped INTEGER,
    extra         JSONB,
    payload       JSONB,                       -- full original result dict
    line_hash     TEXT UNIQUE                  -- sha1 of the source JSONL line (idempotent reload)
);
CREATE INDEX IF NOT EXISTS idx_events_func ON engine_events (func);
CREATE INDEX IF NOT EXISTS idx_events_ts   ON engine_events (ts);
CREATE INDEX IF NOT EXISTS idx_events_cmd  ON engine_events (command);

-- ---------------------------------------------------------------------------
-- agent_runs — a Claude Code session or subagent, rolled up from its transcript
-- ---------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS agent_runs (
    run_id          TEXT PRIMARY KEY,          -- sessionId, or subagent agent-id
    parent_run_id   TEXT,                      -- owning session for a subagent
    is_subagent     BOOLEAN NOT NULL DEFAULT FALSE,
    model           TEXT,                      -- claude-opus-4-7 / -sonnet- / -haiku-
    branch          TEXT,
    cwd             TEXT,
    started_at      TIMESTAMPTZ,
    ended_at        TIMESTAMPTZ,
    wall_clock_s    NUMERIC,
    api_duration_s  NUMERIC,                   -- summed message durationMs (active inference time)
    input_tokens    BIGINT,
    output_tokens   BIGINT,
    cache_read_tokens     BIGINT,
    cache_creation_tokens BIGINT,
    message_count   INTEGER,
    est_cost_usd    NUMERIC,                   -- token counts x model price table
    transcript_path TEXT
);
CREATE INDEX IF NOT EXISTS idx_runs_model ON agent_runs (model);

-- ---------------------------------------------------------------------------
-- attempts — the headline unit: one function worked by one agent in one stretch
-- ---------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS attempts (
    attempt_id    BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    func          TEXT NOT NULL,
    file          TEXT,
    run_id        TEXT REFERENCES agent_runs (run_id) ON DELETE SET NULL,
    model         TEXT,                        -- denormalized for easy grouping
    started_at    TIMESTAMPTZ,
    ended_at      TIMESTAMPTZ,
    wall_clock_s  NUMERIC,
    start_score   INTEGER,
    final_score   INTEGER,
    outcome       TEXT,                        -- matched|canonical-asm|stalled|abandoned|in-progress
    matched       BOOLEAN NOT NULL DEFAULT FALSE,
    commit_sha    TEXT,                        -- commit that landed the match, if any
    input_tokens  BIGINT,                      -- attributed slice of the run's tokens
    output_tokens BIGINT,
    est_cost_usd  NUMERIC,
    n_events      INTEGER,
    notes         TEXT,
    UNIQUE (func, run_id, started_at)
);
CREATE INDEX IF NOT EXISTS idx_attempts_func    ON attempts (func);
CREATE INDEX IF NOT EXISTS idx_attempts_model   ON attempts (model);
CREATE INDEX IF NOT EXISTS idx_attempts_outcome ON attempts (outcome);

-- ---------------------------------------------------------------------------
-- techniques — fingerprinted techniques/findings (dual: slug + content-hash)
-- ---------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS techniques (
    technique_id      BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    slug              TEXT UNIQUE,             -- memory/.claude-rules name: when promoted
    content_hash      TEXT UNIQUE,             -- change-signature hash before promotion
    kind              TEXT,                    -- rule | memory | pattern
    title             TEXT,
    rule_path         TEXT,                    -- .claude/rules/x.md or memory/.../x.md
    first_seen_ts     TIMESTAMPTZ,
    first_seen_commit TEXT,
    description       TEXT
);

CREATE TABLE IF NOT EXISTS attempt_techniques (
    attempt_id   BIGINT REFERENCES attempts (attempt_id) ON DELETE CASCADE,
    technique_id BIGINT REFERENCES techniques (technique_id) ON DELETE CASCADE,
    relation     TEXT,                          -- applied | discovered
    PRIMARY KEY (attempt_id, technique_id)
);

-- ---------------------------------------------------------------------------
-- attempt_logs — archived transcript slices / artifact pointers per attempt
-- ---------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS attempt_logs (
    log_id          BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    attempt_id      BIGINT REFERENCES attempts (attempt_id) ON DELETE CASCADE,
    kind            TEXT,                        -- session-slice | permuter | build
    transcript_path TEXT,
    line_start      INTEGER,
    line_end        INTEGER
);

-- ---------------------------------------------------------------------------
-- sync_meta — bookkeeping for the offline sync
-- ---------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS sync_meta (
    key   TEXT PRIMARY KEY,
    value TEXT,
    ts    TIMESTAMPTZ DEFAULT now()
);
