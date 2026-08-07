#!/usr/bin/env python3
"""Queue-regen regression channel: recompute the whole queue and diff it against
the committed one, WITHOUT mutating engine/queue.json.

WHY THIS IS THE BROADEST CHANNEL WE HAVE
----------------------------------------
`queue regen` recomputes every function's status, verdict and honest pure-C
distance from scratch. Diffing that against the committed queue is therefore a
complete regression report over the WHOLE corpus, not a sample — strictly
broader coverage than spot_check_completed.py, for no extra compute beyond the
regen itself.

It is also the channel that actually caught the 2026-08-07 allowlist-glue
incident: a regen happened to run and somebody diffed its output against HEAD.
This makes that deliberate instead of lucky.

WHAT IT FLAGS (any of these exits non-zero)
-------------------------------------------
  * FUNCTION ADDED   — a function in the recomputed queue that the committed
                       queue does not have. A queue entry means INCOMPLETE, so
                       this is a COMPLETED-C (or canonical) function falling out
                       of its completed state. This is the 2026-08-07 signature.
  * DISTANCE UP      — an existing item whose honest pure-C distance increased.
  * VERDICT CHANGED  — an item whose canonical-gate verdict moved.

Removals and distance DECREASES are progress, reported but never failures.

WHY IT DOES NOT MUTATE
----------------------
engine.queue.generate() ends in save(), so calling it directly rewrites
engine/queue.json — dirt that deadlocks the Grinder's scope check
([[grinder-park-queue-dirt-deadlock]]). This wrapper repoints
`engine.queue.QUEUE_PATH` at a scratch copy for the duration, so generate()
reads its `parked`-preservation input from a COPY of the real queue and writes
its output to the scratch file. The real queue is never opened for writing. The
lock path is derived from QUEUE_PATH, so it moves with it.

PRECONDITION — NOT WHILE THE GRINDER IS LIVE
--------------------------------------------
A regen does one cheat-stripped build per source file (minutes) and reads the
build tree throughout. Running it against a tree the Grinder or a closer is
editing gives a meaningless answer, and a regen racing a `queue done` is exactly
the write race the queue's own fingerprint check exists to catch. This tool
refuses when a driver lock is present or build artifacts are moving, reusing
spot_check_completed.check_no_build_in_flight.

USAGE (WSL, repo root, .venv active)
    python3 tools/spotcheck/queue_regen_diff.py                 # vs the working queue
    python3 tools/spotcheck/queue_regen_diff.py --ref HEAD      # vs the committed queue
    python3 tools/spotcheck/queue_regen_diff.py --json out.json

Exit: 0 clean · 1 regression(s) · 2 preconditions unmet.
"""
from __future__ import annotations

import argparse
import importlib.util
import json
import os
import shutil
import sys
import time
from pathlib import Path

REPO = next(p for p in Path(__file__).resolve().parents if (p / "engine").is_dir())
sys.path.insert(0, str(REPO))
os.chdir(REPO)

from engine import queue as Q  # noqa: E402

_spec = importlib.util.spec_from_file_location(
    "_spotcheck", Path(__file__).resolve().parent / "spot_check_completed.py")
_sc = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_sc)

SCRATCH = "tmp/spotcheck_work/queue_regen.json"


def items_by_func(q: dict) -> dict[str, dict]:
    return {it["func"]: it for it in q.get("items", [])}


def regen_to_scratch(workdir: str) -> dict:
    """Run engine.queue.generate() with QUEUE_PATH repointed at a scratch copy.

    The copy seeds generate()'s `parked`/owner_override preservation with the
    REAL queue's state, so the recomputation is faithful; the save() at the end
    of generate() then lands on the scratch file instead of engine/queue.json.
    """
    scratch = Path(SCRATCH)
    scratch.parent.mkdir(parents=True, exist_ok=True)
    real = Path(Q.QUEUE_PATH)
    if real.exists():
        shutil.copyfile(real, scratch)
    original = Q.QUEUE_PATH
    try:
        Q.QUEUE_PATH = str(scratch)
        return Q.generate(workdir=workdir, preserve=True)
    finally:
        Q.QUEUE_PATH = original
        for stray in (Path(str(scratch) + ".lock"),):
            if stray.exists():
                stray.unlink()


def diff(base: dict, new: dict) -> tuple[list[dict], list[str]]:
    """-> (findings, progress notes)."""
    b, n = items_by_func(base), items_by_func(new)
    findings: list[dict] = []
    progress: list[str] = []

    for func in sorted(set(n) - set(b)):
        it = n[func]
        findings.append({
            "kind": "FUNCTION-ADDED", "func": func,
            "detail": (f"{it['file']}.c: recomputed queue holds it "
                       f"(distance {it.get('distance')}, verdict "
                       f"{it.get('verdict')}, {it.get('rules')} rule(s)) but the "
                       f"baseline does not. A queue entry means INCOMPLETE — a "
                       f"completed function fell out of its completed state. "
                       f"Check `spot_check_completed.py --mode config` FIRST: a "
                       f"dropped detector grant produces exactly this with the C "
                       f"and the build untouched.")})
    for func in sorted(set(b) - set(n)):
        progress.append(f"{func}: left the queue (completed) — not a finding")

    for func in sorted(set(b) & set(n)):
        ob, on = b[func], n[func]
        db, dn = ob.get("distance"), on.get("distance")
        if isinstance(db, int) and isinstance(dn, int) and dn > db:
            findings.append({
                "kind": "DISTANCE-UP", "func": func,
                "detail": f"{on['file']}.c: honest distance {db} -> {dn}"})
        elif isinstance(db, int) and isinstance(dn, int) and dn < db:
            progress.append(f"{func}: distance {db} -> {dn} — not a finding")
        if ob.get("verdict") != on.get("verdict"):
            findings.append({
                "kind": "VERDICT-CHANGED", "func": func,
                "detail": (f"{on['file']}.c: canonical-gate verdict "
                           f"{ob.get('verdict')} -> {on.get('verdict')}")})
    return findings, progress


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--ref", default=None,
                    help="compare against engine/queue.json at this git ref "
                         "(default: the working-tree file)")
    ap.add_argument("--workdir", default="tmp/queue_regen_diff")
    ap.add_argument("--ignore-locks", action="store_true")
    ap.add_argument("--json", metavar="PATH")
    args = ap.parse_args()

    quiet, notes = _sc.check_no_build_in_flight(args.ignore_locks)
    for note in notes:
        print(f"{'  ' if quiet else '  !! '}{note}")
    if not quiet:
        print("\nRefusing to regen against a tree that is being built or driven — "
              "a regen takes minutes and reads the build tree throughout, so the "
              "answer would be meaningless.")
        return 2

    if args.ref:
        text = _sc.git_show(args.ref, Q.QUEUE_PATH)
        if text is None:
            print(f"cannot read {Q.QUEUE_PATH} at {args.ref}")
            return 2
        base, base_label = json.loads(text), args.ref
    else:
        base, base_label = Q.load(), "working tree"

    print(f"baseline: {base_label} ({len(base.get('items', []))} items)")
    print("regenerating (one cheat-stripped build per file; minutes)…")
    t0 = time.time()
    new = regen_to_scratch(args.workdir)
    print(f"recomputed {len(new.get('items', []))} items in {time.time() - t0:.0f}s "
          f"-> {SCRATCH} (engine/queue.json untouched)")

    if new.get("build_failures"):
        print(f"\n!! {len(new['build_failures'])} file(s) failed the honest build; "
              f"their functions could not be scored: "
              f"{json.dumps(new['build_failures'])[:400]}")

    findings, progress = diff(base, new)
    print(f"\n{len(findings)} finding(s), {len(progress)} progress change(s)")
    for p in progress[:20]:
        print(f"  ++ {p}")
    if len(progress) > 20:
        print(f"  ++ … and {len(progress) - 20} more")
    for f in findings:
        print(f"\n  ** {f['kind']} {f['func']}\n     {f['detail']}")

    if args.json:
        Path(args.json).parent.mkdir(parents=True, exist_ok=True)
        Path(args.json).write_text(json.dumps(
            {"baseline": base_label, "findings": findings,
             "progress": progress,
             "build_failures": new.get("build_failures", [])}, indent=2) + "\n")

    if findings:
        print("\nA function ADDED to the queue is the strongest signal here: it "
              "means something that was complete no longer is.")
        return 1
    print("\nOK: the recomputed queue matches the baseline (no additions, no "
          "distance increases, no verdict changes).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
