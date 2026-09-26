"""Scratch integration builder: build a set of TUs from tmp/func_80034708/integ/src (falling back to
src/) with the header dir tmp/func_80034708/integ/include searched before include/, then objdiff each
against build/src/<stem>.o.
usage: python3 tmp/func_80034708/integ.py stem[:g8] [stem ...]
"""
import subprocess, sys, os
sys.path.insert(0, '.')
from engine import pipeline
import engine.buildconfig as cfg
if os.environ.get('RELAND'):
    cfg.GP_FILES.add('code6cac_b3')
    for st in ('code6cac_b3', 'code6cac_b3_post'):
        cfg.EXPAND_LB_FILES.add(st); cfg.RODATA_ALIGN2_FILES.add(st)

ROOT = 'tmp/func_80034708/integ'
for arg in sys.argv[1:]:
    stem, _, flag = arg.partition(':')
    src = f'{ROOT}/src/{stem}.c'
    if not os.path.exists(src):
        src = f'src/{stem}.c'
    out = f'{ROOT}/build/{stem}.o'
    os.makedirs(os.path.dirname(out), exist_ok=True)
    cmd = pipeline.c_pipeline_cmd(stem, out, {"src_override": src})
    cmd = cmd.replace('-Iinclude', f'-I{ROOT}/include -Iinclude', 1)
    if flag == 'g8':
        cmd = cmd.replace(' -G0 ', ' -G8 ', 1)
    r = subprocess.run(['bash', '-o', 'pipefail', '-c', cmd], capture_output=True, text=True)
    if r.returncode:
        print(stem, 'BUILD FAIL'); print(r.stderr[-3000:]); continue
    ref = f'build/src/{stem}.o'
    if os.path.exists(ref):
        d = subprocess.run(['python3', 'tools/objdiff.py', ref, out, '--summary'], capture_output=True, text=True)
        print(stem, d.stdout.strip().splitlines()[-1] if d.stdout.strip() else d.stderr[-500:])
        for l in d.stdout.splitlines():
            if 'CHANGED' in l or 'MISSING' in l.upper():
                print('   ', l.strip())
    else:
        print(stem, 'built (no reference)')
