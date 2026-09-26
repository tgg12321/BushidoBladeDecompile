# Scan the ORIGINAL executable(s) for base+offset addressing into the sprite-record region.
# Tracks registers loaded with lui/addiu (or lui + lo-offset loads) that point into [LO, HI),
# then reports derived addresses (addiu rd, rs, imm / load/store imm(rs)) and which splat label
# each lands in. Also scans data words for pointers into the region.
import struct, re, sys, bisect
LO, HI = 0x8009B7F0, 0x8009BA60
labels = []
for line in open('asm/data/7D920.data.s', errors='replace'):
    m = re.match(r'dlabel D_([0-9A-F]{8})', line)
    if m:
        labels.append(int(m.group(1), 16))
labels.sort()
def lab(a):
    i = bisect.bisect_right(labels, a) - 1
    return labels[i] if i >= 0 else None
def scan(path, base, textlo, texthi, name):
    b = open(path, 'rb').read()[0x800:]
    n = len(b) // 4
    words = struct.unpack('<%dI' % n, b[:n * 4])
    hits = []
    # data pointers
    for i, w in enumerate(words):
        if LO <= w < HI:
            hits.append(('dataword', base + 4 * i, w))
    regs = {}
    for i, w in enumerate(words):
        pc = base + 4 * i
        if not (textlo <= pc < texthi):
            continue
        op = w >> 26; rs = (w >> 21) & 31; rt = (w >> 16) & 31; rd = (w >> 11) & 31
        imm = w & 0xFFFF; simm = imm - 0x10000 if imm & 0x8000 else imm
        # control flow: forget everything at jr ra
        if op == 0 and (w & 0x3F) == 8 and rs == 31:
            regs = {}
            continue
        if op == 0x0F:  # lui
            regs[rt] = ('hi', imm << 16, pc)
            continue
        if op in (0x09, 0x0D) and rs in regs:  # addiu / ori
            kind, val, spc = regs[rs]
            if op == 0x09:
                nv = (val + simm) & 0xFFFFFFFF
            else:
                nv = val | imm
            if kind == 'hi':
                if LO <= nv < HI:
                    regs[rt] = ('addr', nv, pc)
                    hits.append(('form', pc, nv, rt))
                else:
                    regs.pop(rt, None)
            else:
                if op == 0x09:
                    hits.append(('derive', pc, val, nv, rs, rt))
                    regs[rt] = ('addr', nv, pc)
                else:
                    regs.pop(rt, None)
            continue
        if op in (0x20, 0x21, 0x23, 0x24, 0x25, 0x28, 0x29, 0x2B) and rs in regs:  # loads/stores
            kind, val, spc = regs[rs]
            a = (val + simm) & 0xFFFFFFFF
            if kind == 'hi':
                if LO <= a < HI:
                    hits.append(('direct', pc, a))
            else:
                hits.append(('access', pc, val, a))
            if op in (0x20, 0x21, 0x23, 0x24, 0x25):
                regs.pop(rt, None)
            continue
        # any other write to a reg kills tracking
        if op == 0:
            regs.pop(rd, None)
        elif op in (0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F) or 0x20 <= op <= 0x26:
            regs.pop(rt, None)
        elif op == 3:
            for r in (2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 24, 25, 31):
                regs.pop(r, None)
    for h in hits:
        if h[0] == 'dataword':
            print(name, 'DATAWORD at %08X -> %08X (label D_%08X)' % (h[1], h[2], lab(h[2])))
        elif h[0] == 'form':
            print(name, 'FORM %08X: $%d=%08X' % (h[1], h[3], h[2]))
        elif h[0] == 'derive':
            _, pc, v, nv, rs, rt = h
            flag = '  CROSS' if lab(v) != lab(nv) else ''
            print(name, 'DERIVE %08X: $%d=%08X -> $%d=%08X (D_%08X -> D_%08X)%s' % (pc, rs, v, rt, nv, lab(v), lab(nv) or 0, flag))
        elif h[0] == 'access':
            _, pc, v, a = h
            flag = '  CROSS' if lab(v) != lab(a) else ''
            print(name, 'ACCESS %08X: base %08X +%d -> %08X (D_%08X -> D_%08X)%s' % (pc, v, a - v, a, lab(v), lab(a) or 0, flag))
        elif h[0] == 'direct':
            print(name, 'DIRECT %08X: %08X (D_%08X)' % (h[1], h[2], lab(h[2])))
scan('disc/SLUS_006.63', 0x80010000, 0x80010000, 0x8008D080, 'MAIN')
scan('disc/STR/MOVOVL.EXE', 0x801D8800, 0x801D8800, 0x801F6800, 'OVL')
