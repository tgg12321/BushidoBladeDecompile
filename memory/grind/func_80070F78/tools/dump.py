"""RTL dumps of func_80070F78 for a candidate on a mini prefix.
usage: python3 tmp/func_80070F78/dump.py cand.c outname [--pre=path] [--instr] [passes...]"""
import sys, shlex, subprocess
sys.path.insert(0, '.')
from pathlib import Path
from engine import buildconfig as B
FUNC = 'func_80070F78'
args = [a for a in sys.argv[1:] if not a.startswith('--')]
pre = [a[6:] for a in sys.argv[1:] if a.startswith('--pre=')]
pre = pre[0] if pre else 'tmp/func_80070F78/mini/pre_rec.c'
instr = '--instr' in sys.argv
cand, name = args[0], args[1]
passes = args[2:] or ['sched', 'lreg', 'greg']
out = Path('tmp/func_80070F78/dumps'); out.mkdir(parents=True, exist_ok=True)
work = Path('tmp/func_80070F78/dwork'); work.mkdir(parents=True, exist_ok=True)
txt = Path(pre).read_text(encoding='utf-8') + '\n' + Path(cand).read_text(encoding='utf-8')
src = work / 'text1b.c'
src.write_bytes(txt.encode('utf-8'))
cc1 = 'tools/gcc-2.7.2/cc1' if instr else B.CC1
cpp = f"{B.CPP} {B.CPP_FLAGS} {B.CPP_DEFS} {src}"
p1 = subprocess.run(shlex.split(cpp), capture_output=True, text=True)
if p1.returncode:
    print(p1.stderr); sys.exit(1)
cc = f"{cc1} {B.CC_FLAGS} -da -dumpbase {work}/text1b -o {work}/text1b.s"
print(cc)
p2 = subprocess.run(shlex.split(cc), input=p1.stdout, capture_output=True, text=True)
if p2.returncode:
    print(p2.stderr[-3000:]); sys.exit(1)
if instr and p2.stderr:
    (out / f'{name}.instr').write_text(p2.stderr)
for p in passes:
    t = (work / f'text1b.{p}').read_text(errors='replace')
    i = t.find(f'\n;; Function {FUNC}\n')
    j = t.find('\n;; Function ', i + 10)
    (out / f'{name}.{p}').write_text(t[i:j if j > 0 else None])
s = (work / 'text1b.s').read_text().splitlines()
for i, l in enumerate(s):
    if l.startswith(f'{FUNC}:'):
        k = i
        while not s[k].startswith('\t.end'):
            k += 1
        (out / f'{name}.s').write_text('\n'.join(s[i:k + 1]) + '\n')
        for x in s[i:i + 4]:
            if '.frame' in x:
                print(name, x.strip())
        break
