import subprocess, sys, re
sys.path.insert(0, '.')
from engine import pipeline
d = 'tmp/func_800720FC/split'
for stem in ('text1b', 'text1b_mid'):
    pipeline.build_c_object('text1b', f'{d}/{stem}.o', cheat_overrides={'src_override': f'{d}/{stem}.c'})
def funcs(o):
    r = subprocess.run(['mipsel-linux-gnu-objdump', '-d', '-r', '--no-show-raw-insn', '-j', '.text', o], capture_output=True, text=True).stdout
    out = {}; cur = None
    for l in r.splitlines():
        m = re.match(r'^[0-9a-f]+ <(\w+)>:', l)
        if m: cur = m.group(1); out[cur] = []; continue
        if cur and l.strip():
            l = re.sub(r'^\s*[0-9a-f]+:\s*', '', l)
            l = re.sub(r'\b[0-9a-f]{4,}\b', 'X', l)   # addresses/offsets of branch targets
            l = re.sub(r'<[^>]*>', '<>', l)
            out[cur].append(l)
    return out
orig = funcs('build/src/text1b.o')
a = funcs(f'{d}/text1b.o'); b = funcs(f'{d}/text1b_mid.o')
bad = 0
for name, body in list(a.items()) + list(b.items()):
    if name not in orig: print('NEW SYMBOL', name); bad += 1; continue
    if orig[name] != body:
        bad += 1; print('DIFF', name, len(orig[name]), len(body))
missing = set(orig) - set(a) - set(b)
print('missing', sorted(missing)[:10], 'diffs', bad, 'pre', len(a), 'post', len(b), 'orig', len(orig))
for o in (f'{d}/text1b.o', f'{d}/text1b_mid.o', 'build/src/text1b.o'):
    print(subprocess.run(['sh','-c',f'mipsel-linux-gnu-objdump -h {o} | grep -E " .text| .rodata| .data| .bss"'],capture_output=True,text=True).stdout)
import difflib
for n in ('func_8006F97C','func_80070188','func_800747D8','func_80077374'):
    body = a.get(n) or b.get(n)
    print('==', n)
    for l in list(difflib.unified_diff(orig[n], body, lineterm='', n=0))[2:12]: print(l)
