#!/usr/bin/env python3
"""land2.py: the func_80058580 landing on top of b3843cc02 (run ONLY under the landing lock).
src/text1b.c: jtbl transcriptions + INCLUDE_ASM -> candidate.c; include/code6cac.h: s16 unk_6C;
undefined_syms_auto.txt / named_syms.txt: retire the aliases whose only assembled referrer was
asm/funcs/func_80058580.s."""
import re

L = "memory/grind/func_80058580"


def rd(p):
    return open(p, encoding="utf-8").read()


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="\n").write(s)


src = rd("src/text1b.c")
a = src.index("/* func_80058580's three switch tables")
inc = 'INCLUDE_ASM("asm/funcs", func_80058580);'
b = src.index(inc) + len(inc)
src = src[:a] + rd(L + "/candidate.c").rstrip("\n") + src[b:]
wr("src/text1b.c", src)

u = rd("undefined_syms_auto.txt")
for sym in ("D_80099D8B", "D_80099D8C", "D_80099D8E", "D_80099D8F", "D_80099D94", "D_80099D97", "D_80099D9C", "D_80099D9D"):
    m = re.findall(r"^%s = 0x[0-9A-F]+;  /\* alias of D_80099D88\+0x[0-9A-F]+ \(StatusFlagRec record 0\); retire with func_80058580 \(asm/funcs/func_80058580\.s is its only assembled referrer\) \*/\n" % sym, u, re.M)
    assert len(m) == 1, sym
    u = u.replace(m[0], "")
for sym in ("D_8009A851", "D_8009A852", "D_8009A853"):
    row = "%s = 0x%s;\n" % (sym, sym[2:])
    assert u.count(row) == 1, sym
    u = u.replace(row, "")
m = re.findall(r"^D_8009A8CA = 0x8009A8CA;.*\n", u, re.M)
assert len(m) == 1
u = u.replace(m[0], "")
wr("undefined_syms_auto.txt", u)

n = rd("named_syms.txt")
for nm in ("g_status_flag_record_table_80099D88_plus_3 ", "g_status_flag_record_table_80099D88_plus_4 ",
           "g_status_flag_record_table_80099D88_plus_6 ", "g_status_flag_record_table_80099D88_plus_7 ",
           "g_status_flag_record_table_80099D88_plus_7_plus_5 ",
           "g_status_flag_record_table_80099D88_plus_7_plus_13 ", "g_status_flag_record_table_80099D88_plus_7_plus_14 ",
           "g_text1b_addr_8009A851 ", "g_text1b_addr_8009A852 ", "g_text1b_addr_8009A853 ", "g_text1b_addr_8009A8CA "):
    rows = [l for l in n.split("\n") if l.startswith(nm)]
    assert len(rows) == 1, nm
    n = n.replace(rows[0] + "\n", "")
wr("named_syms.txt", n)
print("func_80058580 landing applied")
