#!/usr/bin/env python3
"""Map func_800290B8 user-variable pseudos from the .lreg dump: writes of const / self-increment /
i&1 / i>>1, plus flow stats and dispositions.  Usage: python3 pmap.py tmp/rtl/<tag>"""
import re, sys
d = sys.argv[1]
t = open(d + '/f.lreg').read()
fl = open(d + '/f.flow').read()
g = open(d + '/f.greg').read()
ins = re.split(r'\n(?=\((?:insn|jump_insn|call_insn|code_label|note|barrier) )', t)
rows = {}
for b in ins:
    m = re.match(r'\((\w+) (\d+)', b)
    if not m: continue
    uid = m.group(2)
    s = re.search(r'\(set \(reg/v:SI (\d+)\)\s*(.*)', b, re.S)
    if not s: continue
    r, rhs = s.group(1), ' '.join(s.group(2).split())
    kind = None
    if re.match(r'\(const_int (-?\d+)\)', rhs): kind = 'const ' + re.match(r'\(const_int (-?\d+)\)', rhs).group(1)
    elif re.match(r'\(plus:SI \(reg/v:SI %s\) \(const_int 1\)\)' % r, rhs): kind = 'incr'
    elif rhs.startswith('(and:SI (reg/v:SI 80) (const_int 1))'): kind = 'i & 1'
    elif rhs.startswith('(ashiftrt:SI'): kind = 'i / 2 (ashiftrt)'
    if kind: rows.setdefault(r, []).append('insn %s: %s' % (uid, kind))
for r in sorted(rows, key=int):
    st = re.search(r'^Register %s .*$' % r, fl, re.M)
    dl = re.search(r';; Register %s in -?\d+\.' % r, t) or re.search(r';; Register %s in -?\d+\.' % r, g)
    print('pseudo %s: %s' % (r, '; '.join(rows[r])))
    print('   flow: %s' % (st.group(0) if st else '-'))
    print('   disp: %s' % (dl.group(0) if dl else '(none in f.lreg: not allocated by local-alloc; see the f.greg dispositions below)'))
