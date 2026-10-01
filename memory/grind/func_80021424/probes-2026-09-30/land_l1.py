#!/usr/bin/env python3
"""land_l1.py [--dry] : L1 extras on top of `git apply -3 tmp/func_80021424/round3_full.patch` (landing lock only;
L1_ROOT=<dir> applies them to a scratch tree instead of main).

Every body this landing changes reaches g_practice_menu_table through PracticeMenuRec members. New members, each
typed by the target's own instructions (tmp/prc/acc.py <func> <reg>): func_80021A98 ($s0): s16 unk_40/42/46 (sh),
u8 *unk_50 (lw/sw; holds func_80021424's result, bytes +6/+9 read), s32 unk_54/58 (sw, lw/sw), s16 unk_5C (sh),
u8 unk_60/61 (sb), u16 unk_6A (lhu/sh), s16 unk_6C (lh/sh), s16 unk_6E/70 (sh), u8 unk_AD/AF (sb), u8 unk_B0 (lbu),
s16 unk_154 (sh); func_8001FBE8: s32 unk_74 (sw), s16 unk_7A (lh/sh), s16 unk_94 (sh), u16 unk_272 (lhu/sh).
Bodies: tmp/prc/<func>.<BODY or r3>.c for func_8001FBE8, func_80021A98, func_80022F34.
Externs D_80101F08/F10/F14/F42 leave C; rows: F08 -> func_80026DA4, F10 -> func_80020E74 (aliases of INCLUDE_ASM
referrers); F14 / F42 retire."""
import os
import re
import sys
from pathlib import Path

srcroot = Path(__file__).resolve().parents[2]
root = Path(os.environ.get("L1_ROOT") or srcroot)
BODY = os.environ.get("BODY", "r3")
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


def span(s, func):
    m = re.search(rf"^(?!extern)[A-Za-z_][^\n;]*\b{func}\s*\([^;\n]*$", s, re.M)
    i = s.index("{", m.start()) + 1
    depth = 1
    while depth:
        depth += {"{": 1, "}": -1}.get(s[i], 0)
        i += 1
    return m.start(), i


h = rd("include/code6cac.h")
assert "s16 unk_4C;" in h and "s16 unk_78;" in h, "round3_full.patch not applied"
h = sub1(h, "    u8  unk_40[0x4A - 0x40];\n",
         "    s16 unk_40;\n    s16 unk_42;\n    u8  unk_44[0x46 - 0x44];\n    s16 unk_46;\n    u8  unk_48[0x4A - 0x48];\n",
         "unk_40")
h = sub1(h, "    u8  unk_4E[0x5E - 0x4E];\n",
         "    u8  unk_4E[0x50 - 0x4E];\n"
         "    u8  *unk_50;                   /* func_80021424's result (func_80021A98); func_8001FAE4's argument */\n"
         "    s32 unk_54;\n    s32 unk_58;\n    s16 unk_5C;\n", "unk_4E")
MINE_60 = ("    u8  unk_60;\n    u8  unk_61;\n    u8  unk_62[0x6A - 0x62];\n    u16 unk_6A;\n"
           "    s16 unk_6C;\n    s16 unk_6E;\n    s16 unk_70;\n")
LANEH_60 = "    u8  unk_60[0x6A - 0x60];\n    u16 unk_6A;\n    u8  unk_6C[0x72 - 0x6C];\n"  # laneH's u16 unk_6A, if landed first
if LANEH_60 in h:
    h = sub1(h, LANEH_60, MINE_60, "unk_60 (rebased on laneH's unk_6A)")
else:
    h = sub1(h, "    u8  unk_60[0x72 - 0x60];\n", MINE_60, "unk_60")
h = sub1(h, "    u8  unk_74[0x78 - 0x74];\n", "    s32 unk_74;\n", "unk_74")
h = sub1(h, "    u8  unk_7A[0x7C - 0x7A];\n", "    s16 unk_7A;\n", "unk_7A")
h = sub1(h, "    u8  unk_92[0x96 - 0x92];\n", "    u8  unk_92[0x94 - 0x92];\n    s16 unk_94;\n", "unk_92")
h = sub1(h, "    u8  unk_A1[0xB1 - 0xA1];\n",
         "    u8  unk_A1[0xAD - 0xA1];\n    u8  unk_AD;\n    u8  unk_AE[0xAF - 0xAE];\n    u8  unk_AF;\n    u8  unk_B0;\n",
         "unk_A1")
h = sub1(h, "    u8  unk_154[0x156 - 0x154];\n", "    s16 unk_154;\n", "unk_154")
h = sub1(h, "    u8  unk_26C[0x274 - 0x26C];\n",
         "    u8  unk_26C[0x272 - 0x26C];\n    u16 unk_272;   /* lhu/sh only (func_8001FBE8 0x8001FFA4/B0) */\n", "unk_26C")
for ext in ("extern s16 D_80101F08;\n", "extern s16 D_80101F10;\n", "extern s16 D_80101F14;\n", "extern s16 D_80101F42;\n"):
    h = sub1(h, ext, "", ext)
wr("include/code6cac.h", h)

s = rd("src/code6cac_tu2.c")
for f in ("func_8001FBE8", "func_80021A98", "func_80022F34"):
    a, b = span(s, f)
    s = s[:a] + (srcroot / f"tmp/prc/{f}.{BODY}.c").read_bytes().decode("utf-8").rstrip("\n") + s[b:]
wr("src/code6cac_tu2.c", s)

u = rd("undefined_syms_auto.txt")
u = sub1(u, "D_80101F08 = 0x80101F08;\n", "D_80101F08 = 0x80101F08;  /* alias of g_practice_menu_table+0x40; retire "
         "with func_80026DA4 (asm/funcs/func_80026DA4.s is its only assembled referrer) */\n", "F08 row")
u = sub1(u, "D_80101F10 = 0x80101F10;\n", "D_80101F10 = 0x80101F10;  /* alias of g_practice_menu_table+0x48; retire "
         "with func_80020E74 (asm/funcs/func_80020E74.s is its only assembled referrer) */\n", "F10 row")
u = sub1(u, "D_80101F14 = 0x80101F14;\n", "", "F14 row")
u = sub1(u, "D_80101F42 = 0x80101F42;\n", "", "F42 row")
wr("undefined_syms_auto.txt", u)
print("L1 extras", "checked" if DRY else "applied", "bodies", BODY)
