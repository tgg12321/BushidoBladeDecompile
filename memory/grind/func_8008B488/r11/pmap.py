#!/usr/bin/env python3
"""Map func_8008B488's ADSR-value pseudos from a dump dir (tmp/rtl/<tag>): every
(set (reg/v:HI N) ...) in f.lreg classified as a load of attr->ar/dr/sr/rr/sl
(offset 0x30..0x38 off the attr pointer) or a clamp constant; then flow stats,
local-alloc and global-alloc dispositions, and the ALLOCDBG line.
Usage: python3 pmap.py tmp/rtl/<tag> [tmp/rtl/<dbgtag>]"""
import re, sys
d = sys.argv[1]
dbg = sys.argv[2] if len(sys.argv) > 2 else None
t = open(d + '/f.lreg').read()
fl = open(d + '/f.flow').read()
g = open(d + '/f.greg').read()
FIELD = {48: 'attr->ar', 50: 'attr->dr', 52: 'attr->sr', 54: 'attr->rr', 56: 'attr->sl'}
ins = re.split(r'\n(?=\((?:insn|jump_insn|call_insn|code_label|note|barrier) )', t)
rows = {}
for b in ins:
    m = re.match(r'\((\w+) (\d+)', b)
    if not m:
        continue
    uid = m.group(2)
    s = re.search(r'\(set \(reg/v:HI (\d+)\)\s*(.*)', b, re.S)
    if not s:
        continue
    r, rhs = s.group(1), ' '.join(s.group(2).split())
    kind = None
    mm = re.match(r'\(mem/s:HI \(plus:SI \(reg/v:SI \d+\) \(const_int (\d+)\)\)', rhs)
    if mm and int(mm.group(1)) in FIELD:
        kind = 'load ' + FIELD[int(mm.group(1))]
    elif re.match(r'\(const_int (\d+)\)', rhs):
        kind = 'const ' + hex(int(re.match(r'\(const_int (\d+)\)', rhs).group(1)))
    if kind:
        rows.setdefault(r, []).append('insn %s: %s' % (uid, kind))
alloc = open(dbg + '/stderr.txt').read() if dbg else ''
for r in sorted(rows, key=int):
    if not any('load' in x for x in rows[r]):
        continue
    st = re.search(r'^Register %s .*$' % r, fl, re.M)
    dl = re.search(r';; Register %s in -?\d+\.' % r, t)
    gg = re.search(r'(?m)(?:^| )%s in (-?\d+)' % r,g.split(';; Register dispositions')[-1] if ';; Register dispositions' in g else '')
    print('pseudo %s: %s' % (r, '; '.join(rows[r])))
    print('   flow: %s' % (st.group(0) if st else '-'))
    print('   local-alloc: %s' % (dl.group(0) if dl else '(none: not a local-alloc quantity)'))
    print('   global-alloc disposition: %s' % ('%s in %s' % (r, gg.group(1)) if gg else '-'))
    a = re.search(r'ALLOCDBG func=func_8008B488 ord=\d+ pseudo=%s .*' % r, alloc)
    if a:
        print('   ' + a.group(0))
