"""Score a candidate body for func_80034708 with the engine scorer, optionally compiled -G8.
usage: python3 tmp/func_80034708/gscore.py <cand.c> [--g8] [--diff] [--src-extra file]
Diagnostic only (the sandbox is the official instrument).
"""
import argparse, subprocess, sys, os, json
sys.path.insert(0, '.')
from engine import pipeline, score, inlineasm, cheats

FUNC = 'func_80034708'
ap = argparse.ArgumentParser()
ap.add_argument('cand')
ap.add_argument('--g8', action='store_true')
ap.add_argument('--diff', action='store_true')
ap.add_argument('--tag', default='gs')
a = ap.parse_args()
wd = f'tmp/func_80034708/{a.tag}'
os.makedirs(wd + '/src', exist_ok=True)
base = open('src/code6cac_b.c', encoding='utf-8').read()
body = open(a.cand, encoding='utf-8').read()
txt = inlineasm.substitute_body(base, FUNC, body)
srcp = wd + '/src/code6cac_b.c'
open(srcp, 'w', encoding='utf-8', newline='\n').write(txt)
ov = cheats.empty_overrides(wd + '/cfg')
ov['src_override'] = srcp
out = wd + '/code6cac_b.o'
cmd = pipeline.c_pipeline_cmd('code6cac_b', out, ov)
if a.g8:
    cmd = cmd.replace(' -G0 ', ' -G8 ', 1)
r = subprocess.run(['bash', '-o', 'pipefail', '-c', cmd], capture_output=True, text=True)
if r.returncode:
    sys.stderr.write(r.stderr[-2000:]); sys.exit(1)
s = score.score_func(out, 'build/src/code6cac_b.o', FUNC)
print(json.dumps({k: s[k] for k in s if k in ('score', 'target_insns', 'build_insns')}))
if a.diff:
    d = score.insn_diff(out, 'build/src/code6cac_b.o', FUNC)
    for h in d.get('hunks', []):
        if h.get('class') == 'not-scored':
            continue
        print('@', h['tag'], h['class'], h['target_at'], h['build_at'])
        for l in h.get('target', []):
            print('   T ', l)
        for l in h.get('built', []):
            print('   O ', l)
