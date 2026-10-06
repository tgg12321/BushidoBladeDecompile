#!/usr/bin/env python3
# F01b2 ablations on f01b2.py's output (tmp/p2/f01b2/51268.c): one candidate body per variant under
# tmp/p2/f01b2/abl/<name>.c + list.txt, scored by score_b2.py (the engine sandbox recipe, --disable
# all with cheat-asm stripped, on a whole-file scratch copy: the tree still holds F01b1).
# Every FAKE of the 20 moved bodies, each removed alone and in its cluster.
import os, re
NL = chr(10)
S = open("tmp/p2/f01b2/51268.c", encoding="utf-8").read()
OUT = "tmp/p2/f01b2/abl"
os.makedirs(OUT, exist_ok=True)

def body(f):
    m = re.search(r"\n[a-z0-9_]+ %s\([^;{]*\)\s*\{" % f, S)   # the definition, not a prototype
    i = m.start() + 1
    return S[i:S.index("\n}\n", i) + 3]

def rep(b, a, c, n=1):
    assert b.count(a) == n, (a[:80], b.count(a), n)
    return b.replace(a, c)

def drop_comment(b, start):
    a = b.index(start)
    return b[:a] + b[b.index("*/\n", a) + 3:]

def nolast(b):
    b = re.sub(r"    /\* FAKE: named intermediate - with word 2.*?\*/\n", "", b, count=1, flags=re.S)
    b = re.sub(r"    s32 last;[^\n]*\n", "", b, count=1)
    m = re.search(r"    last = ([^;]+);\n", b)
    b = b.replace(m.group(0), "")
    return rep(b, ".unk8 = last;", ".unk8 = %s;" % m.group(1))

V = {}
# func_80064F68 / func_80064FB4: q (FAKE) alone, `last` (FAKE) alone, both
for f in ("func_80064F68", "func_80064FB4"):
    b0 = body(f)
    b = drop_comment(b0, "    /* FAKE: the third word is read through a pointer")
    b = rep(b, "    s32 *q = &D_800A347C[2];\n", "")
    b = rep(b, "    last = *q;\n", "    last = D_800A347C[2];\n")
    V[f[5:] + "_q"] = (f, b)
    V[f[5:] + "_q_last"] = (f, nolast(b))
    V[f[5:] + "_last"] = (f, nolast(b0))
# `last` (FAKE) in the other ten copiers
for f in ("func_80064E90", "func_80064ED8", "func_80064F20", "func_80065000", "func_8006505C",
          "func_800650A4", "func_800650EC", "func_80065134", "func_80065264", "func_800652AC"):
    V[f[5:] + "_last"] = (f, nolast(body(f)))
# func_80065000's word pointer (F01b1's FAKE), re-ablated in the b2 body: direct / block-pointer local
b0 = body("func_80065000")
b = drop_comment(b0, "    /* FAKE: the word is read through an s32 *")
V["80065000_wptr"] = ("func_80065000", rep(rep(b, "    s32 *q = &D_800A3468->unk00.w;\n", ""), "(*q >> 19)", "(D_800A3468->unk00.w >> 19)"))
V["80065000_wptr_holder"] = ("func_80065000", rep(rep(b, "    s32 *q = &D_800A3468->unk00.w;\n", "    Unk1F800000Unk00 *q = D_800A3468;\n"), "(*q >> 19)", "(q->unk00.w >> 19)"))
# func_8006517C / func_800651F0: the t / t2 intermediates (FAKE), each alone and both. (Their HEAD
# p / ap / bp walkers are gone in f01b2.py: D_800A347C[k] directly is IDENTICAL.)
for f in ("func_8006517C", "func_800651F0"):
    b0 = drop_comment(body(f), "    /* FAKE: named intermediates")
    for name, vs in (("t", ("t",)), ("t2", ("t2",)), ("t_t2", ("t", "t2"))):
        b = b0
        for v in vs:
            b = rep(b, "    s32 %s;\n" % v, "")
            m = re.search(r"    %s = ([^;]+);\n" % v, b)
            b = b.replace(m.group(0), "")
            b = re.sub(r"(\.unk8) = %s;" % v, r"\1 = %s;" % m.group(1), b)
        V["%s_%s" % (f[5:], name)] = (f, b)
# func_80063BD0's three "Shape notes" constructs, each replaced alone
b0 = body("func_80063BD0")
b0n = re.sub(r"    s32 bits; /\* FAKE: named intermediate.*?\*/\n", "    s32 bits;\n", b0, flags=re.S)   # bits' label goes with bits
assert b0n != b0
LOOP = """        for (i = 0; i < D_800A344C[idx]; i++) {
            bits = D_800A3454[idx];
            mask = 1 << i;
            if (!(bits & mask)) {
"""
# (a) the found arm after the loop, reached by goto (the note's alternative)
b = rep(b0, LOOP, """        for (i = 0; i < D_800A344C[idx]; i++) {
            bits = D_800A3454[idx];
            mask = 1 << i;
            if (!(bits & mask)) {
                goto found;
            }
        }
        goto done;
    found:
        {
            {
""")
b = rep(b, """                break;
            }
        }
    } else {""", """            }
        }
    } else {""")
b = rep(b, "    }\n    return 1;\n}\n", "    }\ndone:\n    return 1;\n}\n")
V["80063BD0_goto"] = ("func_80063BD0", b)
# (b) bits / mask: no `bits` (the word read in the test); neither local; mask before bits
b = rep(b0n, LOOP, """        for (i = 0; i < D_800A344C[idx]; i++) {
            mask = 1 << i;
            if (!(D_800A3454[idx] & mask)) {
""")
V["80063BD0_nobits"] = ("func_80063BD0", rep(b, "    s32 bits;\n", ""))
b = rep(b0n, LOOP, """        for (i = 0; i < D_800A344C[idx]; i++) {
            if (!(D_800A3454[idx] & (1 << i))) {
""")
b = rep(b, "                D_800A3454[idx] |= mask;\n", "                D_800A3454[idx] |= 1 << i;\n")
V["80063BD0_nolocals"] = ("func_80063BD0", rep(b, "    s32 bits;\n    s32 mask;\n", ""))
V["80063BD0_order"] = ("func_80063BD0", rep(b0n, "            bits = D_800A3454[idx];\n            mask = 1 << i;\n",
                                             "            mask = 1 << i;\n            bits = D_800A3454[idx];\n"))
# (c) a `return 1` at the end of each arm instead of the one trailing return
b = rep(b0, """                break;
            }
        }
    } else {""", """                break;
            }
        }
        return 1;
    } else {""")
V["80063BD0_ret"] = ("func_80063BD0", b)

with open(OUT + "/list.txt", "w", encoding="utf-8", newline=NL) as fl:
    for name, (f, b) in V.items():
        open("%s/%s.c" % (OUT, name), "w", encoding="utf-8", newline=NL).write(b)
        fl.write("%s %s\n" % (name, f))
print(len(V), "variants")
