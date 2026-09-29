"""Rewrite the joined gte_Lzc islands of a body into the header-exact inline_o.h form
(gte_ldlzc :207-210, gte_nop :1095-1097 x2, gte_stlzc :1074-1077 as engine/gtemacro.py pins them).
usage: python mkhe.py <in.c> <out.c> [--no-dowhile]"""
import re
import sys

src, dst = sys.argv[1], sys.argv[2]
s = open(src, encoding='utf-8').read()
CL = '"$12","$13","$14","$15","memory"'


def he(inp, slot, ind):
    return (ind + '__asm__ volatile ("move  $12,%0": :"r"(' + inp + '):' + CL + ');\n'
            + ind + '__asm__ volatile ("mtc2  $12,$30": : :' + CL + ');\n'
            + ind + '__asm__ volatile ("nop   ": : :' + CL + ');\n'
            + ind + '__asm__ volatile ("nop   ": : :' + CL + ');\n'
            + ind + '__asm__ volatile ("move  $12,%0": :"r"(&' + slot + '):' + CL + ');\n'
            + ind + '__asm__ volatile ("swc2  $31,($12)": : :' + CL + ');\n')


BS = chr(92)
pat = re.compile(
    r'( +)__asm__ volatile\(\n +"move   \$12, %0' + re.escape(BS) + r'n"\n +"mtc2   \$12, \$30' + re.escape(BS)
    + r'n"\n +"nop' + re.escape(BS) + r'n"\n +"nop' + re.escape(BS) + r'n"\n +:: "r"\((\w+)\) : "\$12"\);\n'
    r' +__asm__ volatile\(\n +"move   \$12, %0' + re.escape(BS) + r'n"\n +"swc2   \$31, 0\(\$12\)' + re.escape(BS)
    + r'n"\n +:: "r"\(&(\w+)\) : "\$12", "memory"\);\n')
n = len(pat.findall(s))
assert n == 2, n
s = pat.sub(lambda m: he(m.group(2), m.group(3), m.group(1)), s)
if '--no-dowhile' in sys.argv:
    old = '        do {\n            rec = &D_800F5F68[id * 0x1B8];\n        } while (0);\n'
    assert old in s
    s = s.replace(old, '        rec = &D_800F5F68[id * 0x1B8];\n')
open(dst, 'w', encoding='utf-8', newline='\n').write(s)
print('ok', dst)
