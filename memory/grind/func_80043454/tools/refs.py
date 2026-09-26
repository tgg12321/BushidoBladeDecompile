"""refs.py <dump.lreg> <regno> [...] : per-insn weighted refs of pseudos (depth via LOOP notes)."""
import sys, re
path, regs = sys.argv[1], [int(x) for x in sys.argv[2:]]
fn = 'func_80043454'
lines, on = [], False
for l in open(path, encoding='utf-8', errors='replace'):
    if l.startswith(';; Function '):
        on = fn in l
    if on:
        lines.append(l)
# split into top-level rtx chunks
chunks, cur = [], []
for l in lines:
    if l.startswith('(') and cur:
        chunks.append(''.join(cur)); cur = []
    if l.startswith('(') or cur:
        cur.append(l)
if cur:
    chunks.append(''.join(cur))
depth = 1
tot = {r: 0 for r in regs}
for c in chunks:
    if 'NOTE_INSN_LOOP_BEG' in c:
        depth += 1; continue
    if 'NOTE_INSN_LOOP_END' in c:
        depth -= 1; continue
    head = c.split('\n')[0]
    m = re.match(r'\((\w+) (\d+)', head)
    if not m or m.group(1) not in ('insn', 'jump_insn', 'call_insn'):
        continue
    # strip REG_ notes: cut at the first 'expr_list:REG_' / 'insn_list:REG_'
    body = re.split(r'\((?:expr_list|insn_list):REG_', c)[0]
    for r in regs:
        n = len(re.findall(r'\(reg(?:/\w)*:\w+ %d\)' % r, body))
        if n:
            tot[r] += n * depth
            print('r%d uid %s depth %d x%d  %s' % (r, m.group(2), depth, n, ' '.join(body.split())[:110]))
print(tot)
