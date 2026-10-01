#!/usr/bin/env python3
"""land.py: apply the func_80058580 landing to the working tree (run ONLY under the landing lock).
Bodies from memory/grind/func_80058580/candidate.c + typed/*; writes src/text1b.c, include/code6cac.h,
undefined_syms_auto.txt, named_syms.txt (LF)."""
import os, sys, importlib.util
sys.path.insert(0, "tmp/func_80058580/h")
L = "memory/grind/func_80058580"


def rd(p):
    return open(p, encoding="utf-8").read()


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="\n").write(s)


def edits(p):
    spec = importlib.util.spec_from_file_location("e", p)
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m.EDITS


src = rd("src/text1b.c")
a = src.index("/* func_80058580's three switch tables")
inc = 'INCLUDE_ASM("asm/funcs", func_80058580);'
b = src.index(inc) + len(inc)
src = src[:a] + rd(L + "/candidate.c").rstrip("\n") + src[b:]
a = src.index("void func_80055138(")
b = src.index("\n}\n", a) + 3
src = src[:a] + rd(L + "/typed/f55138.c").rstrip("\n") + "\n" + src[b:]
a = src.index("extern u8 D_8009A830;")
b = src.index("\n}\n", src.index("s32 func_80056FE8(")) + 3
src = src[:a] + rd(L + "/typed/f56fe8.c").rstrip("\n") + "\n" + src[b:]
for o, n in edits(L + "/typed/src_edits.py"):
    assert src.count(o) == 1, o[:60]
    src = src.replace(o, n)
wr("src/text1b.c", src)

hdr = rd("include/code6cac.h")
for o, n in edits(L + "/typed/hdr_edits.py"):
    assert hdr.count(o) == 1, o[:60]
    hdr = hdr.replace(o, n)
wr("include/code6cac.h", hdr)

u = rd("undefined_syms_auto.txt")
old = "D_8009A8CA = 0x8009A8CA;  /* alias of D_8009A8C4+0x6 (entry [0][1], byte 2); retire with func_80058580 (asm/funcs/func_80058580.s is its only assembled referrer) */\n"
assert u.count(old) == 1
u = u.replace(old, "D_8009A8C8 = 0x8009A8C8;  /* CpuLevelEntry D_8009A8C8[][8] (include/code6cac.h); inside asm/data/7D920.data.s dlabel D_8009A8C4 */\n")
import re as _re
for sym in ("D_80099D8B", "D_80099D8C", "D_80099D8E", "D_80099D94", "D_80099D97", "D_80099D9C", "D_80099D9D"):
    m = _re.findall(r"^%s = 0x[0-9A-F]+;  /\* alias of D_80099D88\+0x[0-9A-F]+ \(StatusFlagRec record 0\); retire with func_80058580 \(asm/funcs/func_80058580\.s is its only assembled referrer\) \*/\n" % sym, u, _re.M)
    assert len(m) == 1, sym
    u = u.replace(m[0], "")
old8f = "D_80099D8F = 0x80099D8F;  /* alias of D_80099D88+0x7 (StatusFlagRec record 0); retire with func_80055B60 and func_80058580 (asm/funcs/func_80055B60.s and asm/funcs/func_80058580.s are its assembled referrers) */\n"
assert u.count(old8f) == 1
u = u.replace(old8f, "D_80099D8F = 0x80099D8F;  /* alias of D_80099D88+0x7 (StatusFlagRec record 0); retire with func_80055B60 (asm/funcs/func_80055B60.s is its only assembled referrer) */\n")
for sym in ("D_8009A851", "D_8009A852", "D_8009A853"):
    row = "%s = 0x%s;\n" % (sym, sym[2:])
    assert u.count(row) == 1, sym
    u = u.replace(row, "")
wr("undefined_syms_auto.txt", u)

n = rd("named_syms.txt")
R = [
    ("g_text1b_addr_8009A8CA                                       = 0x8009A8CA;\n", ""),
    ("g_text1b_motion_byte_table_8x3                          = 0x8009A830;  /* 3 x 8-byte byte records; consumed by text1b */",
     "g_text1b_motion_byte_table_8x3                          = 0x8009A830;  /* u8[8] (dlabel D_8009A830); func_80056FE8. 0x8009A838 (s8[8]) and 0x8009A840 (u8[16]) are separate tables, not rows of one 3x8 record */"),
    ("g_text1b_motion_byte_table_8x3_plus_8                   = 0x8009A838;  /* +8 from g_text1b_motion_byte_table_8x3 (0x8009A830) */",
     "g_text1b_motion_byte_table_8x3_plus_8                   = 0x8009A838;  /* s8[8] (dlabel D_8009A838, read lb); func_80056FE8, func_80058580 */"),
    ("g_text1b_motion_byte_table_8x3_plus_8_plus_8                 = 0x8009A840;\n",
     "g_text1b_motion_byte_table_8x3_plus_8_plus_8                 = 0x8009A840;  /* u8[16] (dlabel D_8009A840); func_80056FE8 */\n"),
]
for nm in ("g_status_flag_record_table_80099D88_plus_3 ", "g_status_flag_record_table_80099D88_plus_4 ",
           "g_status_flag_record_table_80099D88_plus_6 ", "g_status_flag_record_table_80099D88_plus_7_plus_5 ",
           "g_status_flag_record_table_80099D88_plus_7_plus_13 ", "g_status_flag_record_table_80099D88_plus_7_plus_14 ",
           "g_text1b_addr_8009A851 ", "g_text1b_addr_8009A852 ", "g_text1b_addr_8009A853 "):
    rows = [l for l in n.split("\n") if l.startswith(nm)]
    assert len(rows) == 1, nm
    R.append((rows[0] + "\n", ""))
for o, w in R:
    assert n.count(o) == 1, o[:50]
    n = n.replace(o, w)
wr("named_syms.txt", n)
print("landed edits applied")
