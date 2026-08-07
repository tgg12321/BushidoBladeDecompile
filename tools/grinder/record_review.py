"""Append one review verdict to metrics/events.jsonl.

Called by tools/grinder/grind.ps1 at every review boundary:

    python tools/grinder/record_review.py <func> <layer> <verdict> [cause]

  <layer>    layer1 | judge | layer2
             layer1 = the pre-Judge cheat-reviewer gate (driver-invoked)
             judge  = the pre-authorized default-FAIL Judge (driver-invoked)
             layer2 = the fresh adversarial reviewer on the MANUAL path
                      (no driver call site — recorded by the operator)
  <verdict>  PASS | FAIL | ESCALATE | NEEDS_USER | (whatever the reviewer emitted)
  <cause>    optional short tag for WHY, so FAIL causes are countable without
             re-reading justifications: construct | evidence | annotation |
             authority | selfvet | banned | scope | ...

Added 2026-08-07 for the review-pipeline audit (owner-approved): the audit had
to mine ledgers and commit prose because no event ever recorded a review
verdict, so the pipeline's own FAIL rate was unmeasurable from metrics/.

Best-effort by the metrics contract (metrics/README.md, engine/metrics.py):
never raises, never prints, exit 0 always. A metrics fault must be invisible to
the driver — a review event is telemetry, never a gate.
"""
from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))

try:  # best-effort telemetry; same guard as tools/grinder/record_usage.py
    from engine.metrics import record_event  # noqa: E402
except Exception:  # pragma: no cover
    def record_event(*_a, **_k):  # type: ignore
        return None


def main() -> None:
    func = sys.argv[1] if len(sys.argv) > 1 else None
    layer = sys.argv[2] if len(sys.argv) > 2 else None
    verdict = sys.argv[3] if len(sys.argv) > 3 else None
    cause = sys.argv[4] if len(sys.argv) > 4 else None
    # `verdict` is one of engine.metrics._NORMALIZED, so it is lifted to a
    # top-level column automatically; layer/cause ride in payload + extra.
    record_event("review", func,
                 {"layer": layer, "verdict": verdict, "cause": cause},
                 extra={"layer": layer, "cause": cause})


if __name__ == "__main__":
    try:
        main()
    except Exception:
        pass  # metrics must never disturb the driver
