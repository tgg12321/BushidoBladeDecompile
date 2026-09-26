"""Normalize cc1psx objdump (psx_obj.txt, with relocs) and the target (target.txt) and diff them.
Relocated operands are rendered as <reloc:SYMBOL[+addend]> on both sides; branch/jump targets masked.
usage: python3 cmp.py <dir>   -> <dir>/psx_vs_target.diff, prints summary"""
import re, sys, difflib
D = sys.argv[1]
REG = {'$fp': 's8', '$s8': 's8'}
def nnum(tok):
    def h(m):
        return str(int(m.group(0), 16) if m.group(0).lower().startswith(('0x', '-0x')) else int(m.group(0)))
    return re.sub(r'-?0x[0-9A-Fa-f]+|-?\b\d+\b', h, tok)
SYMADDR = {}
for l in open('undefined_syms_auto.txt'):
    m = re.match(r'(\w+)\s*=\s*0x([0-9A-Fa-f]+)', l)
    if m: SYMADDR[m.group(1)] = int(m.group(2), 16)
def tgt():
    out = []
    for l in open(D + '/target.txt'):
        l = l.strip()
        if not l: continue
        op, _, rest = l.partition(' ')
        rest = rest.replace('$', '')
        rest = re.sub(r'%gp_rel\((\w+)\)\(gp\)', r'<\1>(gp)', rest)
        rest = re.sub(r'%hi\((\w+)\)', r'<hi:\1>', rest)
        rest = re.sub(r'%lo\((\w+)\)', r'<lo:\1>', rest)
        rest = rest.replace('fp', 's8')
        if op in ('j', 'jal') and not rest.startswith('<'): rest = rest if op == 'jal' else 'L'
        if op.startswith('b'): rest = re.sub(r'\.?L\w+$', 'L', rest)
        out.append(op + ' ' + nnum(rest.replace(', ', ',')))
    return out
def psx():
    out = []
    lines = [l.rstrip('\n') for l in open(D + '/psx_obj.txt')]
    i = 0
    while i < len(lines):
        l = lines[i].strip()
        i += 1
        if not l or 'R_MIPS' in l: continue
        op, _, rest = l.partition('\t')
        rel = None
        if i < len(lines) and 'R_MIPS' in lines[i]:
            rel = lines[i].split()[-1]; kind = lines[i].split()[1]
            i += 1
        if rel:
            sym, add = (rel.split('+') + ['0'])[:2]
            name = sym + ('+' + str(int(add, 16)) if add != '0' else '')
            if kind == 'R_MIPS_GPREL16': rest = re.sub(r'^([^,]+),[^,(]+\(gp\)$', r'\1,<%s>(gp)' % name, rest)
            elif kind == 'R_MIPS_HI16': rest = re.sub(r',0x[0-9a-f]+$|,\d+$', ',<hi:%s>' % name, rest)
            elif kind == 'R_MIPS_LO16':
                # REL relocation: the addend is the instruction's own immediate
                mi = re.search(r'(-?0x[0-9a-f]+|-?\d+)(\(\w+\))?$', rest)
                imm = int(mi.group(1), 0)
                if imm: name = sym + '+' + str(imm)
                rest = re.sub(r'(-?0x[0-9a-f]+|-?\d+)(\(\w+\))?$', lambda m: '<lo:%s>' % name + (m.group(2) or ''), rest)
            elif kind == 'R_MIPS_26': rest = sym
        if op.startswith('b'): rest = re.sub(r',?[0-9a-f]+ <[^>]+>$', '', rest); rest = (rest + ',L') if rest else 'L'
        if op == 'j': rest = 'L'
        out.append(op + ' ' + nnum(rest))
    return out
def canon(lst):
    # resolve <hi:SYM+N>/<lo:SYM+N> to absolute addresses so D_8009BA00+80 == D_8009BA50
    res = []
    for s in lst:
        def f(m):
            sym, _, add = m.group(2).partition('+')
            a = SYMADDR.get(sym)
            if a is None:
                mm = re.match(r'D_([0-9A-F]{8})$', sym)
                a = int(mm.group(1), 16) if mm else None
            if a is None:
                return '<%s:%s>' % (m.group(1), m.group(2))
            a += int(add or 0)
            # hi16 is compared as the emitted high half (objdump shows REL hi16 relocs without the addend)
            return '<hi:%s>' % hex((a + 0x8000) >> 16) if m.group(1) == 'hi' else '<lo:%s>' % hex(a)
        res.append(re.sub(r'<(hi|lo):([\w+]+)>', f, s))
    return res
t, p = canon(tgt()), canon(psx())
t = [x.replace('s8', 'fp') for x in t]; p = [x.replace('s8', 'fp').replace('sll zero,zero,0', 'nop').strip() for x in p]
t = [x.strip() for x in t]
def focus(lst):
    def hit(i, x):
        if '0x8009ba' in x or ('fp' in x and not re.match(r'(sw|lw) fp,\d+\(sp\)', x)):
            return True
        # the lui paired with a following table lo16
        return x.startswith('lui') and i + 1 < len(lst) and '0x8009ba' in lst[i + 1]
    return ['%3d  %s' % (i, x) for i, x in enumerate(lst) if hit(i, x)]
with open(D + '/focus.txt', 'w') as fo:
    NL = chr(10)
    fo.write('## target (asm/funcs/func_800620B8.s): table-address instructions (index = insn number)' + NL + NL.join(focus(t)) + NL)
    fo.write('## cc1psx (%s)' % D.split('/')[-1] + NL + NL.join(focus(p)) + NL)
    fo.write('## frame: target %s | cc1psx %s' % (t[0], p[0]) + NL)
d = list(difflib.unified_diff(t, p, 'target', 'cc1psx', lineterm='', n=1))
open(D + '/psx_vs_target.diff', 'w').write('\n'.join(d) + '\n')
sm = difflib.SequenceMatcher(None, t, p, autojunk=False)
print(D, 'target', len(t), 'cc1psx', len(p), 'equal lines', sum(b.size for b in sm.get_matching_blocks()), 'diff lines', len(d))
