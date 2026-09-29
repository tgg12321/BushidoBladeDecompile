"""Apply the func_80070188 landing (no record merge) to the tree. Run from the
repo root under WSL.
- src/text1b.c: candidate (slot accessors + body) over the INCLUDE_ASM line;
  SelectEntryE534 `pad` -> `unk1`.
- sdata_exclude.txt: func_80070188 row keeps only g_gpu_ot_ptr.
- sdata_syms.txt: + D_800A3590."""
import sys
sys.path.insert(0, '.')
from pathlib import Path
from engine import inlineasm


def sub(t, old, new, n=1):
    c = t.count(old)
    assert c == n, (c, n, old[:90])
    return t.replace(old, new)


p = Path('src/text1b.c')
t = p.read_text(encoding='utf-8')
body = Path('memory/grind/func_80070188/candidate.c').read_text(encoding='utf-8')
t = inlineasm.substitute_body(t, 'func_80070188', body)
t = sub(t, '    u8 value;\n    u8 pad;\n} SelectEntryE534;', '    u8 value;\n    u8 unk1;\n} SelectEntryE534;')
p.write_bytes(t.encode('utf-8'))

p = Path('sdata_exclude.txt')
x = p.read_text(encoding='utf-8')
x = sub(x, 'func_80070188: D_800A3562, D_800A3588, D_800A358C, g_gpu_ot_ptr\n',
        'func_80070188: g_gpu_ot_ptr\n')
p.write_bytes(x.encode('utf-8'))
p = Path('sdata_syms.txt')
x = p.read_text(encoding='utf-8')
x = sub(x, 'D_800A358E\nD_800A3592\n', 'D_800A358E\nD_800A3590\nD_800A3592\n')
p.write_bytes(x.encode('utf-8'))
print('landed')
