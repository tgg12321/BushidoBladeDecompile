"""Swap the inline_o.h gte_rtv0 statements for the inline_c.h one-statement form -> <out>"""
import sys
from pathlib import Path
t = Path(sys.argv[1]).read_bytes().decode()
CL = '"$12","$13","$14","$15","memory"'
import re
pat = re.compile(r'([ \t]*)__asm__ volatile \("nop   ": : :' + re.escape(CL) + r'\);\n\1__asm__ volatile \("nop   ": : :' + re.escape(CL) + r'\);\n\1__asm__ volatile \("\.word 0x4A486012": : :' + re.escape(CL) + r'\);\n')
t2, n = pat.subn(lambda m: m.group(1) + '__asm__ volatile ("nop;" "nop;" ".word 0x4A486012");\n', t)
print(sys.argv[1], n)
Path(sys.argv[2]).write_bytes(t2.encode())
