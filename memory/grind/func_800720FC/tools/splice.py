"""Splice a body into src/text1b.c (replacing the INCLUDE_ASM line) and retype
D_800A3578 to s16. Usage: splice.py <body.c> [--retype]"""
import sys
sys.path.insert(0, '.')
from pathlib import Path
from engine import inlineasm
p = Path('src/text1b.c')
txt = p.read_text(encoding='utf-8')
body = Path(sys.argv[1]).read_text(encoding='utf-8')
txt = inlineasm.substitute_body(txt, 'func_800720FC', body)
if '--retype' in sys.argv:
    txt = txt.replace('extern u16 D_800A3578;', 'extern s16 D_800A3578;')
p.write_bytes(txt.encode('utf-8'))
print('spliced')
