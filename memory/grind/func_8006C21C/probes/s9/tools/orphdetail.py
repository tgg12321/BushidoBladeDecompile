#!/usr/bin/env python3
"""usage: orphdetail.py <stem> <func> <orphan_reg> : show the flow-dump chain around an orphan
(the orphan's setter, its consumer, and the consumer's consumer) + the combine result."""
import re, sys
stem, fn, reg = sys.argv[1:4]
def sec(path):
    t = open(path).read()
    m = re.search(r'\n;; Function ' + re.escape(fn) + r'\n(.*?)(?=\n;; Function |\Z)', t, re.S)
    return re.split(r'\n(?=\((?:insn|jump_insn|call_insn|code_label|note|barrier) )', m.group(1))
def one(i):
    return ' '.join(i.split())
flow = sec(f'tmp/c21c9/census/{stem}.i.flow')
comb = sec(f'tmp/c21c9/census/{stem}.i.combine')
regs = [reg]
seen = set()
for _ in range(3):
    new = []
    for r in regs:
        for i in flow:
            if re.search(r'\(reg(?:/\w)?:\w+ %s\)' % r, i) and i not in seen:
                seen.add(i)
                m = re.match(r'\((?:insn|jump_insn|call_insn) \d+ \d+ \d+ \(set \(reg(?:/\w)?:\w+ (\d+)\)', one(i))
                if m and m.group(1) != r:
                    new.append(m.group(1))
    regs = new
uids = sorted(int(re.match(r'\(\w+ (\d+)', one(i)).group(1)) for i in seen)
print('--- flow')
for i in flow:
    m = re.match(r'\(\w+ (\d+)', one(i))
    if m and int(m.group(1)) in uids:
        print(one(i)[:400])
print('--- combine (same uids + orphan use)')
for i in comb:
    m = re.match(r'\(\w+ (\d+)', one(i))
    if m and (int(m.group(1)) in uids or re.search(r'\(use \(reg:SI %s\)' % reg, one(i))):
        print(one(i)[:400])
