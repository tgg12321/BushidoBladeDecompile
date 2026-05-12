#!/usr/bin/env python3
"""Run an automated attempt at retiring a function, with a hard
time-box. Returns a structured one-line verdict regardless of outcome.

Designed for autonomous-mode coordinators: spawn this on each function,
get a deterministic report in bounded wall-clock, decide whether to
move on or escalate.

Pipeline (each phase has its own internal timeout):
  1. preflight check (~5s)
  2. dc.sh retire <func> (sets up the working state)
  3. dc.sh attempt <func> (smart_match + permute_capped)
  4. dc.sh build-active <func>
  5. dc.sh verify-c <func> (bridge-aware verify)
  6. report

If any phase fails or exceeds the budget, the tool returns a STUCK
verdict with the last useful state captured. The function's marker is
LEFT SET so the parent can decide whether to release, re-attempt with
a different approach, or escalate to the user.

Usage:
    python3 tools/attempt_or_bail.py <func> [--budget-seconds N]
        [--skip-attempt]

Default budget: 1800s (30 min).

Output is ONE LINE on stdout:
    MATCHED <func> sha=<sha> recipe=<one-line>
    STUCK <func> phase=<phase> elapsed=<seconds> last=<short msg>
    OUT_OF_SCOPE <func> reason=<classifier verdict>
"""
from __future__ import annotations

import argparse
import re
import signal
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def run(cmd: list[str], timeout: int) -> tuple[int, str, str]:
    """Run a command, capture stdout/stderr, return (rc, out, err)."""
    try:
        r = subprocess.run(cmd, capture_output=True, text=True,
                           cwd=str(ROOT), timeout=timeout)
        return r.returncode, r.stdout, r.stderr
    except subprocess.TimeoutExpired as e:
        return 124, e.stdout or "", e.stderr or ""


def preflight_class(func: str) -> tuple[str, str]:
    """Run preflight; return (recommendation_string, classification_string)."""
    rc, out, _ = run(["bash", "tools/dc.sh", "preflight", func], timeout=60)
    if rc != 0:
        return "?", "?"
    rec_match = re.search(r"→ (.+)", out)
    cls_match = re.search(r"classification: (\S+)", out)
    return (
        rec_match.group(1).strip() if rec_match else "?",
        cls_match.group(1).strip() if cls_match else "?",
    )


def out_of_scope(rec: str) -> bool:
    return (rec.startswith("out-of-scope") or
            rec.startswith("already decomped"))


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("--budget-seconds", type=int, default=1800,
                    help="Total wall-clock budget (default 1800s = 30 min)")
    ap.add_argument("--skip-attempt", action="store_true",
                    help="Skip the dc.sh attempt phase (smart_match/permute)")
    args = ap.parse_args()

    func = args.func
    budget = args.budget_seconds
    start = time.time()

    def elapsed() -> int:
        return int(time.time() - start)

    def remaining() -> int:
        return max(1, budget - elapsed())

    # Phase 1: preflight
    rec, cls = preflight_class(func)
    if out_of_scope(rec):
        print(f"OUT_OF_SCOPE {func} reason={rec.replace(' ', '_')}")
        return 0
    if "already decomped" in rec:
        print(f"OUT_OF_SCOPE {func} reason=already_decomped")
        return 0

    # Phase 2: retire (sets marker, comments bridge)
    rc, out, err = run(["bash", "tools/dc.sh", "retire", func], timeout=60)
    if rc != 0:
        # `dc.sh retire` refuses if not bridged, etc. Capture the reason.
        last = (out + err).strip().splitlines()[-1] if (out + err).strip() else "retire failed"
        print(f"STUCK {func} phase=retire elapsed={elapsed()} last={last[:80]!r}")
        return 1

    # Phase 3 (optional): attempt smart_match + permute
    if not args.skip_attempt:
        rc, out, err = run(
            ["bash", "tools/dc.sh", "attempt", func],
            timeout=min(remaining(), 600),  # cap attempt at 10 min
        )
        # attempt's verdict is in stdout; MATCHED/NEAR_MISS/HARD/SKIPPED
        # If MATCHED, the C body is in auto_matches/<func>.c; we still
        # need to integrate via inline-replace.
        if "MATCHED" in out:
            # Try to integrate
            rc, out2, err2 = run(
                ["bash", "tools/dc.sh", "inline-replace",
                 func, f"auto_matches/{func}.c"],
                timeout=120,
            )
            if rc != 0:
                last = (out2 + err2).strip().splitlines()[-1] if (out2 + err2).strip() else "inline-replace failed"
                print(f"STUCK {func} phase=inline-replace elapsed={elapsed()} last={last[:80]!r}")
                return 1

    # Phase 4: build-active
    rc, out, err = run(
        ["bash", "tools/dc.sh", "build-active", func, "--no-verify"],
        timeout=min(remaining(), 180),
    )
    if rc != 0:
        last = (out + err).strip().splitlines()[-1] if (out + err).strip() else "build failed"
        print(f"STUCK {func} phase=build elapsed={elapsed()} last={last[:80]!r}")
        return 1

    # Phase 5: verify-c
    rc, out, err = run(
        ["bash", "tools/dc.sh", "verify-c", func],
        timeout=min(remaining(), 60),
    )
    if "MATCH" in out and rc == 0:
        # Commit the match
        rc2, out2, err2 = run(
            ["git", "commit", "-am",
             f"attempt-or-bail: retire {func} (class={cls})"],
            timeout=600,  # hook does clean-rebuild verify
        )
        if rc2 == 0:
            sha_rc, sha_out, _ = run(["git", "rev-parse", "--short", "HEAD"], timeout=10)
            sha = sha_out.strip() if sha_rc == 0 else "?"
            print(f"MATCHED {func} sha={sha} recipe=auto-attempt class={cls} elapsed={elapsed()}s")
            return 0
        last = (out2 + err2).strip().splitlines()[-1] if (out2 + err2).strip() else "commit failed"
        print(f"STUCK {func} phase=commit elapsed={elapsed()} last={last[:80]!r}")
        return 1

    # verify-c reports diffs or refuses
    last = (out + err).strip().splitlines()[-1] if (out + err).strip() else "verify mismatch"
    print(f"STUCK {func} phase=verify-c elapsed={elapsed()} last={last[:80]!r}")
    return 1


if __name__ == "__main__":
    sys.exit(main())
