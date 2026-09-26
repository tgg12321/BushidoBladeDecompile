"""Scratch whole-file build with header + source edits, scoring EVERY function vs build/.
usage (WSL, repo root): python3 tmp/func_80055138/wf2.py <stem> [candidate.c]
Edits applied (the aggregate/retype model for func_80055138's landing):
  include/code6cac.h : cpu_practice_honmokuroku_data_tbl -> u8 [][4];
                       + StatusFlagRec D_80099D88[] record table
  src/text1b.c       : drop local `extern u16 D_80099D88;`, respell func_80055948's use,
                       splice candidate over INCLUDE_ASM (if given)
  src/code6cac_b.c   : func_80033DF4 `&tbl + (tableIndex * 4)` -> `tbl[tableIndex]`
"""
import sys, os, subprocess, re
sys.path.insert(0, '.')
from engine import score as S
from engine import pipeline as P

stem = sys.argv[1]
cand = sys.argv[2] if len(sys.argv) > 2 and not sys.argv[2].startswith('--') else None
W = 'tmp/func_80055138/wf2'
os.makedirs(W + '/inc', exist_ok=True)

h = open('include/code6cac.h').read()
old = 'extern u8 cpu_practice_honmokuroku_data_tbl;\n'
assert h.count(old) == 1
h = h.replace(old, 'extern u8 cpu_practice_honmokuroku_data_tbl[][4];\n')
anchor = 'extern u8 D_80101EC8;\n'
assert h.count(anchor) == 1
h = h.replace(anchor, anchor + (
    '/* Per-character status-flag record table (0x80099D88, stride 0x18). */\n'
    'typedef struct StatusFlagRec {\n'
    '    u16 flags;\n'
    '    u8  unk2;\n'
    '    u8  unk3;\n'
    '    u8  unk4[0x18 - 4];\n'
    '} StatusFlagRec;\n'
    'extern StatusFlagRec D_80099D88[];\n'))
open(W + '/inc/code6cac.h', 'w').write(h)

src = open(f'src/{stem}.c').read()
if stem == 'text1b':
    if cand:
        c = open(cand).read()
        MARK = 'INCLUDE_ASM("asm/funcs", func_80055138);'
        assert src.count(MARK) == 1
        src = src.replace(MARK, c.rstrip('\n'))
    for o, n in (('extern u16 D_80099D88;\n', ''),
                 ('(*(&D_80099D88 + idx * 12) & 0xBF00)', '(D_80099D88[idx].flags & 0xBF00)')):
        assert src.count(o) == 1, o
        src = src.replace(o, n)
if stem == 'code6cac_b':
    o = 'u8 *table = &cpu_practice_honmokuroku_data_tbl + (tableIndex * 4);'
    assert src.count(o) == 1
    src = src.replace(o, 'u8 *table = cpu_practice_honmokuroku_data_tbl[tableIndex];')
open(f'{W}/{stem}.c', 'w').write(src)

out = f'{W}/{stem}.o'
cmd = P.c_pipeline_cmd(stem, out, None)
assert f'src/{stem}.c' in cmd
cmd = cmd.replace(f'src/{stem}.c', f'{W}/{stem}.c', 1).replace('-Iinclude', f'-I{W}/inc -Iinclude', 1)
r = subprocess.run(['bash', '-c', 'set -o pipefail; ' + cmd], capture_output=True, text=True)
if r.returncode:
    print('BUILD FAILED\n', r.stderr[-3000:])
    sys.exit(1)
ref = f'build/src/{stem}.o'
nm = subprocess.run(['mipsel-linux-gnu-nm', ref], capture_output=True, text=True).stdout
funcs = sorted({l.split()[2] for l in nm.splitlines() if len(l.split()) == 3 and l.split()[1] in 'Tt'})
if '--only' in sys.argv:
    print('func_80055138', S.score_func(out, ref, 'func_80055138'))
    sys.exit(0)
bad = 0
for f in funcs:
    try:
        s = S.score_func(out, ref, f)
    except Exception as e:
        print('ERR', f, e)
        bad += 1
        continue
    if s['score'] or s['target_insns'] != s['build_insns']:
        print('DIFF', f, s)
        bad += 1
print(f'{stem}: {len(funcs)} functions scored, {bad} differ')
