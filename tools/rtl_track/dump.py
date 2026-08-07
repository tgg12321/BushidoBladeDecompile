#!/usr/bin/env python3
"""Dump EVERY RTL pass of GCC 2.7.2 for one translation unit.

`cc1 -da` writes one file per optimization pass (`in.i.rtl`, `in.i.jump`,
`in.i.cse`, ... `in.i.dbr`) next to its input, so this runs cc1 with the work
dir as cwd and a fixed input name. With the full pass series on disk, `find.py`
tracks where an instruction class appears or disappears and `region.py` prints
the surrounding RTL — that is how a "GCC ate my instruction" question gets an
answer with a pass name attached instead of a guess.

The compiler flags come from `engine.buildconfig` (the same values the oracle
build uses, GP / no-strength-reduce variants included) so the dumped RTL is the
RTL of the real build, and the flags can never drift from the Makefile mirror.

Usage (WSL, venv active, from the repo root):
  python3 tools/rtl_track/dump.py <tag> --stem <stem>      # cpp src/<stem>.c first
  python3 tools/rtl_track/dump.py <tag> --input <file.i>   # use an existing .i

Dumps land in tmp/rtl/<tag>/ (wiped on each run) and the pass files are listed
on stdout.

Example:
  python3 tools/rtl_track/dump.py cfg --stem config
  python3 tools/rtl_track/find.py cfg func_8003F388 'mult'
"""
from __future__ import annotations

import argparse
import shlex
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(ROOT))

from engine import buildconfig as cfg  # noqa: E402


def cc_flags_for(stem: str | None) -> str:
    """The cc1 flags the real build uses for this stem (mirrors the per-stem
    selection in engine.pipeline.c_pipeline_cmd). Unknown stem -> the defaults."""
    if stem is None:
        return cfg.CC_FLAGS
    flags = cfg.CC_FLAGS_GP if stem in cfg.GP_FILES else cfg.CC_FLAGS
    if stem in cfg.NO_SR_FILES:
        flags += " -fno-strength-reduce"
    return flags


def preprocess(stem: str, out_i: Path) -> None:
    cmd = "%s %s %s src/%s.c" % (cfg.CPP, cfg.CPP_FLAGS, cfg.CPP_DEFS, stem)
    result = subprocess.run(["bash", "-c", cmd], cwd=ROOT,
                            capture_output=True, text=True)
    if not result.stdout:
        raise SystemExit("cpp produced nothing for src/%s.c\n%s"
                         % (stem, result.stderr[-2000:]))
    out_i.write_text(result.stdout)


def main() -> int:
    ap = argparse.ArgumentParser(description="Dump all GCC 2.7.2 RTL passes.")
    ap.add_argument("tag", help="label for the dump dir tmp/rtl/<tag>")
    ap.add_argument("--stem", help="preprocess src/<stem>.c as the input")
    ap.add_argument("--input", help="use an existing preprocessed .i file")
    args = ap.parse_args()

    if bool(args.stem) == bool(args.input):
        ap.error("give exactly one of --stem or --input")

    out = ROOT / "tmp" / "rtl" / args.tag
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True)

    in_i = out / "in.i"
    if args.stem:
        preprocess(args.stem, in_i)
    else:
        shutil.copyfile(args.input, in_i)

    # -da makes cc1 write <input>.<pass> per pass; cwd=out keeps them together,
    # so cc1 needs an absolute path (quoted — the repo path contains spaces).
    cmd = "%s %s -da in.i -o in.s" % (shlex.quote(str(ROOT / cfg.CC1)),
                                      cc_flags_for(args.stem))
    # cc1 (GCC 2.7.2 SN) exits non-zero on several m2c-decompiled files yet still
    # emits correct output, so its exit code is not a failure signal here either.
    subprocess.run(["bash", "-c", cmd], cwd=out, capture_output=True, text=True)

    dumps = sorted(p.name for p in out.iterdir())
    if len(dumps) <= 1:
        print("NO DUMPS PRODUCED — check the input compiles")
        return 1
    for name in dumps:
        print(name)
    return 0


if __name__ == "__main__":
    sys.exit(main())
