"""Q8 proof: build code6cac_b3.c through the real pipeline at -G0 and at -G8 and list, for each small
extern outside the cursor set, every instruction carrying a relocation against it (raw word + reloc).
Identical lists => -G8 does not change that extern's compiled instructions."""
import subprocess, sys, re, os, json
sys.path.insert(0, '.')
from engine import pipeline
import engine.buildconfig as cfg
SYMS = ['D_800A37B8', 'D_800A3690', 'D_800A36F9']
SRC = 'tmp/func_80034708/integ/src/code6cac_b3.c'
OUT = 'tmp/func_80034708/q8'
os.makedirs(OUT, exist_ok=True)
cfg.EXPAND_LB_FILES.add('code6cac_b3'); cfg.RODATA_ALIGN2_FILES.add('code6cac_b3')
res = {}
for flag in ('-G0', '-G8'):
    if flag == '-G8':
        cfg.GP_FILES.add('code6cac_b3')
    else:
        cfg.GP_FILES.discard('code6cac_b3')
    o = f'{OUT}/b3{flag}.o'
    cmd = pipeline.c_pipeline_cmd('code6cac_b3', o, {"src_override": SRC})
    cmd = cmd.replace('-Iinclude', '-Itmp/func_80034708/integ/include -Iinclude', 1)
    assert (' -G8 ' in cmd) == (flag == '-G8'), cmd
    r = subprocess.run(['bash', '-o', 'pipefail', '-c', cmd], capture_output=True, text=True)
    assert r.returncode == 0, r.stderr[-800:]
    dis = subprocess.run(['mipsel-linux-gnu-objdump', '-dr', o], capture_output=True, text=True).stdout
    open(f'{OUT}/b3{flag}.dis', 'w').write(dis)
    lines = dis.split('\n')
    rows = []
    for i, l in enumerate(lines):
        m = re.match(r'\s+([0-9a-f]+):\s+(R_MIPS_\w+)\s+(\S+)', l)
        if m and m.group(3) in SYMS:
            ins = lines[i - 1]
            mi = re.match(r'\s*([0-9a-f]+):\s+([0-9a-f]{8})\s+(.*)', ins)
            rows.append((m.group(3), mi.group(2), re.sub(r'\s+', ' ', mi.group(3)).strip(), m.group(2)))
    res[flag] = rows
for s in SYMS:
    a = [r[1:] for r in res['-G0'] if r[0] == s]
    b = [r[1:] for r in res['-G8'] if r[0] == s]
    print(f'{s}: -G0 {len(a)} relocated insns, -G8 {len(b)}, identical={a == b}')
    for x in a:
        print('   ', x)
json.dump(res, open(f'{OUT}/q8.json', 'w'), indent=1)
