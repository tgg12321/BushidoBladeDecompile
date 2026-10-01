"""Checklist 1001c item 4 scan: raw `*(T *)(p + 0xNN)` sites in src/ at offsets that func_80023F08's data model
newly types in PracticeMenuRec, with the member type at that offset and an AGREE / WIDTH / SIGN verdict.
The handle `p` is not resolved (many sites are other record types); WIDTH rows need a look.
usage: python consumer_scan.py > scan.txt"""
import glob
import re

# offset -> (member, C type) for the members this landing adds or retypes
M = {0x24: ("unk_24.unk_00[0]", "s16"), 0x26: ("unk_24.unk_00[1]", "s16"), 0x28: ("unk_24.unk_00[2]", "s16"),
     0x2A: ("unk_24.unk_00[3]", "s16"), 0x2C: ("unk_24.held", "u32"), 0x30: ("unk_24.pressed", "u32"),
     0x34: ("unk_24.released", "u32"), 0x38: ("unk_24.unheld", "u32"),
     0x42: ("unk_42", "s16"), 0x44: ("unk_44", "s16"), 0x46: ("unk_46", "s16"), 0x4A: ("unk_4A", "s16"),
     0x4C: ("unk_4C", "s16"), 0x50: ("unk_50", "ptr"), 0x54: ("unk_54", "ptr"), 0x62: ("unk_62", "u8"),
     0x63: ("unk_63", "u8"), 0x64: ("unk_64", "u16"), 0x66: ("unk_66", "u16"), 0x68: ("unk_68", "s16"),
     0x70: ("unk_70", "s16"), 0x74: ("unk_74", "s32"), 0x78: ("unk_78", "s16"), 0x7A: ("unk_7A", "s16"),
     0x7C: ("unk_7C", "ptr"), 0x80: ("unk_80", "s16"), 0x82: ("unk_82", "u16"), 0x94: ("unk_94", "s16"),
     0x98: ("unk_98.vx", "s16"), 0x9A: ("unk_98.vy", "s16"), 0x9C: ("unk_98.vz", "s16"), 0x9E: ("unk_98.pad", "s16"),
     0xA5: ("unk_A5", "u8"), 0xA6: ("unk_A6", "u8"), 0xA7: ("unk_A7", "u8"), 0xA8: ("unk_A8", "u8"),
     0xA9: ("unk_A9", "u8"), 0xAA: ("unk_AA", "u8"), 0xAB: ("unk_AB", "u8"), 0xAC: ("unk_AC", "u8"),
     0xB3: ("unk_B3", "u8"), 0xB4: ("unk_B4", "u8"), 0x154: ("unk_154", "s16"), 0x1DA: ("unk_1DA", "s16"),
     0x25C: ("unk_25C.x", "s32"), 0x260: ("unk_25C.y", "s32"), 0x264: ("unk_25C.z", "s32"),
     0x288: ("unk_288[0]", "u16"), 0x28A: ("unk_288[1]", "u16"),
     0x314: ("unk_314", "u16"), 0x316: ("unk_316", "u16"), 0x318: ("unk_318", "s16"), 0x31C: ("unk_31C", "s16"),
     0x320: ("unk_320.x", "s32"), 0x324: ("unk_320.y", "s32"), 0x328: ("unk_320.z", "s32")}
for o in range(0x290, 0x314, 2):
    M[o] = ("unk_290 (MotionFrame)", "s16" if o == 0x290 else "u16")
def main():
    W = {"s8": 1, "u8": 1, "s16": 2, "u16": 2, "s32": 4, "u32": 4, "ptr": 4}
    pat = re.compile(r"\*\s*\(\s*(\w+)\s*(\*+)\s*\)\s*\(\s*(?:\(\s*u8\s*\*\s*\)\s*)?(\w+)\s*\+\s*(0x[0-9A-Fa-f]+)\s*\)")
    rows = []
    for f in sorted(glob.glob("src/*.c")):
        for n, line in enumerate(open(f, encoding="utf-8", errors="replace"), 1):
            for m in pat.finditer(line):
                off = int(m.group(4), 16)
                if off not in M:
                    continue
                t = m.group(1) if len(m.group(2)) == 1 else "ptr"
                if t not in W:
                    t2 = "ptr" if t.endswith("_t") or t[0].isupper() else t
                    t = t2
                mem, mt = M[off]
                if t not in W:
                    v = "?"
                elif W[t] != W[mt]:
                    v = "WIDTH"
                elif t != mt and "ptr" not in (t, mt):
                    v = "SIGN"
                else:
                    v = "AGREE"
                rows.append((f.replace("\\", "/"), n, m.group(0), mem, mt, v))
    for r in rows:
        print("%-28s %5d  %-44s %-22s %-4s %s" % r)
    from collections import Counter
    print("#", Counter(r[5] for r in rows))


if __name__ == '__main__':
    main()
