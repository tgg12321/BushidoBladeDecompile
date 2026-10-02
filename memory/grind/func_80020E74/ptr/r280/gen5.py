import re
import subprocess
from pathlib import Path
D = Path('tmp/func_80020E74/a6/r280/s5')
D.mkdir(exist_ok=True)
src = Path('src/code6cac_tu2.c').read_text(encoding='utf-8')


def func(name):
    m = re.search(r'^[^\n]*\b' + name + r'\([^)]*\) \{\n.*?^\}\n', src, re.M | re.S)
    return m.group(0)


def w(n, t):
    (D / n).write_bytes(t.encode())


f = func('func_80021280')
w('r280_val_a3.c', f.replace("u16 val = a2->unk_48;", "u16 val = a3;"))
f = func('func_800224E0')
w('e0_ptrcmp.c', f.replace("(s32)p < (s32)end", "p < end"))
f = func('func_8001DB9C')
w('db9c_nocast.c', f.replace("(u16)0xFFFF", "0xFFFF"))
