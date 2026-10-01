#!/usr/bin/env python3
"""Step 14 (Q65): two objects that become (K2) statics in step 10 are declared two ways inside their own
file; a static has ONE declaration, so each gets its one truthful type first (own byte-identical commit).

  text1b_tu1c, D_800A3468: the file declares it `s32` (file scope and eight functions: an integer holding the
    object's address, `*(s32 *)(D_800A3468 + 0xC)`) and, in func_80060A68 alone, as a block-scope
    `extern struct Ob *` (member access). It keeps the file's `s32`; func_80060A68 reaches the object through
    a cast view macro, the spelling text1b.c already uses for its s32 work pointer
    (`#define W ((Work_80053E9C *)D_800A33F4)`). Body change: func_80060A68 needs its own layer-2.
  text1a_post, g_anim_hit_flags: an array everywhere (func_800420E8 indexes it with a0 < 2, func_8004211C
    reads [0]) except func_800420D0, which declared a block-scope scalar to clear element 0. One declaration:
    the array; func_800420D0 writes element 0. Body change: func_800420D0 needs its own layer-2.
Per-word names of array elements (layer-2 round 1, step-15 finding 1: a [1]-sized static overrun by its own
file's code is cross-symbol storage). Each array gets its evidenced size and the element's second name goes;
the element is written through the array. Byte-identical (same address, same width, same access).
  text1a_post: g_anim_hit_flags is s16[2] (func_800420E8 writes [a0] for a0 < 2) and g_anim_hit_data s32[2]
    (the same); `g_anim_counter` (0x800A3382) is g_anim_hit_flags[1] and `D_800A3388` is g_anim_hit_data[1].
    Body changes: func_800420D0 (clears [1] and [0]), func_8004211C (reads [0] and [1] of each).
  text1b_tu1c: D_800A344C (u32) and D_800A3454 (s32) are per-lane pairs (func_80063BD0 / func_80063E10 /
    func_800644FC index them by idx / lane; func_80060C60 clears both lanes): `D_800A3450` is D_800A344C[1], `D_800A3458` is
    D_800A3454[1]. Body change: func_80060C60.
  text1b_tu1d: D_800A35C8 is s16[2] (func_8006F100 reads/writes [i] per player; func_80070188 and
    func_80070F78 write [0] and [1]); the declaration was incomplete (no body change).
The merged names' symbol-file rows go when nothing still names them (C or an INCLUDE_ASM function's .s).
(A8, owner ruling Q79, rules: 8c57bc4ab): every `.lcomm` static is 4-aligned (Sony probe lcomm_align_probe),
so a gp-reached name at a 2-mod-4 address of a static block is the second halfword of the slot's static: the
five s16 pairs D_800A33C8/CA, D_800A33E8/EA (text1b), D_800A345C/5E, D_800A350C/0E, D_800A3510/12
(text1b_tu1c) become s16[2] arrays (A8_PAIRS below; body changes in each function naming them).
usage: s14_apply.py <tree>"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from adoptlib import *
os.chdir(sys.argv[1])
p = "src/text1b_tu1c.c"
t = rd(p)
a = "    extern struct Ob *D_800A3468;\n"
assert t.count(a) == 1
i = t.index(a)
fstart = t.rindex("\nvoid func_80060A68(void) {", 0, i)
fend = t.index("\n}\n", i) + 3
body = t[fstart:fend].replace(a, "")
n = body.count("D_800A3468->")
body = body.replace("D_800A3468->", "OB->")
body = body.replace("\nvoid func_80060A68(void) {",
                    "\nextern s32 D_800A3468;\n#define OB ((struct Ob *)D_800A3468)\nvoid func_80060A68(void) {", 1)
body = body.rstrip(NL) + "\n#undef OB\n"
wr(p, t[:fstart] + body + t[fend:])
print(f"text1b_tu1c func_80060A68: {n} member accesses through OB")
p = "src/text1a_post.c"
sub1(p, "void func_800420D0(void) {\n    extern s16 g_anim_hit_flags;\n    g_anim_counter = 0;\n    g_anim_hit_flags = 0;\n",
     "extern s16 g_anim_hit_flags[];\nvoid func_800420D0(void) {\n    g_anim_counter = 0;\n    g_anim_hit_flags[0] = 0;\n")
# the later file-scope array declaration (before func_800420E8) now repeats the one above: drop it
t = rd(p)
k = t.index("void func_800420D0(void) {")
rest = t[k:]
assert rest.count("extern s16 g_anim_hit_flags[];\n") == 1
wr(p, t[:k] + rest.replace("extern s16 g_anim_hit_flags[];\n", "", 1))
# ---- per-word names of array elements -> the array's elements
p = "src/text1a_post.c"
sub1(p, "extern s16 g_anim_counter;\n", "")
sub1(p, "extern s16 g_anim_hit_flags[];\nvoid func_800420D0(void) {\n    g_anim_counter = 0;\n    g_anim_hit_flags[0] = 0;\n",
     "extern s16 g_anim_hit_flags[2];\nvoid func_800420D0(void) {\n    g_anim_hit_flags[1] = 0;\n    g_anim_hit_flags[0] = 0;\n")
sub1(p, "extern s32 g_anim_hit_data[];\n", "extern s32 g_anim_hit_data[2];\n")
sub1(p, "extern s32 D_800A3388;\n", "")
sub1(p, "    s32 val = g_anim_hit_flags[0] * 2 + g_anim_counter;\n", "    s32 val = g_anim_hit_flags[0] * 2 + g_anim_hit_flags[1];\n")
t = rd(p)
assert t.count("D_800A3388") == 2, t.count("D_800A3388")
wr(p, t.replace("D_800A3388", "g_anim_hit_data[1]"))
p = "src/text1b_tu1c.c"
sub1(p, "extern s32 D_800A3458;\nextern s32 D_800A3454[];\nextern s32 D_800A3450;\nextern u32 D_800A344C[];\n",
     "extern s32 D_800A3454[2];\nextern u32 D_800A344C[2];\n")
sub1(p, "    D_800A3458 = 0;\n    D_800A3454[0] = 0;\n    D_800A3450 = 0;\n    D_800A344C[0] = 0;\n",
     "    D_800A3454[1] = 0;\n    D_800A3454[0] = 0;\n    D_800A344C[1] = 0;\n    D_800A344C[0] = 0;\n")
sub1("src/text1b_tu1d.c", "extern s16 D_800A35C8[];\n", "extern s16 D_800A35C8[2];\n")
import glob, re as _re
srcs = {f: rd(f) for f in glob.glob("src/*.c") + glob.glob("include/*.h")}
inc = set()
for f, s in srcs.items():
    inc |= set(_re.findall(r'INCLUDE_ASM\(\s*"[^"]*"\s*,\s*(\w+)\s*\)', s))
asm = {fn: rd(f"asm/funcs/{fn}.s") for fn in inc if os.path.exists(f"asm/funcs/{fn}.s")}
for nm in ("g_anim_counter", "D_800A3388", "D_800A3450", "D_800A3458", "D_800A35CA"):
    users = [f for f, s in srcs.items() if _re.search(r"\b%s\b" % nm, s)] + \
            [fn for fn, s in asm.items() if _re.search(r"\b%s\b" % nm, s)]
    if users:
        print(f"row kept: {nm} still named by {users}")
        continue
    for sf in ("undefined_syms_auto.txt", "named_syms.txt"):
        L = rd(sf).split(NL)
        K = [l for l in L if not _re.match(r"^\s*%s\s*=" % nm, l)]
        if len(K) != len(L):
            wr(sf, NL.join(K))
            print(f"row removed: {sf} {nm}")
# ---- (A8, Q79): one .lcomm static per 4-byte slot. Sony ASPSX + PSYLINK place every static 4-aligned
# (lcomm_align_probe), so a name at a 2-mod-4 address in the static region is the second halfword of the
# static that starts the slot. Each pair below is two s16 names in one slot of one file's static block; the
# pair becomes `s16 X[2]` and the second name X[1]. Evidence: the slot itself, and text1b's own
# `(&D_800A33E8)[idx]` (indexing past the first name), and the pair pointer `&D_800A350C` handed to
# func_800692C0 as an `s16 *`.
A8_PAIRS = [("D_800A33C8", "D_800A33CA"), ("D_800A33E8", "D_800A33EA"), ("D_800A345C", "D_800A345E"),
            ("D_800A350C", "D_800A350E"), ("D_800A3510", "D_800A3512")]


def a8_merge(text, X, Y):
    text = _re.sub(r"^[ \t]*extern\s+s16\s+%s\s*;[^\n]*\n" % Y, "", text, flags=_re.M)
    text = _re.sub(r"^([ \t]*)extern\s+s16\s+%s\s*;" % X, r"\1extern s16 @@X[2];", text, flags=_re.M)
    text = _re.sub(r"\(&%s\)\[" % X, "@@X[", text)            # (&X)[i] -> X[i]
    text = _re.sub(r"&%s\b(?!\s*\[)" % X, "@@X", text)          # &X (the pair's address) -> X
    text = _re.sub(r"\b%s\b" % Y, "@@X[1]", text)                # the second halfword
    text = _re.sub(r"\b%s\b(?!\s*\[)" % X, "@@X[0]", text)       # the first halfword
    return text.replace("@@X", X)


for X, Y in A8_PAIRS:
    hit = []
    for f in sorted(glob.glob("src/*.c") + glob.glob("include/*.h")):
        s = rd(f)
        if not _re.search(r"\b(%s|%s)\b" % (X, Y), s):
            continue
        s2 = a8_merge(s, X, Y)
        if s2 != s:
            wr(f, s2)
            hit.append(f)
    print(f"A8 {X}[2] (was {X} + {Y}): {hit}")
    users = [f for f in glob.glob("src/*.c") + glob.glob("include/*.h") if _re.search(r"\b%s\b" % Y, rd(f))] + \
            [fn for fn, s in asm.items() if _re.search(r"\b%s\b" % Y, s)]
    if users:
        print(f"row kept: {Y} still named by {users}")
        continue
    for sf in ("undefined_syms_auto.txt", "named_syms.txt"):
        L = rd(sf).split(NL)
        K = [l for l in L if not _re.match(r"^\s*%s\s*=" % Y, l)]
        if len(K) != len(L):
            wr(sf, NL.join(K))
            print(f"row removed: {sf} {Y}")
print("step 14 applied")
