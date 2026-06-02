"""WIP candidate registry — session-survivable checkpoints for INCOMPLETE work.

A WIP entry under memory/wip/<func>/ records a candidate C body that lowers a
function's honest pure-C floor below HEAD's, alongside the measured score, the
technique, the cheat-reviewer verdict, the remaining structural gap, and the
next-step hypotheses. The entry is NOT in the build pipeline (memory/ is
outside cpp/cc1/maspsx); the canonical src/ is unchanged. The WIP is evidence
+ resume-point, not active code.

This module is READ-ONLY. WIP entries are authored by agents (worker writes
candidate.c + meta.json directly). Engine surfaces them via:
- queue_top.py hook (SessionStart) — append a one-line WIP banner if the top
  active item has a WIP entry
- queue next CLI — include wip block in the JSON output
- queue regen — does not touch memory/wip/ (entries are sticky like parks).

The contract + schema are documented in memory/wip/README.md. Failures here
are SILENT (return None / empty list) — never raise into the queue hot path
or the SessionStart hook. Mirrors metrics.py / queue_top.py's non-interference
contract.
"""
from __future__ import annotations

import json
from pathlib import Path

WIP_ROOT = Path("memory/wip")


def _wip_dir(func: str) -> Path:
    return WIP_ROOT / func


def has_wip(func: str) -> bool:
    """True if memory/wip/<func>/meta.json exists and is readable."""
    try:
        return (_wip_dir(func) / "meta.json").is_file()
    except Exception:
        return False


def load_meta(func: str) -> dict | None:
    """Return the parsed meta.json for func, or None if missing/unreadable.

    Never raises — a malformed meta.json silently returns None so the queue
    hot path keeps moving."""
    try:
        p = _wip_dir(func) / "meta.json"
        if not p.is_file():
            return None
        return json.loads(p.read_text())
    except Exception:
        return None


def summary(func: str) -> dict | None:
    """Compact summary for surfacing in queue next / SessionStart hook.

    Returns a dict with the key facts a resuming agent needs at a glance:
      {func, candidate_floor, head_floor, sessions_count, latest_lever,
       reviewer_verdict, notes_path}

    None if no WIP entry exists. Never raises."""
    m = load_meta(func)
    if m is None:
        return None
    try:
        scores = m.get("scores", {}) or {}
        sessions = m.get("sessions", []) or []
        reviewer = m.get("reviewer", {}) or {}
        latest = sessions[-1] if sessions else {}
        notes_path = _wip_dir(func) / "notes.md"
        return {
            "func": m.get("function", func),
            "candidate_floor": scores.get("candidate_floor"),
            "head_floor": scores.get("head_floor"),
            "metric": scores.get("metric"),
            "sessions_count": len(sessions),
            "latest_lever": latest.get("lever"),
            "latest_date": latest.get("date"),
            "reviewer_verdict": reviewer.get("verdict"),
            "rejected_count": len(m.get("rejected_forms", []) or []),
            "notes_path": str(notes_path) if notes_path.is_file() else None,
            "candidate_path": str(_wip_dir(func) / "candidate.c"),
            "meta_path": str(_wip_dir(func) / "meta.json"),
        }
    except Exception:
        return None


def list_all() -> list[dict]:
    """All WIP entries' summaries, sorted by func name. Never raises."""
    try:
        if not WIP_ROOT.is_dir():
            return []
        out = []
        for d in sorted(WIP_ROOT.iterdir()):
            if not d.is_dir():
                continue
            s = summary(d.name)
            if s is not None:
                out.append(s)
        return out
    except Exception:
        return []
