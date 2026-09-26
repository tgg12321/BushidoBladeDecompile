"""vscore.py <variant-dir> [...]: compile <dir>/src/code6cac_b3.c (-G8, <dir>/include first) through the
real pipeline and score func_80034708 against build/src/code6cac_b.o. Prints score + scored hunks."""
import subprocess, sys, json
sys.path.insert(0, '.')
from engine import pipeline, score
for V in sys.argv[1:]:
    out = f'{V}/b3.o'
    cmd = pipeline.c_pipeline_cmd('code6cac_b3', out, {"src_override": f'{V}/src/code6cac_b3.c'})
    cmd = cmd.replace('-Iinclude', f'-I{V}/include -Iinclude', 1).replace(' -G0 ', ' -G8 ', 1)
    r = subprocess.run(['bash', '-o', 'pipefail', '-c', cmd], capture_output=True, text=True)
    if r.returncode:
        print(V, 'BUILD FAIL', r.stderr[-800:]); continue
    s = score.score_func(out, 'build/src/code6cac_b.o', 'func_80034708')
    print(V, json.dumps({k: s[k] for k in ('score', 'target_insns', 'build_insns')}))
    if '-v' in sys.argv[0:1] or len(sys.argv) == 2:
        d = score.insn_diff(out, 'build/src/code6cac_b.o', 'func_80034708')
        for h in d['hunks']:
            if h['class'] != 'not-scored':
                print('   @', h['class'], h['target'], h['built'])
