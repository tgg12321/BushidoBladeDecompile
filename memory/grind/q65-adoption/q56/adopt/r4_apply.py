#!/usr/bin/env python3
"""M4 reconciliation (Q65/Q67, per-file-gp-model.md "Merge": declarations reconciled FIRST, own byte-identical
commit). text1b_tu2 + text1a_b_mid_rodata + text1b_b are one original file (owner ruling Q67); each symbol
they declare differently gets its one truthful type, from the target bytes and each consumer's use.

  func_8006E950   text1b_b calls it with an integer first argument (li $a0, 0x32 / 0x5F); the definition
                  (text1b_tu1d) uses $a0 only as an integer (it is func_80036EA8's second argument):
                  `void (s32, s32 *)`. The definition's own `s32 *a0` + `(s32)a0` becomes `s32 a0`.
  func_8006E49C   defined (text1b_tu1c) `s32 (s32, s32 *)`.
  func_8007352C   defined (text1b_tu1e) `s32 (s32 env_addr)`.
  D_8009BD24      the Unk8009BD24Record[2][5] array (text1b_tu2, text1b); text1b_b's func_80077D00 returns
                  its address. The typedef comes along verbatim from text1b_tu2 (the merge drops the repeat).
usage: r4_apply.py <tree>"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from adoptlib import *
os.chdir(sys.argv[1])


def subn(p, a, b, n):
    t = rd(p)
    assert t.count(a) == n, (p, a[:120], t.count(a), n)
    wr(p, t.replace(a, b))


T2, BB, D1 = "src/text1b_tu2.c", "src/text1b_b.c", "src/text1b_tu1d.c"
sub1(D1, "void func_8006E950(s32 *a0, s32 *a1) {", "void func_8006E950(s32 a0, s32 *a1) {")
sub1(D1, "    s0_addr = (s32)a0;\n    game_FrameLoop();", "    s0_addr = a0;\n    game_FrameLoop();")
sub1(T2, "void func_8006E950(s32 *a0, s32 *a1);", "void func_8006E950(s32 a0, s32 *a1);")
sub1(BB, "extern s32 func_8006E950(s32, s32 *);", "extern void func_8006E950(s32, s32 *);")
sub1(BB, "extern u8 *func_8006E49C(s32, s32);", "extern s32 func_8006E49C(s32, s32 *);")
sub1(BB, "func_8006E49C(r, D_800A35F4);", "func_8006E49C(r, (s32 *)D_800A35F4);")
sub1(BB, "func_8006E49C(r, D_800A360C);", "func_8006E49C(r, (s32 *)D_800A360C);")
subn(BB, "extern s32 func_8007352C(s32 *);", "extern s32 func_8007352C(s32);", 2)
subn(BB, "func_8007352C(&s.header);", "func_8007352C((s32)&s.header);", 5)
subn(BB, "func_8007352C(&s.a);", "func_8007352C((s32)&s.a);", 2)
t2 = rd(T2)
i = t2.index("typedef struct {\n    u8 chr;")
td = t2[i:t2.index("} Unk8009BD24Record;", i) + len("} Unk8009BD24Record;")] + "\n"
sub1(BB, "extern s32 D_8009BD24;\ns32* func_80077D00(void) {\n    return &D_8009BD24;",
     td + "extern Unk8009BD24Record D_8009BD24[2][5];\ns32* func_80077D00(void) {\n    return (s32 *)D_8009BD24;")
print("M4 reconciliation applied")
