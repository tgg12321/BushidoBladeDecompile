#!/usr/bin/env python3
"""SessionStart hook — surface the TOP of the consolidated INCOMPLETE-work queue.

Every session opens knowing exactly what to work, with zero triage / no hunting:
the agent works the top active item to COMPLETED-C (or, only if the function is
genuinely hand-coded asm, COMPLETED-INLINE-ASM-CANONICAL), then takes the next.
Reads the committed engine/queue.json directly (NO toolchain / NO build — fast
and always safe). Never blocks the session; any error is swallowed (stdlib only,
mirrors metrics_preflight.py's non-interference contract).
"""
import json
import os
import sys
from pathlib import Path


def main() -> int:
    try:
        try:
            sys.stdin.read()  # drain the hook payload; unused
        except Exception:
            pass
        root = os.environ.get("CLAUDE_PROJECT_DIR") or "."
        qp = Path(root) / "engine" / "queue.json"
        if not qp.exists():
            print("[queue] engine/queue.json missing — build the worklist with "
                  "`python3 -m engine.cli queue regen` (in WSL).")
            return 0
        q = json.loads(qp.read_text())
        items = q.get("items", [])
        active = sum(1 for it in items if it.get("status") == "active")
        top = next((it for it in items if it.get("status") == "active"), None)
        if not top:
            print(f"[queue] no active items ({active} active). Run `queue regen`, or "
                  "remaining work is parked/authorize-only — see `queue status`.")
            return 0
        print(
            f"[queue] WORK THE TOP — {top['func']} ({top['file']}.c) | verdict "
            f"{top['verdict']} | pure-C distance {top['distance']} | "
            f"{top.get('rules', 0)} regfix/asmfix rule(s) | {active} active remaining.\n"
            f"        Loop: canonical {top['func']} -> sandbox {top['func']} --disable all "
            f"(edit src toward 0) -> retire/verify-oracle -> queue done {top['func']} -> "
            f"commit. Finish THIS to COMPLETED-C before taking another. "
            f"(`engine queue next` for the full record.)")
        # Surface a WIP checkpoint if one exists for the top function. The WIP
        # registry (memory/wip/<func>/) carries the prior session's best
        # candidate body + measured floor + next-step hypotheses, so a fresh
        # agent can resume in one read instead of re-deriving levers from the
        # memory notes / git log. See memory/wip/README.md.
        try:
            wip_meta = Path(root) / "memory" / "wip" / top["func"] / "meta.json"
            if wip_meta.is_file():
                w = json.loads(wip_meta.read_text())
                sc = w.get("scores", {}) or {}
                sessions = w.get("sessions", []) or []
                rev = (w.get("reviewer", {}) or {}).get("verdict")
                cand = sc.get("candidate_floor")
                head = sc.get("head_floor")
                latest = sessions[-1].get("lever") if sessions else None
                rev_str = f"reviewer={rev}" if rev else "reviewer=not-invoked"
                lever_str = f" — last lever: {latest}" if latest else ""
                print(
                    f"[queue] WIP CHECKPOINT — candidate floor {cand} "
                    f"(HEAD floor {head}), {len(sessions)} session(s), "
                    f"{rev_str}{lever_str}.\n"
                    f"        Resume: read memory/wip/{top['func']}/meta.json + "
                    f"notes.md; apply candidate.c to src/{top.get('file','')}.c "
                    f"to start from the score-{cand} floor.")
        except Exception:
            pass
    except Exception:
        pass
    return 0


if __name__ == "__main__":
    sys.exit(main())
