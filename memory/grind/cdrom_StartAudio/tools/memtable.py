"""(a4')(4) member table for CdState D_80101E58 over 0x80101E58..0x80101E99 (owner ruling Q43).
The object is judged on two functions: cdrom_StartAudio (head 0x80101E58..62 by &E62-0xA, and the
E60<->E6C link by the Q2 (a1)/(a2) case) and func_80036940 (0x80101E6C..99 by &E8C-0x20 / &E98-0x2C).
Per member: every access by those two functions' ORIGINAL instructions (asm/funcs), including `la`
forms and the N(reg) / addiu-reg uses of that register; for a member neither touches (forced in), every
other function's original access (Q13/Q14 evidence).
usage (WSL, repo root): python3 memory/grind/cdrom_StartAudio/tools/memtable.py > memory/grind/cdrom_StartAudio/member_table.md"""
import re, glob
from pathlib import Path

LO, HI = 0x80101E58, 0x80101E9C          # object 0x80101E58..0x80101E99 + tail padding E9A..9B
JUDGED = ('cdrom_StartAudio', 'func_80036940')
MEMBERS = [  # (offset, width, type, C member)
    (0x00, 1, 'u8', 'file'), (0x01, 1, 'u8', 'chan'), (0x02, 2, None, '(alignment padding before unk04)'),
    (0x04, 4, 's32', 'unk04'),
    (0x08, 2, 's16', 'rec.unk00'), (0x0A, 2, 's16', 'rec.unk02'), (0x0C, 2, 's16', 'rec.unk04'),
    (0x0E, 2, 's16', 'rec.unk06'), (0x10, 2, 's16', 'rec.unk08'), (0x12, 2, 's16', 'rec.unk0A'),
    (0x14, 4, 's32', 'rec.pair.a'), (0x18, 4, 's32', 'rec.pair.b'),
    (0x1C, 4, 's32', 'rec.unk14'), (0x20, 4, 's32', 'rec.unk18'), (0x24, 4, 's32', 'rec.unk1C'),
    (0x28, 4, 's32', 'rec.sectors_remaining'), (0x2C, 4, 's32', 'rec.dest_buffer'),
    (0x30, 4, 's32', 'rec.unk28'), (0x34, 4, 's32', 'rec.unk2C'), (0x38, 1, 'u8', 'rec.unk30'),
    (0x39, 3, None, '(alignment padding)'), (0x3C, 4, 's32', 'rec.unk34'), (0x40, 2, 's16', 'rec.unk38'),
    (0x42, 2, None, '(inside the object, no member; func_80036140 asm only, via the D_80101E9A alias row)'),
]
SYMDEF = re.compile(r'^\s*(\w+)\s*=\s*0x([0-9A-Fa-f]+)\s*;')
INSN = re.compile(r'/\*\s*[0-9A-F]+\s+([0-9A-F]{8})\s+[0-9A-F]{8}\s*\*/\s+(\w+)\s+(.*)$')
LOREF = re.compile(r'%(?:hi|lo)\((\w+)(?:\s*\+\s*(0x[0-9A-Fa-f]+|\d+))?\)')

sym = {}
for f in ('undefined_syms_auto.txt', 'named_syms.txt', 'symbol_addrs.txt'):
    for l in open(f, encoding='utf-8', errors='replace'):
        m = SYMDEF.match(l)
        if m:
            sym.setdefault(m.group(1), int(m.group(2), 16))
# Shipped names whose rows retired with the CdState merge: splat D_<addr> names carry their address;
# the two census names are the 80036140 member table's (landing/member_table.md +0x28/+0x2C).
SHIPPED = {'g_cdread_sectors_remaining': 0x80101E80, 'g_cdread_dest_buffer': 0x80101E84}


def addr_of(name):
    if name in sym:
        return sym[name]
    if name in SHIPPED:
        return SHIPPED[name]
    m = re.fullmatch(r'D_([0-9A-F]{8})', name)
    return int(m.group(1), 16) if m else None


def imm(s):
    s = s.strip()
    return int(s, 16) if s.lstrip('-').startswith('0x') else int(s)


def accesses(path):
    """-> [(effective address, text)] for every access into [LO, HI)."""
    lines = [l.rstrip() for l in open(path)]
    out = []
    for i, l in enumerate(lines):
        m = INSN.search(l)
        if not m:
            continue
        addr, op, ops = m.groups()
        r = LOREF.search(ops)
        if not r or addr_of(r.group(1)) is None:
            continue
        ea = addr_of(r.group(1)) + (imm(r.group(2)) if r.group(2) else 0)
        if not (LO <= ea < HI):
            continue
        if op == 'lui':
            continue                      # the %lo half carries the access
        out.append((ea, f'{addr} {op} {ops.strip()}'))
        if op == 'addiu' and '%lo(' in ops:   # `la reg,sym`: follow uses of reg until it is redefined
            reg = ops.split(',')[0].strip()
            for l2 in lines[i + 1:i + 80]:
                m2 = INSN.search(l2)
                if not m2:
                    continue
                a2, op2, ops2 = m2.group(1), m2.group(2), m2.group(3).strip()
                mm = re.search(r'(-?0x[0-9A-Fa-f]+|-?\d+)\(' + re.escape(reg) + r'\)', ops2)
                if mm:
                    out.append((ea + imm(mm.group(1)), f'  {a2} {op2} {ops2}   [via {reg} = &0x{ea:08X}]'))
                ma = re.match(r'(\$\w+),\s*' + re.escape(reg) + r',\s*(-?0x[0-9A-Fa-f]+|-?\d+)$', ops2)
                if op2 == 'addiu' and ma:
                    out.append((ea + imm(ma.group(2)), f'  {a2} {op2} {ops2}   [via {reg} = &0x{ea:08X}; address formed]'))
                if re.match(re.escape(reg) + r',', ops2) and op2 not in ('sw', 'sh', 'sb'):
                    break
                if op2 in ('jr',) or (op2 == 'jal' and reg in ('$v0', '$v1', '$a0', '$a1', '$a2', '$a3', '$t0')):
                    break
    return out


acc = {Path(f).stem: accesses(f) for f in sorted(glob.glob('asm/funcs/*.s'))}
print('# (a4′)(4) member table — CdState D_80101E58, 0x80101E58..0x80101E99 (owner ruling Q43)')
print('# Judged functions: cdrom_StartAudio, func_80036940. Original instructions from asm/funcs/*.s;')
print('# "[via reg]" = an access through a register holding a CdState address (la base + offset).')
print('# Generated by memory/grind/cdrom_StartAudio/tools/memtable.py.\n')
for off, w, ty, name in MEMBERS:
    lo, hi = LO + off, LO + off + w
    head = f'## +0x{off:02X} 0x{lo:08X} width {w} ' + (f'`{ty} {name}`' if ty else name)
    print(head)
    own = [(f, t) for f in JUDGED for ea, t in acc.get(f, []) if lo <= ea < hi]
    other = [(f, t) for f in acc if f not in JUDGED for ea, t in acc[f] if lo <= ea < hi]
    if own:
        for f, t in own:
            print(f'    {f}: {t}')
    else:
        print('    NOT accessed by cdrom_StartAudio or func_80036940.')
    if other:
        print('  other functions:')
        for f, t in other:
            print(f'    {f}: {t}')
    print()
