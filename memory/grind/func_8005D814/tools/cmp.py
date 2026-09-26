#!/usr/bin/env python3
"""Side-by-side diff of target vs ours (branch targets masked, relocs dropped).
Mode arg: 'full' (operands) or 'op' (mnemonic only)."""
import re, sys, difflib
mode = sys.argv[1] if len(sys.argv) > 1 else 'full'
D = 'tmp/func_8005D814/'
ABI = {'s8': 'fp'}
def norm(l):
    l = l.split(None, 1)[1] if l[:3].strip().isdigit() else l
    rel = l.split(';')[1].strip() if ';' in l else ''
    l = l.split(';')[0].strip()
    if 'LO16' in rel:
        l = re.sub(r',-?\w+$', ',SYM', l) if '(' not in l else re.sub(r'-?\w+\(', 'SYM(', l)
    if 'HI16' in rel:
        l = re.sub(r',\w+$', ',SYM', l)
    if 'R_MIPS_26' in rel:
        l = l.split()[0] + ' ' + rel.split()[-1]
    l = re.sub(r'<[^>]*>', '', l)
    l = re.sub(r'\$', '', l)
    l = re.sub(r'\.L[0-9A-F]+', 'L', l)
    l = re.sub(r'%(hi|lo)\([^)]*\)', 'SYM', l)
    l = re.sub(r'\((0x[0-9a-fA-F]+) >> 16\)', lambda m: hex(int(m.group(1),16) >> 16), l)
    l = re.sub(r'\((0x[0-9a-fA-F]+) & 0xFFFF\)', lambda m: hex(int(m.group(1),16) & 0xFFFF), l)
    l = re.sub(r'%(hi|lo)\([^)]*\)\(', 'SYM(', l)
    parts = l.split(None, 1)
    op = parts[0]
    ops = parts[1] if len(parts) > 1 else ''
    ops = ops.replace(' ', '')
    ops = re.sub(r'\bs8\b', 'fp', ops)
    # normalize hex/dec immediates to dec
    ops = re.sub(r'-?0x[0-9a-fA-F]+', lambda m: str(int(m.group(0), 16)), ops)
    alias = {'addu': None}
    if op in ('beqz', 'bnez', 'beq', 'bne', 'j', 'b', 'bltz', 'bgez', 'blez', 'bgtz'):
        ops = ','.join(ops.split(',')[:-1]) if op != 'j' else ''
    if op == 'addu' and ops.endswith(',zero'):
        op, ops = 'move', ops[:-5]
    if op == 'addiu' and ',zero,' in ops:
        a = ops.split(','); op, ops = 'li', a[0] + ',' + a[2]
    if op == 'lui' and ops.endswith(',0'):
        ops = ops[:-2] + ',SYM'
    if op == 'lui':
        ops = ops.split(',')[0] + ',SYM' if not re.search(r',\d+$', ops) or True else ops
    if op == 'addiu' and ops.endswith(',0'):
        pass
    if mode == 'op':
        return op
    return op + ' ' + ops
def load(f):
    return [norm(l.rstrip('\n')) for l in open(D + f)]
t = load('target.txt'); o = load('ours.txt')
sm = difflib.SequenceMatcher(None, t, o, autojunk=False)
n = 0
for tag, i1, i2, j1, j2 in sm.get_opcodes():
    if tag == 'equal':
        continue
    n += max(i2 - i1, j2 - j1)
    print('@@ %s t[%d:%d] o[%d:%d]' % (tag, i1, i2, j1, j2))
    for k in range(i1, i2):
        print('  - %3d %s' % (k, t[k]))
    for k in range(j1, j2):
        print('  + %3d %s' % (k, o[k]))
print('diff lines', n, 'target', len(t), 'ours', len(o))
