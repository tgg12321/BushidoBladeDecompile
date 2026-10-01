#!/usr/bin/env python3
"""harness.py <variant-dir> func... : score scratch TUs against build/src/<stem>.o.

<variant-dir> (under tmp/prc/) holds edited copies <stem>.c plus code6cac.h (picked up before include/ because a
quoted #include searches the including file's directory first). Each present <stem>.c is compiled with the
engine's exact per-file recipe (src_override) and every named function is scored with engine.score.score_func
against build/src/<stem>.o (the byte-correct reference the sandbox uses). Function -> stem from src grep."""
import re
import subprocess
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(root))
from engine import pipeline, score  # noqa: E402

vdir = Path(sys.argv[1])
if not vdir.is_absolute():
    vdir = root / vdir
funcs = sys.argv[2:]
built = {}
for c in sorted(vdir.glob("*.c")):
    stem = c.stem
    o = vdir / f"{stem}.o"
    cmd = pipeline.c_pipeline_cmd(stem, str(o.relative_to(root)), {"src_override": str(c.relative_to(root))})
    r = subprocess.run(["bash", "-c", cmd], cwd=root, capture_output=True)
    if r.returncode != 0:
        err = r.stderr.decode("utf-8", "replace")
        lines = [l for l in err.splitlines() if re.search(r"\.c:\d+: |rror", l)][:6]
        print(f"{stem}: BUILD FAILED", *lines, sep="\n  ")
        continue
    built[stem] = o
for f in funcs:
    stem = next((s for s in built if re.search(rf"^(?!extern)[A-Za-z_][^\n;]*\b{f}\s*\([^;\n]*$",(vdir / f"{s}.c").read_bytes().decode("utf-8"), re.M)), None)
    if stem is None:
        print(f"{f}: not in a built variant TU")
        continue
    res = score.score_func(str(built[stem].relative_to(root)), f"build/src/{stem}.o", f)
    print(f"{f:16s} {stem:16s} score {res['score']:4d}  insns {res['build_insns']}/{res['target_insns']}")
