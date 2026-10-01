"""fsb.py <full_text1b.c> [func ...]: score functions of a whole modified copy of src/text1b.c (rules empty,
cheat-asm not stripped - text1b has none outside canonical islands) against build/src/text1b.o.
Run from the repo root inside WSL with the venv active."""
import sys, json
sys.path.insert(0, ".")
from engine import cheats, pipeline, score
src = sys.argv[1]
funcs = sys.argv[2:] or ["func_80048FFC"]
ov = cheats.empty_overrides("tmp/f48ffc/cfg")
ov["src_override"] = src
out = "tmp/f48ffc/text1b.o"
pipeline.build_c_object("text1b", out, cheat_overrides=ov)
for f in funcs:
    try:
        r = score.score_func(out, "build/src/text1b.o", f)
        print(f, json.dumps({k: r[k] for k in ("score", "target_insns", "build_insns") if k in r}))
    except KeyError as e:
        print(f, "MISSING", e)
if "--diff" in sys.argv or True:
    from engine.cli import _print_insn_diff
    for f in funcs:
        try:
            _print_insn_diff(score.insn_diff(out, "build/src/text1b.o", f))
        except Exception as e:
            print("diff failed", e)
