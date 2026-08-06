#!/usr/bin/env python3
"""ra_solver.honest_model — extract a function's allocation model from the
CHEAT-STRIPPED source instead of the on-main source.

Why this exists.  `extract.py` compiles `src/<stem>.c` as it stands.  For a
PARKED function that source carries asm pins that already force target's
registers, so the extracted model describes an allocation that is already
correct — the inverse question ("what flips us to target?") is degenerate
against it.  The honest model is the one the cheat-invisible sandbox scores:
pins and cheat-asm stripped, no regfix rules.

This does NOT reimplement extract.py.  It overrides exactly one function,
`run_dumps`, to read the stripped source, redirects the work directory so the
on-main campaign artifacts in tmp/ra_solver_work/ are not clobbered, and then
calls extract.main() unchanged — so the greg/flow/lreg parsing, the ent<->greg
DP alignment, the MD-class detection and the per-pseudo FINDREG preference
harvest all stay single-sourced in extract.py.

Prerequisite: bash tools/ra_solver/mkasm_honest.sh <stem>   (writes the
stripped source to tmp/inverse_work/src/<stem>.c)

Usage (WSL, repo root, venv active):
  python3 tools/ra_solver/honest_model.py <func> <stem> [--out model.json]
"""
import argparse
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))

import extract                                    # noqa: E402

WORK = ROOT / "tmp" / "inverse_work" / "ra"
SRC = ROOT / "tmp" / "inverse_work" / "src"


def run_dumps_stripped(stem: str) -> dict:
    """extract.run_dumps, reading the stripped source.  Kept deliberately
    line-for-line parallel to the original so drift is visible in a diff."""
    WORK.mkdir(parents=True, exist_ok=True)
    src = SRC / f"{stem}.c"
    if not src.exists():
        sys.exit(f"missing {src} — run: bash tools/ra_solver/mkasm_honest.sh {stem}")
    ifile = WORK / f"{stem}.i"
    subprocess.run(f'{extract.CPP} "{src}" > "{ifile}"', shell=True, cwd=ROOT,
                   check=True, stderr=subprocess.DEVNULL)
    env = dict(os.environ, BB2_ALLOC_DEBUG="1")
    r = subprocess.run(
        f'"{extract.CC1}" {extract.CC1_FLAGS} -da "{ifile}" '
        f'-o "{WORK / (stem + ".s")}"',
        shell=True, cwd=ROOT, env=env, capture_output=True, text=True)
    out = {"stderr": r.stderr}
    for ext in ("greg", "flow", "lreg"):
        p = Path(str(ifile) + "." + ext)
        out[ext] = p.read_text(errors="replace") if p.exists() else ""
    out["asm"] = (WORK / (stem + ".s")).read_text(errors="replace")
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("func")
    ap.add_argument("stem")
    ap.add_argument("--out")
    ap.add_argument("--inspect", action="store_true")
    a = ap.parse_args()

    extract.run_dumps = run_dumps_stripped
    extract.WORK = WORK                     # .i reuse for the FINDREG harvest
    WORK.mkdir(parents=True, exist_ok=True)
    out = a.out or str(WORK / f"{a.func}.model.json")
    sys.argv = ["extract", a.func, a.stem, "--out", out] + \
               (["--inspect"] if a.inspect else [])
    return extract.main()


if __name__ == "__main__":
    sys.exit(main() or 0)
