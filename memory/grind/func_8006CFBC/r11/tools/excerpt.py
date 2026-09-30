import os, re, sys
D = os.path.dirname(os.path.abspath(__file__))
out = []
for tag, regs in (('c1', ['79', '92', '189', '221']), ('split3', ['85', '92', '175', '190', '202', '223'])):
    d = os.path.join(D, 'dump_' + tag)
    out.append('=' * 70)
    out.append('spelling %s (%s)' % (tag, {'c1': 'reuse: one `temp`, three writes',
                                          'split3': 'one-variable-per-value: cells1/cells2/cells3'}[tag]))
    sched = open(os.path.join(d, 'f.i.sched.fn')).read()
    adds = re.findall(r'\(insn (\d+) \d+ \d+ \(set \((reg[^)]*)\)\s*\(plus:SI \(reg:SI (\d+)\)\s*\(const_int 12\)\)\)', sched)
    out.append('.sched: the three `+ 12` adds (uid, dest, src) : %s' % adds[:3])
    # insn order after sched1 in each add's block: list insn uids in the stretch around it
    uids = re.findall(r'^\((insn|jump_insn|call_insn|code_label|note) (\d+) ', sched, re.M)
    order = [u for k, u in uids]
    for a in adds[:3]:
        i = order.index(a[0])
        out.append('  .sched insn order around add %s: %s' % (a[0], ' '.join(order[max(0, i - 3):i + 3])))
    for uid in [a[0] for a in adds[:3]]:
        m = re.search(r'\n\(insn %s [^\n]*\n(?:[^\n]+\n)*?\n' % uid, sched)
    lreg = open(os.path.join(d, 'f.i.lreg.fn')).read()
    for r in regs:
        m = re.search(r'^Register %s [^\n]*' % r, lreg, re.M)
        out.append('  .lreg: ' + (m.group(0) if m else 'Register %s: (no line)' % r))
    greg = open(os.path.join(d, 'f.i.greg.fn')).read()
    disp = greg[greg.index(';; Register dispositions'):]
    disp = disp[:disp.index('\n\n')]
    got = re.findall(r'\b(\d+) in (\d+)', disp)
    out.append('  .greg dispositions: ' + ', '.join('%s in $%s' % (a, b) for a, b in got if a in regs))
    s = open(os.path.join(D, 'sdbg_%s.txt' % tag)).read()
    for a in adds[:3]:
        m = re.search(r'SCHEDDBG ADJPRI insn=%s [^\n]*' % a[0], s)
        out.append('  sched1 ' + (m.group(0) if m else 'no ADJPRI line for %s' % a[0]))
open(os.path.join(D, 'excerpts.txt'), 'w', newline='\n').write('\n'.join(out) + '\n')
print('\n'.join(out))
