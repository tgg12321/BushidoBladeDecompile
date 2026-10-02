"""Data-model harness: build modified copies of src/<stem>.c (with a modified
include/code6cac.h placed beside them, which cpp finds first) through the faithful
pipeline and compare every function against build/src/<stem>.o.

usage: python3 tmp/func_80020E74/dm.py <dir> <stem> [<stem> ...]
<dir> holds <stem>.c copies and code6cac.h.  Run from repo root inside WSL."""
import sys
sys.path.insert(0, '.')
from pathlib import Path
from engine import pipeline, score

d = Path(sys.argv[1])
bad = 0
for stem in sys.argv[2:]:
    src = d / f"{stem}.c"
    out = d / "obj" / f"{stem}.o"
    pipeline.build_c_object(stem, str(out), cheat_overrides={"src_override": str(src)})
    ref = f"build/src/{stem}.o"
    tref = score._o_func_table(ref)
    tout = score._o_func_table(str(out))
    for fn in sorted(set(tref) | set(tout)):
        if fn not in tref or fn not in tout:
            print(f"{stem}: {fn} only in {'ref' if fn in tref else 'ours'}")
            continue
        a = score.normalized_insns(ref, fn)
        b = score.normalized_insns(str(out), fn)
        if a != b:
            bad += 1
            s = score.score_func(str(out), ref, fn)
            print(f"{stem}: {fn} DIFFERS score={s.get('score')}")
    print(f"{stem}: {len(tout)} funcs compared")
print("TOTAL differing:", bad)
