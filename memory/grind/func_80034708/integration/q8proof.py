"""Q8 screening-scope proof (compiler-flags-canonical.md § "Screening scope", c8f0f7e42).
Builds the landing's code6cac_b3.c through the full per-file pipeline (cc1, prologue_fix, maspsx,
rodata-align sed, multu_pad, as) twice -- as a -G8 TU and as a -G0 TU, every other flag/list as in
the landing -- and lists, side by side, every instruction carrying a relocation against each small
extern (offset, raw word, relocation type/symbol; REL addends live in the raw word).
Writes integration/q8/q8_proof.md (+ both disassemblies, both command lines)."""
import subprocess, sys, re, os, json
sys.path.insert(0, '.')
from engine import pipeline
import engine.buildconfig as cfg
SYMS = {'D_800A37B8': 's32, 4 bytes', 'D_800A3690': 'u8, 1 byte', 'D_800A36F9': 'u8, 1 byte'}
SRC = 'tmp/func_80034708/integ/src/code6cac_b3.c'
OUT = 'tmp/func_80034708/q8'
os.makedirs(OUT, exist_ok=True)
cfg.EXPAND_LB_FILES.add('code6cac_b3'); cfg.RODATA_ALIGN2_FILES.add('code6cac_b3')
res, cmds = {}, {}
for flag in ('-G8', '-G0'):
    if flag == '-G8':
        cfg.GP_FILES.add('code6cac_b3')
    else:
        cfg.GP_FILES.discard('code6cac_b3')
    o = f'{OUT}/b3{flag}.o'
    cmd = pipeline.c_pipeline_cmd('code6cac_b3', o, {"src_override": SRC})
    cmd = cmd.replace('-Iinclude', '-Itmp/func_80034708/integ/include -Iinclude', 1)
    assert (' -G8 ' in cmd) == (flag == '-G8')
    cmds[flag] = cmd
    r = subprocess.run(['bash', '-o', 'pipefail', '-c', cmd], capture_output=True, text=True)
    assert r.returncode == 0, r.stderr[-800:]
    dis = subprocess.run(['mipsel-linux-gnu-objdump', '-dr', o], capture_output=True, text=True).stdout
    open(f'{OUT}/b3{flag}.dis', 'w').write(dis)
    lines = dis.split('\n')
    rows = []
    for i, l in enumerate(lines):
        m = re.match(r'\s+([0-9a-f]+):\s+(R_MIPS_\w+)\s+(\S+)', l)
        if m and m.group(3) in SYMS:
            mi = re.match(r'\s*([0-9a-f]+):\s+([0-9a-f]{8})\s+(.*)', lines[i - 1])
            rows.append({'sym': m.group(3), 'off': m.group(1), 'word': mi.group(2),
                         'insn': re.sub(r'\s+', ' ', mi.group(3)).strip(), 'reloc': m.group(2)})
    res[flag] = rows
md = ['# Q8 proof — func_80034708 / src/code6cac_b3.c (-G8 vs -G0, full per-file pipeline)', '',
      '## Build command lines', '']
for f in ('-G8', '-G0'):
    md += [f'{f}:', '```', cmds[f], '```', '']
ok_all = True
for s, size in SYMS.items():
    a = [r for r in res['-G8'] if r['sym'] == s]
    b = [r for r in res['-G0'] if r['sym'] == s]
    same = len(a) == len(b) and all((x['word'], x['reloc']) == (y['word'], y['reloc'])
                                     for x, y in zip(a, b))
    ok_all &= same
    md += [f'## {s} ({size}) — {len(a)} instructions at -G8, {len(b)} at -G0, identical: {same}', '',
           '| -G8 offset | -G8 word | -G8 reloc | -G0 offset | -G0 word | -G0 reloc | insn |',
           '|---|---|---|---|---|---|---|']
    for x, y in zip(a, b):
        md.append(f"| {x['off']} | {x['word']} | {x['reloc']} {s} | {y['off']} | {y['word']} | {y['reloc']} {s} | `{x['insn']}` |")
    md.append('')
open(f'{OUT}/q8_proof.md', 'w', newline='\n').write('\n'.join(md))
json.dump({'rows': res, 'cmds': cmds}, open(f'{OUT}/q8.json', 'w'), indent=1)
print('all identical:', ok_all)
for s in SYMS:
    print(s, sum(r['sym'] == s for r in res['-G8']), sum(r['sym'] == s for r in res['-G0']))
