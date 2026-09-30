"""Score a candidate body WITHOUT the cheat-asm strip (for islands whose macros are not yet in
engine/gtemacro.py PINNED). Measurement aid only: landing still needs the PINNED entries so the
real sandbox keeps the units.

usage: python3 tmp/gte/score_nostrip.py <func> <candidate.c> [more candidates...] [--diff]
"""
import sys
from pathlib import Path

sys.path.insert(0, ".")
from engine import cheats, inlineasm, pipeline, score, sandbox  # noqa: E402

args = [a for a in sys.argv[1:] if not a.startswith("--")]
show = "--diff" in sys.argv
func, cands = args[0], args[1:]
stem = sandbox.func_file(func)
for c in cands:
    wd = Path("tmp/gte/wd") / Path(c).stem
    wd.mkdir(parents=True, exist_ok=True)
    base = Path(f"src/{stem}.c").read_text(encoding="utf-8")
    text = inlineasm.substitute_body(base, func, Path(c).read_text(encoding="utf-8"))
    ov = cheats.empty_overrides(str(wd / "cfg"))
    so = wd / f"{stem}.c"
    so.write_bytes(text.encode("utf-8"))
    ov["src_override"] = str(so)
    o = str(wd / f"{stem}.o")
    try:
        pipeline.build_c_object(stem, o, cheat_overrides=ov)
        r = score.score_func(o, f"build/src/{stem}.o", func)
        print(f"{c:50s} score {r['score']:4d}  insns {r['build_insns']}/{r['target_insns']}")
        if show:
            from engine import cli
            cli._print_insn_diff(score.insn_diff(o, f"build/src/{stem}.o", func))
    except Exception as e:  # noqa: BLE001
        print(f"{c:50s} FAILED {str(e).splitlines()[-1][:200]}")
