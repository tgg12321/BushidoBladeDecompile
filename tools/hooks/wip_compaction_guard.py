#!/usr/bin/env python3
"""commit-msg guard: WIP checkpoints must stay CURRENT-STATE docs.

Why (token-usage audit 2026-06-12): the sys_VSync WIP grew to a 300+-line
notes.md of stacked per-session TL;DRs and an unbounded meta.json sessions[]
ledger. Every subsequent session READS the whole thing, so per-session read
cost grows monotonically with session count — exactly the multi-session
functions that need checkpoints most pay the most. The history is already in
git (`git log -p memory/wip/<func>/`); the files themselves must describe the
CURRENT state only.

Enforced caps (on the STAGED content of any memory/wip/** file in the commit):
  - notes.md           <= MAX_NOTES_LINES lines
  - meta.json          parses as JSON; sessions[] has <= MAX_SESSIONS entries
                       (fold older entries into a one-line-each
                       `prior_sessions_summary` list instead)

Override: include `[skip-wip-compaction]` in the commit message body
(for genuinely exceptional checkpoints — say why in the commit body).

Contract details: memory/wip/README.md §"Compaction contract".
"""
from __future__ import annotations

import json
import subprocess
import sys

MAX_NOTES_LINES = 120
MAX_SESSIONS = 3

OVERRIDE_TAG = "[skip-wip-compaction]"


def staged_files() -> list[str]:
    out = subprocess.run(
        ["git", "diff", "--cached", "--name-only", "--diff-filter=ACM"],
        capture_output=True, text=True,
    ).stdout
    return [l.strip() for l in out.splitlines() if l.strip()]


def staged_content(path: str) -> str:
    return subprocess.run(
        ["git", "show", f":{path}"], capture_output=True, text=True,
    ).stdout


def main() -> int:
    if len(sys.argv) > 1:
        try:
            with open(sys.argv[1], encoding="utf-8", errors="replace") as f:
                if OVERRIDE_TAG in f.read():
                    print(f"wip_compaction_guard: ALLOW — {OVERRIDE_TAG} override present")
                    return 0
        except OSError:
            pass

    problems: list[str] = []
    for path in staged_files():
        p = path.replace("\\", "/")
        if not p.startswith("memory/wip/"):
            continue
        name = p.rsplit("/", 1)[-1]
        if name == "notes.md":
            n = len(staged_content(p).splitlines())
            if n > MAX_NOTES_LINES:
                problems.append(
                    f"{p}: {n} lines (cap {MAX_NOTES_LINES}). notes.md is a "
                    f"CURRENT-STATE doc — replace stacked per-session TL;DRs "
                    f"with ONE current TL;DR; history lives in git "
                    f"(`git log -p {p}`)."
                )
        elif name == "meta.json":
            raw = staged_content(p)
            try:
                meta = json.loads(raw)
            except json.JSONDecodeError as e:
                problems.append(f"{p}: not valid JSON ({e})")
                continue
            sessions = meta.get("sessions") or []
            if len(sessions) > MAX_SESSIONS:
                problems.append(
                    f"{p}: sessions[] has {len(sessions)} entries (cap "
                    f"{MAX_SESSIONS}). Keep the last {MAX_SESSIONS} verbatim; "
                    f"fold older entries into `prior_sessions_summary` "
                    f"(one line each — date, lever, floor delta, outcome)."
                )

    if problems:
        print("wip_compaction_guard: BLOCK — WIP checkpoint files must stay "
              "current-state docs (memory/wip/README.md §Compaction contract):")
        for pr in problems:
            print(f"  - {pr}")
        print(f"  Override (exceptional, justify in body): add {OVERRIDE_TAG} "
              f"to the commit message.")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
