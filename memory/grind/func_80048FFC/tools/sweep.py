"""sweep.py <base.c> <variants.py>: variants.py defines VARIANTS = [(name, [(old, new), ...]), ...]; each is
applied to base.c, spliced (mk.py logic, gpu.h included), built and scored. Prints score per variant."""
import sys, subprocess, json, re
sys.path.insert(0, ".")
from engine import cheats, pipeline, score
base = open(sys.argv[1], encoding="utf-8").read()
ns = {}
exec(open(sys.argv[2], encoding="utf-8").read(), ns)
src = open("src/text1b.c", encoding="utf-8").read()
a = src.index("extern u8 D_800EF848[];\n")
em = 'INCLUDE_ASM("asm/funcs", func_80048FFC);\n'
b = src.index(em) + len(em)
ov = cheats.empty_overrides("tmp/f48ffc/cfg")
for name, reps in ns["VARIANTS"]:
    v = base
    ok = True
    for old, new in reps:
        if old not in v:
            print(name, "PATTERN MISSING:", old[:60]); ok = False; break
        v = v.replace(old, new, 1)
    if not ok:
        continue
    out = src[:a] + v + src[b:]
    out = out.replace('#include "gte.h"\n', '#include "gte.h"\n#include "gpu.h"\n', 1)
    p = f"tmp/f48ffc/sw_{name}.c"
    open(p, "w", encoding="utf-8", newline="\n").write(out)
    open(f"tmp/f48ffc/sw_{name}.var.c", "w", encoding="utf-8", newline="\n").write(v)
    ov["src_override"] = p
    o = f"tmp/f48ffc/sw_{name}.o"
    try:
        pipeline.build_c_object("text1b", o, cheat_overrides=ov)
        r = score.score_func(o, "build/src/text1b.o", "func_80048FFC")
        d = score.insn_diff(o, "build/src/text1b.o", "func_80048FFC")
        print(f"{name:24s} score={r['score']:4} insns={r['build_insns']} hunks={len(d['hunks'])}")
    except Exception as e:
        print(name, "FAIL", str(e)[-400:])
