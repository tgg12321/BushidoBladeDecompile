"""Score a FULL-FILE variant for several functions (engine sandbox, --disable all,
cheat-asm stripped). Usage:
  python3 tmp/laneD/score.py <full_file.c> <func> [<func>...] [--diff]
"""
import sys
from engine import sandbox, inlineasm

args = [a for a in sys.argv[1:] if not a.startswith("--")]
path, funcs = args[0], args[1:]
full = open(path, encoding="utf-8").read()
assert "\r" not in full, "CRLF in candidate"
inlineasm.substitute_body = lambda text, f, body: full
for func in funcs:
    r = sandbox.sandbox_score(func, disable="all", strip_cheat_asm=True,
                              workdir="tmp/laneD/sandbox", candidate=path)
    print(func, r.get("score"), r.get("build_insns"), "/", r.get("target_insns"),
          r.get("error") or "")
    if "--diff" in sys.argv and r.get("scorable") and (r.get("score") or "--hunks" in sys.argv):
        from engine import score
        from engine.cli import _print_insn_diff
        _print_insn_diff(score.insn_diff(r["disabled_o"], f"build/src/{r['file']}.o", func))
