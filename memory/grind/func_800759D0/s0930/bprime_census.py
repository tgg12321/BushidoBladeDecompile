"""(b') path census for func_800759D0's `cells` writes (Ruling 9 amendment (b')).

1. D_8009BCF8 unk0 bytes from the original EXE (the only values loop 1's `entry`
   and func_80075F80's confirmed picks can take).
2. For every sheet each write site can reach, BASE, BASE+K, the record's extent
   (N headers * 12 + count * 8, count = header 0's byte +2) and whether BASE+K is
   the first cell (normal), inside the record on something else (FAIL), or
   outside it (anomaly).
3. Raw bytes around the placeholder sheet [21] and every table-referenced sheet
   start near it, so the anomaly record can cite what lies at BASE+K.
"""
import struct

exe = open("disc/SLUS_006.63", "rb").read()
d = open("disc/TIM2D/D_SEL.BIN", "rb").read()
u32 = lambda o: struct.unpack_from("<I", d, o)[0]


def exe_off(va):
    return va - 0x80010000 + 0x800


bcf8 = [exe[exe_off(0x8009BCF8) + 2 * k] for k in range(20)]
bcf9 = [exe[exe_off(0x8009BCF8) + 2 * k + 1] for k in range(20)]
print("D_8009BCF8[k].unk0 k=0..19:", bcf8)
print("D_8009BCF8[k].unk1 k=0..19:", bcf9)
print("distinct unk0:", sorted(set(bcf8)), "max", max(bcf8))


def hdr_ok(o):
    tp0, tp1, cnt, p3, cx, cy = struct.unpack_from("<BBBBHH", d, o)
    return tp1 == 0 and p3 == 0 and cnt >= 1 and 480 <= cy < 512


def nhdr(s):
    n = 0
    while hdr_ok(s + 12 * n):
        n += 1
    return n


def extent(s):
    n = nhdr(s)
    cnt = d[s + 2]
    return n, cnt, s + 12 * n + 8 * cnt


def classify(s, k):
    n, cnt, end = extent(s)
    t = s + k
    if t == s + 12 * n:
        return f"NORMAL first cell (N={n}, count={cnt}, record 0x{s:X}..0x{end:X})"
    if s <= t < end:
        return f"INSIDE record on a different sub-object -> FAIL (N={n}, record 0x{s:X}..0x{end:X})"
    return f"OUTSIDE record (N={n}, count={cnt}, record 0x{s:X}..0x{end:X}, BASE+K=0x{t:X})"


t14 = u32(0x14)
print("\n== head: table=root+0x14, sheet [0], K=0xC ==")
s = u32(t14)
print(f"  [0] 0x{s:X}: {classify(s, 0xC)}")

print("\n== loop 1: table[entry+1], entry in D_8009BCF8 unk0, K=0x24 ==")
for e in sorted(set(bcf8)):
    s = u32(t14 + 4 * (e + 1))
    print(f"  entry {e:2} -> [{e + 1:2}] 0x{s:X}: {classify(s, 0x24)}")

print("\n== loop 2: table[arg2[i]+1], arg2[i] in unk0 values U {0x14}, K=0x24 ==")
for e in sorted(set(bcf8) | {0x14}):
    s = u32(t14 + 4 * (e + 1))
    print(f"  arg2 {e:2} -> [{e + 1:2}] 0x{s:X}: {classify(s, 0x24)}")

print("\n== loop 3: table=root[0x20+f65*4], table[i], i<f65+3, f65 in 0..2, K=0x24 ==")
for f65 in range(3):
    t = u32(0x20 + 4 * f65)
    for i in range(f65 + 3):
        s = u32(t + 4 * i)
        print(f"  f65={f65} [{i}] 0x{s:X}: {classify(s, 0x24)}")

print("\n== all sheet starts referenced by root tables +0x14/+0x20..+0x34 near 0xC84 ==")
starts = set()
for slot in (0x14, 0x20, 0x24, 0x28, 0x2C, 0x30, 0x34):
    T = u32(slot)
    for k in range(64):
        v = u32(T + 4 * k)
        if v == 0:
            break
        if v < len(d) and hdr_ok(v):
            starts.add((v, slot, k))
for v, slot, k in sorted(starts):
    if 0xC00 <= v <= 0xD40:
        n, cnt, end = extent(v)
        print(f"  0x{v:X} (root+0x{slot:02X}[{k}]) N={n} count={cnt} ends 0x{end:X}")

print("\n== raw bytes 0xC84..0xCD0 (12-byte rows from 0xC84) ==")
for o in range(0xC84, 0xCD0, 4):
    print(f"  0x{o:X}: {d[o:o + 4].hex()}")
