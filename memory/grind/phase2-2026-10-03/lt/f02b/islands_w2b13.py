#!/usr/bin/env python3
# Print every GTE island of func_8002CD58 / func_8002DAD0 / func_8002D780 at BASE and in the working tree, paired
# in order, and check Q115's conditions mechanically: the same number of islands; template text, constraints,
# clobbers and operand order character-identical; only operand expressions differ. Prints each operand old -> new.
# usage (repo root): python3 memory/grind/phase2-2026-10-03/lt/f02b/islands_w2b13.py [BASE]
import re, subprocess, sys

sys.path.insert(0, ".")
from engine import completion, inlineasm  # noqa: E402

BASE = sys.argv[1] if len(sys.argv) > 1 else "e10725c27"
FUNCS = ("func_8002CD58", "func_8002DAD0", "func_8002D780")
P = "src/main/17AFC.c"


def islands(text, func):
    return [text[s:e] for s, e in completion.blocks(text, func)]


def split_operands(isl):
    """-> (skeleton with each operand expression replaced by @N, [expressions])"""
    exprs = []

    def repl(m):
        # m.group(2) starts at the expression; find its balanced end
        return m.group(0)
    out, i = [], 0
    for m in re.finditer(r'"[=+&]*[a-zA-Z]+"\s*\(', isl):
        j = m.end()
        depth, k = 1, j
        while depth:
            depth += {"(": 1, ")": -1}.get(isl[k], 0)
            k += 1
        exprs.append(isl[j:k - 1])
        out.append(isl[i:j] + "@%d" % (len(exprs) - 1))
        i = k - 1
    out.append(isl[i:])
    return "".join(out), exprs


old_text = subprocess.run(["git", "show", "%s:%s" % (BASE, P)], capture_output=True, text=True,
                          encoding="utf-8", check=True).stdout
new_text = open(P, encoding="utf-8").read()
bad = 0
for f in FUNCS:
    a, b = islands(old_text, f), islands(new_text, f)
    print("== %s: %d islands at %s, %d now" % (f, len(a), BASE, len(b)))
    if len(a) != len(b):
        bad += 1
        continue
    for n, (x, y) in enumerate(zip(a, b)):
        sx, ex = split_operands(x)
        sy, ey = split_operands(y)
        same = sx == sy and len(ex) == len(ey)
        bad += not same
        for k, (eo, en) in enumerate(zip(ex, ey)):
            mark = "" if eo == en else "   <- changed"
            print("  island %2d operand %d: %-28s -> %s%s" % (n, k, eo, en, mark))
        if not ex:
            print("  island %2d: no operands" % n)
        if not same:
            print("  island %2d: TEMPLATE / CONSTRAINT / CLOBBER / ORDER DIFFERS" % n)
print("islands check:", "OK" if not bad else "%d FAILURE(S)" % bad)
sys.exit(1 if bad else 0)
