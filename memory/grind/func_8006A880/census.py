"""func_8006A880 Ruling 9 (b) census (manual s2, 2026-09-26).

Which disc file is the ctx[1] resource root, and how many 12-byte headers does every sheet
this function reaches have (so where its cells start)?

Sites (all K = 0xC; nibble = D_800A34F8 & 0xF, kept 0..7 by func_800693CC: +1 wraps at 8,
-1 wraps 0 -> 7):
  root+0x18 [0..7]      SPRT sheets (Phase A [0], row loop [0..6], Phase D [7]) -> func_8007352C
  root+0x40 [nibble]    SPRT sheets (Phase B)                                  -> func_8007352C
  root+0x24 [8+nibble]  SPRT sheets (Phase F)                                  -> func_8007352C
  root+0x24 [nibble]    POLY_FT4 sheets (Phase G)                              -> func_80073728
Header shapes (src/text1b.c): SprtHdrA {u8 tp0,tp1,count,pad3; u16 cx,cy; u8 ubase,pad9,
vbase,..} and Ft4Sheet {u8 tp0,tp1,count,pad3; u16 cx,cy; u16 ubase,vbase}; both 12 bytes,
cells 8 bytes. A 12-byte slot is header-shaped iff pad3 == 0, count >= 1 and the CLUT row cy
is in VRAM's bottom band (480 <= cy < 512); SPRT headers additionally have tp1 == 0 (the
func_800759D0 census test). Header count N is the run of header-shaped slots; the sheet's
cells start at +12*N. Independent check: the NEXT sheet in the table starts exactly at
s + 12*N + 8*count (sheets are packed), which confirms N from the layout alone.

Usage: python3 memory/grind/func_8006A880/census.py disc/TIM2D/<file>.BIN ...
"""
import struct, sys, hashlib

SITES = [(0x18, range(0, 8), 'sprt'), (0x40, range(0, 8), 'sprt'),
         (0x24, range(8, 16), 'sprt'), (0x24, range(0, 8), 'ft4')]


def census(path):
    d = open(path, "rb").read()
    u32 = lambda o: struct.unpack_from("<I", d, o)[0]
    root = []
    while 4 * len(root) < len(d) and len(root) < 64:
        v = u32(4 * len(root))
        if v == 0xFFFFFFFF:
            break
        root.append(v)

    def hdr_ok(o, kind):
        if o + 12 > len(d):
            return False
        tp0, tp1, cnt, p3, cx, cy = struct.unpack_from("<BBBBHH", d, o)
        ok = p3 == 0 and cnt >= 1 and 480 <= cy < 512
        return ok and (tp1 == 0 if kind == 'sprt' else True)

    def nhdr(s, kind):
        n = 0
        while hdr_ok(s + 12 * n, kind):
            n += 1
        return n

    out, ok = [], True
    for slot, rng, kind in SITES:
        if slot // 4 >= len(root) or root[slot // 4] >= len(d):
            return None
        T = root[slot // 4]
        out.append(f"root+0x{slot:02X} ({kind}): table @0x{T:X}")
        for k in rng:
            s = u32(T + 4 * k)
            if s == 0 or s >= len(d):
                out.append(f"   [{k:2}] 0x{s:X} NOT A SHEET")
                ok = False
                continue
            N = nhdr(s, kind)
            cnt = d[s + 2]
            nxt = u32(T + 4 * (k + 1))
            packed = (nxt == s + 12 * N + 8 * cnt)
            x, y, uu, vv, w, h = struct.unpack_from("<hhBBBB", d, s + 12 * N)
            out.append(f"   [{k:2}] sheet @0x{s:X}: {N} header(s), count {cnt}, cells @+0x{12 * N:X}"
                       f" first {{x={x},y={y},u={uu},v={vv},w={w},h={h}}}; next sheet @0x{nxt:X}"
                       f" {'== s+12N+8*count' if packed else '(not adjacent)'}")
            ok = ok and N == 1
    return ok, root, out, hashlib.sha256(d).hexdigest()


for path in sys.argv[1:]:
    r = census(path)
    if r is None:
        print(f"{path}: root does not fit")
        continue
    ok, root, out, sha = r
    print(f"== {path} sha256 {sha}")
    print(f"   root[{len(root)}] = {[hex(x) for x in root]}")
    print(f"   every reached sheet has exactly one header: {ok}")
    for line in out:
        print(line)
