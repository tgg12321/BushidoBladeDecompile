"""Rewrite a banked body's GTE islands as verbatim inline_o.h macro statements.

usage: python tmp/gte/convert.py <body.c> <out.c> <plan>
plan: ';'-separated per-island replacements, in island order; each is a '+'-joined list of
      macro calls, e.g. "gte_ldlzc(dist_sq)+gte_nop()+gte_nop()+gte_stlzc(&sp_tmp);gte_SetRotMatrix(mat)"
      '-' drops the island (its statements were folded into an earlier replacement).
All /* */ comments of the body are removed (they describe the old islands); a short
provenance comment is written above each replaced island.
"""
import re
import sys
from pathlib import Path

sys.path.insert(0, "tmp/gte")
from verbatim import emit, header_lines  # noqa: E402

src, out, plan = sys.argv[1], sys.argv[2], sys.argv[3]
t = Path(src).read_bytes().decode("utf-8")
if t.lstrip().startswith("/*"):
    t = t[t.index("*/") + 2:].lstrip("\n")
t = re.sub(r"[ \t]*/\*.*?\*/[ \t]*\n", "", t, flags=re.S)   # whole-line comments
t = re.sub(r"[ \t]*/\*.*?\*/", "", t, flags=re.S)            # trailing comments
isl = list(re.finditer(r"([ \t]*)__asm__ volatile\s*\(.*?\);[ \t]*\n", t, flags=re.S))
steps = plan.split(";")
assert len(steps) == len(isl), (len(steps), len(isl))
res, pos = [], 0
for m, step in zip(isl, steps):
    res.append(t[pos:m.start()])
    pos = m.end()
    if step == "-":
        continue
    ind = m.group(1)
    calls = [c for c in step.split("+") if c]
    names = []
    body = ""
    for c in calls:
        mm = re.match(r"(gte_\w+)\((.*)\)$", c)
        name, a = mm.group(1), mm.group(2)
        args = [x.strip() for x in a.split(",")] if a.strip() else []
        body += emit(name, args, ind)
        lo, hi = header_lines(name)
        names.append(f"{name} :{lo}-{hi}")
    res.append(f"{ind}/* inline_o.h: " + ", ".join(names) + " */\n" + body)
res.append(t[pos:])
Path(out).write_bytes("".join(res).encode("utf-8"))
print("wrote", out)
