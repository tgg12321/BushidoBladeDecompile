#!/usr/bin/env python3
"""land3.py [--dry] : func_80021424 itself through PracticeMenuRec (landing lock only; after land.py + land2.py).

PracticeMenuRec gains s16 unk_4C (+0x4C: lh 0x80021850) and s16 unk_78 (+0x78: sh 0x80021440/0x800214DC/
0x80021584); func_80021424 takes (PracticeMenuRec *rec, s32 id, s16 *out) and reads members; the TU's extern
follows the definition. The caller func_8001FBE8 is left as is (see CALLER below)."""
import re
import sys
from pathlib import Path

import os
SRCROOT = Path(__file__).resolve().parents[2]
root = Path(os.environ.get("L1_ROOT") or SRCROOT)
DRY = "--dry" in sys.argv
CALLER = "--caller-cast" in sys.argv


def rd(p):
    return (root / p).read_bytes().decode("utf-8")


def wr(p, s):
    if DRY:
        print("dry: would write", p, len(s))
        return
    (root / p).write_bytes(s.encode("utf-8"))


def sub1(s, a, b, what):
    assert s.count(a) == 1, (what, s.count(a))
    return s.replace(a, b)


def func_span(s, func):
    m = re.search(rf"^[A-Za-z_][^\n;]*\b{func}\s*\([^;{{]*\)\s*\{{", s, re.M)
    i = m.end()
    depth = 1
    while depth:
        depth += {"{": 1, "}": -1}.get(s[i], 0)
        i += 1
    return m.start(), i


h = rd("include/code6cac.h")
h = sub1(h, "    u8  unk_4C[0x5E - 0x4C];\n",
         "    s16 unk_4C;                    /* != 0: func_80021424 takes the character index from the other record (unk_00) */\n"
         "    u8  unk_4E[0x5E - 0x4E];\n", "unk_4C pad")
h = sub1(h, "    u8  unk_74[0x7C - 0x74];\n",
         "    u8  unk_74[0x78 - 0x74];\n"
         "    s16 unk_78;                    /* 0/1, set by func_80021424 */\n"
         "    u8  unk_7A[0x7C - 0x7A];\n", "unk_74 pad")
wr("include/code6cac.h", h)

s = rd("src/code6cac_tu2.c")
s = sub1(s, "extern void *func_80021424(u8 *, s32, u8 *);\n",
         "extern void *func_80021424(PracticeMenuRec *, s32, s16 *);\n", "extern")
a, b = func_span(s, "func_80021424")
s = s[:a] + (SRCROOT / "tmp/func_80021424/func_80021424.r3.c").read_bytes().decode("utf-8").rstrip("\n") + s[b:]
if CALLER:
    s = sub1(s, "        snd = func_80021424(rec, ent->id, rec + 0x5E);\n",
             "        snd = func_80021424((PracticeMenuRec *)rec, ent->id, (s16 *)(rec + 0x5E));\n", "caller")
wr("src/code6cac_tu2.c", s)
print("done (caller cast:", CALLER, ")")
