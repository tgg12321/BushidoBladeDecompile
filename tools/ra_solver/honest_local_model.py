#!/usr/bin/env python3
"""ra_solver.honest_local_model — local-alloc (block_alloc) quantity dump from
the CHEAT-STRIPPED source.

Same rationale as honest_model.py, one level down: local_extract.py compiles
`src/<stem>.c`, which for a parked function carries the asm pins that already
force target's registers.  The residual only exists in the pin-free build.

Overrides local_extract.run (the one function that reads the source) and
redirects WORK, then calls local_extract.main() unchanged, so the QTYDBG
segmentation — the subtle part, where rows are attributed to the function named
by the NEXT ALLOCDBG banner because local_alloc runs before global_alloc — stays
single-sourced there.

Prerequisite: bash tools/ra_solver/mkasm_honest.sh <stem>

Usage (WSL, repo root, venv active):
  python3 tools/ra_solver/honest_local_model.py <stem> [--func NAME]
  -> tmp/inverse_work/ra/<stem>.local.json
"""
import argparse
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))

import local_extract                               # noqa: E402

WORK = ROOT / "tmp" / "inverse_work" / "ra"
SRC = ROOT / "tmp" / "inverse_work" / "src"


def run_stripped(stem):
    WORK.mkdir(parents=True, exist_ok=True)
    src = SRC / f"{stem}.c"
    if not src.exists():
        sys.exit(f"missing {src} — run: bash tools/ra_solver/mkasm_honest.sh {stem}")
    ifile = WORK / f"{stem}.i"
    subprocess.run(f'{local_extract.CPP} "{src}" > "{ifile}"', shell=True,
                   cwd=ROOT, check=True, stderr=subprocess.DEVNULL)
    env = dict(os.environ, BB2_QTY_DEBUG="1", BB2_ALLOC_DEBUG="1")
    r = subprocess.run(
        f'"{local_extract.CC1}" {local_extract.CC1_FLAGS} -da "{ifile}" '
        f'-o "{WORK / (stem + ".s")}"',
        shell=True, cwd=ROOT, env=env, capture_output=True, text=True)
    return r.stderr


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("stem")
    ap.add_argument("--func")
    a = ap.parse_args()
    local_extract.run = run_stripped
    local_extract.WORK = WORK
    WORK.mkdir(parents=True, exist_ok=True)
    sys.argv = ["local_extract", a.stem] + (["--func", a.func] if a.func else [])
    return local_extract.main()


if __name__ == "__main__":
    sys.exit(main() or 0)
