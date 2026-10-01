#!/usr/bin/env python3
"""Score a scratch copy of a src/<stem>.c (whole file, any number of edited functions) the way
`sandbox --disable all` does: cheat-asm stripped, empty rule config, against build/src/<stem>.o.
A scratch game.h placed next to the scratch .c wins over include/ for the quoted #include.
usage (WSL, repo root, venv): python3 tmp/j759/harness.py <stem> <scratch.c> <func> [<func>...]"""
import sys
from pathlib import Path
sys.path.insert(0, ".")
from engine import cheats, inlineasm, pipeline, score  # noqa: E402

stem, scratch, funcs = sys.argv[1], sys.argv[2], sys.argv[3:]
wd = Path("tmp/j759/work") / stem
wd.mkdir(parents=True, exist_ok=True)
ov = cheats.empty_overrides(str(wd / "cfg"))
src_ovr = Path(scratch).parent / ("_stripped_" + Path(scratch).name)
inlineasm.write_stripped(stem, str(src_ovr), Path(scratch).read_text(encoding="utf-8"))
ov["src_override"] = str(src_ovr)
out_o = str(wd / f"{stem}.o")
pipeline.build_c_object(stem, out_o, cheat_overrides=ov)
for f in funcs:
    try:
        r = score.score_func(out_o, f"build/src/{stem}.o", f)
        print(f"{f} score={r['score']} build={r.get('build_insns')} target={r.get('target_insns')}")
    except KeyError as e:
        print(f"{f} MISSING ({e})")
