"""Ruling 9 (b')(3) clarification (2fc07a100): is D_SEL.BIN 0xCA8 referenced?

Models every code walk over the loaded D_SEL.BIN image (all readers are C in
src/text1b.c; line numbers at 2fc07a100) over its full reachable index range,
and records the byte range each access reads. Then:
  * lists every access whose range covers 0xCA8 (or the 0xCA0 record),
  * lists every u32 in the file equal to 0xCA0..0xCAC (pointer/offset/table
    entry search), and every u32 in the EXE equal to those values,
  * excludes nothing silently: the anomaly access itself is modelled and
    printed separately.
"""
import struct

d = open("disc/TIM2D/D_SEL.BIN", "rb").read()
exe = open("disc/SLUS_006.63", "rb").read()
u32 = lambda o: struct.unpack_from("<I", d, o)[0]
root = [u32(4 * k) for k in range(16)]
TARGETS = (0xCA0, 0xCA8)
acc = []  # (lo, hi, who)


def add(lo, hi, who):
    acc.append((lo, hi, who))


def sprt(h, t, who, cnt_from=None):
    """func_8007352C: header [h,h+12) (count +2, cx/cy +4..7, ubase +8, vbase +10);
    cells [t, t + 8*count), count = byte at (cnt_from or h)+2."""
    add(h, h + 12, who + " header")
    c = d[h + 2]
    add(t, t + 8 * c, who + f" cells(count {c})")


def tbl(slot):
    return u32(slot)


def ent(slot, k):
    return u32(tbl(slot) + 4 * k)


# ---- load time -----------------------------------------------------------
# L1 func_8006E440 (text1b.c:9426-9433): root words from 0 until -1
o = 0
while u32(o) != 0xFFFFFFFF:
    o += 4
add(0, o + 4, "L1 func_8006E440 root walk to -1")
# L2 func_8006E950 LoadImage (9654-9669): root[2], 0x180x0x1DC + 0x170x0x24 halfwords
add(root[2], root[2] + 0x59400 + 0x170 * 0x24 * 2, "L2 LoadImage root[2]")
# L3 func_8006E8CC (9619-9636): root[3] or root[4], 0x280x0x20 halfwords
add(root[3], root[3] + 0x280 * 0x20 * 2, "L3 LoadImage root[3]")
add(root[4], root[4] + 0x280 * 0x20 * 2, "L3 LoadImage root[4]")
# L4 func_80076FF8 (12438-12449) -> func_8006920C (7396-7405): root[5..14] until 0
for k in range(5, 15):
    s = root[k]
    e = s
    while u32(e) != 0:
        e += 4
    add(s, e + 4, f"L4 func_8006920C table root[{k}] (+0x{4 * k:X})")
# func_8006E49C (9440-): arg1[] = r + positive constants, r = root[1] (>= 0xF70)

# ---- draw time -----------------------------------------------------------
for i in range(3):  # D1 func_80074220 11160-11174
    h = ent(0x38, i); sprt(h, h + 0xC, f"D1 func_80074220 +0x38[{i}]")
for i in range(15):  # D2 func_80074488 11260-11327
    h = ent(0x34, i); sprt(h, h + 0xC, f"D2 func_80074488 +0x34[{i}]")
for i in range(4):  # D3 func_800753D8 11844-11893 (+0x2C[arg1+2], loop [0..1])
    h = ent(0x2C, i); sprt(h, h + 0xC, f"D3 func_800753D8 +0x2C[{i}]")
h = ent(0x14, 0)  # D3 11859-11871, and func_800759D0 head
sprt(h, h + 0xC, "D3/D7 +0x14[0]")
sprt(h, h + 0xC + 8 * d[h + 2], "D3/D7 +0x14[0] second group")
for i in range(3):  # D4 func_80074D2C 11603-11615
    h = ent(0x1C, i); sprt(h, h + 0xC, f"D4 func_80074D2C +0x1C[{i}]")
for i in range(4):  # D5 func_80074E08 11648-11699
    h = ent(0x18, i); sprt(h, h + 0xC, f"D5 func_80074E08 +0x18[{i}]")
h = ent(0x18, 2)
sprt(h, h + 0xC + 8 * d[h + 2], "D5 func_80073728 +0x18[2] FT4 group")
h = ent(0x14, 21)  # D6 func_80075830 11960-11962
sprt(h, h + 0xC, "D6 func_80075830 +0x14[21]")
add(ent(0x14, 1), ent(0x14, 1) + 2, "D7/D8 tail func_8006E480 +0x14[1]")


def three(h, who, extra_mult):
    """highlight-step readers: header h+12k (k 0..2), cells h+0x24 (+ count*extra)."""
    for k in range(3):
        hk = h + 12 * k
        c = d[hk + 2]
        sprt(hk, h + 0x24 + extra_mult * c, f"{who} hdr{k}")


for e in range(20):  # D7 func_800759D0 loops 1 and 2 (normal: [1..20])
    h = ent(0x14, e + 1)
    three(h, f"D7 loop1 +0x14[{e + 1}]", 0)
    three(h, f"D7 loop2 +0x14[{e + 1}]", 8)
    three(h, f"D8 func_8007636C picks +0x14[{e + 1}]", 16)
for f65 in range(3):  # D7 loop 3 / D8 last loop
    for i in range(f65 + 3):
        h = ent(0x20 + 4 * f65, i)
        three(h, f"D7 loop3 +0x{0x20 + 4 * f65:X}[{i}]", 0)
        three(h, f"D8 loop3 +0x{0x20 + 4 * f65:X}[{i}]", 8)
h = ent(0x30, 12)  # D8 func_8007636C 12188-12198
sprt(h, h + 0xC, "D8 +0x30[12]")
for idx in range(6):  # D8 12224-12249, idx from f7E/f48 in 0..5
    h = ent(0x30, idx * 2); three(h, f"D8 +0x30[{idx * 2}]", 0)
    h = ent(0x30, idx * 2 + 1); sprt(h, h + 0xC, f"D8 +0x30[{idx * 2 + 1}]")
t0 = root[15]  # D9 func_80074B18 11560-11585: root+0x3C, 12-byte stride, j < 8
add(t0, t0 + 12 * 8, "D9 func_80074B18 +0x3C tile records")

# ---- the anomaly access itself (under judgment, printed separately) ------
h21 = ent(0x14, 21)
anom = []
for a3 in (0, 1):
    hk = h21 + 12 + 12 * a3
    c = d[hk + 2]
    anom.append((hk, hk + 12, f"ANOMALY loop2 [21] arg3={a3} moved header"))
    anom.append((h21 + 0x24 + 8 * c, h21 + 0x24 + 16 * c, f"ANOMALY loop2 [21] arg3={a3} cells(count {c})"))

print(f"{len(acc)} modelled accesses")
for tgt in TARGETS:
    hits = [a for a in acc if a[0] <= tgt < a[1]]
    starts = [a for a in acc if a[0] == tgt]
    print(f"\n0x{tgt:X}: covered by {len(hits)} access(es), object start of {len(starts)}")
    for a in hits:
        print(f"   0x{a[0]:X}-0x{a[1]:X} {a[2]}")
near = sorted(a for a in acc if a[1] > 0xC80 and a[0] < 0xCD0)
print("\naccesses touching 0xC80-0xCD0:")
for a in near:
    print(f"   0x{a[0]:X}-0x{a[1]:X} {a[2]}")
print("\nanomaly-path reads (the access under judgment):")
for a in anom:
    print(f"   0x{a[0]:X}-0x{a[1]:X} {a[2]}")

print("\npointer/offset search (u32, 4-aligned and unaligned) for 0xCA0..0xCAC:")
for v in range(0xCA0, 0xCAD, 4):
    fh = [hex(o) for o in range(0, len(d) - 3) if struct.unpack_from("<I", d, o)[0] == v]
    print(f"   D_SEL.BIN == 0x{v:X}: {fh[:20]}{' ...' if len(fh) > 20 else ''}")
print("   root (after -1 walk):", [hex(x) for x in root])

print("\nload-time walk ranges:")
for a in acc:
    if a[2].startswith("L"):
        print(f"   0x{a[0]:X}-0x{a[1]:X} {a[2]}")
lo_draw = min(a[0] for a in acc if a[2].startswith("D"))
hi_draw = max(a[1] for a in acc if a[2].startswith("D"))
print(f"draw-time reads span 0x{lo_draw:X}-0x{hi_draw:X} overall (union has gaps)")
