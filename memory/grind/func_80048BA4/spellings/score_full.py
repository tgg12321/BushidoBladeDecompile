"""Score a FULL-FILE variant of src/<stem>.c for <func> through the engine
sandbox (same strip/--disable all path as `engine.cli sandbox --disable all
--candidate`), for spellings that change file-scope declarations and so cannot
be expressed as a body-only candidate. Usage:
  python3 tmp/ffc/score_full.py <func> <full_file.c> [--diff]
"""
import sys
from engine import sandbox, inlineasm

func, path = sys.argv[1], sys.argv[2]
full = open(path, encoding="utf-8").read()
assert "\r" not in full, "CRLF in candidate"
inlineasm.substitute_body = lambda text, f, body: full
r = sandbox.sandbox_score(func, disable="all", strip_cheat_asm=True,
                          workdir="tmp/ffc/sandbox", candidate=path)
print({k: r.get(k) for k in ("func", "score", "target_insns", "build_insns",
                             "cheat_asm_stripped", "candidate", "error")})
if "--diff" in sys.argv and r.get("scorable"):
    from engine import score
    from engine.cli import _print_insn_diff
    _print_insn_diff(score.insn_diff(r["disabled_o"], f"build/src/{r['file']}.o", func))
