"""Score several functions of a FULL modified copy of src/system.c (never touches src/).

usage: python3 tmp/CD_cw/score_full.py <full-system.c> <func> [<func> ...] [--diff]
Mirrors engine.sandbox.sandbox_score(disable='all', strip_cheat_asm=True).
"""
import sys
from pathlib import Path

sys.path.insert(0, ".")
from engine import cheats, inlineasm, pipeline, score  # noqa: E402

args = [a for a in sys.argv[1:] if not a.startswith("--")]
show_diff = "--diff" in sys.argv
path, funcs = args[0], args[1:]
stem = "system"
wd = Path("tmp/CD_cw/wd")
wd.mkdir(parents=True, exist_ok=True)
text = Path(path).read_text(encoding="utf-8")
ov = cheats.empty_overrides(str(wd / "cfg"))
src_ovr = str(wd / "src" / f"{stem}.c")
n = inlineasm.write_stripped(stem, src_ovr, text)
ov["src_override"] = src_ovr
out_o = str(wd / f"{stem}.o")
pipeline.build_c_object(stem, out_o, cheat_overrides=ov)
print(f"stripped={n}")
# the landing adds `Alarm = 0x800F19B8;` to named_syms.txt; pre-seed it here
score._symtab()["Alarm"] = 0x800F19B8
for f in funcs:
    try:
        r = score.score_func(out_o, f"build/src/{stem}.o", f)
    except KeyError as e:
        print(f"{f}: absent ({e})")
        continue
    print(f"{f}: score={r.get('score')} target={r.get('target_insns')} ours={r.get('build_insns')}")
    if show_diff:
        from engine import cli
        cli._print_insn_diff(score.insn_diff(out_o, f"build/src/{stem}.o", f))
