import struct, sys
for path, base, off in [('disc/STR/MOVOVL.EXE', 0x801D8800, 0x800), ('disc/SLUS_006.63', 0x80010000, 0x800)]:
    d = open(path, 'rb').read()[off:]
    n = len(d) // 4
    w = struct.unpack('<%dI' % n, d[:n*4])
    hits = []
    for i, x in enumerate(w):
        if x >> 26 == 0x0F and (x & 0xFFFF) == 0x8010:
            rt = (x >> 16) & 31
            for j in range(i + 1, min(i + 12, n)):
                y = w[j]; op = y >> 26; rs = (y >> 21) & 31
                if rs == rt and op in (0x09, 0x20, 0x21, 0x23, 0x24, 0x25, 0x28, 0x29, 0x2B, 0x22, 0x26, 0x2A, 0x2E):
                    imm = y & 0xFFFF; imm = imm - 0x10000 if imm & 0x8000 else imm
                    a = 0x80100000 + imm
                    if 0x80101E58 <= a < 0x80101EC8:
                        hits.append((hex(base + 4*i), hex(a), op))
                    break
    print(path, len(hits))
    import collections
    c = collections.Counter(h[1] for h in hits)
    print(sorted(c.items()))
