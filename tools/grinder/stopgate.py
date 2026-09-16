#!/usr/bin/env python3
"""Stop-time self-repair gate for grind sessions.

THE WASTE IT CLOSES: the driver validates a session's outcome JSON and checks
its file scope only AFTER the agent's turn has ended (grind.ps1 steps 5 and 6).
At that point the only available verdict is discard — the session's entire body
of work is thrown away and respawned from scratch. Measured over grind.log
2026-09-08..16: 61 of 423 sessions discarded (14%), of which 34 were
outcome-schema failures and 17 were scope violations. Both classes are sessions
that did real work and then mis-shaped the paperwork.

Running the SAME predicates at Stop time turns each of those from a lost
session into a few hundred tokens of in-session repair: the hook blocks the
stop, hands the agent the validator's own message, and the agent fixes it
before its process exits.

CONTRACT: prints a block reason on stdout, or nothing when the stop is fine.
ALWAYS exits 0 — this gate is advisory. The driver's own checks stay
authoritative, so a bug or drift here can only fail to catch something the
driver still catches; it can never let a bad session through.

RETRY CAP: an agent that cannot satisfy the validator must not be trapped in a
block loop, so each session gets at most MAX_BLOCKS stop-blocks; after that the
gate goes quiet and the driver discards as it does today (no worse than the
status quo).
"""
from __future__ import annotations

import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import grindlib  # noqa: E402

MAX_BLOCKS = 3


def _counter_path(root: str, sid: str) -> str:
    return os.path.join(root, "tmp", "grind", f"stopgate.{sid or 'nosid'}.count")


def _blocks_used(root: str, sid: str) -> int:
    try:
        with open(_counter_path(root, sid), encoding="utf-8") as f:
            return int(f.read().strip() or 0)
    except (OSError, ValueError):
        return 0


def _bump(root: str, sid: str) -> None:
    # Read the count BEFORE opening for write: open(..., "w") truncates, so
    # computing the new value inside the with-block reads back an empty file
    # and re-writes 1 forever — the cap would never engage.
    nxt = _blocks_used(root, sid) + 1
    p = _counter_path(root, sid)
    try:
        os.makedirs(os.path.dirname(p), exist_ok=True)
        with open(p, "w", encoding="utf-8", newline="\n") as f:
            f.write(str(nxt))
    except OSError:
        pass


def _outcome_reason(root: str, func: str, outcome_path: str, modality: str) -> str:
    if not outcome_path:
        return ""  # nothing to check against -> fail open
    if not os.path.exists(outcome_path):
        return (
            f"STOP BLOCKED -- you have not written your outcome JSON.\n\n"
            f"A grind session is a ONE-SHOT process. The instant you stop, your\n"
            f"process exits; with no outcome file on disk the driver discards your\n"
            f"ENTIRE session as if it never ran, and every measurement you made is\n"
            f"lost. Write it now, to this exact path:\n\n  {outcome_path}\n")
    try:
        with open(outcome_path, encoding="utf-8") as f:
            o = json.load(f)
    except (OSError, ValueError) as e:
        return (
            f"STOP BLOCKED -- your outcome JSON at\n  {outcome_path}\n"
            f"is not parseable ({e}). The driver discards a session it cannot read.\n"
            f"Rewrite it as valid JSON before ending your turn.\n")
    ok, why = grindlib.validate_outcome(o, modality, root, func or None)
    if ok:
        return ""
    return (
        f"STOP BLOCKED -- your outcome JSON would be REJECTED by the driver:\n\n"
        f"  {why}\n\n"
        f"This is the exact check grind.ps1 runs the moment your turn ends, and a\n"
        f"session that fails it is discarded whole -- all of your measurements,\n"
        f"banked forms and reasoning, gone, and the function respawns from the last\n"
        f"committed ledger. You are still running, so fix it NOW: amend\n"
        f"  {outcome_path}\n"
        f"so it satisfies the message above, then end your turn.\n\n"
        f"Do NOT invent a result to satisfy the schema -- if you genuinely have no\n"
        f"CONFIRMED/KILLED hypothesis with a numeric measurement, report what you\n"
        f"actually measured in the shape the validator asks for.\n")


def _scope_reason(root: str, func: str) -> str:
    try:
        violations = grindlib.scope_violations(root, func)
    except Exception:
        return ""  # fail open
    if not violations:
        return ""
    listed = "\n".join(f"    {v}" for v in violations)
    return (
        f"STOP BLOCKED -- you have edited files outside your allowed surface:\n\n"
        f"{listed}\n\n"
        f"The driver's scope check runs the moment your turn ends and discards the\n"
        f"WHOLE session over this -- your decomp work included. Your surface is your\n"
        f"function's own src file, memory/grind/{func or '<func>'}/, and tmp/.\n\n"
        f"Revert each file above (`git checkout -- <path>`, or delete it if it is\n"
        f"untracked), then end your turn. If one of those edits is genuinely\n"
        f"required for the match, do NOT keep it: report it in your outcome as a\n"
        f"needed pipeline change and let the integration-handoff path grant it.\n")


def main() -> int:
    root = sys.argv[1] if len(sys.argv) > 1 else "."
    func = os.environ.get("GRIND_FUNC", "")
    if not func:
        return 0
    sid = os.environ.get("CLAUDE_SESSION_ID", "")
    if _blocks_used(root, sid) >= MAX_BLOCKS:
        return 0
    reason = _outcome_reason(root, func,
                             os.environ.get("GRIND_OUTCOME_PATH", ""),
                             os.environ.get("GRIND_MODALITY", "")) \
        or _scope_reason(root, func)
    if reason:
        _bump(root, sid)
        sys.stdout.write(reason)
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception:
        sys.exit(0)  # fail open, always
