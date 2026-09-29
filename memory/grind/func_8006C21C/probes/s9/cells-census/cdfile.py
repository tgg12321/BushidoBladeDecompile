#!/usr/bin/env python3
"""Resolve CD file number D_8008F12C[grp]+idx -> g_cd_file_table entry -> LBA/size -> disc file."""
import struct, sys, os
exe = open('disc/SLUS_006.63', 'rb').read()
va = lambda a: a - 0x80010000 + 0x800
grp, idx = int(sys.argv[1]), int(sys.argv[2])
base = struct.unpack_from('<h', exe, va(0x8008F12C) + 2 * grp)[0]
n = base + idx
m, s, f, x, size = struct.unpack_from('<BBBBI', exe, va(0x8008EC34) + 8 * n)
bcd = lambda b: (b >> 4) * 10 + (b & 15)
lba = (bcd(m) * 60 + bcd(s)) * 75 + bcd(f) - 150
print(f'group base D_8008F12C[{grp}]={base}; file #{n}: loc {m:02X}:{s:02X}:{f:02X} (lba {lba}) word2=0x{size:X} ({size})')
cands = []
for root, _, fs in os.walk('disc'):
    for fn in fs:
        p = os.path.join(root, fn)
        if os.path.getsize(p) == size:
            cands.append(p)
print('disc files with that exact size:', cands)
