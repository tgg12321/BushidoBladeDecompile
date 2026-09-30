"""Census of D_SEL.BIN sprite sheets: header count per sheet, per root-table slot.

Root (select-screen resource file, D_800A36A0+4 -> arg0[0]): u32 offsets. Slot +0xNN
holds the offset of a table of sheet offsets. A sheet is N 12-byte SprtHdrA headers
(tp0, tp1, count, pad | cx, cy | ubase, pad, vbase, pad) then 8-byte SprtEntA cells.

Header count is read two independent ways:
  * a 12-byte slot is header-shaped iff tp1 == 0, pad3 == 0, count >= 1 and the
    CLUT row cy is in VRAM's bottom band (>= 480, where every header's CLUT sits);
  * the cells that follow must be cell-shaped (w, h nonzero).
Prints, for each sheet: N, the cell-array offset N*12, and the first cell.
"""
import struct, sys

d = open("disc/TIM2D/D_SEL.BIN", "rb").read()
u32 = lambda o: struct.unpack_from("<I", d, o)[0]
root = [u32(4 * k) for k in range(16)]
starts = sorted(set(root))


def hdr_ok(o):
    tp0, tp1, cnt, p3, cx, cy = struct.unpack_from("<BBBBHH", d, o)
    return tp1 == 0 and p3 == 0 and cnt >= 1 and 480 <= cy < 512


def nhdr(s):
    n = 0
    while hdr_ok(s + 12 * n):
        n += 1
    return n


for slot in (0x14, 0x20, 0x24, 0x28, 0x2C, 0x30, 0x34):
    T = u32(slot)
    end = min([x for x in starts if x > T] + [len(d)])
    n = (end - T) // 4
    print(f"root+0x{slot:02X}: table @0x{T:X}, {n} entries")
    for k in range(n):
        s = u32(T + 4 * k)
        if s == 0 or s >= len(d):
            print(f"   [{k:2}] 0x{s:X} (not a sheet)")
            continue
        N = nhdr(s)
        cells = s + 12 * N
        x, y, uu, vv, w, h = struct.unpack_from("<hhBBBB", d, cells)
        counts = [d[s + 12 * i + 2] for i in range(N)]
        print(f"   [{k:2}] sheet @0x{s:X}: {N} header(s) counts={counts} "
              f"-> cells @+0x{12 * N:X}: first {{x={x},y={y},u={uu},v={vv},w={w},h={h}}}")
