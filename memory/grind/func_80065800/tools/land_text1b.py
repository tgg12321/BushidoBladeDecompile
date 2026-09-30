"""Aggregate-merge prong (c) in text1b.c (run after land.py, under the landing lock): the block the
5c543ce1d boundary commit moved into text1b.c still declares four per-word externs for bytes
inside D_800F0CA0[18] (records 12/13: D_800F0D30, D_800F0D34, D_800F0D3C, D_800F0D40). Nothing
in text1b.c uses them and their symbol rows are retired by land.py, so the declarations go."""
from pathlib import Path

P = 'src/text1b.c'
t = Path(P).read_bytes().decode('utf-8')
assert '\r' not in t
for s in ('D_800F0D30', 'D_800F0D34', 'D_800F0D3C', 'D_800F0D40'):
    line = 'extern s32 %s;\n' % s
    assert t.count(s) == 1 and t.count(line) == 1, s
    t = t.replace(line, '')
Path(P).write_bytes(t.encode('utf-8'))
print('text1b.c: dropped 4 unused per-word externs')
