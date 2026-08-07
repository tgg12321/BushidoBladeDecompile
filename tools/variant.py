#!/usr/bin/env python3
"""Honest-pipeline candidate harness — compile an arbitrary candidate .c through
the full per-file build pipeline with EVERY cheat stage disabled, then score one
function against the byte-correct build reference.

This is the fast inner loop for pure-C grinding: it never writes to `src/`,
`build/`, or any pipeline config, so a candidate can be measured without
touching a tracked file and without disturbing a concurrent build. The cheat
stages (regfix, regfix_stage2, asmfix, prologue_fix) are neutralized via
`engine.cheats.empty_overrides`, so the reported score IS the honest pure-C
distance — the same number `engine sandbox <func> --disable all` reports for the
committed source.

Requires `build/src/<stem>.o` to already exist (the canonical reference); it is
only READ, never rebuilt.

Usage (WSL, venv active, from the repo root):
  python3 tools/variant.py <stem> <func> <candidate.c> [--tag NAME] [--diff]

  stem          src/<stem>.c the candidate replaces (selects the pipeline flags)
  func          the function to score
  candidate.c   path to the candidate translation unit
  --tag         label for the work dir tmp/variants/<func>/<tag> (default: "v")
  --diff        also print a unified diff of the normalized instruction streams

Example:
  python3 tools/variant.py config func_8003F388 tmp/cand/config_a.c --tag a --diff
"""
from __future__ import annotations

import argparse
import difflib
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

from engine import cheats, pipeline, score  # noqa: E402


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Score a candidate .c through the cheat-free pipeline.")
    ap.add_argument("stem", help="src/<stem>.c the candidate stands in for")
    ap.add_argument("func", help="function to score")
    ap.add_argument("candidate", help="path to the candidate .c")
    ap.add_argument("--tag", default="v", help="work-dir label (default: v)")
    ap.add_argument("--diff", action="store_true",
                    help="print a unified diff of the instruction streams")
    args = ap.parse_args()

    # The pipeline embeds these paths in a shell command and the engine's objdump
    # helpers assume the repo root — so work in repo-relative paths from ROOT.
    # (The repo path contains spaces; absolute paths would need quoting at every
    # stage boundary.)
    os.chdir(ROOT)
    work = Path("tmp") / "variants" / args.func / args.tag
    work.mkdir(parents=True, exist_ok=True)

    overrides = cheats.empty_overrides(str(work / "cfg"))
    overrides["src_override"] = args.candidate

    out_o = str(work / (args.stem + ".o"))
    cmd = pipeline.c_pipeline_cmd(args.stem, out_o, overrides)
    result = subprocess.run(["bash", "-c", cmd], capture_output=True, text=True)
    if not Path(out_o).exists():
        print("BUILD FAILED (as produced no object)")
        print(result.stderr[-3000:])
        return 2

    ref = "build/src/%s.o" % args.stem
    if not Path(ref).exists():
        print("MISSING REFERENCE: %s (build the tree first)" % ref)
        return 3

    res = score.score_func(out_o, ref, args.func)
    print("tag=%-18s score=%-4s ours=%s target=%s"
          % (args.tag, res["score"], res["build_insns"], res["target_insns"]))

    if args.diff:
        ours = score.normalized_insns(out_o, args.func)
        target = score.normalized_insns(ref, args.func)
        for line in difflib.unified_diff(ours, target, "ours", "target",
                                         lineterm="", n=4):
            print(line)
    return 0


if __name__ == "__main__":
    sys.exit(main())
