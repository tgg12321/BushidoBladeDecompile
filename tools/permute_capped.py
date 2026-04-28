#!/usr/bin/env python3
"""Run decomp-permuter with hard time cap + flat-score early termination.

Wraps tools/decomp-permuter/permuter.py and watches its stdout for score
updates. Kills the run early when:

  - A zero score (match) is reported (`--stop-on-zero` is forwarded).
  - No new best score has been observed for `--max-flat-seconds` seconds.
  - The hard `--max-time` budget is exceeded.

Reports a one-line summary on exit:
    permute_capped: <func> matched=Y/N best=<score> attempts=<N>
                    elapsed=<s>s reason=<match|flat|timeout|error>

Exit codes:
    0 — match found (best == 0)
    1 — no match, terminated (flat / timeout)
    2 — error (permuter dir missing, base.c won't compile, etc.)

This eliminates the "permuter ran 8 attempts at the same flat score" pattern
documented in past session logs, where the model paid token cost for each
attempt's diff with no signal.
"""
from __future__ import annotations

import argparse
import os
import queue
import re
import shutil
import signal
import subprocess
import sys
import threading
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Permuter prints lines like:
#   iteration 42, 12.3s/it, 5 alternatives,  best score: 30
#   [base 1] (base) score 274
# We grab any "score N" or "best score: N" pattern.
SCORE_RE = re.compile(r"(?:best\s+score|score)\s*[:=]?\s*(-?\d+)", re.IGNORECASE)


def find_permuter_dir(arg: str) -> Path | None:
    """Resolve <func> or <permuter/func> to a permuter directory."""
    p = Path(arg)
    if p.is_dir() and (p / "compile.sh").exists():
        return p
    candidate = ROOT / "permuter" / arg
    if candidate.is_dir() and (candidate / "compile.sh").exists():
        return candidate
    return None


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func", help="Function name OR explicit permuter dir path")
    ap.add_argument("--max-time", type=int, default=600,
                    help="Hard time cap in seconds (default: 600)")
    ap.add_argument("--max-flat-seconds", type=int, default=90,
                    help="Kill if no new best within this many seconds (default: 90)")
    ap.add_argument("--threads", "-j", type=int, default=4,
                    help="Permuter parallelism (default: 4)")
    ap.add_argument("--stop-on-zero", action="store_true", default=True,
                    help="Stop as soon as a score-0 match is found (default on)")
    ap.add_argument("--show-output", action="store_true",
                    help="Stream permuter output live in addition to summary")
    ap.add_argument("--no-better-only", action="store_true",
                    help="Don't pass --better-only (more verbose; for debugging)")
    args = ap.parse_args()

    pdir = find_permuter_dir(args.func)
    if pdir is None:
        print(f"ERROR: permuter dir not found for {args.func} "
              f"(expected permuter/{args.func}/ or explicit path)", file=sys.stderr)
        return 2

    permuter = ROOT / "tools" / "decomp-permuter" / "permuter.py"
    if not permuter.exists():
        print(f"ERROR: {permuter} missing", file=sys.stderr)
        return 2

    cmd = [sys.executable, str(permuter), str(pdir),
           "-j", str(args.threads)]
    if args.stop_on_zero:
        cmd.append("--stop-on-zero")
    if not args.no_better_only:
        cmd.append("--better-only")

    started = time.monotonic()
    last_improvement = started
    best_score: int | None = None
    attempts = 0
    matched = False
    reason = "timeout"

    popen_kwargs = dict(
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, bufsize=1, cwd=str(ROOT),
    )
    if os.name == "posix":
        popen_kwargs["start_new_session"] = True

    proc = subprocess.Popen(cmd, **popen_kwargs)

    def kill_proc(sig: int = signal.SIGTERM):
        try:
            if os.name == "posix":
                # Use the new session pgid so we don't signal ourselves.
                os.killpg(proc.pid, sig)
            else:
                proc.terminate()
        except (ProcessLookupError, PermissionError, OSError):
            pass

    # Stream stdout into a queue so the main loop can poll with timeouts
    # and enforce time/flat-score caps even when the permuter is silent.
    out_q: queue.Queue[str | None] = queue.Queue()

    def reader():
        try:
            for line in proc.stdout:
                out_q.put(line)
        finally:
            out_q.put(None)

    threading.Thread(target=reader, daemon=True).start()

    try:
        while True:
            now = time.monotonic()
            elapsed = now - started
            if elapsed > args.max_time:
                reason = "timeout"
                kill_proc()
                break
            flat_for = now - last_improvement
            if best_score is not None and flat_for > args.max_flat_seconds:
                reason = "flat"
                kill_proc()
                break

            try:
                line = out_q.get(timeout=1.0)
            except queue.Empty:
                if proc.poll() is not None:
                    reason = "match" if matched else "exit"
                    break
                continue
            if line is None:
                # EOF
                reason = "match" if matched else "exit"
                break

            if args.show_output:
                sys.stdout.write(line)
                sys.stdout.flush()

            attempts += 1

            for m in SCORE_RE.finditer(line):
                score = int(m.group(1))
                if best_score is None or score < best_score:
                    best_score = score
                    last_improvement = time.monotonic()
                if score == 0:
                    matched = True
                    reason = "match"
                    last_improvement = time.monotonic()

        if proc.poll() is None:
            kill_proc()
            try:
                proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                kill_proc(signal.SIGKILL)
    finally:
        if proc.poll() is None:
            kill_proc(signal.SIGKILL)

    elapsed = int(time.monotonic() - started)
    best_str = "?" if best_score is None else str(best_score)
    print(f"permute_capped: {pdir.name} matched={'Y' if matched else 'N'} "
          f"best={best_str} attempts={attempts} elapsed={elapsed}s "
          f"reason={reason}")
    return 0 if matched else 1


if __name__ == "__main__":
    sys.exit(main())
