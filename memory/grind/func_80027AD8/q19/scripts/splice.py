"""Splice a candidate body into src/code6cac_b.c, replacing the jtbl INCLUDE_RODATA + INCLUDE_ASM lines.
usage: python3 splice.py <candidate.c>"""
import sys
p = 'C:/Users/Trenton/desktop/Bushido Blade 2 Decompile/src/code6cac_b.c'
src = open(p, encoding='utf-8', newline='').read()
old = 'INCLUDE_RODATA("asm/rodata", jtbl_80010548);\nINCLUDE_ASM("asm/funcs", func_80027AD8);\n'
assert src.count(old) == 1, 'splice point not found'
body = open(sys.argv[1], encoding='utf-8').read()
if not body.endswith('\n'):
    body += '\n'
src = src.replace(old, body)
open(p, 'w', encoding='utf-8', newline='\n').write(src)
print('spliced', len(body.splitlines()), 'lines')
