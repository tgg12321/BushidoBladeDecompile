#!/usr/bin/env python3
"""Ruling 9 (b) census for func_8006C21C's `cells = s.header + 0xC` sites.

Resource: disc/TIM2D/MOD.BIN = CD file #23 (func_80068F70 -> func_8006E950(2, buf) ->
func_80036EA8(2, 2) = D_8008F12C[2] + 2 = 21 + 2; g_cd_file_table[23] size 0xA0608 ==
MOD.BIN's size). The root (D_800A34FC->0x24, arg0[1] here) is the file start; header word
+0x30 (relocated by func_8006E440) points at the sheet table, whose entries are relocated by
func_8006919C (header words +0x14..+0x40). A sheet is N 12-byte SprtHdrA headers
(tp0, tp1, count, pad | cx, cy | ubase, pad, vbase, pad) followed by 8-byte SprtEntA cells
(x, y, u, v, w, h). Header count is read two independent ways, as in tmp/f759d0/sheet_census.py:
  * a 12-byte slot is header-shaped iff tp1 == 0, pad3 == 0, count >= 1, CLUT row cy in
    VRAM's bottom band (480..511);
  * the `count` cells after the headers must be cell-shaped (w, h nonzero) and the slot right
    after the first header must NOT be header-shaped when N == 1.
"""
import struct

d = open('disc/TIM2D/MOD.BIN', 'rb').read()
u32 = lambda o: struct.unpack_from('<I', d, o)[0]
TABLE = u32(0x30)


def hdr_ok(o):
    tp0, tp1, cnt, p3, cx, cy = struct.unpack_from('<BBBBHH', d, o)
    return tp1 == 0 and p3 == 0 and cnt >= 1 and 480 <= cy < 512


def nhdr(s):
    n = 0
    while hdr_ok(s + 12 * n):
        n += 1
    return n


SITES = [('phase 1 (table[0])', [0]),
         ('phase 2 inner (table[work + 13], work 0..3)', [13, 14, 15, 16]),
         ('phase 4 head (table[1])', [1]),
         ('phase 4 loop (table[work + 2], work 0..5)', [2, 3, 4, 5, 6, 7])]
print(f'MOD.BIN size 0x{len(d):X}; header +0x30 -> sheet table @0x{TABLE:X}')
print('| site | slot | sheet @ | headers N | counts | K = 12*N | first cell (x,y,u,v,w,h) | all count cells shaped |')
print('|---|---|---|---|---|---|---|---|')
bad = 0
for site, slots in SITES:
    for k in slots:
        s = u32(TABLE + 4 * k)
        N = nhdr(s)
        counts = [d[s + 12 * i + 2] for i in range(N)]
        cells = s + 12 * N
        first = struct.unpack_from('<hhBBBB', d, cells)
        total = sum(counts)
        shaped = all(struct.unpack_from('<hhBBBB', d, cells + 8 * c)[4] and
                     struct.unpack_from('<hhBBBB', d, cells + 8 * c)[5] for c in range(total))
        if N != 1 or not shaped:
            bad += 1
        print(f'| {site} | {k} | 0x{s:X} | {N} | {counts} | 0x{12 * N:X} | {first} | {shaped} |')
print('RESULT:', 'every reached sheet has exactly one header (K = 0xC holds)' if not bad
      else f'{bad} slot(s) break K = 0xC')
