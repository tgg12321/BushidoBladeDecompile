"""engine.metrics — best-effort, silent event capture for the decomp loop.

This is the ONLY metrics code the engine hot path imports. Its contract is
absolute and load-bearing:

  * record_event() NEVER raises, NEVER prints to stdout/stderr, and NEVER
    blocks. Any failure (bad path, disk full, serialization error, missing
    dir) is swallowed. A metrics fault must be invisible to the agent and
    must not perturb the JSON an engine command prints.
  * It has NO third-party dependencies (stdlib only) and does NOT touch
    Postgres. The hot path only appends one line to a local JSONL file —
    `metrics/events.jsonl` — which is the durable, committed source of truth.
    The Postgres analytics layer is populated OFFLINE by tools/metrics/sync.py,
    so Postgres being down is query-staleness, never data loss and never a
    hot-path failure.

Env knobs:
  BB2_METRICS_DISABLE   if set (truthy), record_event is a no-op.
  BB2_METRICS_LOG       override the events-log path (default metrics/events.jsonl).
  CLAUDE_SESSION_ID     stamped onto the event if present (best-effort agent attribution).

The regression suite (engine.test_engine.test_metrics) pins the silence +
swallow-on-failure guarantees, so they can't regress.
"""
from __future__ import annotations

import json
import os
from datetime import datetime, timezone
from pathlib import Path

SCHEMA = 1
_ROOT = Path(__file__).resolve().parents[1]

# result-dict keys we lift into top-level columns for cheap querying; the full
# result is always preserved under `payload` so nothing is lost.
_NORMALIZED = (
    "file", "score", "verdict", "ok", "sha1",
    "asm_insns", "target_insns", "build_insns", "total_dropped",
)


def _git_head(root: Path):
    """(commit, branch) read straight from .git — no subprocess. Best-effort."""
    try:
        head = (root / ".git" / "HEAD").read_text().strip()
        if head.startswith("ref:"):
            ref = head[4:].strip()
            branch = ref.rsplit("/", 1)[-1]
            refpath = root / ".git" / ref
            if refpath.exists():
                return refpath.read_text().strip(), branch
            packed = root / ".git" / "packed-refs"
            if packed.exists():
                for line in packed.read_text().splitlines():
                    if line and line[0] not in "#^" and line.endswith(ref):
                        return line.split()[0], branch
            return None, branch
        return head, None  # detached HEAD
    except Exception:
        return None, None


def record_event(command, func=None, result=None, *, exit_code=None, extra=None):
    """Append one event to the JSONL log. Best-effort: never raises, never prints.

    command  the engine subcommand ("sandbox", "canonical", "retire", ...)
    func     the function worked, or None for whole-build commands
    result   the command's result dict (json-serializable); fully preserved
    extra    optional dict of command-specific context (e.g. {"disable": "all"})
    """
    try:
        if os.environ.get("BB2_METRICS_DISABLE"):
            return
        logp = os.environ.get("BB2_METRICS_LOG")
        path = Path(logp) if logp else _ROOT / "metrics" / "events.jsonl"
        commit, branch = _git_head(_ROOT)
        res = result if isinstance(result, dict) else {}
        rec = {
            "schema": SCHEMA,
            "ts": datetime.now(timezone.utc).isoformat(timespec="milliseconds"),
            "command": command,
            "func": func,
            "session_id": os.environ.get("CLAUDE_SESSION_ID"),
            "git_commit": commit,
            "branch": branch,
            "cwd": os.getcwd(),
            "pid": os.getpid(),
            "ppid": os.getppid() if hasattr(os, "getppid") else None,
            "exit_code": exit_code,
            "extra": extra,
            "payload": result,
        }
        for k in _NORMALIZED:
            rec[k] = res.get(k)
        line = json.dumps(rec, default=str)
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open("a", encoding="utf-8") as f:
            f.write(line + "\n")
    except Exception:
        # The non-negotiable: metrics must never disturb the engine or agent.
        return
