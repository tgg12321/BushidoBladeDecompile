"""prio.py <dumpdir> : per-pseudo refs/live/priority + final hard reg for func_80043454.
Only pseudos that got a callee-saved reg or were spilled are interesting."""
import sys, re, math
d = sys.argv[1]
fn = 'func_80043454'
def section(path):
    out, on = [], False
    for l in open(path, encoding='utf-8', errors='replace'):
        if l.startswith(';; Function '):
            on = fn in l
        if on:
            out.append(l)
    return out
lreg = section(d + '/t.i.lreg')
greg = section(d + '/t.i.greg')
info = {}
for l in lreg:
    m = re.match(r'Register (\d+) used (\d+) times across (\d+) insns(.*)', l)
    if m:
        info[int(m.group(1))] = (int(m.group(2)), int(m.group(3)), m.group(4))
disp = {}
txt = ''.join(greg)
m = re.search(r';; Register dispositions:(.*?)\n\n', txt, re.S)
if m:
    for a, b in re.findall(r'(\d+) in (\d+)', m.group(1)):
        disp[int(a)] = int(b)
names = {16+i: 's%d' % i for i in range(8)}
names[30] = 'fp'
rows = []
for r, (refs, live, rest) in info.items():
    if 'crosses' not in rest and r not in disp:
        pass
    hr = disp.get(r)
    fl = int(math.log2(refs)) if refs > 0 else 0
    pr = fl * refs / live * 10000 if live else 0
    if 'crosses' in rest or hr is None:
        rows.append((pr, r, refs, live, names.get(hr, hr if hr is not None else 'MEM'), rest.strip()))
for row in sorted(rows, reverse=True):
    print('%10.0f  r%-4d refs=%-4d live=%-4d -> %-5s %s' % row)
