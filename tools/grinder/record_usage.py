"""Append a grind agent's token usage to metrics/events.jsonl.

Called by tools/grinder/grind.ps1 after every real (non-mock) agent spawn:

    python tools/grinder/record_usage.py <agent_log> <func> <role>

<agent_log> is the per-spawn CLI capture (outcome_*.json.agent.log) — the
Out-String of `claude -p ... --output-format json`, whose result line carries
usage (tokens, turns, duration, cost, error state). That file is OVERWRITTEN on
the next spawn, so this is the only durable per-session usage record; it lands
in metrics/events.jsonl (command "grind-agent-usage") and is swept into the
ledger commit at each session boundary like every other metrics event.

Best-effort by the metrics contract (metrics/README.md): never raises, never
prints, exit 0 always. A malformed/absent log simply records nothing.
"""
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))

try:  # best-effort telemetry; same guard as tools/permuter_campaign.py
    from engine.metrics import record_event  # noqa: E402
except Exception:  # pragma: no cover
    def record_event(*_a, **_k):  # type: ignore
        return None


def _parse_result_blob(raw: str):
    """Extract the outermost JSON object from the CLI capture (which may carry
    stray stderr lines around it) — same first-{ .. last-} approach as the
    driver's own Get-JsonObject."""
    lo, hi = raw.find("{"), raw.rfind("}")
    if lo < 0 or hi <= lo:
        return None
    try:
        return json.loads(raw[lo:hi + 1])
    except Exception:
        return None


def main() -> None:
    log_path = Path(sys.argv[1])
    func = sys.argv[2] if len(sys.argv) > 2 else None
    role = sys.argv[3] if len(sys.argv) > 3 else "session"
    d = _parse_result_blob(log_path.read_text(encoding="utf-8", errors="replace"))
    if not isinstance(d, dict):
        return
    usage = d.get("usage") or {}
    payload = {
        "role": role,
        "models": sorted((d.get("modelUsage") or {}).keys()),
        "agent_session_id": d.get("session_id"),
        "subtype": d.get("subtype"),
        "is_error": d.get("is_error"),
        "terminal_reason": d.get("terminal_reason"),
        "num_turns": d.get("num_turns"),
        "duration_ms": d.get("duration_ms"),
        "duration_api_ms": d.get("duration_api_ms"),
        "total_cost_usd": d.get("total_cost_usd"),
        "input_tokens": usage.get("input_tokens"),
        "output_tokens": usage.get("output_tokens"),
        "cache_read_input_tokens": usage.get("cache_read_input_tokens"),
        "cache_creation_input_tokens": usage.get("cache_creation_input_tokens"),
        "result_head": (d.get("result") or "")[:150],
    }
    record_event("grind-agent-usage", func, payload, extra={"role": role})


if __name__ == "__main__":
    try:
        main()
    except Exception:
        pass  # metrics must never disturb the driver
