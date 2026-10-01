#!/usr/bin/env python3
"""land2.py [--dry] : the rev-21424 fix on top of land.py (landing lock only).

PracticeMenuRec gains s16 unk_4A (+0x4A, the character index: lh in func_800213A0/80021904/974/9E4/A3C) and
s16 unk_86 (+0x86: lh/sh in func_800213A0, lh in func_80021904, sh in func_800218C8); the mid-record per-word
handles D_80101F12 / D_80101F4C / D_80101F4E leave C: func_800218C8, func_80021904, func_80021974,
func_800219E4, func_80021A3C read g_practice_menu_table[a0].member, func_800213A0 takes a PracticeMenuRec *.
Symbol rows: D_80101F12 stays as an alias (func_80023F08 is INCLUDE_ASM and names it); F4C / F4E retire."""
import re
import subprocess
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[2]
DRY = "--dry" in sys.argv


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


def set_body(s, func, new):
    a, b = func_span(s, func)
    return s[:a] + new.rstrip("\n") + s[b:]


h = rd("include/code6cac.h")
h = sub1(h, "    u8  unk_40[0x5E - 0x40];\n",
         "    u8  unk_40[0x4A - 0x40];\n"
         "    s16 unk_4A;                    /* character index: row of D_800A3860 / D_801027B0 (func_800213A0, func_80021424) */\n"
         "    u8  unk_4C[0x5E - 0x4C];\n", "unk_40 pad")
h = sub1(h, "    u8  unk_86[0x88 - 0x86];\n",
         "    s16 unk_86;                    /* wrapped by D_800A3860[unk_4A]->f14 (func_800213A0); copied from unk_84 (func_800218C8) */\n",
         "unk_86 pad")
h = sub1(h, "extern s16 D_80101F12;\n", "", "D_80101F12 extern")
h = sub1(h, "extern s16 D_80101F4C;\nextern s16 D_80101F4E;\n", "", "D_80101F4C/4E externs")
wr("include/code6cac.h", h)

s = rd("src/code6cac_tu2.c")
s = sub1(s, "            func_800213A0((s16 *)rec);\n", "            func_800213A0((PracticeMenuRec *)rec);\n",
         "func_800213A0 call")
bodies = (d := root / "tmp/func_80021424")
for f in ("func_800213A0", "func_800218C8", "func_80021904", "func_80021974", "func_800219E4", "func_80021A3C"):
    s = set_body(s, f, (d / f"{f}.r2.c").read_bytes().decode("utf-8"))
wr("src/code6cac_tu2.c", s)

u = rd("undefined_syms_auto.txt")
u = sub1(u, "D_80101F12 = 0x80101F12;\n",
         "D_80101F12 = 0x80101F12;  /* alias of g_practice_menu_table+0x4A (.unk_4A); retire with func_80023F08 "
         "(asm/funcs/func_80023F08.s is its only assembled referrer) */\n", "D_80101F12 row")
u = sub1(u, "D_80101F4C = 0x80101F4C;\n", "", "D_80101F4C row")
u = sub1(u, "D_80101F4E = 0x80101F4E;\n", "", "D_80101F4E row")
wr("undefined_syms_auto.txt", u)
if DRY:
    sys.exit(0)
files = ["include/code6cac.h", "src/code6cac_tu2.c", "src/code6cac_c2.c", "undefined_syms_auto.txt"]
patch = subprocess.run(["git", "diff", "HEAD", "--"] + files, cwd=root, capture_output=True).stdout
(d / "mine.patch").write_bytes(patch)
print("applied; patch bytes", len(patch))
