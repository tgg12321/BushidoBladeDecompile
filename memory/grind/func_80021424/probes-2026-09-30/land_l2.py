#!/usr/bin/env python3
"""land_l2.py : L2 on top of L1 (landing lock only; L1_ROOT=<dir> for a scratch tree).

Bodies tmp/prc/<func>.<L2BODY or l2r1>.c for func_8001C8DC, func_8001CE60, func_8001E404, func_8001EFA0,
func_8001E878, func_8001EA84, func_8001FB34 (retyped to PracticeMenuRec *); func_8001FBE8 passes rec to
func_8001FB34 without the cast. New PracticeMenuRec members: u8 unk_B3 (sb func_8001EA84 0x8001ECF8),
s16 unk_26C (lh func_8001FB34). Extern/row retirement is decided by the census at landing, not here."""
import os
import re
from pathlib import Path

srcroot = Path(__file__).resolve().parents[2]
root = Path(os.environ.get("L1_ROOT") or srcroot)
BODY = os.environ.get("L2BODY", "l2r1")


def rd(p):
    return (root / p).read_bytes().decode("utf-8")


def wr(p, s):
    (root / p).write_bytes(s.encode("utf-8"))


def sub1(s, a, b, what):
    assert s.count(a) == 1, (what, s.count(a))
    return s.replace(a, b)


def span(s, func):
    m = re.search(rf"^(?!extern)[A-Za-z_][^\n;]*\b{func}\s*\([^;\n]*$", s, re.M)
    i = s.index("{", m.start()) + 1
    depth = 1
    while depth:
        depth += {"{": 1, "}": -1}.get(s[i], 0)
        i += 1
    return m.start(), i


h = rd("include/code6cac.h")
h = sub1(h, "    u8  unk_B3[0xB8 - 0xB3];\n", "    u8  unk_B3;\n    u8  unk_B4[0xB8 - 0xB4];\n", "unk_B3")
h = sub1(h, "    u8  unk_26C[0x272 - 0x26C];\n", "    s16 unk_26C;\n    u8  unk_26E[0x272 - 0x26E];\n", "unk_26C")
wr("include/code6cac.h", h)

s = rd("src/code6cac_tu2.c")
for f in ("func_8001C8DC", "func_8001CE60", "func_8001E404", "func_8001EFA0", "func_8001E878", "func_8001EA84",
          "func_8001FB34"):
    a, b = span(s, f)
    s = s[:a] + (srcroot / f"tmp/prc/{f}.{BODY}.c").read_bytes().decode("utf-8").rstrip("\n") + s[b:]
s = sub1(s, "        if (func_8001FB34((s32 *)rec, data[3] & 0x80) == 0) {\n",
         "        if (func_8001FB34(rec, data[3] & 0x80) == 0) {\n", "FBE8 call")
wr("src/code6cac_tu2.c", s)
print("L2 applied, bodies", BODY)
