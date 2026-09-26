"""Score permuter outputs with the header-model whole-file build (func_80055138 only).
usage (WSL, repo root): python3 tmp/func_80055138/permeval.py [maxscore]"""
import sys, os, glob, subprocess, re
lim = int(sys.argv[1]) if len(sys.argv) > 1 else 10**9
outs = []
for d in glob.glob('tmp/func_80055138/' + (sys.argv[2] if len(sys.argv) > 2 else 'perm') + '/output-*'):
    m = re.search(r'output-(\d+)-(\d+)$', d)
    if m and int(m.group(1)) <= lim:
        outs.append((int(m.group(1)), d))
for sc, d in sorted(outs):
    s = open(d + '/source.c').read()
    i = s.find('extern u8 D_8009A8C4')
    c = s[i:]
    p = 'tmp/func_80055138/pe_cand.c'
    open(p, 'w').write(c)
    r = subprocess.run(['python3', 'tmp/func_80055138/wf2.py', 'text1b', p, '--only'], capture_output=True, text=True)
    line = [l for l in r.stdout.splitlines() if 'func_80055138' in l or 'FAILED' in l]
    print(sc, d, line[:1] if line else r.stdout[-300:])
