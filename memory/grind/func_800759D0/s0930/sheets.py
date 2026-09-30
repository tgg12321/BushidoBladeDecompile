"""Dump the sprite-sheet records reached through SEL.BIN's header table at +0x14.

A sheet (func_8007352C's EnvA.header / .table): N 12-byte SprtHdrA headers
(tp0,tp1,count,pad, cx,cy, ubase,pad,vbase,pad) followed by 8-byte SprtEntA
cells (x,y,u,v,w,h). Prints each sheet's headers until the first cell block,
so the header count (and hence the cell-array offset N*12) can be read off.
"""
import struct, sys

path = sys.argv[1] if len(sys.argv) > 1 else "disc/TIM2D/SEL.BIN"
hoff = int(sys.argv[2], 0) if len(sys.argv) > 2 else 0x14
nent = int(sys.argv[3]) if len(sys.argv) > 3 else 14
d = open(path, "rb").read()
u32 = lambda o: struct.unpack_from("<I", d, o)[0]

T = u32(hoff)
print(f"{path}: header+0x{hoff:X} -> table @0x{T:X}")
ents = [u32(T + 4 * k) for k in range(nent)]
for k, S in enumerate(ents):
    if S >= len(d):
        print(f"  [{k}] 0x{S:X} out of file")
        continue
    nxt = min([e for e in ents if e > S] + [len(d)])
    hdrs = []
    for h in range(4):
        o = S + 12 * h
        tp0, tp1, cnt, p3, cx, cy, ub, p9, vb, p11 = struct.unpack_from("<BBBBHHBBBB", d, o)
        hdrs.append((tp0, tp1, cnt, p3, cx, cy, ub, vb))
    print(f"  [{k}] sheet @0x{S:X} (next record @0x{nxt:X}, span 0x{nxt - S:X})")
    for h, hd in enumerate(hdrs):
        print(f"      +0x{12*h:02X} hdr? tp={hd[0]:02X},{hd[1]:02X} count={hd[2]} pad={hd[3]} clut=({hd[4]},{hd[5]}) ub={hd[6]} vb={hd[7]}")
    for c in range(3):
        for base in (0xC, 0x24):
            x, y, uu, vv, w, h = struct.unpack_from("<hhBBBB", d, S + base + 8 * c)
            print(f"      cell @+0x{base:02X}[{c}] x={x} y={y} u={uu} v={vv} w={w} h={h}")
